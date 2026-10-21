
#include "playerbot/playerbot.h"
#include "ShamanActions.h"
#include "playerbot/strategy/generic/TbcRotationRules.h"

using namespace ai;

bool ai::UseTbcAirTotem(PlayerbotAI* ai, const std::string& spell)
{
    // Explicit assignments take precedence over automatic composition choices.
    const std::pair<const char*, const char*> assignments[] = {
        {"totem air windfury", "windfury totem"},
        {"totem air wrath", "wrath of air totem"},
        {"totem air grace", "grace of air totem"}
    };
    for (const auto& assignment : assignments)
        if (ai->HasStrategy(assignment.first, BotState::BOT_STATE_COMBAT))
            return spell == assignment.second;

    Player* bot = ai->GetBot();
    unsigned melee = 0, casters = 0;
    auto count = [&](Player* member)
    {
        if (!member || !member->IsAlive() || !member->IsInWorld() ||
            member->GetMapId() != bot->GetMapId() ||
            !bot->IsWithinDistInMap(member, 30.0f) ||
            (bot->GetGroup() && member->GetSubGroup() != bot->GetSubGroup()))
            return;
        if (member->getClass() == CLASS_HUNTER)
            return; // Hunters do not benefit from Windfury's melee imbue.
        if (ai->IsRanged(member) || ai->IsHeal(member))
            ++casters;
        else if (member->getClass() != CLASS_DRUID)
            ++melee; // Feral attacks cannot receive the weapon imbue.
    };
    if (Group* group = bot->GetGroup())
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            count(ref->getSource());
    else
        count(bot);

    const char* preferred = tbc::PreferWindfury(melee, casters) ? "windfury totem" :
        (casters ? "wrath of air totem" : "grace of air totem");
    // Lower-level bots may not know the preferred totem yet.
    const char* choices[] = {preferred, "windfury totem", "grace of air totem", "wrath of air totem"};
    for (const char* choice : choices)
        if (ai->GetAiObjectContext()->GetValue<uint32>("spell id", choice)->Get())
            return spell == choice;
    return false;
}
