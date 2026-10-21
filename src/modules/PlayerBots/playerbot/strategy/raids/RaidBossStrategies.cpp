#include "playerbot/playerbot.h"
#include "RaidBossStrategies.h"
#include "RaidBossHelpers.h"
#include "gruul/GruulHelpers.h"
#include "dungeons/DungeonBossStrategy.h"
#include <sstream>
#include <filesystem>
#include <mutex>

using namespace ai;

const std::vector<std::pair<uint32, std::string>>& RaidBossMapStrategies::GetAll()
{
    static const std::vector<std::pair<uint32, std::string>> strategies = []
    {
        std::vector<std::pair<uint32, std::string>> entries = {
        // Burning Crusade raids
        { 532, "boss karazhan" },
        { 565, "boss gruul" },
        { 544, "boss magtheridon" },
        { 548, "boss ssc" },
        { 550, "boss tempest keep" },
        { 534, "boss hyjal" },
        { 564, "boss black temple" },
        { 568, "boss zul'aman" },

        };
        const auto& classicRaids = GetClassicRaidBossMaps();
        entries.insert(entries.begin(), classicRaids.begin(), classicRaids.end());
        const auto& dungeons = GetDungeonBossMaps();
        entries.insert(entries.end(), dungeons.begin(), dungeons.end());
        return entries;
    }();

    return strategies;
}

std::string RaidBossMapStrategies::GetStrategy(uint32 mapId)
{
    for (const auto& entry : GetAll())
    {
        if (entry.first == mapId)
            return entry.second;
    }

    return "";
}

bool RaidBossMapStrategyTrigger::IsActive()
{
    if (!bot->IsInWorld() || bot->IsBeingTeleported())
        return false;

    const std::string wanted = RaidBossMapStrategies::GetStrategy(bot->GetMapId());
    for (const auto& entry : RaidBossMapStrategies::GetAll())
    {
        // Only strategies which exist (a raid can be listed before its strategy is ported)
        if (!ai->GetAiObjectContext()->GetStrategy(entry.second))
            continue;

        for (uint8 state = 0; state < uint8(BotState::BOT_STATE_ALL); ++state)
        {
            if (ai->HasStrategy(entry.second, BotState(state)) != (entry.second == wanted))
                return true;
        }
    }

    return false;
}

bool UpdateRaidBossMapStrategyAction::Execute(Event& event)
{
    const std::string wanted = RaidBossMapStrategies::GetStrategy(bot->GetMapId());

    bool changed = false;
    for (const auto& entry : RaidBossMapStrategies::GetAll())
    {
        if (!ai->GetAiObjectContext()->GetStrategy(entry.second))
            continue;

        const bool enable = entry.second == wanted;
        bool strategyChanged = false;
        for (uint8 state = 0; state < uint8(BotState::BOT_STATE_ALL); ++state)
        {
            if (ai->HasStrategy(entry.second, BotState(state)) != enable)
            {
                ai->ChangeStrategy((enable ? "+" : "-") + entry.second, BotState(state));
                strategyChanged = true;
            }
        }

        if (strategyChanged)
        {
            sLog.outString("[RaidTactics] bot=%s map=%u instance=%u strategy=\"%s\" %s combat=%u noncombat=%u reaction=%u",
                bot->GetName(), bot->GetMapId(), bot->GetInstanceId(), entry.second.c_str(), enable ? "ENABLED" : "DISABLED",
                ai->HasStrategy(entry.second, BotState::BOT_STATE_COMBAT),
                ai->HasStrategy(entry.second, BotState::BOT_STATE_NON_COMBAT),
                ai->HasStrategy(entry.second, BotState::BOT_STATE_REACTION));
            if (enable && ai->GetMaster() && !ai->GetMaster()->GetPlayerbotAI())
                ai->TellPlayerNoFacing(ai->GetMaster(), "[RaidTactics] Aktiv: " + entry.second + ". Status: debug raid",
                    PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
            changed = true;
        }
    }

    return changed;
}

void RaidBossMapStrategies::Trace(PlayerbotAI* ai, BotState state, const std::string& source,
                                 const std::string& action, bool executed)
{
    auto* debug = dynamic_cast<RaidBossDebugValue*>(ai->GetAiObjectContext()->GetUntypedValue("raid boss debug"));
    if (!debug || !debug->Get())
        return;

    const std::string strategy = GetStrategy(ai->GetBot()->GetMapId());
    if (strategy.empty() || !ai->HasStrategy(strategy, state))
        return;
    const std::string strategyKey = strategy + std::to_string(uint8(state));
    auto found = debug->strategyTriggers.find(strategyKey);
    if (found == debug->strategyTriggers.end())
    {
        std::set<std::string> names;
        std::list<TriggerNode*> nodes;
        ai->GetAiObjectContext()->GetStrategy(strategy)->InitTriggers(nodes, state);
        for (TriggerNode* node : nodes)
        {
            names.insert(node->getName());
            delete node;
        }
        found = debug->strategyTriggers.emplace(strategyKey, std::move(names)).first;
    }
    if (!found->second.count(source))
        return;

    // Keep failed attempts visible too. Repeated position checks must not flood the log.
    const std::string key = std::to_string(uint8(state)) + action + (executed ? ":ok" : ":failed");
    const time_t now = time(nullptr);
    time_t& last = debug->lastLog[key];
    if (now - last < 5)
        return;
    last = now;

    Player* bot = ai->GetBot();
    sLog.outString("[RaidTactics] bot=%s map=%u instance=%u strategy=\"%s\" state=%u trigger=\"%s\" action=\"%s\" result=%s",
        bot->GetName(), bot->GetMapId(), bot->GetInstanceId(), strategy.c_str(), uint8(state), source.c_str(),
        action.c_str(), executed ? "EXECUTED" : "FAILED");
}

namespace
{
    void WriteDecisionLog(const std::string& line)
    {
        static std::mutex mutex;
        std::lock_guard<std::mutex> guard(mutex);
        static FILE* file = nullptr;
        static size_t bytes = 0;
        static unsigned segment = 0;
        static const std::string run = std::to_string(time(nullptr)) + "-" + std::to_string(WorldTimer::getMSTime());
        static time_t checked = 0;
        static bool room = true;
        const time_t now = time(nullptr);
        const std::string configured = sConfig.GetStringDefault("LogsDir");
        const std::filesystem::path directory = configured.empty() ? "." : configured;
        if (now - checked >= 10)
        {
            std::error_code error;
            const auto space = std::filesystem::space(directory, error);
            room = error || space.available > 2ULL * 1024 * 1024 * 1024;
            if (!room)
                sLog.outError("[BotDecision] Detailed logging paused: less than 2 GiB disk space available.");
            checked = now;
        }
        if (!room)
            return;
        if (file && bytes >= 64 * 1024 * 1024)
        {
            fclose(file);
            file = nullptr;
            bytes = 0;
            ++segment;
        }
        if (!file)
        {
            const auto path = directory / ("raid-ai-" + run + "-" + std::to_string(segment) + ".log");
            file = fopen(path.string().c_str(), "a");
            if (!file)
                return;
            sLog.outString("[BotDecision] Detailed action log: %s", path.string().c_str());
        }
        const std::string record = sPlayerbotAIConfig.GetTimestampStr() + " " + line + "\n";
        bytes += fwrite(record.data(), 1, record.size(), file);
        fflush(file);
    }
}

void RaidBossMapStrategies::TraceDecision(PlayerbotAI* ai, BotState state, Action* action,
                                         const std::string& source, const char* result, const std::string& detail)
{
    Player* bot = ai->GetBot();
    if (!bot->IsInWorld() || bot->IsBeingTeleported() || !bot->GetMap()->IsDungeon())
        return;
    auto* debug = dynamic_cast<RaidBossDebugValue*>(ai->GetAiObjectContext()->GetUntypedValue("raid boss debug"));
    if (!debug)
        return;

    // The marker opts human-led instance groups into detailed diagnostics, also
    // after a restart. Random world bots are intentionally outside this scope.
    static time_t markerChecked = 0;
    static bool markerEnabled = false;
    const time_t now = time(nullptr);
    if (now - markerChecked >= 10)
    {
        FILE* marker = fopen("raid-ai-debug.enabled", "r");
        markerEnabled = marker != nullptr;
        if (marker)
            fclose(marker);
        markerChecked = now;
    }
    Player* master = ai->GetMaster();
    const bool automatic = markerEnabled && master && !master->GetPlayerbotAI() && bot->GetGroup() &&
        master->GetGroup() == bot->GetGroup() && !debug->lastLog.count("full disabled");
    if (!debug->lastLog.count("full enabled") && !automatic)
        return;
    debug->Set(true);
    if (ai->GetActionHistorySize() < 512)
        ai->SetActionHistorySize(512);

    const std::string name = action ? action->getName() : "unknown";
    const std::string key = "decision:" + std::to_string(uint8(state)) + name + result + source + detail;
    time_t& last = debug->lastLog[key];
    const bool attempted = std::string(result) == "START" || std::string(result) == "EXECUTED" || std::string(result) == "STATE";
    if (now - last < (attempted ? 1 : 5))
        return;
    last = now;

    Unit* target = ai->GetUnit(ai->GetAiObjectContext()->GetValue<ObjectGuid>("current target")->Get());
    Unit* actionTarget = action ? action->GetTarget() : nullptr;
    Spell* cast = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    Spell* channel = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    std::ostringstream out;
    out << "[BotDecision] ms=" << WorldTimer::getMSTime() << " bot=" << bot->GetName()
        << " map=" << bot->GetMapId() << " instance=" << bot->GetInstanceId()
        << " state=" << uint32(state) << " combat=" << bot->IsInCombat() << " alive=" << bot->IsAlive()
        << " action=\"" << name << "\" result=" << result << " detail=\"" << detail
        << "\" trigger=\"" << source << "\" relevance=" << (action ? action->getRelevance() : 0)
        << " duration=" << (action ? action->GetDuration() : 0) << " hp=" << bot->GetHealthPercent()
        << " powerType=" << uint32(bot->GetPowerType()) << " power=" << bot->GetPower(bot->GetPowerType())
        << " moving=" << bot->IsMoving() << " cast=" << (cast ? cast->m_spellInfo->Id : 0)
        << " channel=" << (channel ? channel->m_spellInfo->Id : 0)
        << " pos=" << bot->GetPositionX() << "," << bot->GetPositionY() << "," << bot->GetPositionZ()
        << " target=" << (target ? target->GetObjectGuid().GetCounter() : 0)
        << " targetEntry=" << (target ? target->GetEntry() : 0)
        << " targetHp=" << (target ? target->GetHealthPercent() : 0)
        << " distance=" << (target ? raidboss::ExactDist2d(bot, target) : 0)
        << " victim=" << (target && target->GetVictim() ? target->GetVictim()->GetName() : "none")
        << " actionTarget=" << (actionTarget ? actionTarget->GetObjectGuid().GetCounter() : 0)
        << " actionTargetHp=" << (actionTarget ? actionTarget->GetHealthPercent() : 0);
    if (std::string(result) == "STATE")
    {
        auto auras = [&](Unit* unit)
        {
            if (unit)
                for (const auto& entry : unit->GetSpellAuraHolderMap())
                    out << entry.first << ":" << entry.second->GetStackAmount() << ":" << entry.second->GetAuraDuration() << ",";
        };
        out << " auras=";
        auras(bot);
        out << " targetAuras=";
        auras(target);
        Spell* targetCast = target ? target->GetCurrentSpell(CURRENT_GENERIC_SPELL) : nullptr;
        out << " targetCast=" << (targetCast ? targetCast->m_spellInfo->Id : 0);
    }
    WriteDecisionLog(out.str());
}

bool RaidBossMapStrategies::Debug(PlayerbotAI* ai, Player* requester, const std::string& command)
{
    auto* context = ai->GetAiObjectContext();
    auto* debug = dynamic_cast<RaidBossDebugValue*>(context->GetUntypedValue("raid boss debug"));
    if (!debug)
        return false;

    if (command == "raid on" || command == "raid off" || command == "raid full")
    {
        debug->Set(command != "raid off");
        debug->lastLog.clear();
        if (command == "raid full") debug->lastLog["full enabled"] = 1;
        if (command == "raid off") debug->lastLog["full disabled"] = 1;
    }
    else if (command != "raid" && command != "raid status" && command != "raid dungeons" && command != "raid classic" && command != "raid all")
    {
        ai->TellPlayerNoFacing(requester, "Usage: debug raid [status|on|off|full|dungeons|classic|all]");
        return true;
    }

    auto tell = [&](const std::string& text)
    {
        ai->TellPlayerNoFacing(requester, "[RaidTactics] " + text,
            PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
    };
    if (command == "raid dungeons" || command == "raid classic" || command == "raid all")
    {
        const auto& maps = command == "raid all" ? GetAll() : (command == "raid classic" ? GetClassicRaidBossMaps() : GetDungeonBossMaps());
        size_t missing = 0, encounterCount = 0, triggerCount = 0, actionCount = 0;
        for (const auto& entry : maps)
        {
            Strategy* strategy = context->GetStrategy(entry.second);
            const size_t encounters = tbcraid::EncounterRegistry::Instance().GetEncounterCount(entry.first);
            encounterCount += encounters;
            if (!strategy || strategy->getName() != entry.second || (!encounters && command != "raid all"))
            {
                tell("FEHLENDE Instanz-Strategie/Encounter: " + entry.second);
                ++missing;
                continue;
            }
            for (BotState state : { BotState::BOT_STATE_COMBAT, BotState::BOT_STATE_NON_COMBAT, BotState::BOT_STATE_REACTION })
            {
                std::list<TriggerNode*> nodes;
                strategy->InitTriggers(nodes, state);
                if (nodes.empty() && (command != "raid all" || state == BotState::BOT_STATE_COMBAT))
                {
                    tell("FEHLENDE Instanz-Trigger: " + entry.second);
                    ++missing;
                }
                for (TriggerNode* node : nodes)
                {
                    ++triggerCount;
                    Trigger* trigger = context->GetTrigger(node->getName());
                    if (!trigger)
                    {
                        tell("FEHLENDER Trigger: " + node->getName());
                        ++missing;
                        delete node;
                        continue;
                    }
                    node->setTrigger(trigger);
                    NextAction** handlers = node->getHandlers();
                    for (NextAction** action = handlers; action && *action; ++action)
                    {
                        ++actionCount;
                        if (!context->GetAction((*action)->getName()))
                        {
                            tell("FEHLENDE Aktion: " + (*action)->getName());
                            ++missing;
                        }
                    }
                    NextAction::destroy(handlers);
                    delete node;
                }
            }
        }
        tell("Instanz-Pruefung: Maps=" + std::to_string(maps.size()) +
             " Encounter=" + std::to_string(encounterCount) + " Trigger=" + std::to_string(triggerCount) +
             " Aktionen=" + std::to_string(actionCount) + " Fehlend=" + std::to_string(missing));
        return true;
    }
    Player* bot = ai->GetBot();
    const std::string wanted = GetStrategy(bot->GetMapId());
    tell("Map=" + std::to_string(bot->GetMapId()) + " Instanz=" + std::to_string(bot->GetInstanceId()) +
         " Aktionslog=" + (debug->Get() ? "AN (Server.log)" : "AUS"));
    if (wanted.empty())
    {
        tell("Fuer diese Map ist keine eigene Boss-Strategie hinterlegt.");
        return true;
    }

    Strategy* strategy = context->GetStrategy(wanted);
    tell(wanted + " registriert=" + (strategy ? "JA" : "NEIN") +
         " Kampf=" + (ai->HasStrategy(wanted, BotState::BOT_STATE_COMBAT) ? "AN" : "AUS") +
         " Ruhe=" + (ai->HasStrategy(wanted, BotState::BOT_STATE_NON_COMBAT) ? "AN" : "AUS") +
         " Reaktion=" + (ai->HasStrategy(wanted, BotState::BOT_STATE_REACTION) ? "AN" : "AUS") +
         " dungeon=" + (ai->HasStrategy("dungeon", BotState::BOT_STATE_NON_COMBAT) ? "AN" : "AUS"));

    size_t triggerCount = 0, actionCount = 0, missing = 0;
    if (strategy)
    {
        for (BotState state : { BotState::BOT_STATE_COMBAT, BotState::BOT_STATE_NON_COMBAT, BotState::BOT_STATE_REACTION })
        {
            std::list<TriggerNode*> nodes;
            strategy->InitTriggers(nodes, state);
            for (TriggerNode* node : nodes)
            {
                ++triggerCount;
                Trigger* trigger = context->GetTrigger(node->getName());
                if (!trigger)
                {
                    tell("FEHLENDER Trigger: " + node->getName());
                    ++missing;
                    delete node;
                    continue;
                }
                node->setTrigger(trigger);
                NextAction** handlers = node->getHandlers();
                for (NextAction** actions = handlers; actions && *actions; ++actions)
                {
                    ++actionCount;
                    if (!context->GetAction((*actions)->getName()))
                    {
                        tell("FEHLENDE Aktion: " + (*actions)->getName());
                        ++missing;
                    }
                }
                NextAction::destroy(handlers);
                delete node;
            }
        }
    }
    tell("Verdrahtung: Trigger=" + std::to_string(triggerCount) + " Aktionen=" + std::to_string(actionCount) +
         " Fehlend=" + std::to_string(missing) + " Encounter=" +
         (raidboss::IsEncounterInProgress(bot, bot->GetMapId()) ? "AKTIV" : "INAKTIV"));
    if (IsDungeonBossMap(bot->GetMapId()) || IsClassicRaidBossMap(bot->GetMapId()))
    {
        const auto* encounter = TbcRaidHelper::GetEncounter(ai);
        tell("Instanz-Encounter=" + (encounter ? encounter->name : std::string("KEINER")));
    }

    if (bot->GetMapId() == gruul::GRUUL_MAP_ID)
    {
        auto name = [](Player* player) { return player ? std::string(player->GetName()) : std::string("FEHLT"); };
        tell("Maulgar=" + name(raidboss::GetGroupMainTank(bot)) + " Olm=" + name(raidboss::GetGroupAssistTank(bot, 0)) +
             " Blindeye=" + name(raidboss::GetGroupAssistTank(bot, 1)));
        tell("Krosh-Magier=" + name(gruul::GetKroshMageTank(ai)) + " Kiggler-Eule=" + name(gruul::GetKigglerMoonkinTank(ai)) +
             " Heiler=" + std::to_string(raidboss::GetGroupHealers(bot).size()));
        tell("Maulgar sichtbar=" + std::string(gruul::FindMaulgar(ai) ? "JA" : "NEIN") +
             " Krosh sichtbar=" + (gruul::FindKrosh(ai) ? "JA" : "NEIN") +
             " Rolle=" + (PlayerbotAI::IsTank(bot) ? "Tank" : PlayerbotAI::IsHeal(bot) ? "Heiler" : "DPS"));
    }
    return true;
}
