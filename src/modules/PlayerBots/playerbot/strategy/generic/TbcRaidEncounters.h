#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class Player;
class Unit;
class Creature;
class PlayerbotAI;

namespace ai
{
    // Data driven boss tactics for the Burning Crusade raids
    // (Gruul's Lair, Magtheridon's Lair, Serpentshrine Cavern, Tempest Keep,
    // Mount Hyjal, Black Temple, Zul'Aman, Sunwell Plateau and the Karazhan bosses
    // that have no dedicated strategy).
    //
    // Every encounter describes what a real raid does on that boss:
    //  - which creatures / ground effects / objects to stay away from
    //  - how far players have to spread out (always or while affected by a debuff)
    //  - which debuffs force a player to run away from the raid
    //  - in which order adds have to die
    //  - when the tanks have to swap
    //  - special mechanics handled by dedicated actions (cubes, spines, ...)
    namespace tbcraid
    {
        enum RaidRole : uint8
        {
            ROLE_TANK     = 0x01,
            ROLE_HEAL     = 0x02,
            ROLE_MELEE    = 0x04,   // melee damage dealers
            ROLE_RANGED   = 0x08,   // ranged damage dealers
            ROLE_DPS      = ROLE_MELEE | ROLE_RANGED,
            ROLE_CASTERS  = ROLE_HEAL | ROLE_RANGED,
            ROLE_NON_TANK = ROLE_HEAL | ROLE_DPS,
            ROLE_ALL      = ROLE_TANK | ROLE_NON_TANK,
        };

        enum SpecialMechanic : uint32
        {
            SPECIAL_NONE              = 0x00,
            SPECIAL_MANTICRON_CUBES   = 0x01,   // Magtheridon: interrupt Blast Nova with the 5 cubes
            SPECIAL_NAJENTUS_SPINES   = 0x02,   // Naj'entus: free impaled players, throw spines at Tidal Shield
            SPECIAL_TAINTED_CORES     = 0x04,   // Lady Vashj: loot Tainted Cores, pass them to the shield generators
            SPECIAL_LURKER_SPOUT      = 0x08,   // The Lurker Below: jump into the water during Spout
            SPECIAL_TERON_SPIRITS     = 0x10,   // Teron Gorefiend: use the Vengeful Spirit abilities on the constructs
            SPECIAL_BLUE_DRAGON       = 0x20,   // Kil'jaeden: use the Blue Dragon orbs, Shield of the Blue on Darkness
            SPECIAL_HYDROSS_TANKING   = 0x40,   // Hydross: drag the boss in and out of the purification circle
            SPECIAL_OFFTANK_NO_TAUNT  = 0x80,   // off tanks stay second on threat (Hurtful Strike, Saber Lash)
            SPECIAL_STACK_OFFTANKS    = 0x100,  // off tanks stand on the main tank (Saber Lash)
        };

        // Keep a distance from a creature (optionally only while it has an aura / casts a spell / targets the bot)
        struct AvoidCreature
        {
            uint32 entry;
            float radius;
            uint8 roles;
            uint32 requiredAura = 0;        // the creature must have this aura
            uint32 requiredCast = 0;        // the creature must be casting this spell
            bool onlyIfTargetingMe = false; // the creature must be attacking the bot (fixate / gaze)
        };

        // Keep a distance from a game object (fire on the ground, ...)
        struct AvoidGameObject
        {
            uint32 entry;
            float radius;
            uint8 roles;
        };

        // Leave ground effects created by a spell (Death and Decay, Rain of Fire, Blizzard, ...)
        struct AvoidAreaSpell
        {
            uint32 spellId;
            float radius;                   // 0 = use the radius of the effect
            uint8 roles;
        };

        // Keep a minimum distance to all other raid members
        struct SpreadRule
        {
            float distance;
            uint8 roles;
            uint32 requiredAuraOnSelf = 0;  // only while the bot has this aura
            uint32 requiredAuraOnBoss = 0;  // only while the boss has this aura
        };

        // While the bot has this debuff it must run away from the raid
        struct DebuffRunRule
        {
            uint32 spellId;
            float distance;
            float maxManaPct = 100.0f;      // only when the mana is below this value (Mark of Kaz'rogal)
        };

        // Stack on the raid member which has this aura (Akil'zon Electrical Storm)
        struct GatherRule
        {
            uint32 spellId;
            float distance;
        };

        // Kill order. The first matching entry in the list is attacked first.
        struct PriorityTarget
        {
            uint32 entry;
            uint8 roles;
            bool onlyIfTargetingMe = false; // personal adds (Inner Demons)
            uint32 requiredAuraOnSelf = 0;  // only while the bot has this aura (Spectral Realm)
            uint32 excludedAuraOnSelf = 0;  // not while the bot has this aura
        };

        // Come closer to the boss while having enough stacks of a debuff (Bloodboil rotation)
        struct DebuffApproachRule
        {
            uint32 spellId;
            uint8 minStacks;
            float distance;
        };

        // Melee soak a frontal attack, rotate out when the debuff stacks too high (Meteor Slash)
        struct FrontSoakRule
        {
            uint32 bossEntry;
            uint32 spellId;
            uint8 maxStacks;
        };

        // The off tank taunts when the current tank reached the debuff stacks
        struct TankSwapRule
        {
            uint32 bossEntry;
            uint32 spellId;
            uint8 stacks;
        };

        // Two tanks: the first one tanks while the boss lacks the aura, the second while the boss has it
        struct FormTankRule
        {
            uint32 bossEntry;
            uint32 auraId;
        };

        // Don't stand in front of the boss (cleave, breath, smash)
        struct AvoidFrontRule
        {
            uint32 bossEntry;
            float range;
            uint8 roles;
        };

        struct Encounter
        {
            std::string name;
            uint32 mapId = 0;
            std::vector<uint32> entries;    // creatures which identify the encounter, the first one is the boss
            std::vector<AvoidCreature> avoidCreatures;
            std::vector<AvoidGameObject> avoidObjects;
            std::vector<AvoidAreaSpell> avoidAreas;
            std::vector<SpreadRule> spread;
            std::vector<DebuffRunRule> debuffRun;
            std::vector<GatherRule> gather;
            std::vector<PriorityTarget> priority;
            std::vector<TankSwapRule> tankSwap;
            std::vector<FormTankRule> formTank;
            std::vector<AvoidFrontRule> avoidFront;
            std::vector<DebuffApproachRule> debuffApproach;
            std::vector<FrontSoakRule> frontSoak;
            uint32 special = SPECIAL_NONE;

            // Builder helpers to keep the encounter table readable
            Encounter& Avoid(uint32 entry, float radius, uint8 roles = ROLE_ALL) { avoidCreatures.push_back({ entry, radius, roles }); return *this; }
            Encounter& AvoidWhileAura(uint32 entry, uint32 aura, float radius, uint8 roles) { avoidCreatures.push_back({ entry, radius, roles, aura, 0, false }); return *this; }
            Encounter& AvoidWhileCasting(uint32 entry, uint32 spell, float radius, uint8 roles) { avoidCreatures.push_back({ entry, radius, roles, 0, spell, false }); return *this; }
            Encounter& Kite(uint32 entry, float radius, uint8 roles = ROLE_NON_TANK) { avoidCreatures.push_back({ entry, radius, roles, 0, 0, true }); return *this; }
            Encounter& AvoidObject(uint32 entry, float radius, uint8 roles = ROLE_ALL) { avoidObjects.push_back({ entry, radius, roles }); return *this; }
            Encounter& AvoidArea(uint32 spellId, float radius = 0.0f, uint8 roles = ROLE_ALL) { avoidAreas.push_back({ spellId, radius, roles }); return *this; }
            Encounter& Spread(float distance, uint8 roles) { spread.push_back({ distance, roles }); return *this; }
            Encounter& SpreadWithAura(float distance, uint8 roles, uint32 auraOnSelf) { spread.push_back({ distance, roles, auraOnSelf, 0 }); return *this; }
            Encounter& SpreadWhileBossAura(float distance, uint8 roles, uint32 auraOnBoss) { spread.push_back({ distance, roles, 0, auraOnBoss }); return *this; }
            Encounter& RunWithDebuff(uint32 spellId, float distance, float maxManaPct = 100.0f) { debuffRun.push_back({ spellId, distance, maxManaPct }); return *this; }
            Encounter& GatherOn(uint32 spellId, float distance) { gather.push_back({ spellId, distance }); return *this; }
            Encounter& Kill(uint32 entry, uint8 roles = ROLE_DPS) { priority.push_back({ entry, roles, false }); return *this; }
            Encounter& KillMine(uint32 entry) { priority.push_back({ entry, ROLE_ALL, true }); return *this; }
            Encounter& KillWithAura(uint32 entry, uint8 roles, uint32 auraOnSelf) { priority.push_back({ entry, roles, false, auraOnSelf, 0 }); return *this; }
            Encounter& KillWithoutAura(uint32 entry, uint8 roles, uint32 auraOnSelf) { priority.push_back({ entry, roles, false, 0, auraOnSelf }); return *this; }
            Encounter& ApproachWithDebuff(uint32 spellId, uint8 minStacks, float distance) { debuffApproach.push_back({ spellId, minStacks, distance }); return *this; }
            Encounter& SoakFront(uint32 bossEntry, uint32 spellId, uint8 maxStacks) { frontSoak.push_back({ bossEntry, spellId, maxStacks }); return *this; }
            Encounter& TankSwap(uint32 bossEntry, uint32 spellId, uint8 stacks) { tankSwap.push_back({ bossEntry, spellId, stacks }); return *this; }
            Encounter& FormTank(uint32 bossEntry, uint32 auraId) { formTank.push_back({ bossEntry, auraId }); return *this; }
            Encounter& AvoidFront(uint32 bossEntry, float range = 15.0f, uint8 roles = ROLE_NON_TANK) { avoidFront.push_back({ bossEntry, range, roles }); return *this; }
            Encounter& Special(uint32 mechanic) { special |= mechanic; return *this; }
        };

        class EncounterRegistry
        {
        public:
            static EncounterRegistry& Instance();

            const Encounter* FindByCreature(uint32 mapId, uint32 entry) const;
            bool IsRaidMap(uint32 mapId) const;

        private:
            EncounterRegistry();
            Encounter& Add(const std::string& name, uint32 mapId, const std::vector<uint32>& entries);

            std::vector<Encounter*> encounters;
            std::unordered_map<uint32, std::vector<const Encounter*>> byCreature;
            std::vector<uint32> raidMaps;
        };

        // Role of a raid member
        uint8 GetRole(PlayerbotAI* ai, Player* player);
        inline bool HasRole(uint8 roles, uint8 role) { return (roles & role) != 0; }
    }
}
