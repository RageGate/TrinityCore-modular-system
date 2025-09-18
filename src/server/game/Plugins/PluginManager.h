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

#ifndef TRINITY_PLUGIN_MANAGER_H
#define TRINITY_PLUGIN_MANAGER_H

#include "IPlugin.h"
#include "Define.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <functional>

#ifdef _WIN32
#include <windows.h>
typedef HMODULE PluginHandle;
#else
typedef void* PluginHandle;
#endif

struct LoadedPlugin
{
    std::unique_ptr<IPlugin> plugin;
    PluginHandle handle;
    std::string filePath;
    PluginCreateFunc createFunc;
    PluginDestroyFunc destroyFunc;
    
    LoadedPlugin() : handle(nullptr), createFunc(nullptr), destroyFunc(nullptr) { }
};

class TC_GAME_API PluginManager
{
public:
    static PluginManager* Instance();
    
    ~PluginManager();
    
    // Plugin loading/unloading
    bool LoadPlugin(std::string const& filePath);
    bool UnloadPlugin(std::string const& pluginName);
    bool ReloadPlugin(std::string const& pluginName);
    
    // Plugin management
    bool InitializePlugin(std::string const& pluginName);
    bool StartPlugin(std::string const& pluginName);
    bool StopPlugin(std::string const& pluginName);
    
    // Bulk operations
    void LoadAllPlugins(std::string const& pluginDirectory);
    void InitializeAllPlugins();
    void StartAllPlugins();
    void StopAllPlugins();
    void UnloadAllPlugins();
    
    // Plugin queries
    IPlugin* GetPlugin(std::string const& pluginName);
    std::vector<std::string> GetLoadedPluginNames() const;
    std::vector<IPlugin*> GetPluginsByState(PluginState state) const;
    bool IsPluginLoaded(std::string const& pluginName) const;
    
    // Event system
    void RegisterEventHandler(std::string const& pluginName, IEventHandler* handler);
    void UnregisterEventHandler(std::string const& pluginName);
    
    // Event dispatching
    void OnPlayerLogin(Player* player);
    void OnPlayerLogout(Player* player);
    void OnPlayerLevelChanged(Player* player, uint8 oldLevel);
    void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg);
    void OnPlayerKill(Player* killer, Player* killed);
    void OnPlayerKillCreature(Player* killer, Creature* killed);
    void OnCreatureKill(Creature* killer, Unit* killed);
    void OnCreatureDeath(Creature* creature, Unit* killer);
    void OnCreatureRespawn(Creature* creature);
    void OnGameObjectUse(GameObject* go, Player* player);
    void OnGameObjectDestroyed(GameObject* go, Player* player);
    void OnWorldUpdate(uint32 diff);
    void OnMapUpdate(Map* map, uint32 diff);
    bool OnPacketReceive(WorldSession* session, WorldPacket& packet);
    bool OnPacketSend(WorldSession* session, WorldPacket const& packet);
    void OnServerStart();
    void OnServerStop();
    void OnConfigReload();
    
    // Configuration
    void SetPluginDirectory(std::string const& directory) { _pluginDirectory = directory; }
    std::string const& GetPluginDirectory() const { return _pluginDirectory; }
    
    // Plugin communication
    void* GetPluginInterface(std::string const& pluginName, std::string const& interfaceName);
    
    // Dependency management
    bool CheckPluginDependencies(std::string const& pluginName);
    std::vector<std::string> GetPluginLoadOrder();
    
    // Error handling
    std::string GetLastError() const { return _lastError; }
    
private:
    PluginManager();
    
    // Internal plugin operations
    bool LoadPluginLibrary(std::string const& filePath, LoadedPlugin& loadedPlugin);
    void UnloadPluginLibrary(LoadedPlugin& loadedPlugin);
    bool ValidatePlugin(IPlugin* plugin);
    
    // Dependency resolution
    bool ResolveDependencies(std::string const& pluginName, std::vector<std::string>& loadOrder);
    bool HasCircularDependency(std::string const& pluginName, std::vector<std::string> const& visited);
    
    // Event handler management
    std::vector<IEventHandler*> GetEventHandlersByPriority();
    
    // Thread safety
    mutable std::mutex _pluginsMutex;
    mutable std::mutex _eventHandlersMutex;
    
    // Plugin storage
    std::unordered_map<std::string, std::unique_ptr<LoadedPlugin>> _loadedPlugins;
    std::unordered_map<std::string, IEventHandler*> _eventHandlers;
    
    // Configuration
    std::string _pluginDirectory;
    std::string _lastError;
    
    // Singleton instance
    static std::unique_ptr<PluginManager> _instance;
    static std::mutex _instanceMutex;
};

#define sPluginManager PluginManager::Instance()

#endif // TRINITY_PLUGIN_MANAGER_H