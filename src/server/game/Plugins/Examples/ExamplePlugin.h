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

#ifndef TRINITY_EXAMPLE_PLUGIN_H
#define TRINITY_EXAMPLE_PLUGIN_H

#include "IPlugin.h"
#include "PluginConfig.h"
#include "Player.h"
#include "WorldSession.h"
#include "Chat.h"
#include "Log.h"
#include <memory>

/*
 * Example Plugin Implementation
 * 
 * This plugin demonstrates:
 * - Basic plugin structure and lifecycle
 * - Event handling for player login/logout
 * - Configuration management
 * - Chat command registration
 * - Player data tracking
 */

class ExampleEventHandler : public IEventHandler
{
public:
    ExampleEventHandler(class ExamplePlugin* plugin) : _plugin(plugin) { }
    
    // Player Events
    void OnPlayerLogin(Player* player) override;
    void OnPlayerLogout(Player* player) override;
    void OnPlayerLevelChanged(Player* player, uint8 oldLevel) override;
    void OnPlayerChat(Player* player, uint32 type, uint32 lang, std::string& msg) override;
    
    // World Events
    void OnWorldUpdate(uint32 diff) override;
    
    // Server Events
    void OnServerStart() override;
    void OnServerStop() override;
    void OnConfigReload() override;
    
private:
    class ExamplePlugin* _plugin;
};

class TC_GAME_API ExamplePlugin : public IPlugin
{
public:
    ExamplePlugin();
    virtual ~ExamplePlugin();
    
    // IPlugin interface implementation
    bool Load() override;
    bool Initialize() override;
    void Start() override;
    void Stop() override;
    void Unload() override;
    
    PluginInfo const& GetInfo() const override { return _info; }
    PluginState GetState() const override { return _state; }
    
    IEventHandler* GetEventHandler() override { return _eventHandler.get(); }
    
    bool LoadConfig(std::string const& configPath) override;
    void ReloadConfig() override;
    
    std::vector<std::string> const& GetDependencies() const override { return _dependencies; }
    bool CheckDependencies() const override;
    
    // Plugin-specific functionality
    void SendWelcomeMessage(Player* player);
    void TrackPlayerLogin(Player* player);
    void TrackPlayerLogout(Player* player);
    void ProcessLevelUpReward(Player* player, uint8 oldLevel);
    
    // Chat command handlers
    bool HandleExampleCommand(Player* player, std::string const& args);
    bool HandleStatsCommand(Player* player, std::string const& args);
    bool HandleReloadCommand(Player* player, std::string const& args);
    
    // Configuration accessors
    bool IsWelcomeMessageEnabled() const { return _welcomeMessageEnabled; }
    bool IsLevelUpRewardEnabled() const { return _levelUpRewardEnabled; }
    uint32 GetLevelUpRewardItem() const { return _levelUpRewardItem; }
    uint32 GetLevelUpRewardCount() const { return _levelUpRewardCount; }
    std::string const& GetWelcomeMessage() const { return _welcomeMessage; }
    
    // Statistics
    uint32 GetTotalLogins() const { return _totalLogins; }
    uint32 GetCurrentOnlinePlayers() const { return _currentOnlinePlayers; }
    uint32 GetTotalLevelUps() const { return _totalLevelUps; }
    
private:
    // Plugin information
    PluginInfo _info;
    std::vector<std::string> _dependencies;
    
    // Event handler
    std::unique_ptr<ExampleEventHandler> _eventHandler;
    
    // Configuration
    std::unique_ptr<PluginConfig> _config;
    bool _welcomeMessageEnabled;
    bool _levelUpRewardEnabled;
    std::string _welcomeMessage;
    uint32 _levelUpRewardItem;
    uint32 _levelUpRewardCount;
    uint32 _updateInterval;
    
    // Statistics tracking
    uint32 _totalLogins;
    uint32 _currentOnlinePlayers;
    uint32 _totalLevelUps;
    uint32 _lastUpdateTime;
    
    // Helper methods
    void InitializeConfig();
    void RegisterChatCommands();
    void UnregisterChatCommands();
    void UpdateStatistics(uint32 diff);
    void SaveStatistics();
    void LoadStatistics();
};

// Chat command structure for the example plugin
struct ExampleChatCommand
{
    std::string command;
    std::string description;
    uint32 securityLevel;
    std::function<bool(Player*, std::string const&)> handler;
    
    ExampleChatCommand(std::string const& cmd, std::string const& desc, uint32 security,
                      std::function<bool(Player*, std::string const&)> h)
        : command(cmd), description(desc), securityLevel(security), handler(h) { }
};

// Plugin registration macro usage
DECLARE_TRINITY_PLUGIN(ExamplePlugin);

#endif // TRINITY_EXAMPLE_PLUGIN_H