#include "playerbot/playerbot.h"
#include "playerbot/BotPerformanceLog.h"
#include "playerbot/AiFactory.h"
#include "playerbot/PlayerbotAIConfig.h"
#include <filesystem>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <unordered_map>

std::atomic<bool> botperf::activeFights(false);

namespace
{
    bool enabled = false;
    uint32 minFightSeconds = 15;
    bool instancesOnly = true;
    bool includeMaster = true;

    // A group counts as out of combat after this long without any member in combat
    const uint32 FIGHT_END_GRACE_MS = 3000;
    // Pet and totem damage is credited to the owner under its own key range
    const uint32 PET_KEY = 0x80000000;
    const uint32 MELEE_KEY = 0;
    // Instant casts keep the bot busy for the global cooldown
    const uint32 GCD_MS = 1500;

    struct SpellStat
    {
        uint32 casts = 0, hits = 0, misses = 0;
        uint64 damage = 0, heal = 0, overheal = 0;
    };

    struct Member
    {
        std::string name, spec, role;
        uint8 cls = 0;
        bool bot = true;
        std::string stats;
        uint64 damage = 0, heal = 0, overheal = 0;
        std::map<uint32, uint64> damageByEntry;
        std::map<uint32, SpellStat> spells;
        uint32 swings = 0, swingMiss = 0, swingDodge = 0, swingParry = 0, swingGlance = 0, swingCrit = 0;
        uint32 spellHits = 0, spellMiss = 0, spellResist = 0, spellDodgeParry = 0, spellImmune = 0;
        uint32 combatMs = 0, castMs = 0, channelMs = 0, autoMs = 0, gcdMs = 0, moveOnlyMs = 0, idleMs = 0, deadMs = 0;
        uint32 movingMs = 0, outOfRangeMs = 0, noTargetMs = 0, lowManaMs = 0;
        uint32 lastSampleMs = 0, lastCastMs = 0, firstCombatMs = 0, lastCombatMs = 0, deathMs = 0;
        uint32 deaths = 0;
        bool dead = false;
        float minManaPct = 100.0f;
        bool usesMana = false;
    };

    struct Victim
    {
        std::string name;
        uint64 damage = 0;
        bool boss = false;
        ObjectGuid guid;
    };

    struct Fight
    {
        uint32 mapId = 0, instanceId = 0;
        uint32 startMs = 0, lastCombatMs = 0;
        time_t startTime = 0;
        std::map<ObjectGuid, Member> members;
        std::map<uint32, Victim> victims;
    };

    std::mutex mutex;
    std::map<uint32, Fight> fights;                     // by group id
    std::unordered_map<uint64, uint32> fightOfPlayer;   // player guid -> group id
    uint32 fightCounter = 0;

    const char* SPEC_NAMES[12][3] = {
        { "?", "?", "?" },
        { "Arms", "Fury", "Protection" },
        { "Holy", "Protection", "Retribution" },
        { "BeastMastery", "Marksmanship", "Survival" },
        { "Assassination", "Combat", "Subtlety" },
        { "Discipline", "Holy", "Shadow" },
        { "?", "?", "?" },
        { "Elemental", "Enhancement", "Restoration" },
        { "Arcane", "Fire", "Frost" },
        { "Affliction", "Demonology", "Destruction" },
        { "?", "?", "?" },
        { "Balance", "Feral", "Restoration" },
    };

    const char* CLASS_NAMES[12] = { "?", "Warrior", "Paladin", "Hunter", "Rogue", "Priest", "?", "Shaman", "Mage", "Warlock", "?", "Druid" };

    Member* FindMember(Player* player)
    {
        auto found = fightOfPlayer.find(player->GetObjectGuid().GetRawValue());
        if (found == fightOfPlayer.end())
            return nullptr;
        auto fight = fights.find(found->second);
        if (fight == fights.end())
            return nullptr;
        auto member = fight->second.members.find(player->GetObjectGuid());
        return member == fight->second.members.end() ? nullptr : &member->second;
    }

    std::string DescribeStats(Player* player, PlayerbotAI* ai)
    {
        uint32 items = 0, itemLevels = 0, emptySlots = 0;
        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
                continue;
            Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (item && item->GetProto())
            {
                ++items;
                itemLevels += item->GetProto()->ItemLevel;
            }
            else if (slot != EQUIPMENT_SLOT_OFFHAND && slot != EQUIPMENT_SLOT_RANGED)
                ++emptySlots;
        }

        int32 spellPower = 0;
        float spellCrit = 0.0f;
        for (uint8 school = SPELL_SCHOOL_HOLY; school < MAX_SPELL_SCHOOL; ++school)
        {
            spellPower = std::max(spellPower, player->SpellBaseDamageBonusDone(SpellSchoolMask(1 << school)));
            spellCrit = std::max(spellCrit, player->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + school));
        }

        std::ostringstream out;
        out << std::fixed << std::setprecision(1)
            << "ilvl=" << (items ? float(itemLevels) / items : 0.0f) << " emptySlots=" << emptySlots
            << " gs=" << ai->GetEquipGearScore(player, false, false)
            << " AP=" << uint32(player->GetTotalAttackPowerValue(BASE_ATTACK))
            << " RAP=" << uint32(player->GetTotalAttackPowerValue(RANGED_ATTACK))
            << " SP=" << spellPower << " heal=" << player->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL)
            << " hit=" << player->m_modWeaponHitChance[BASE_ATTACK] << "%"
            << " rangedHit=" << player->m_modWeaponHitChance[RANGED_ATTACK] << "%"
            << " spellHit=" << player->m_modSpellHitChance << "%"
            << " crit=" << player->GetFloatValue(PLAYER_CRIT_PERCENTAGE) << "%"
            << " rangedCrit=" << player->GetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE) << "%"
            << " spellCrit=" << spellCrit << "%"
            << " expertise=" << player->GetExpertisePercent(BASE_ATTACK) << "%"
            << " meleeHaste=" << player->GetRatingBonusValue(CR_HASTE_MELEE) << "%"
            << " spellHaste=" << player->GetRatingBonusValue(CR_HASTE_SPELL) << "%"
            << " maxMana=" << player->GetMaxPower(POWER_MANA);
        return out.str();
    }

    void InitMember(Member& member, Player* player, PlayerbotAI* ai)
    {
        member.name = player->GetName();
        member.cls = player->getClass();
        member.bot = player->GetPlayerbotAI() != nullptr;
        const int tab = AiFactory::GetPlayerSpecTab(player);
        member.spec = member.cls < 12 && tab >= 0 && tab < 3 ? SPEC_NAMES[member.cls][tab] : "?";
        member.role = PlayerbotAI::IsTank(player) ? "tank" : PlayerbotAI::IsHeal(player) ? "heal" : ai->IsRanged(player) ? "ranged" : "melee";
        member.usesMana = player->GetPowerType() == POWER_MANA;
        member.stats = DescribeStats(player, ai);
    }

    void Sample(Member& member, Player* player, PlayerbotAI* ai, uint32 now)
    {
        const uint32 dt = member.lastSampleMs ? std::min<uint32>(WorldTimer::getMSTimeDiff(member.lastSampleMs, now), 2000) : 0;
        member.lastSampleMs = now;

        // A dead player leaves combat, so the death is counted before the combat check
        if (!player->IsAlive())
        {
            if (!member.firstCombatMs)
                return;
            if (!member.dead)
            {
                member.dead = true;
                ++member.deaths;
                if (!member.deathMs)
                    member.deathMs = now;
            }
            member.combatMs += dt;
            member.deadMs += dt;
            return;
        }
        member.dead = false;

        if (!player->IsInCombat())
            return;

        if (!member.firstCombatMs)
            member.firstCombatMs = now;
        member.lastCombatMs = now;
        member.combatMs += dt;

        Unit* target = player->GetVictim();
        if (!target && ai)
            target = ai->GetUnit(ai->GetAiObjectContext()->GetValue<ObjectGuid>("current target")->Get());
        if (!target && !player->GetPlayerbotAI())
            target = player->GetMap()->GetUnit(player->GetSelectionGuid());
        if (target && (!target->IsAlive() || !player->CanAttack(target)))
            target = nullptr;

        const bool moving = player->IsMoving();
        const bool melee = member.role == "melee" || member.role == "tank";
        bool inRange = true;
        if (target)
            inRange = melee ? player->CanReachWithMeleeAttack(target) : player->GetDistance(target, true, DIST_CALC_COMBAT_REACH) <= 30.0f;

        if (moving)
            member.movingMs += dt;
        if (target && !inRange)
            member.outOfRangeMs += dt;
        if (!target && member.role != "heal")
            member.noTargetMs += dt;
        if (member.usesMana)
        {
            const float mana = player->GetPowerPercent(POWER_MANA);
            member.minManaPct = std::min(member.minManaPct, mana);
            if (mana < 10.0f)
                member.lowManaMs += dt;
        }

        // Exactly one activity bucket per sample; casting wins over swinging
        if (player->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            member.castMs += dt;
        else if (player->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
            member.channelMs += dt;
        else if (player->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL) ||
                 (player->hasUnitState(UNIT_STAT_MELEE_ATTACKING) && player->GetVictim() && player->CanReachWithMeleeAttack(player->GetVictim())))
            member.autoMs += dt;
        else if (member.lastCastMs && WorldTimer::getMSTimeDiff(member.lastCastMs, now) < GCD_MS)
            member.gcdMs += dt;
        else if (moving)
            member.moveOnlyMs += dt;
        else
            member.idleMs += dt;
    }

    std::string SpellName(uint32 key)
    {
        const bool pet = key & PET_KEY;
        const uint32 id = key & ~PET_KEY;
        std::string name;
        if (id == MELEE_KEY)
            name = "Melee";
        else if (SpellEntry const* spell = sSpellTemplate.LookupEntry<SpellEntry>(id))
            name = std::string(spell->SpellName[0]) + "(" + std::to_string(id) + ")";
        else
            name = "Spell(" + std::to_string(id) + ")";
        return pet ? "Pet:" + name : name;
    }

    std::string Percent(uint32 part, uint32 total)
    {
        std::ostringstream out;
        out << std::fixed << std::setprecision(0) << (total ? 100.0f * part / total : 0.0f) << "%";
        return out.str();
    }

    std::vector<std::string> Hints(const Member& member, uint32 durationMs)
    {
        std::vector<std::string> hints;
        const uint32 combat = std::max<uint32>(member.combatMs, 1);
        const uint32 alive = std::max<uint32>(combat - member.deadMs, 1);
        if (member.deathMs)
            hints.push_back(std::to_string(member.deaths) + "x tot, zuerst nach " +
                std::to_string(WorldTimer::getMSTimeDiff(member.firstCombatMs, member.deathMs) / 1000) + "s");
        if (member.combatMs + 5000 < durationMs)
            hints.push_back("nur " + Percent(member.combatMs, durationMs) + " des Kampfes im Kampf");
        if (100 * member.idleMs > 20 * alive)
            hints.push_back("Leerlauf " + Percent(member.idleMs, alive));
        if (100 * member.movingMs > 25 * alive)
            hints.push_back("Bewegung " + Percent(member.movingMs, alive));
        if (100 * member.outOfRangeMs > 15 * alive)
            hints.push_back("ausser Reichweite " + Percent(member.outOfRangeMs, alive));
        if (100 * member.noTargetMs > 10 * alive)
            hints.push_back("ohne Ziel " + Percent(member.noTargetMs, alive));
        if (member.usesMana && 100 * member.lowManaMs > 15 * alive)
            hints.push_back("OOM (<10% Mana) " + Percent(member.lowManaMs, alive));
        if (member.swings >= 20 && 100 * member.swingMiss > 10 * member.swings)
            hints.push_back("Melee-Miss " + Percent(member.swingMiss, member.swings));
        if (member.swings >= 20 && 100 * (member.swingDodge + member.swingParry) > 8 * member.swings)
            hints.push_back("Dodge/Parry " + Percent(member.swingDodge + member.swingParry, member.swings) + " (Expertise/von hinten?)");
        const uint32 spellAttempts = member.spellHits + member.spellMiss + member.spellResist + member.spellDodgeParry + member.spellImmune;
        if (spellAttempts >= 10 && 100 * (member.spellMiss + member.spellResist) > 8 * spellAttempts)
            hints.push_back("Spell-Miss/Resist " + Percent(member.spellMiss + member.spellResist, spellAttempts));
        if (spellAttempts >= 10 && 100 * member.spellImmune > 5 * spellAttempts)
            hints.push_back("Immun " + Percent(member.spellImmune, spellAttempts));
        return hints;
    }

    void WriteReport(uint32 groupId, Fight& fight, Map* map)
    {
        uint64 totalDamage = 0, totalHeal = 0;
        uint32 totalDeaths = 0;
        for (auto& entry : fight.members)
        {
            totalDamage += entry.second.damage;
            totalHeal += entry.second.heal;
            totalDeaths += entry.second.deaths;
        }
        const uint32 durationMs = WorldTimer::getMSTimeDiff(fight.startMs, fight.lastCombatMs);
        if (durationMs < minFightSeconds * 1000 || (!totalDamage && !totalHeal))
            return;
        const float seconds = durationMs / 1000.0f;

        // Encounter = most damaged boss, otherwise the most damaged creature
        const Victim* encounter = nullptr;
        uint32 encounterEntry = 0;
        for (auto& entry : fight.victims)
        {
            const Victim& victim = entry.second;
            if (!encounter || (victim.boss && !encounter->boss) || (victim.boss == encounter->boss && victim.damage > encounter->damage))
            {
                encounter = &victim;
                encounterEntry = entry.first;
            }
        }
        std::string result = "END";
        if (encounter && map)
        {
            Creature* creature = map->GetCreature(encounter->guid);
            if (!creature || !creature->IsAlive())
                result = "KILL";
            else
                result = "ALIVE(" + std::to_string(uint32(creature->GetHealthPercent())) + "%)";
        }

        std::ostringstream out;
        out << std::fixed << std::setprecision(1);
        char date[32];
        strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", localtime(&fight.startTime));
        out << "==== Fight #" << ++fightCounter << " " << date << " group=" << groupId << " map=" << fight.mapId
            << " instance=" << fight.instanceId << " encounter=\"" << (encounter ? encounter->name : "?") << "\" entry=" << encounterEntry
            << (encounter && encounter->boss ? " boss" : "") << " result=" << result << " duration=" << seconds << "s"
            << " raidDamage=" << totalDamage << " raidDps=" << totalDamage / seconds
            << " raidHeal=" << totalHeal << " raidHps=" << totalHeal / seconds << " deaths=" << totalDeaths << "\n";

        std::vector<std::pair<ObjectGuid, Member*>> sorted;
        for (auto& entry : fight.members)
            sorted.emplace_back(entry.first, &entry.second);
        std::sort(sorted.begin(), sorted.end(), [](auto& a, auto& b)
        {
            return a.second->damage != b.second->damage ? a.second->damage > b.second->damage : a.second->heal > b.second->heal;
        });

        out << "  " << std::left << std::setw(13) << "Name" << std::setw(22) << "Class/Spec" << std::setw(7) << "Role"
            << std::right << std::setw(9) << "DPS" << std::setw(7) << "Dmg%" << std::setw(9) << "BossDPS" << std::setw(8) << "HPS"
            << std::setw(7) << "OH%" << std::setw(7) << "Cast" << std::setw(7) << "Chan" << std::setw(7) << "Auto"
            << std::setw(7) << "GCD" << std::setw(7) << "Idle" << std::setw(7) << "MvOnly" << std::setw(7) << "Moving"
            << std::setw(7) << "OOR" << std::setw(7) << "NoTgt" << std::setw(7) << "Dead" << std::setw(7) << "CPM" << "\n";
        for (auto& entry : sorted)
        {
            const Member& m = *entry.second;
            const uint32 alive = std::max<uint32>(m.combatMs - m.deadMs, 1);
            uint32 casts = 0;
            for (auto& spell : m.spells)
            {
                SpellEntry const* info = sSpellTemplate.LookupEntry<SpellEntry>(spell.first);
                if (!(spell.first & PET_KEY) && !(info && IsAutoRepeatRangedSpell(info)))
                    casts += spell.second.casts;
            }
            const uint64 bossDamage = encounter && m.damageByEntry.count(encounterEntry) ? m.damageByEntry.at(encounterEntry) : 0;
            const std::string name = m.bot ? m.name : m.name + "*";
            out << "  " << std::left << std::setw(13) << name << std::setw(22) << (std::string(CLASS_NAMES[m.cls < 12 ? m.cls : 0]) + "/" + m.spec)
                << std::setw(7) << m.role << std::right
                << std::setw(9) << m.damage / seconds << std::setw(7) << Percent(uint32(m.damage / 100), uint32(totalDamage / 100))
                << std::setw(9) << bossDamage / seconds << std::setw(8) << m.heal / seconds
                << std::setw(7) << Percent(uint32(m.overheal / 100), uint32((m.heal + m.overheal) / 100))
                << std::setw(7) << Percent(m.castMs, alive) << std::setw(7) << Percent(m.channelMs, alive)
                << std::setw(7) << Percent(m.autoMs, alive) << std::setw(7) << Percent(m.gcdMs, alive)
                << std::setw(7) << Percent(m.idleMs, alive) << std::setw(7) << Percent(m.moveOnlyMs, alive)
                << std::setw(7) << Percent(m.movingMs, alive) << std::setw(7) << Percent(m.outOfRangeMs, alive)
                << std::setw(7) << Percent(m.noTargetMs, alive) << std::setw(7) << Percent(m.deadMs, std::max<uint32>(m.combatMs, 1))
                << std::setw(7) << casts * 60000.0f / alive << "\n";
        }

        out << "  -- Details (Zeiten relativ zur lebendigen Kampfzeit; * = Spieler)\n";
        for (auto& entry : sorted)
        {
            const Member& m = *entry.second;
            out << "  " << m.name << ": combat=" << m.combatMs / 1000.0f << "s dmg=" << m.damage << " heal=" << m.heal
                << " overheal=" << m.overheal << " " << m.stats;
            if (m.usesMana)
                out << " minMana=" << m.minManaPct << "% lowMana=" << m.lowManaMs / 1000.0f << "s";
            out << "\n    melee: swings=" << m.swings << " miss=" << Percent(m.swingMiss, m.swings)
                << " dodge=" << Percent(m.swingDodge, m.swings) << " parry=" << Percent(m.swingParry, m.swings)
                << " glance=" << Percent(m.swingGlance, m.swings) << " crit=" << Percent(m.swingCrit, m.swings);
            const uint32 attempts = m.spellHits + m.spellMiss + m.spellResist + m.spellDodgeParry + m.spellImmune;
            out << " | spells: hits=" << m.spellHits << " miss=" << Percent(m.spellMiss, attempts)
                << " resist=" << Percent(m.spellResist, attempts) << " dodge/parry=" << Percent(m.spellDodgeParry, attempts)
                << " immune=" << Percent(m.spellImmune, attempts) << "\n";

            std::vector<std::pair<uint32, const SpellStat*>> spells;
            for (auto& spell : m.spells)
                spells.emplace_back(spell.first, &spell.second);
            std::sort(spells.begin(), spells.end(), [](auto& a, auto& b)
            {
                return a.second->damage + a.second->heal != b.second->damage + b.second->heal ?
                    a.second->damage + a.second->heal > b.second->damage + b.second->heal : a.second->casts > b.second->casts;
            });
            for (auto& spell : spells)
            {
                const SpellStat& s = *spell.second;
                out << "    " << std::left << std::setw(36) << SpellName(spell.first) << std::right
                    << " casts=" << std::setw(4) << s.casts << " hits=" << std::setw(4) << s.hits << " miss=" << std::setw(3) << s.misses
                    << " dmg=" << std::setw(8) << s.damage << " (" << Percent(uint32(s.damage / 100), uint32(std::max<uint64>(m.damage, 1) / 100)) << ")"
                    << " heal=" << s.heal << " overheal=" << s.overheal << "\n";
            }
            const std::vector<std::string> hints = Hints(m, durationMs);
            if (!hints.empty())
            {
                out << "    Hinweise:";
                for (auto& hint : hints)
                    out << " [" << hint << "]";
                out << "\n";
            }
        }
        out << "\n";

        static FILE* file = nullptr;
        if (!file)
        {
            const std::string configured = sConfig.GetStringDefault("LogsDir");
            const std::filesystem::path directory = configured.empty() ? "." : configured;
            const auto path = directory / ("bot-dps-" + std::to_string(time(nullptr)) + ".log");
            file = fopen(path.string().c_str(), "a");
            if (!file)
                return;
            sLog.outString("[BotPerf] Bot performance log: %s", path.string().c_str());
        }
        const std::string text = out.str();
        fwrite(text.data(), 1, text.size(), file);
        fflush(file);
        sLog.outString("[BotPerf] fight written: encounter=\"%s\" result=%s duration=%.1fs raidDps=%.0f",
            encounter ? encounter->name.c_str() : "?", result.c_str(), seconds, totalDamage / seconds);
    }

    Player* TrackedPlayer(Unit* unit)
    {
        Player* player = unit ? unit->GetBeneficiaryPlayer() : nullptr;
        if (!player || (instancesOnly && !player->GetMap()->IsDungeon()))
            return nullptr;
        return player;
    }
}

void botperf::Configure(bool enable, uint32 minSeconds, bool instances, bool master)
{
    std::lock_guard<std::mutex> guard(mutex);
    enabled = enable;
    minFightSeconds = minSeconds;
    instancesOnly = instances;
    includeMaster = master;
    if (!enabled)
    {
        fights.clear();
        fightOfPlayer.clear();
        activeFights = false;
    }
}

void botperf::Update(PlayerbotAI* ai)
{
    if (!enabled)
        return;
    Player* bot = ai->GetBot();
    if (!bot->IsInWorld() || bot->IsBeingTeleported())
        return;
    Player* master = ai->GetMaster();
    Group* group = bot->GetGroup();
    // Only human-led groups: random bots everywhere would flood the log
    if (!group || !master || master->GetPlayerbotAI() || master->GetGroup() != group)
        return;
    if (instancesOnly && !bot->GetMap()->IsDungeon())
        return;

    const uint32 now = WorldTimer::getMSTime();
    const uint32 groupId = group->GetId();
    std::lock_guard<std::mutex> guard(mutex);
    auto found = fights.find(groupId);
    if (found == fights.end())
    {
        if (!bot->IsInCombat())
            return;
        Fight& fight = fights[groupId];
        fight.mapId = bot->GetMapId();
        fight.instanceId = bot->GetInstanceId();
        fight.startMs = now;
        fight.lastCombatMs = now;
        fight.startTime = time(nullptr);
        found = fights.find(groupId);
        activeFights = true;
    }
    Fight& fight = found->second;

    auto track = [&](Player* player)
    {
        auto member = fight.members.find(player->GetObjectGuid());
        if (member == fight.members.end())
        {
            if (!player->IsInCombat())
                return;
            member = fight.members.emplace(player->GetObjectGuid(), Member()).first;
            InitMember(member->second, player, ai);
            member->second.lastSampleMs = now;
            fightOfPlayer[player->GetObjectGuid().GetRawValue()] = groupId;
        }
        Sample(member->second, player, player->GetPlayerbotAI(), now);
        if (player->IsInCombat())
            fight.lastCombatMs = now;
    };
    if (bot->GetMapId() == fight.mapId && bot->GetInstanceId() == fight.instanceId)
        track(bot);
    if (includeMaster && master->IsInWorld() && master->GetMapId() == fight.mapId && master->GetInstanceId() == fight.instanceId)
        track(master);

    if (WorldTimer::getMSTimeDiff(fight.lastCombatMs, now) < FIGHT_END_GRACE_MS)
        return;

    WriteReport(groupId, fight, bot->GetMapId() == fight.mapId ? bot->GetMap() : nullptr);
    for (auto& member : fight.members)
        fightOfPlayer.erase(member.first.GetRawValue());
    fights.erase(found);
    activeFights = !fights.empty();
}

void botperf::RecordDamage(Unit* dealer, Unit* victim, uint32 damage, SpellEntry const* spell, bool periodic)
{
    // Only damage to hostile creatures counts; self damage and PvP are noise here
    if (!victim || victim->GetTypeId() != TYPEID_UNIT || victim->IsPlayerControlled())
        return;
    Player* player = TrackedPlayer(dealer);
    if (!player)
        return;
    damage = std::min(damage, victim->GetHealth());

    std::lock_guard<std::mutex> guard(mutex);
    Member* member = FindMember(player);
    if (!member)
        return;
    member->damage += damage;
    member->damageByEntry[victim->GetEntry()] += damage;
    uint32 key = spell ? spell->Id : MELEE_KEY;
    if (dealer != player)
        key |= PET_KEY;
    SpellStat& stat = member->spells[key];
    stat.damage += damage;
    if (periodic || dealer != player)
        ++stat.hits;

    Fight& fight = fights[fightOfPlayer[player->GetObjectGuid().GetRawValue()]];
    Victim& target = fight.victims[victim->GetEntry()];
    if (target.name.empty())
    {
        Creature* creature = static_cast<Creature*>(victim);
        target.name = creature->GetName();
        target.boss = creature->IsWorldBoss();
    }
    target.guid = victim->GetObjectGuid();
    target.damage += damage;
}

void botperf::RecordHeal(Unit* healer, Unit* /*victim*/, uint32 amount, int32 gain, SpellEntry const* spell)
{
    Player* player = TrackedPlayer(healer);
    if (!player)
        return;
    const uint32 effective = gain > 0 ? uint32(gain) : 0;
    const uint32 overheal = amount > effective ? amount - effective : 0;

    std::lock_guard<std::mutex> guard(mutex);
    Member* member = FindMember(player);
    if (!member)
        return;
    member->heal += effective;
    member->overheal += overheal;
    uint32 key = spell ? spell->Id : MELEE_KEY;
    if (healer != player)
        key |= PET_KEY;
    SpellStat& stat = member->spells[key];
    stat.heal += effective;
    stat.overheal += overheal;
}

void botperf::RecordMelee(Unit* attacker, uint32 outcome)
{
    Player* player = TrackedPlayer(attacker);
    if (!player)
        return;
    std::lock_guard<std::mutex> guard(mutex);
    Member* member = FindMember(player);
    if (!member)
        return;
    if (attacker != player)
    {
        ++member->spells[MELEE_KEY | PET_KEY].hits;
        return;
    }
    ++member->swings;
    SpellStat& stat = member->spells[MELEE_KEY];
    switch (outcome)
    {
        case MELEE_HIT_MISS: ++member->swingMiss; ++stat.misses; break;
        case MELEE_HIT_DODGE: ++member->swingDodge; ++stat.misses; break;
        case MELEE_HIT_PARRY: ++member->swingParry; ++stat.misses; break;
        case MELEE_HIT_GLANCING: ++member->swingGlance; ++stat.hits; break;
        case MELEE_HIT_CRIT: ++member->swingCrit; ++stat.hits; break;
        default: ++stat.hits; break;
    }
}

void botperf::RecordSpellResult(Unit* caster, Unit* target, SpellEntry const* spell, uint32 missInfo)
{
    if (!spell || !target || target->GetTypeId() != TYPEID_UNIT || target->IsPlayerControlled())
        return;
    Player* player = TrackedPlayer(caster);
    if (!player || caster != player)
        return;
    std::lock_guard<std::mutex> guard(mutex);
    Member* member = FindMember(player);
    if (!member)
        return;
    SpellStat& stat = member->spells[spell->Id];
    switch (missInfo)
    {
        case SPELL_MISS_NONE: ++member->spellHits; ++stat.hits; return;
        case SPELL_MISS_EVADE: return;
        case SPELL_MISS_MISS: ++member->spellMiss; break;
        case SPELL_MISS_RESIST: ++member->spellResist; break;
        case SPELL_MISS_DODGE: case SPELL_MISS_PARRY: case SPELL_MISS_BLOCK: case SPELL_MISS_DEFLECT: ++member->spellDodgeParry; break;
        case SPELL_MISS_IMMUNE: case SPELL_MISS_IMMUNE2: ++member->spellImmune; break;
        default: break;
    }
    ++stat.misses;
}

void botperf::RecordCast(Unit* caster, SpellEntry const* spell)
{
    if (!spell || caster->GetTypeId() != TYPEID_PLAYER)
        return;
    // Proc side effects (Arcane Blast debuff, Vampiric Touch mana) arrive as untriggered casts;
    // real casts cost power, take cast time or start a cooldown
    if (!spell->StartRecoveryTime && !spell->RecoveryTime && !spell->CategoryRecoveryTime && !spell->manaCost &&
        !spell->ManaCostPercentage && spell->CastingTimeIndex <= 1 && !IsAutoRepeatRangedSpell(spell))
        return;
    Player* player = TrackedPlayer(caster);
    if (!player)
        return;
    // Spells outside the spellbook without a cooldown are side effects too (Arcane Blast debuff 36032)
    if (!player->HasSpell(spell->Id) && !spell->RecoveryTime && !spell->CategoryRecoveryTime)
        return;
    std::lock_guard<std::mutex> guard(mutex);
    Member* member = FindMember(player);
    if (!member)
        return;
    ++member->spells[spell->Id].casts;
    if (!IsAutoRepeatRangedSpell(spell))
        member->lastCastMs = WorldTimer::getMSTime();
}
