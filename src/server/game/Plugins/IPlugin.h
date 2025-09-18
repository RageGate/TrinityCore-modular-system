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

#ifndef TRINITY_IPLUGIN_H
#define TRINITY_IPLUGIN_H

#include "Define.h"
#include <string>
#include <vector>
#include <memory>

class Player;
class WorldSession;
class WorldObject;
class GameObject;
class Creature;
class Unit;
class Map;
class WorldPacket;

enum class PluginState : uint8
{
    UNLOADED = 0,
    LOADING = 1,
    LOADED = 2,
    INITIALIZING = 3,
    INITIALIZED = 4,
    RUNNING = 5,
    STOPPING = 6,
    ERROR = 7
};

enum class PluginPriority : uint8
{
    LOWEST = 0,
    LOW = 1,
    NORMAL = 2,
    HIGH = 3,
    HIGHEST = 4,
    CRITICAL = 5
};

struct PluginInfo
{
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    std::string website;
    std::vector<std::string> dependencies;
    PluginPriority priority;
    bool autoLoad;
    
    PluginInfo() : priority(PluginPriority::NORMAL), autoLoad(true) { }
};

class TC_GAME_API IEventHandler
{
public:
    virtual ~IEventHandler() = default;
    
    // Player Events
    virtual void OnPlayerLogin(Player* /*player*/) { }
    virtual void OnPlayerLogout(Player* /*player*/) { }
    virtual void OnPlayerLevelChanged(Player* /*player*/, uint8 /*oldLevel*/) { }
    virtual void OnPlayerChat(Player* /*player*/, uint32 /*type*/, uint32 /*lang*/, std::string& /*msg*/) { }
    virtual void OnPlayerKill(Player* /*killer*/, Player* /*killed*/) { }
    virtual void OnPlayerKillCreature(Player* /*killer*/, Creature* /*killed*/) { }
    
    // Creature Events
    virtual void OnCreatureKill(Creature* /*killer*/, Unit* /*killed*/) { }
    virtual void OnCreatureDeath(Creature* /*creature*/, Unit* /*killer*/) { }
    virtual void OnCreatureRespawn(Creature* /*creature*/) { }
    
    // GameObject Events
    virtual void OnGameObjectUse(GameObject* /*go*/, Player* /*player*/) { }
    virtual void OnGameObjectDestroyed(GameObject* /*go*/, Player* /*player*/) { }
    
    // World Events
    virtual void OnWorldUpdate(uint32 /*diff*/) { }
    virtual void OnMapUpdate(Map* /*map*/, uint32 /*diff*/) { }
    
    // Packet Events
    virtual bool OnPacketReceive(WorldSession* /*session*/, WorldPacket& /*packet*/) { return true; }
    virtual bool OnPacketSend(WorldSession* /*session*/, WorldPacket const& /*packet*/) { return true; }
    
    // Server Events
    virtual void OnServerStart() { }
    virtual void OnServerStop() { }
    virtual void OnConfigReload() { }
};

class TC_GAME_API IPlugin
{
public:
    virtual ~IPlugin() = default;
    
    // Plugin lifecycle
    virtual bool Load() = 0;
    virtual bool Initialize() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Unload() = 0;
    
    // Plugin information
    virtual PluginInfo const& GetInfo() const = 0;
    virtual PluginState GetState() const = 0;
    
    // Event handling
    virtual IEventHandler* GetEventHandler() { return nullptr; }
    
    // Configuration
    virtual bool LoadConfig(std::string const& /*configPath*/) { return true; }
    virtual void ReloadConfig() { }
    
    // Dependencies
    virtual std::vector<std::string> const& GetDependencies() const = 0;
    virtual bool CheckDependencies() const = 0;
    
    // Plugin communication
    virtual void* GetInterface(std::string const& /*interfaceName*/) { return nullptr; }
    virtual bool HasInterface(std::string const& /*interfaceName*/) const { return false; }
    
protected:
    PluginState _state = PluginState::UNLOADED;
};

// Plugin factory function type
typedef std::unique_ptr<IPlugin>(*)() PluginCreateFunc;
typedef void(*)() PluginDestroyFunc;

// Macros for plugin registration
#define TRINITY_PLUGIN_EXPORT extern "C" TC_GAME_API

#define DECLARE_TRINITY_PLUGIN(PluginClass) \
    TRINITY_PLUGIN_EXPORT std::unique_ptr<IPlugin> CreatePlugin() \
    { \
        return std::make_unique<PluginClass>(); \
    } \
    TRINITY_PLUGIN_EXPORT void DestroyPlugin() \
    { \
        /* Cleanup if needed */ \
    }

#endif // TRINITY_IPLUGIN_H