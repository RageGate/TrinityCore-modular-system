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

#ifndef TRINITY_PLUGIN_HOOKS_H
#define TRINITY_PLUGIN_HOOKS_H

#include "Define.h"

class Player;
class WorldSession;
class WorldObject;
class GameObject;
class Creature;
class Unit;
class Map;
class WorldPacket;
class Spell;
class Item;
class Quest;
class Group;
class Guild;
class Battleground;
class InstanceScript;

/*
 * Plugin Hook Macros
 * These macros should be placed at strategic points in TrinityCore's source code
 * to allow plugins to hook into core functionality.
 */

// Player Event Hooks
#define PLUGIN_HOOK_PLAYER_LOGIN(player) \
    do { \
        if (sPluginManager) \
            sPluginManager->OnPlayerLogin(player); \
    } while(0)

#define PLUGIN_HOOK_PLAYER_LOGOUT(player) \
    do { \
        if (sPluginManager) \
            sPluginManager->OnPlayerLogout(player); \
    } while(0)

#define PLUGIN_HOOK_PLAYER_LEVEL_CHANGED(player, oldLevel) \
    do { \
        if (sPluginManager) \
            sPluginManager->OnPlayerLevelChanged(player, oldLevel); \
    } while(0)

#define PLUGIN_HOOK_PLAYER_CHAT(player, type, lang, msg) \
    do { \
        if (sPluginManager) \
            sPluginManager->OnPlayerChat(player, type, lang, msg); \
    } while(0)

#define PLUGIN_HOOK_PLAYER_KILL_PLAYER(killer, killed) \
    do { \
        if (sPluginManager) \
            sPluginManager->OnPlayerKill(killer, killed); \
    } while(0)

#define PLUGIN_HOOK_PLAYER_KILL_CREATURE(killer, killed) \
    do { \
        if (sPluginManager) \
            sPluginManager->OnPlayerKillCreature(killer, killed); \
    } while(0)

// Creature Event Hooks
#define PLUGIN_HOOK_CREATURE_KILL(killer, killed) \
    do { \
        if (sPluginManager) \
            sPluginManager->OnCreatureKill(killer, killed); \
    } while(0)

#define PLUGIN_HOOK_CREATURE_DEATH(creature, killer) \
    do { \
        if (sPluginManager) \
            sPluginManager->OnCreatureDeath(creature, killer); \
    } while(0)

#define PLUGIN_HOOK_CREATURE_RESPAWN(creature) \
    do { \
        if (sPluginManager) \
            sPluginManager->OnCreatureRespawn(creature); \
    } while(0)

// GameObject Event Hooks
#define PLUGIN_HOOK_GAMEOBJECT_USE(go, player) \
    do { \
        if (sPluginManager) \
            sPluginManager->OnGameObjectUse(go, player); \
    } while(0)

#define PLUGIN_HOOK_GAMEOBJECT_DESTROYED(go, player) \
    do { \
        if (sPluginManager) \
            sPluginManager->OnGameObjectDestroyed(go, player); \
    } while(0)

// World Event Hooks
#define PLUGIN_HOOK_WORLD_UPDATE(diff) \
    do { \
        if (sPluginManager) \
            sPluginManager->OnWorldUpdate(diff); \
    } while(0)

#define PLUGIN_HOOK_MAP_UPDATE(map, diff) \
    do { \
        if (sPluginManager) \
            sPluginManager->OnMapUpdate(map, diff); \
    } while(0)

// Packet Event Hooks
#define PLUGIN_HOOK_PACKET_RECEIVE(session, packet) \
    (sPluginManager ? sPluginManager->OnPacketReceive(session, packet) : true)

#define PLUGIN_HOOK_PACKET_SEND(session, packet) \
    (sPluginManager ? sPluginManager->OnPacketSend(session, packet) : true)

// Server Event Hooks
#define PLUGIN_HOOK_SERVER_START() \
    do { \
        if (sPluginManager) \
            sPluginManager->OnServerStart(); \
    } while(0)

#define PLUGIN_HOOK_SERVER_STOP() \
    do { \
        if (sPluginManager) \
            sPluginManager->OnServerStop(); \
    } while(0)

#define PLUGIN_HOOK_CONFIG_RELOAD() \
    do { \
        if (sPluginManager) \
            sPluginManager->OnConfigReload(); \
    } while(0)

/*
 * Extended Event Hooks for Advanced Plugin Functionality
 * These provide more granular control over game events
 */

class TC_GAME_API PluginHooks
{
public:
    // Spell Events
    static void OnSpellCast(Unit* caster, Spell* spell, bool skipCheck = false);
    static void OnSpellHit(Unit* caster, Unit* target, Spell* spell);
    static void OnSpellEffect(Unit* caster, Unit* target, uint32 spellId, uint32 effIndex);
    
    // Item Events
    static void OnItemUse(Player* player, Item* item);
    static void OnItemEquip(Player* player, Item* item, uint8 slot);
    static void OnItemUnequip(Player* player, Item* item, uint8 slot);
    static void OnItemLoot(Player* player, Item* item, uint32 count);
    
    // Quest Events
    static void OnQuestAccept(Player* player, Quest const* quest);
    static void OnQuestComplete(Player* player, Quest const* quest);
    static void OnQuestAbandon(Player* player, Quest const* quest);
    static void OnQuestReward(Player* player, Quest const* quest);
    
    // Group Events
    static void OnGroupCreate(Group* group, Player* leader);
    static void OnGroupDisband(Group* group);
    static void OnGroupMemberAdd(Group* group, Player* player);
    static void OnGroupMemberRemove(Group* group, Player* player);
    
    // Guild Events
    static void OnGuildCreate(Guild* guild, Player* leader);
    static void OnGuildDisband(Guild* guild);
    static void OnGuildMemberAdd(Guild* guild, Player* player);
    static void OnGuildMemberRemove(Guild* guild, Player* player);
    
    // Battleground Events
    static void OnBattlegroundStart(Battleground* bg);
    static void OnBattlegroundEnd(Battleground* bg);
    static void OnBattlegroundPlayerJoin(Battleground* bg, Player* player);
    static void OnBattlegroundPlayerLeave(Battleground* bg, Player* player);
    
    // Instance Events
    static void OnInstanceCreate(InstanceScript* instance);
    static void OnInstanceDestroy(InstanceScript* instance);
    static void OnInstancePlayerEnter(InstanceScript* instance, Player* player);
    static void OnInstancePlayerLeave(InstanceScript* instance, Player* player);
    
    // Combat Events
    static void OnCombatStart(Unit* attacker, Unit* victim);
    static void OnCombatStop(Unit* unit);
    static void OnDamageDealt(Unit* attacker, Unit* victim, uint32 damage, uint32 spellId);
    static void OnHealingDone(Unit* healer, Unit* target, uint32 healing, uint32 spellId);
    
    // Auction House Events
    static void OnAuctionAdd(Player* player, uint32 itemEntry, uint32 count, uint32 price);
    static void OnAuctionSuccessful(Player* seller, Player* buyer, uint32 itemEntry, uint32 price);
    static void OnAuctionExpire(Player* player, uint32 itemEntry);
    
    // Mail Events
    static void OnMailSend(Player* sender, Player* receiver, std::string const& subject, std::string const& body);
    static void OnMailReceive(Player* player, uint32 mailId);
    
    // Trade Events
    static void OnTradeStart(Player* player1, Player* player2);
    static void OnTradeComplete(Player* player1, Player* player2);
    static void OnTradeCancel(Player* player1, Player* player2);
    
    // Channel Events
    static void OnChannelJoin(Player* player, std::string const& channelName);
    static void OnChannelLeave(Player* player, std::string const& channelName);
    static void OnChannelMessage(Player* player, std::string const& channelName, std::string const& message);
    
    // Weather Events
    static void OnWeatherChange(uint32 mapId, uint32 zoneId, uint32 weatherType, float grade);
    
    // Transport Events
    static void OnTransportAddPassenger(Unit* transport, Player* player);
    static void OnTransportRemovePassenger(Unit* transport, Player* player);
    
    // Achievement Events
    static void OnAchievementEarned(Player* player, uint32 achievementId);
    static void OnCriteriaProgress(Player* player, uint32 criteriaId, uint32 progress);
    
    // Talent Events
    static void OnTalentReset(Player* player);
    static void OnTalentLearn(Player* player, uint32 talentId, uint32 rank);
    
    // Pet Events
    static void OnPetSummon(Player* player, Unit* pet);
    static void OnPetDismiss(Player* player, Unit* pet);
    static void OnPetLevelUp(Unit* pet, uint8 newLevel);
};

#endif // TRINITY_PLUGIN_HOOKS_H