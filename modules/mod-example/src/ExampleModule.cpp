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

#include "ExampleModule.h"
#include "Player.h"
#include "Chat.h"
#include "World.h"
#include "WorldSession.h"
#include "ScriptMgr.h"
#include "Config.h"
#include "Log.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "GameTime.h"
#include <fstream>
#include <sstream>
#include <iomanip>

// Event Handler Implementation
class ExampleModuleEventHandler : public PluginEventHandler
{
public:
    ExampleModuleEventHandler(ExampleModule* module) : _module(module) {}
    
    void OnPlayerLogin(Player* player) override
    {
        if (_module && _module->IsEnabled())
        {
            _module->HandlePlayerLogin(player);
        }
    }
    
    void OnPlayerLogout(Player* player) override
    {
        if (_module && _module->IsEnabled())
        {
            _module->HandlePlayerLogout(player);
        }
    }
    
    void OnPlayerLevelChanged(Player* player, uint8 oldLevel) override
    {
        if (_module && _module->IsEnabled())
        {
            _module->HandlePlayerLevelChanged(player, oldLevel);
        }
    }
    
    void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg) override
    {
        if (_module && _module->IsEnabled())
        {
            _module->HandlePlayerChat(player, type, lang, msg);
        }
    }
    
private:
    ExampleModule* _module;
};

// Chat Command Handler
class ExampleModuleChatHandler
{
public:
    static bool HandleExampleInfoCommand(ChatHandler* handler, char const* args)
    {
        ExampleModule* module = ExampleModule::GetInstance();
        if (!module || !module->IsEnabled())
        {
            handler->SendSysMessage("Example module is not available.");
            return true;
        }
        
        std::ostringstream ss;
        ss << "Example Module Information:\n";
        ss << "Version: " << module->GetVersion() << "\n";
        ss << "Status: " << (module->IsEnabled() ? "Enabled" : "Disabled") << "\n";
        ss << "Players tracked: " << module->GetTrackedPlayersCount() << "\n";
        ss << "Total logins: " << module->GetTotalLogins() << "\n";
        ss << "Total level ups: " << module->GetTotalLevelUps();
        
        handler->SendSysMessage(ss.str().c_str());
        return true;
    }
    
    static bool HandleExampleStatsCommand(ChatHandler* handler, char const* args)
    {
        ExampleModule* module = ExampleModule::GetInstance();
        if (!module || !module->IsEnabled())
        {
            handler->SendSysMessage("Example module is not available.");
            return true;
        }
        
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;
            
        auto playerData = module->GetPlayerData(player->GetGUID());
        if (!playerData)
        {
            handler->SendSysMessage("No data found for your character.");
            return true;
        }
        
        std::ostringstream ss;
        ss << "Your Statistics:\n";
        ss << "Login count: " << playerData->loginCount << "\n";
        ss << "Time played this session: " << (GameTime::GetGameTime() - playerData->sessionStartTime) << " seconds\n";
        ss << "Last level up: " << (playerData->lastLevelUpTime > 0 ? 
            std::to_string(GameTime::GetGameTime() - playerData->lastLevelUpTime) + " seconds ago" : "Never");
        
        handler->SendSysMessage(ss.str().c_str());
        return true;
    }
    
    static bool HandleExampleReloadCommand(ChatHandler* handler, char const* args)
    {
        ExampleModule* module = ExampleModule::GetInstance();
        if (!module)
        {
            handler->SendSysMessage("Example module is not available.");
            return true;
        }
        
        if (module->ReloadConfiguration())
        {
            handler->SendSysMessage("Example module configuration reloaded successfully.");
        }
        else
        {
            handler->SendSysMessage("Failed to reload example module configuration.");
        }
        return true;
    }
};

// ExampleModule Implementation
ExampleModule* ExampleModule::_instance = nullptr;

ExampleModule::ExampleModule()
    : _enabled(false)
    , _debugMode(false)
    , _eventHandler(nullptr)
    , _totalLogins(0)
    , _totalLevelUps(0)
    , _lastStatsUpdate(0)
    , _lastConfigCheck(0)
{
    _instance = this;
}

ExampleModule::~ExampleModule()
{
    Shutdown();
    _instance = nullptr;
}

ExampleModule* ExampleModule::GetInstance()
{
    return _instance;
}

bool ExampleModule::Initialize()
{
    TC_LOG_INFO("modules", "Initializing Example Module...");
    
    // Load configuration
    if (!LoadConfiguration())
    {
        TC_LOG_ERROR("modules", "Failed to load Example Module configuration");
        return false;
    }
    
    // Check dependencies
    if (!CheckDependencies())
    {
        TC_LOG_ERROR("modules", "Example Module dependency check failed");
        return false;
    }
    
    // Initialize event handler
    _eventHandler = std::make_unique<ExampleModuleEventHandler>(this);
    
    // Register chat commands
    RegisterChatCommands();
    
    // Initialize statistics
    _totalLogins = 0;
    _totalLevelUps = 0;
    _lastStatsUpdate = GameTime::GetGameTime();
    _lastConfigCheck = GameTime::GetGameTime();
    
    _enabled = true;
    
    TC_LOG_INFO("modules", "Example Module initialized successfully");
    return true;
}

void ExampleModule::Shutdown()
{
    if (!_enabled)
        return;
        
    TC_LOG_INFO("modules", "Shutting down Example Module...");
    
    // Save statistics
    SaveStatistics();
    
    // Clear player data
    {
        std::lock_guard<std::mutex> lock(_playerDataMutex);
        _playerData.clear();
    }
    
    // Unregister chat commands
    UnregisterChatCommands();
    
    // Clean up event handler
    _eventHandler.reset();
    
    _enabled = false;
    
    TC_LOG_INFO("modules", "Example Module shut down successfully");
}

void ExampleModule::Update(uint32 diff)
{
    if (!_enabled)
        return;
        
    uint32 currentTime = GameTime::GetGameTime();
    
    // Update statistics periodically
    uint32 statsInterval = sConfigMgr->GetIntDefault("Statistics.UpdateInterval", 30000) / 1000;
    if (currentTime - _lastStatsUpdate >= statsInterval)
    {
        UpdateStatistics();
        _lastStatsUpdate = currentTime;
    }
    
    // Check for configuration changes
    uint32 configInterval = sConfigMgr->GetIntDefault("Advanced.ConfigReloadInterval", 0);
    if (configInterval > 0 && currentTime - _lastConfigCheck >= configInterval)
    {
        if (sConfigMgr->GetBoolDefault("Advanced.EnableHotReload", true))
        {
            ReloadConfiguration();
        }
        _lastConfigCheck = currentTime;
    }
    
    // Clean up offline player data
    CleanupPlayerData();
}

bool ExampleModule::LoadConfiguration()
{
    try
    {
        _enabled = sConfigMgr->GetBoolDefault("ExampleModule.Enabled", true);
        _debugMode = sConfigMgr->GetBoolDefault("ExampleModule.DebugMode", false);
        
        // Load welcome message settings
        _config.welcomeEnabled = sConfigMgr->GetBoolDefault("WelcomeMessage.Enabled", true);
        _config.welcomeText = sConfigMgr->GetStringDefault("WelcomeMessage.Text", "Welcome to the server!");
        _config.welcomeDelay = sConfigMgr->GetIntDefault("WelcomeMessage.Delay", 5);
        _config.welcomeFirstLoginOnly = sConfigMgr->GetBoolDefault("WelcomeMessage.ShowOnlyFirstLogin", false);
        
        // Load level up reward settings
        _config.rewardEnabled = sConfigMgr->GetBoolDefault("LevelUpReward.Enabled", false);
        _config.rewardItemId = sConfigMgr->GetIntDefault("LevelUpReward.ItemId", 6948);
        _config.rewardCount = sConfigMgr->GetIntDefault("LevelUpReward.Count", 1);
        _config.rewardMinLevel = sConfigMgr->GetIntDefault("LevelUpReward.MinLevel", 10);
        _config.rewardMaxLevel = sConfigMgr->GetIntDefault("LevelUpReward.MaxLevel", 0);
        _config.rewardInterval = sConfigMgr->GetIntDefault("LevelUpReward.Interval", 5);
        
        // Load other settings
        _config.statisticsEnabled = sConfigMgr->GetBoolDefault("Statistics.Enabled", true);
        _config.commandsEnabled = sConfigMgr->GetBoolDefault("Commands.Enabled", true);
        _config.playerDataEnabled = sConfigMgr->GetBoolDefault("PlayerData.Enabled", true);
        
        if (_debugMode)
        {
            TC_LOG_DEBUG("modules", "Example Module configuration loaded successfully");
        }
        
        return true;
    }
    catch (const std::exception& e)
    {
        TC_LOG_ERROR("modules", "Failed to load Example Module configuration: {}", e.what());
        return false;
    }
}

bool ExampleModule::ReloadConfiguration()
{
    TC_LOG_INFO("modules", "Reloading Example Module configuration...");
    
    // Backup current config if enabled
    if (sConfigMgr->GetBoolDefault("Advanced.BackupConfig", true))
    {
        // Implementation would backup current config
    }
    
    bool result = LoadConfiguration();
    
    if (result)
    {
        TC_LOG_INFO("modules", "Example Module configuration reloaded successfully");
    }
    else
    {
        TC_LOG_ERROR("modules", "Failed to reload Example Module configuration");
    }
    
    return result;
}

bool ExampleModule::CheckDependencies()
{
    // Check if required systems are available
    if (!sWorld)
    {
        TC_LOG_ERROR("modules", "World system not available");
        return false;
    }
    
    if (!sConfigMgr)
    {
        TC_LOG_ERROR("modules", "Configuration manager not available");
        return false;
    }
    
    // Check database connectivity if needed
    // if (!WorldDatabase.IsConnected())
    // {
    //     TC_LOG_ERROR("modules", "World database not connected");
    //     return false;
    // }
    
    return true;
}

void ExampleModule::RegisterChatCommands()
{
    if (!_config.commandsEnabled)
        return;
        
    // Register chat commands with the command system
    // This would typically involve registering with TrinityCore's command system
    // Implementation depends on TrinityCore's command registration mechanism
    
    if (_debugMode)
    {
        TC_LOG_DEBUG("modules", "Example Module chat commands registered");
    }
}

void ExampleModule::UnregisterChatCommands()
{
    // Unregister chat commands
    // Implementation depends on TrinityCore's command system
    
    if (_debugMode)
    {
        TC_LOG_DEBUG("modules", "Example Module chat commands unregistered");
    }
}

void ExampleModule::HandlePlayerLogin(Player* player)
{
    if (!player || !_config.playerDataEnabled)
        return;
        
    ObjectGuid guid = player->GetGUID();
    
    {
        std::lock_guard<std::mutex> lock(_playerDataMutex);
        
        auto& data = _playerData[guid];
        data.loginCount++;
        data.sessionStartTime = GameTime::GetGameTime();
        data.lastSeenTime = GameTime::GetGameTime();
        
        _totalLogins++;
    }
    
    // Send welcome message
    if (_config.welcomeEnabled)
    {
        bool shouldShow = true;
        if (_config.welcomeFirstLoginOnly)
        {
            std::lock_guard<std::mutex> lock(_playerDataMutex);
            auto it = _playerData.find(guid);
            if (it != _playerData.end() && it->second.loginCount > 1)
            {
                shouldShow = false;
            }
        }
        
        if (shouldShow)
        {
            // Schedule welcome message
            player->GetScheduler().Schedule(Milliseconds(_config.welcomeDelay * 1000), [this, player](TaskContext /*context*/)
            {
                if (player && player->IsInWorld())
                {
                    ChatHandler(player->GetSession()).SendSysMessage(_config.welcomeText.c_str());
                }
            });
        }
    }
    
    if (_debugMode)
    {
        TC_LOG_DEBUG("modules", "Player {} logged in, total logins: {}", 
                    player->GetName(), _totalLogins);
    }
}

void ExampleModule::HandlePlayerLogout(Player* player)
{
    if (!player || !_config.playerDataEnabled)
        return;
        
    ObjectGuid guid = player->GetGUID();
    
    {
        std::lock_guard<std::mutex> lock(_playerDataMutex);
        
        auto it = _playerData.find(guid);
        if (it != _playerData.end())
        {
            it->second.lastSeenTime = GameTime::GetGameTime();
            it->second.totalPlayTime += GameTime::GetGameTime() - it->second.sessionStartTime;
        }
    }
    
    if (_debugMode)
    {
        TC_LOG_DEBUG("modules", "Player {} logged out", player->GetName());
    }
}

void ExampleModule::HandlePlayerLevelChanged(Player* player, uint8 oldLevel)
{
    if (!player)
        return;
        
    uint8 newLevel = player->getLevel();
    ObjectGuid guid = player->GetGUID();
    
    {
        std::lock_guard<std::mutex> lock(_playerDataMutex);
        
        auto& data = _playerData[guid];
        data.lastLevelUpTime = GameTime::GetGameTime();
        
        _totalLevelUps++;
    }
    
    // Give level up reward if configured
    if (_config.rewardEnabled && 
        newLevel >= _config.rewardMinLevel &&
        (_config.rewardMaxLevel == 0 || newLevel <= _config.rewardMaxLevel) &&
        (newLevel - _config.rewardMinLevel) % _config.rewardInterval == 0)
    {
        // Add item to player's inventory
        if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(_config.rewardItemId))
        {
            player->AddItem(_config.rewardItemId, _config.rewardCount);
            
            std::ostringstream ss;
            ss << "Congratulations on reaching level " << static_cast<uint32>(newLevel) 
               << "! You have received a reward.";
            ChatHandler(player->GetSession()).SendSysMessage(ss.str().c_str());
        }
    }
    
    if (_debugMode)
    {
        TC_LOG_DEBUG("modules", "Player {} leveled from {} to {}, total level ups: {}", 
                    player->GetName(), oldLevel, newLevel, _totalLevelUps);
    }
}

void ExampleModule::HandlePlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg)
{
    if (!player || !sConfigMgr->GetBoolDefault("Features.ChatLogging", false))
        return;
        
    // Log chat messages if enabled
    if (_debugMode)
    {
        TC_LOG_DEBUG("modules", "Player {} chat: {}", player->GetName(), msg);
    }
}

std::shared_ptr<ExampleModule::PlayerData> ExampleModule::GetPlayerData(ObjectGuid guid)
{
    std::lock_guard<std::mutex> lock(_playerDataMutex);
    
    auto it = _playerData.find(guid);
    if (it != _playerData.end())
    {
        return std::make_shared<PlayerData>(it->second);
    }
    
    return nullptr;
}

uint32 ExampleModule::GetTrackedPlayersCount() const
{
    std::lock_guard<std::mutex> lock(_playerDataMutex);
    return static_cast<uint32>(_playerData.size());
}

void ExampleModule::UpdateStatistics()
{
    if (!_config.statisticsEnabled)
        return;
        
    // Update internal statistics
    uint32 currentTime = GameTime::GetGameTime();
    
    // Save statistics to file if configured
    uint32 saveInterval = sConfigMgr->GetIntDefault("Statistics.SaveInterval", 300);
    static uint32 lastSave = 0;
    
    if (currentTime - lastSave >= saveInterval)
    {
        SaveStatistics();
        lastSave = currentTime;
    }
}

void ExampleModule::SaveStatistics()
{
    if (!_config.statisticsEnabled)
        return;
        
    try
    {
        std::string filename = sConfigMgr->GetStringDefault("Statistics.FileName", "example_module_stats.txt");
        std::ofstream file(filename);
        
        if (file.is_open())
        {
            file << "Example Module Statistics\n";
            file << "========================\n";
            file << "Generated: " << std::put_time(std::localtime(&(time_t){GameTime::GetGameTime()}), "%Y-%m-%d %H:%M:%S") << "\n";
            file << "Total Logins: " << _totalLogins << "\n";
            file << "Total Level Ups: " << _totalLevelUps << "\n";
            file << "Tracked Players: " << GetTrackedPlayersCount() << "\n";
            
            file.close();
            
            if (_debugMode)
            {
                TC_LOG_DEBUG("modules", "Statistics saved to {}", filename);
            }
        }
    }
    catch (const std::exception& e)
    {
        TC_LOG_ERROR("modules", "Failed to save statistics: {}", e.what());
    }
}

void ExampleModule::CleanupPlayerData()
{
    if (!_config.playerDataEnabled)
        return;
        
    uint32 cleanupInterval = sConfigMgr->GetIntDefault("PlayerData.CleanupInterval", 3600);
    bool persistOffline = sConfigMgr->GetBoolDefault("PlayerData.PersistOffline", true);
    uint32 maxEntries = sConfigMgr->GetIntDefault("PlayerData.MaxEntries", 10000);
    
    if (persistOffline)
        return; // Don't cleanup if we want to persist offline data
        
    std::lock_guard<std::mutex> lock(_playerDataMutex);
    
    uint32 currentTime = GameTime::GetGameTime();
    auto it = _playerData.begin();
    
    while (it != _playerData.end())
    {
        // Remove data for players who have been offline for too long
        if (currentTime - it->second.lastSeenTime > cleanupInterval)
        {
            it = _playerData.erase(it);
        }
        else
        {
            ++it;
        }
    }
    
    // Enforce maximum entries limit
    while (_playerData.size() > maxEntries)
    {
        // Remove oldest entries
        auto oldest = std::min_element(_playerData.begin(), _playerData.end(),
            [](const auto& a, const auto& b) {
                return a.second.lastSeenTime < b.second.lastSeenTime;
            });
            
        if (oldest != _playerData.end())
        {
            _playerData.erase(oldest);
        }
        else
        {
            break;
        }
    }
}

// Plugin registration
REGISTER_PLUGIN(ExampleModule);