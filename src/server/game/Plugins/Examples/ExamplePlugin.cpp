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

#include "ExamplePlugin.h"
#include "Player.h"
#include "WorldSession.h"
#include "Chat.h"
#include "Log.h"
#include "Item.h"
#include "ObjectMgr.h"
#include "GameTime.h"
#include <fstream>

// ExampleEventHandler Implementation
void ExampleEventHandler::OnPlayerLogin(Player* player)
{
    if (!player || !_plugin)
        return;
    
    _plugin->TrackPlayerLogin(player);
    
    if (_plugin->IsWelcomeMessageEnabled())
        _plugin->SendWelcomeMessage(player);
}

void ExampleEventHandler::OnPlayerLogout(Player* player)
{
    if (!player || !_plugin)
        return;
    
    _plugin->TrackPlayerLogout(player);
}

void ExampleEventHandler::OnPlayerLevelChanged(Player* player, uint8 oldLevel)
{
    if (!player || !_plugin)
        return;
    
    if (_plugin->IsLevelUpRewardEnabled())
        _plugin->ProcessLevelUpReward(player, oldLevel);
}

void ExampleEventHandler::OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg)
{
    // Example: Log all chat messages (could be used for moderation)
    if (player)
    {
        TC_LOG_DEBUG("plugins.example", "Player %s (GUID: %u) said: %s", 
                    player->GetName().c_str(), player->GetGUID().GetCounter(), msg.c_str());
    }
}

void ExampleEventHandler::OnWorldUpdate(uint32 diff)
{
    if (_plugin)
        _plugin->UpdateStatistics(diff);
}

void ExampleEventHandler::OnServerStart()
{
    TC_LOG_INFO("plugins.example", "Example Plugin: Server started");
}

void ExampleEventHandler::OnServerStop()
{
    TC_LOG_INFO("plugins.example", "Example Plugin: Server stopping");
    if (_plugin)
        _plugin->SaveStatistics();
}

void ExampleEventHandler::OnConfigReload()
{
    if (_plugin)
    {
        _plugin->ReloadConfig();
        TC_LOG_INFO("plugins.example", "Example Plugin: Configuration reloaded");
    }
}

// ExamplePlugin Implementation
ExamplePlugin::ExamplePlugin()
    : _welcomeMessageEnabled(true)
    , _levelUpRewardEnabled(false)
    , _welcomeMessage("Welcome to the server!")
    , _levelUpRewardItem(0)
    , _levelUpRewardCount(1)
    , _updateInterval(10000) // 10 seconds
    , _totalLogins(0)
    , _currentOnlinePlayers(0)
    , _totalLevelUps(0)
    , _lastUpdateTime(0)
{
    // Initialize plugin information
    _info.name = "ExamplePlugin";
    _info.version = "1.0.0";
    _info.author = "TrinityCore Team";
    _info.description = "Example plugin demonstrating the TrinityCore plugin system";
    _info.website = "https://trinitycore.org";
    _info.priority = PluginPriority::NORMAL;
    _info.autoLoad = true;
    
    // No dependencies for this example
    _dependencies.clear();
    
    // Create event handler
    _eventHandler = std::make_unique<ExampleEventHandler>(this);
    
    TC_LOG_INFO("plugins.example", "Example Plugin created");
}

ExamplePlugin::~ExamplePlugin()
{
    TC_LOG_INFO("plugins.example", "Example Plugin destroyed");
}

bool ExamplePlugin::Load()
{
    TC_LOG_INFO("plugins.example", "Loading Example Plugin...");
    
    _state = PluginState::LOADING;
    
    // Create configuration object
    _config = std::make_unique<PluginConfig>();
    
    // Load statistics from file
    LoadStatistics();
    
    _state = PluginState::LOADED;
    TC_LOG_INFO("plugins.example", "Example Plugin loaded successfully");
    return true;
}

bool ExamplePlugin::Initialize()
{
    TC_LOG_INFO("plugins.example", "Initializing Example Plugin...");
    
    _state = PluginState::INITIALIZING;
    
    // Initialize configuration with defaults
    InitializeConfig();
    
    // Register chat commands
    RegisterChatCommands();
    
    _state = PluginState::INITIALIZED;
    TC_LOG_INFO("plugins.example", "Example Plugin initialized successfully");
    return true;
}

void ExamplePlugin::Start()
{
    TC_LOG_INFO("plugins.example", "Starting Example Plugin...");
    
    _state = PluginState::RUNNING;
    _lastUpdateTime = GameTime::GetGameTimeMS();
    
    TC_LOG_INFO("plugins.example", "Example Plugin started successfully");
}

void ExamplePlugin::Stop()
{
    TC_LOG_INFO("plugins.example", "Stopping Example Plugin...");
    
    _state = PluginState::STOPPING;
    
    // Save statistics before stopping
    SaveStatistics();
    
    // Unregister chat commands
    UnregisterChatCommands();
    
    _state = PluginState::LOADED;
    TC_LOG_INFO("plugins.example", "Example Plugin stopped");
}

void ExamplePlugin::Unload()
{
    TC_LOG_INFO("plugins.example", "Unloading Example Plugin...");
    
    // Clean up resources
    _config.reset();
    
    _state = PluginState::UNLOADED;
    TC_LOG_INFO("plugins.example", "Example Plugin unloaded");
}

bool ExamplePlugin::LoadConfig(std::string const& configPath)
{
    if (!_config)
        return false;
    
    if (!_config->LoadFromFile(configPath))
    {
        TC_LOG_WARN("plugins.example", "Failed to load config from %s, using defaults", configPath.c_str());
        InitializeConfig();
        return false;
    }
    
    // Load configuration values
    _welcomeMessageEnabled = _config->GetBool("WelcomeMessage.Enabled", true);
    _welcomeMessage = _config->GetString("WelcomeMessage.Text", "Welcome to the server!");
    _levelUpRewardEnabled = _config->GetBool("LevelUpReward.Enabled", false);
    _levelUpRewardItem = _config->GetUInt("LevelUpReward.ItemId", 0);
    _levelUpRewardCount = _config->GetUInt("LevelUpReward.Count", 1);
    _updateInterval = _config->GetUInt("UpdateInterval", 10000);
    
    TC_LOG_INFO("plugins.example", "Configuration loaded from %s", configPath.c_str());
    return true;
}

void ExamplePlugin::ReloadConfig()
{
    if (_config)
    {
        // Reload configuration values
        _welcomeMessageEnabled = _config->GetBool("WelcomeMessage.Enabled", true);
        _welcomeMessage = _config->GetString("WelcomeMessage.Text", "Welcome to the server!");
        _levelUpRewardEnabled = _config->GetBool("LevelUpReward.Enabled", false);
        _levelUpRewardItem = _config->GetUInt("LevelUpReward.ItemId", 0);
        _levelUpRewardCount = _config->GetUInt("LevelUpReward.Count", 1);
        _updateInterval = _config->GetUInt("UpdateInterval", 10000);
        
        TC_LOG_INFO("plugins.example", "Configuration reloaded");
    }
}

bool ExamplePlugin::CheckDependencies() const
{
    // This example plugin has no dependencies
    return true;
}

void ExamplePlugin::SendWelcomeMessage(Player* player)
{
    if (!player || _welcomeMessage.empty())
        return;
    
    ChatHandler(player->GetSession()).SendSysMessage(_welcomeMessage.c_str());
}

void ExamplePlugin::TrackPlayerLogin(Player* player)
{
    if (!player)
        return;
    
    ++_totalLogins;
    ++_currentOnlinePlayers;
    
    TC_LOG_DEBUG("plugins.example", "Player %s logged in. Total logins: %u, Online: %u", 
                player->GetName().c_str(), _totalLogins, _currentOnlinePlayers);
}

void ExamplePlugin::TrackPlayerLogout(Player* player)
{
    if (!player)
        return;
    
    if (_currentOnlinePlayers > 0)
        --_currentOnlinePlayers;
    
    TC_LOG_DEBUG("plugins.example", "Player %s logged out. Online: %u", 
                player->GetName().c_str(), _currentOnlinePlayers);
}

void ExamplePlugin::ProcessLevelUpReward(Player* player, uint8 oldLevel)
{
    if (!player || _levelUpRewardItem == 0)
        return;
    
    uint8 newLevel = player->GetLevel();
    if (newLevel <= oldLevel)
        return;
    
    // Give reward item
    ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(_levelUpRewardItem);
    if (!itemTemplate)
    {
        TC_LOG_ERROR("plugins.example", "Invalid reward item ID: %u", _levelUpRewardItem);
        return;
    }
    
    // Add item to player's inventory
    uint32 noSpaceForCount = 0;
    ItemPosCountVec dest;
    InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, _levelUpRewardItem, _levelUpRewardCount, &noSpaceForCount);
    
    if (msg == EQUIP_ERR_OK)
    {
        Item* item = player->StoreNewItem(dest, _levelUpRewardItem, true);
        if (item)
        {
            player->SendNewItem(item, _levelUpRewardCount, true, false);
            ++_totalLevelUps;
            
            TC_LOG_DEBUG("plugins.example", "Gave level up reward to %s (Level %u -> %u)", 
                        player->GetName().c_str(), oldLevel, newLevel);
        }
    }
    else
    {
        ChatHandler(player->GetSession()).SendSysMessage("Your inventory is full! Level up reward could not be given.");
    }
}

void ExamplePlugin::InitializeConfig()
{
    if (!_config)
        return;
    
    // Set default configuration values
    _config->SetBool("WelcomeMessage.Enabled", _welcomeMessageEnabled);
    _config->SetString("WelcomeMessage.Text", _welcomeMessage);
    _config->SetBool("LevelUpReward.Enabled", _levelUpRewardEnabled);
    _config->SetUInt("LevelUpReward.ItemId", _levelUpRewardItem);
    _config->SetUInt("LevelUpReward.Count", _levelUpRewardCount);
    _config->SetUInt("UpdateInterval", _updateInterval);
}

void ExamplePlugin::RegisterChatCommands()
{
    // In a real implementation, you would register these commands with TrinityCore's command system
    TC_LOG_DEBUG("plugins.example", "Chat commands registered");
}

void ExamplePlugin::UnregisterChatCommands()
{
    // In a real implementation, you would unregister the commands here
    TC_LOG_DEBUG("plugins.example", "Chat commands unregistered");
}

void ExamplePlugin::UpdateStatistics(uint32 diff)
{
    uint32 currentTime = GameTime::GetGameTimeMS();
    
    if (currentTime - _lastUpdateTime >= _updateInterval)
    {
        _lastUpdateTime = currentTime;
        
        // Perform periodic tasks here
        TC_LOG_TRACE("plugins.example", "Statistics updated - Logins: %u, Online: %u, Level-ups: %u", 
                    _totalLogins, _currentOnlinePlayers, _totalLevelUps);
    }
}

void ExamplePlugin::SaveStatistics()
{
    std::ofstream file("plugins/example_stats.txt");
    if (file.is_open())
    {
        file << "TotalLogins=" << _totalLogins << std::endl;
        file << "TotalLevelUps=" << _totalLevelUps << std::endl;
        file.close();
        
        TC_LOG_DEBUG("plugins.example", "Statistics saved");
    }
}

void ExamplePlugin::LoadStatistics()
{
    std::ifstream file("plugins/example_stats.txt");
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            size_t pos = line.find('=');
            if (pos != std::string::npos)
            {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                if (key == "TotalLogins")
                    _totalLogins = std::stoul(value);
                else if (key == "TotalLevelUps")
                    _totalLevelUps = std::stoul(value);
            }
        }
        file.close();
        
        TC_LOG_DEBUG("plugins.example", "Statistics loaded - Logins: %u, Level-ups: %u", 
                    _totalLogins, _totalLevelUps);
    }
}

// Chat command handlers (simplified examples)
bool ExamplePlugin::HandleExampleCommand(Player* player, std::string const& args)
{
    if (!player)
        return false;
    
    ChatHandler handler(player->GetSession());
    handler.SendSysMessage("Example Plugin is running!");
    return true;
}

bool ExamplePlugin::HandleStatsCommand(Player* player, std::string const& args)
{
    if (!player)
        return false;
    
    ChatHandler handler(player->GetSession());
    handler.PSendSysMessage("Plugin Statistics:");
    handler.PSendSysMessage("Total Logins: %u", _totalLogins);
    handler.PSendSysMessage("Current Online: %u", _currentOnlinePlayers);
    handler.PSendSysMessage("Total Level-ups: %u", _totalLevelUps);
    return true;
}

bool ExamplePlugin::HandleReloadCommand(Player* player, std::string const& args)
{
    if (!player)
        return false;
    
    ReloadConfig();
    
    ChatHandler handler(player->GetSession());
    handler.SendSysMessage("Example Plugin configuration reloaded.");
    return true;
}