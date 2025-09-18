/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "PluginManager.h"
#include "Log.h"
#include "Util.h"
#include <filesystem>
#include <algorithm>

#ifndef _WIN32
#include <dlfcn.h>
#endif

std::unique_ptr<PluginManager> PluginManager::_instance;
std::mutex PluginManager::_instanceMutex;

PluginManager* PluginManager::Instance()
{
    std::lock_guard<std::mutex> lock(_instanceMutex);
    if (!_instance)
        _instance = std::unique_ptr<PluginManager>(new PluginManager());
    return _instance.get();
}

PluginManager::PluginManager()
{
    TC_LOG_INFO("server.loading", "Initializing Plugin Manager...");
}

PluginManager::~PluginManager()
{
    UnloadAllPlugins();
}

bool PluginManager::LoadPlugin(std::string const& filePath)
{
    std::lock_guard<std::mutex> lock(_pluginsMutex);
    
    if (!std::filesystem::exists(filePath))
    {
        _lastError = "Plugin file does not exist: " + filePath;
        TC_LOG_ERROR("plugins", "%s", _lastError.c_str());
        return false;
    }
    
    auto loadedPlugin = std::make_unique<LoadedPlugin>();
    
    if (!LoadPluginLibrary(filePath, *loadedPlugin))
        return false;
    
    if (!loadedPlugin->plugin)
    {
        _lastError = "Failed to create plugin instance from: " + filePath;
        TC_LOG_ERROR("plugins", "%s", _lastError.c_str());
        UnloadPluginLibrary(*loadedPlugin);
        return false;
    }
    
    if (!ValidatePlugin(loadedPlugin->plugin.get()))
    {
        UnloadPluginLibrary(*loadedPlugin);
        return false;
    }
    
    std::string pluginName = loadedPlugin->plugin->GetInfo().name;
    
    if (_loadedPlugins.find(pluginName) != _loadedPlugins.end())
    {
        _lastError = "Plugin already loaded: " + pluginName;
        TC_LOG_ERROR("plugins", "%s", _lastError.c_str());
        UnloadPluginLibrary(*loadedPlugin);
        return false;
    }
    
    if (!loadedPlugin->plugin->Load())
    {
        _lastError = "Plugin failed to load: " + pluginName;
        TC_LOG_ERROR("plugins", "%s", _lastError.c_str());
        UnloadPluginLibrary(*loadedPlugin);
        return false;
    }
    
    _loadedPlugins[pluginName] = std::move(loadedPlugin);
    
    TC_LOG_INFO("plugins", "Successfully loaded plugin: %s v%s by %s", 
                pluginName.c_str(), 
                _loadedPlugins[pluginName]->plugin->GetInfo().version.c_str(),
                _loadedPlugins[pluginName]->plugin->GetInfo().author.c_str());
    
    return true;
}

bool PluginManager::LoadPluginLibrary(std::string const& filePath, LoadedPlugin& loadedPlugin)
{
#ifdef _WIN32
    loadedPlugin.handle = LoadLibraryA(filePath.c_str());
    if (!loadedPlugin.handle)
    {
        _lastError = "Failed to load library: " + filePath + " (Error: " + std::to_string(GetLastError()) + ")";
        TC_LOG_ERROR("plugins", "%s", _lastError.c_str());
        return false;
    }
    
    loadedPlugin.createFunc = reinterpret_cast<PluginCreateFunc>(GetProcAddress(loadedPlugin.handle, "CreatePlugin"));
    loadedPlugin.destroyFunc = reinterpret_cast<PluginDestroyFunc>(GetProcAddress(loadedPlugin.handle, "DestroyPlugin"));
#else
    loadedPlugin.handle = dlopen(filePath.c_str(), RTLD_LAZY);
    if (!loadedPlugin.handle)
    {
        _lastError = "Failed to load library: " + filePath + " (" + dlerror() + ")";
        TC_LOG_ERROR("plugins", "%s", _lastError.c_str());
        return false;
    }
    
    loadedPlugin.createFunc = reinterpret_cast<PluginCreateFunc>(dlsym(loadedPlugin.handle, "CreatePlugin"));
    loadedPlugin.destroyFunc = reinterpret_cast<PluginDestroyFunc>(dlsym(loadedPlugin.handle, "DestroyPlugin"));
#endif
    
    if (!loadedPlugin.createFunc)
    {
        _lastError = "Plugin does not export CreatePlugin function: " + filePath;
        TC_LOG_ERROR("plugins", "%s", _lastError.c_str());
        UnloadPluginLibrary(loadedPlugin);
        return false;
    }
    
    loadedPlugin.plugin = loadedPlugin.createFunc();
    loadedPlugin.filePath = filePath;
    
    return true;
}

void PluginManager::UnloadPluginLibrary(LoadedPlugin& loadedPlugin)
{
    if (loadedPlugin.handle)
    {
#ifdef _WIN32
        FreeLibrary(loadedPlugin.handle);
#else
        dlclose(loadedPlugin.handle);
#endif
        loadedPlugin.handle = nullptr;
    }
    
    loadedPlugin.createFunc = nullptr;
    loadedPlugin.destroyFunc = nullptr;
    loadedPlugin.plugin.reset();
}

bool PluginManager::ValidatePlugin(IPlugin* plugin)
{
    if (!plugin)
        return false;
    
    PluginInfo const& info = plugin->GetInfo();
    
    if (info.name.empty())
    {
        _lastError = "Plugin name cannot be empty";
        TC_LOG_ERROR("plugins", "%s", _lastError.c_str());
        return false;
    }
    
    if (info.version.empty())
    {
        _lastError = "Plugin version cannot be empty for plugin: " + info.name;
        TC_LOG_ERROR("plugins", "%s", _lastError.c_str());
        return false;
    }
    
    return true;
}

bool PluginManager::UnloadPlugin(std::string const& pluginName)
{
    std::lock_guard<std::mutex> lock(_pluginsMutex);
    
    auto it = _loadedPlugins.find(pluginName);
    if (it == _loadedPlugins.end())
    {
        _lastError = "Plugin not found: " + pluginName;
        TC_LOG_ERROR("plugins", "%s", _lastError.c_str());
        return false;
    }
    
    LoadedPlugin* loadedPlugin = it->second.get();
    
    if (loadedPlugin->plugin->GetState() == PluginState::RUNNING)
        loadedPlugin->plugin->Stop();
    
    loadedPlugin->plugin->Unload();
    
    // Remove event handler if registered
    UnregisterEventHandler(pluginName);
    
    UnloadPluginLibrary(*loadedPlugin);
    _loadedPlugins.erase(it);
    
    TC_LOG_INFO("plugins", "Successfully unloaded plugin: %s", pluginName.c_str());
    return true;
}

void PluginManager::LoadAllPlugins(std::string const& pluginDirectory)
{
    _pluginDirectory = pluginDirectory;
    
    if (!std::filesystem::exists(pluginDirectory))
    {
        TC_LOG_WARN("plugins", "Plugin directory does not exist: %s", pluginDirectory.c_str());
        return;
    }
    
    TC_LOG_INFO("plugins", "Loading plugins from directory: %s", pluginDirectory.c_str());
    
    for (auto const& entry : std::filesystem::directory_iterator(pluginDirectory))
    {
        if (entry.is_regular_file())
        {
            std::string extension = entry.path().extension().string();
#ifdef _WIN32
            if (extension == ".dll")
#else
            if (extension == ".so")
#endif
            {
                LoadPlugin(entry.path().string());
            }
        }
    }
}

void PluginManager::InitializeAllPlugins()
{
    std::vector<std::string> loadOrder = GetPluginLoadOrder();
    
    for (std::string const& pluginName : loadOrder)
    {
        InitializePlugin(pluginName);
    }
}

bool PluginManager::InitializePlugin(std::string const& pluginName)
{
    std::lock_guard<std::mutex> lock(_pluginsMutex);
    
    auto it = _loadedPlugins.find(pluginName);
    if (it == _loadedPlugins.end())
        return false;
    
    IPlugin* plugin = it->second->plugin.get();
    
    if (!CheckPluginDependencies(pluginName))
    {
        TC_LOG_ERROR("plugins", "Plugin dependencies not met: %s", pluginName.c_str());
        return false;
    }
    
    if (!plugin->Initialize())
    {
        TC_LOG_ERROR("plugins", "Failed to initialize plugin: %s", pluginName.c_str());
        return false;
    }
    
    // Register event handler if available
    IEventHandler* eventHandler = plugin->GetEventHandler();
    if (eventHandler)
        RegisterEventHandler(pluginName, eventHandler);
    
    TC_LOG_INFO("plugins", "Successfully initialized plugin: %s", pluginName.c_str());
    return true;
}

void PluginManager::StartAllPlugins()
{
    std::lock_guard<std::mutex> lock(_pluginsMutex);
    
    for (auto const& pair : _loadedPlugins)
    {
        IPlugin* plugin = pair.second->plugin.get();
        if (plugin->GetState() == PluginState::INITIALIZED)
        {
            plugin->Start();
            TC_LOG_INFO("plugins", "Started plugin: %s", pair.first.c_str());
        }
    }
}

void PluginManager::StopAllPlugins()
{
    std::lock_guard<std::mutex> lock(_pluginsMutex);
    
    for (auto const& pair : _loadedPlugins)
    {
        IPlugin* plugin = pair.second->plugin.get();
        if (plugin->GetState() == PluginState::RUNNING)
        {
            plugin->Stop();
            TC_LOG_INFO("plugins", "Stopped plugin: %s", pair.first.c_str());
        }
    }
}

void PluginManager::UnloadAllPlugins()
{
    std::vector<std::string> pluginNames = GetLoadedPluginNames();
    
    for (std::string const& pluginName : pluginNames)
        UnloadPlugin(pluginName);
}

IPlugin* PluginManager::GetPlugin(std::string const& pluginName)
{
    std::lock_guard<std::mutex> lock(_pluginsMutex);
    
    auto it = _loadedPlugins.find(pluginName);
    if (it != _loadedPlugins.end())
        return it->second->plugin.get();
    
    return nullptr;
}

std::vector<std::string> PluginManager::GetLoadedPluginNames() const
{
    std::lock_guard<std::mutex> lock(_pluginsMutex);
    
    std::vector<std::string> names;
    names.reserve(_loadedPlugins.size());
    
    for (auto const& pair : _loadedPlugins)
        names.push_back(pair.first);
    
    return names;
}

void PluginManager::RegisterEventHandler(std::string const& pluginName, IEventHandler* handler)
{
    std::lock_guard<std::mutex> lock(_eventHandlersMutex);
    _eventHandlers[pluginName] = handler;
}

void PluginManager::UnregisterEventHandler(std::string const& pluginName)
{
    std::lock_guard<std::mutex> lock(_eventHandlersMutex);
    _eventHandlers.erase(pluginName);
}

// Event dispatching implementations
void PluginManager::OnPlayerLogin(Player* player)
{
    std::vector<IEventHandler*> handlers = GetEventHandlersByPriority();
    for (IEventHandler* handler : handlers)
        handler->OnPlayerLogin(player);
}

void PluginManager::OnPlayerLogout(Player* player)
{
    std::vector<IEventHandler*> handlers = GetEventHandlersByPriority();
    for (IEventHandler* handler : handlers)
        handler->OnPlayerLogout(player);
}

void PluginManager::OnWorldUpdate(uint32 diff)
{
    std::vector<IEventHandler*> handlers = GetEventHandlersByPriority();
    for (IEventHandler* handler : handlers)
        handler->OnWorldUpdate(diff);
}

std::vector<IEventHandler*> PluginManager::GetEventHandlersByPriority()
{
    std::lock_guard<std::mutex> lock(_eventHandlersMutex);
    
    std::vector<std::pair<PluginPriority, IEventHandler*>> prioritizedHandlers;
    
    for (auto const& pair : _eventHandlers)
    {
        IPlugin* plugin = GetPlugin(pair.first);
        if (plugin && plugin->GetState() == PluginState::RUNNING)
        {
            PluginPriority priority = plugin->GetInfo().priority;
            prioritizedHandlers.emplace_back(priority, pair.second);
        }
    }
    
    // Sort by priority (highest first)
    std::sort(prioritizedHandlers.begin(), prioritizedHandlers.end(),
              [](auto const& a, auto const& b) { return a.first > b.first; });
    
    std::vector<IEventHandler*> handlers;
    handlers.reserve(prioritizedHandlers.size());
    
    for (auto const& pair : prioritizedHandlers)
        handlers.push_back(pair.second);
    
    return handlers;
}

std::vector<std::string> PluginManager::GetPluginLoadOrder()
{
    std::vector<std::string> loadOrder;
    std::vector<std::string> pluginNames = GetLoadedPluginNames();
    
    for (std::string const& pluginName : pluginNames)
    {
        if (std::find(loadOrder.begin(), loadOrder.end(), pluginName) == loadOrder.end())
        {
            ResolveDependencies(pluginName, loadOrder);
        }
    }
    
    return loadOrder;
}

bool PluginManager::ResolveDependencies(std::string const& pluginName, std::vector<std::string>& loadOrder)
{
    IPlugin* plugin = GetPlugin(pluginName);
    if (!plugin)
        return false;
    
    std::vector<std::string> const& dependencies = plugin->GetDependencies();
    
    for (std::string const& dependency : dependencies)
    {
        if (std::find(loadOrder.begin(), loadOrder.end(), dependency) == loadOrder.end())
        {
            if (!ResolveDependencies(dependency, loadOrder))
                return false;
        }
    }
    
    loadOrder.push_back(pluginName);
    return true;
}

bool PluginManager::CheckPluginDependencies(std::string const& pluginName)
{
    IPlugin* plugin = GetPlugin(pluginName);
    if (!plugin)
        return false;
    
    return plugin->CheckDependencies();
}