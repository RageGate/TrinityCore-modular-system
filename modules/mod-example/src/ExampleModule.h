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

#ifndef EXAMPLE_MODULE_H
#define EXAMPLE_MODULE_H

#include "IPlugin.h"
#include "PluginConfig.h"
#include "ExampleEventHandler.h"
#include "ExampleCommands.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <atomic>
#include <mutex>

namespace TC
{
    namespace Modules
    {
        /**
         * @brief Example Module demonstrating TrinityCore modular system
         * 
         * This module showcases:
         * - Player event handling
         * - Configuration management
         * - Chat command registration
         * - Statistics tracking
         * - Thread-safe operations
         */
        class ExampleModule : public IPlugin
        {
        public:
            ExampleModule();
            virtual ~ExampleModule();

            // IPlugin interface implementation
            bool Load() override;
            bool Initialize() override;
            void Start() override;
            void Stop() override;
            void Unload() override;

            // Configuration management
            bool LoadConfig(std::string const& configPath) override;
            void ReloadConfig() override;

            // Dependency management
            bool CheckDependencies() const override;

            // Module-specific functionality
            void SendWelcomeMessage(Player* player);
            void TrackPlayerLogin(Player* player);
            void TrackPlayerLogout(Player* player);
            void ProcessLevelUpReward(Player* player, uint8 oldLevel);
            void UpdateStatistics(uint32 diff);
            void SaveStatistics();
            void LoadStatistics();

            // Configuration accessors
            bool IsWelcomeMessageEnabled() const { return _welcomeMessageEnabled; }
            bool IsLevelUpRewardEnabled() const { return _levelUpRewardEnabled; }
            std::string const& GetWelcomeMessage() const { return _welcomeMessage; }
            uint32 GetLevelUpRewardItem() const { return _levelUpRewardItem; }
            uint32 GetLevelUpRewardCount() const { return _levelUpRewardCount; }
            uint32 GetUpdateInterval() const { return _updateInterval; }

            // Statistics accessors
            uint32 GetTotalLogins() const { return _totalLogins.load(); }
            uint32 GetCurrentOnlinePlayers() const { return _currentOnlinePlayers.load(); }
            uint32 GetTotalLevelUps() const { return _totalLevelUps.load(); }

            // Thread-safe player data management
            void SetPlayerData(uint32 playerGuid, std::string const& key, std::string const& value);
            std::string GetPlayerData(uint32 playerGuid, std::string const& key, std::string const& defaultValue = "");
            void RemovePlayerData(uint32 playerGuid);

            // Event broadcasting
            void BroadcastMessage(std::string const& message, uint32 type = 0);
            void BroadcastToGMs(std::string const& message);

            // Utility functions
            bool IsPlayerOnline(uint32 playerGuid) const;
            Player* GetPlayerByGuid(uint32 playerGuid) const;
            uint32 GetOnlinePlayerCount() const;

        private:
            // Configuration initialization
            void InitializeConfig();
            void ApplyConfigChanges();

            // Command registration
            void RegisterChatCommands();
            void UnregisterChatCommands();

            // Statistics management
            void ResetStatistics();
            void ScheduleStatisticsSave();

            // Player data cleanup
            void CleanupPlayerData();

            // Configuration variables
            bool _welcomeMessageEnabled;
            bool _levelUpRewardEnabled;
            std::string _welcomeMessage;
            uint32 _levelUpRewardItem;
            uint32 _levelUpRewardCount;
            uint32 _updateInterval;
            uint32 _statisticsSaveInterval;
            bool _debugMode;
            uint32 _maxPlayerDataEntries;

            // Statistics (thread-safe)
            std::atomic<uint32> _totalLogins;
            std::atomic<uint32> _currentOnlinePlayers;
            std::atomic<uint32> _totalLevelUps;
            std::atomic<uint32> _totalChatMessages;
            std::atomic<uint32> _totalCommandsExecuted;

            // Timing
            uint32 _lastUpdateTime;
            uint32 _lastStatisticsSave;

            // Component instances
            std::unique_ptr<PluginConfig> _config;
            std::unique_ptr<ExampleEventHandler> _eventHandler;
            std::unique_ptr<ExampleCommands> _commandHandler;

            // Player data storage (thread-safe)
            mutable std::mutex _playerDataMutex;
            std::unordered_map<uint32, std::unordered_map<std::string, std::string>> _playerData;

            // Online players tracking
            mutable std::mutex _onlinePlayersMutex;
            std::unordered_set<uint32> _onlinePlayers;

            // Module state
            mutable std::mutex _stateMutex;
            bool _isRunning;
            bool _configLoaded;

            // Constants
            static constexpr uint32 DEFAULT_UPDATE_INTERVAL = 10000; // 10 seconds
            static constexpr uint32 DEFAULT_STATISTICS_SAVE_INTERVAL = 300000; // 5 minutes
            static constexpr uint32 MAX_PLAYER_DATA_ENTRIES = 10000;
            static constexpr uint32 CLEANUP_INTERVAL = 3600000; // 1 hour
        };

        // Global module instance accessor
        ExampleModule* GetExampleModule();
    }
}

// Module registration macro
REGISTER_PLUGIN(TC::Modules::ExampleModule)

#endif // EXAMPLE_MODULE_H