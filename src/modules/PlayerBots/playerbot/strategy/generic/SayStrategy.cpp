
#include "playerbot/playerbot.h"
#include "SayStrategy.h"

using namespace ai;

// Chat is flavour only: a higher relevance took the combat tick from boss mechanics (escapes are ACTION_EMERGENCY + 1)
void SayStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "critical health",
        NextAction::array(0, new NextAction("say::critical health", ACTION_IDLE), NULL)));

    triggers.push_back(new TriggerNode(
        "low health",
        NextAction::array(0, new NextAction("say::low health", ACTION_IDLE), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("say::low mana", ACTION_IDLE), NULL)));

    triggers.push_back(new TriggerNode(
        "tank aoe",
        NextAction::array(0, new NextAction("say::taunt", ACTION_IDLE), NULL)));

    triggers.push_back(new TriggerNode(
        "ranged high aoe",
        NextAction::array(0, new NextAction("say::aoe", ACTION_IDLE), NULL)));
}
