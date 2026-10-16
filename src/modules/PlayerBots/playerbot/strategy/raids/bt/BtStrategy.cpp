// Ported from mod-playerbots (AzerothCore, GPL v2+, src/Ai/Raid/BT/BTStrategy) and adapted for this core.
#include "playerbot/playerbot.h"
#include "BtStrategy.h"
#include "BtHelpers.h"
#include "BtMultipliers.h"
#include "playerbot/strategy/raids/RaidBossHelpers.h"

using namespace ai;

void BtStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "high warlord naj'entus pulling boss",
        NextAction::array(0, new NextAction("high warlord naj'entus misdirect boss to main tank", raidboss::ACTION_RAID + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "high warlord naj'entus boss engaged by tanks",
        NextAction::array(0, new NextAction("high warlord naj'entus tanks position boss", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "high warlord naj'entus casts needle spines",
        NextAction::array(0, new NextAction("high warlord naj'entus disperse ranged", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "high warlord naj'entus player is impaled",
        NextAction::array(0, new NextAction("high warlord naj'entus remove impaling spine", bt::BT_EMERGENCY_1), NULL)));

    triggers.push_back(new TriggerNode(
        "high warlord naj'entus boss has tidal shield",
        NextAction::array(0, new NextAction("high warlord naj'entus throw impaling spine", raidboss::ACTION_RAID + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "supremus pulling boss or changing phase",
        NextAction::array(0, new NextAction("supremus misdirect boss to main tank", raidboss::ACTION_RAID + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "supremus boss engaged by ranged",
        NextAction::array(0, new NextAction("supremus disperse ranged", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "supremus boss is fixated on bot",
        NextAction::array(0, new NextAction("supremus kite boss", bt::BT_EMERGENCY_7), NULL)));

    triggers.push_back(new TriggerNode(
        "supremus volcano is nearby",
        NextAction::array(0, new NextAction("supremus move away from volcanos", bt::BT_EMERGENCY_6), NULL)));

    triggers.push_back(new TriggerNode(
        "supremus need to manage phase timer",
        NextAction::array(0, new NextAction("supremus manage phase timer", bt::BT_EMERGENCY_10), NULL)));

    triggers.push_back(new TriggerNode(
        "shade of akama killing channelers starts phase 2",
        NextAction::array(0, new NextAction("shade of akama melee dps prioritize channelers", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "teron gorefiend pulling boss",
        NextAction::array(0, new NextAction("teron gorefiend misdirect boss to main tank", raidboss::ACTION_RAID + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "teron gorefiend boss engaged by tanks",
        NextAction::array(0, new NextAction("teron gorefiend tanks position boss", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "teron gorefiend boss engaged by ranged",
        NextAction::array(0, new NextAction("teron gorefiend position ranged on balcony", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "teron gorefiend boss is casting shadow of death",
        NextAction::array(0, new NextAction("teron gorefiend avoid shadow of death", bt::BT_EMERGENCY_10), NULL)));

    triggers.push_back(new TriggerNode(
        "teron gorefiend bot has shadow of death",
        NextAction::array(0, new NextAction("teron gorefiend move to corner to die", bt::BT_EMERGENCY_10), NULL)));

    triggers.push_back(new TriggerNode(
        "teron gorefiend bot transformed into vengeful spirit",
        NextAction::array(0, new NextAction("teron gorefiend control and destroy shadowy constructs", bt::BT_EMERGENCY_10), NULL)));

    triggers.push_back(new TriggerNode(
        "gurtogg bloodboil pulling boss",
        NextAction::array(0, new NextAction("gurtogg bloodboil misdirect boss to main tank", raidboss::ACTION_RAID + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "gurtogg bloodboil boss engaged by tanks",
        NextAction::array(0, new NextAction("gurtogg bloodboil tanks position boss", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "gurtogg bloodboil boss casts bloodboil",
        NextAction::array(0, new NextAction("gurtogg bloodboil rotate ranged groups", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "gurtogg bloodboil bot has fel rage",
        NextAction::array(0, new NextAction("gurtogg bloodboil ranged move away from enraged player", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "gurtogg bloodboil need to manage phase timer",
        NextAction::array(0, new NextAction("gurtogg bloodboil manage phase timer", bt::BT_EMERGENCY_10), NULL)));

    triggers.push_back(new TriggerNode(
        "reliquary of souls aggro resets upon phase change",
        NextAction::array(0, new NextAction("reliquary of souls misdirect boss to main tank", raidboss::ACTION_RAID + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "reliquary of souls essence of suffering fixates on closest target",
        NextAction::array(0, new NextAction("reliquary of souls adjust distance from suffering", raidboss::ACTION_RAID + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "reliquary of souls essence of suffering disables healing",
        NextAction::array(0, new NextAction("reliquary of souls healers dps suffering", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "reliquary of souls essence of desire has rune shield",
        NextAction::array(0, new NextAction("reliquary of souls spellsteal rune shield", bt::BT_EMERGENCY_6), NULL)));

    triggers.push_back(new TriggerNode(
        "reliquary of souls essence of desire casting deaden",
        NextAction::array(0, new NextAction("reliquary of souls spell reflect deaden", bt::BT_EMERGENCY_6), NULL)));

    triggers.push_back(new TriggerNode(
        "mother shahraz pulling boss",
        NextAction::array(0, new NextAction("mother shahraz misdirect boss to main tank", raidboss::ACTION_RAID + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "mother shahraz boss engaged by tanks",
        NextAction::array(0, new NextAction("mother shahraz tanks position boss under pillar", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "mother shahraz tanks are positioning boss",
        NextAction::array(0, new NextAction("mother shahraz melee dps wait at safe position", bt::BT_EMERGENCY_1), NULL)));

    triggers.push_back(new TriggerNode(
        "mother shahraz sinister beam knocks back players",
        NextAction::array(0, new NextAction("mother shahraz position ranged under pillar", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "mother shahraz bots are linked by fatal attraction",
        NextAction::array(0, new NextAction("mother shahraz run away to break fatal attraction", bt::BT_EMERGENCY_10), NULL)));

    triggers.push_back(new TriggerNode(
        "illidari council pulling bosses",
        NextAction::array(0, new NextAction("illidari council misdirect bosses to tanks", raidboss::ACTION_RAID + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "illidari council gathios engaged by main tank",
        NextAction::array(0, new NextAction("illidari council main tank position gathios", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "illidari council gathios casting judgement of command",
        NextAction::array(0, new NextAction("illidari council main tank reflect judgement of command", bt::BT_EMERGENCY_1), NULL)));

    triggers.push_back(new TriggerNode(
        "illidari council malande engaged by first assist tank",
        NextAction::array(0, new NextAction("illidari council first assist tank focus malande", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "illidari council darkshadow engaged by second assist tank",
        NextAction::array(0, new NextAction("illidari council second assist tank position darkshadow", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "illidari council zerevor engaged by mage tank",
        NextAction::array(0, new NextAction("illidari council mage tank position zerevor", bt::BT_EMERGENCY_6), NULL)));

    triggers.push_back(new TriggerNode(
        "illidari council mage tank needs dedicated healer",
        NextAction::array(0, new NextAction("illidari council position mage tank healer", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "illidari council zerevor casts dangerous aoes",
        NextAction::array(0, new NextAction("illidari council disperse ranged", raidboss::ACTION_RAID + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "illidari council pets screw up the pull",
        NextAction::array(0, new NextAction("illidari council command pets to attack gathios", raidboss::ACTION_RAID + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "illidari council determining dps assignments",
        NextAction::array(0, new NextAction("illidari council assign dps targets", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "illidari council need to manage dps timer",
        NextAction::array(0, new NextAction("illidari council manage dps timer", bt::BT_EMERGENCY_10), NULL)));

    triggers.push_back(new TriggerNode(
        "illidan stormrage tank needs aggro",
        NextAction::array(0, new NextAction("illidan stormrage misdirect to tank", raidboss::ACTION_RAID + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "illidan stormrage boss casts flame crash in front of main tank",
        NextAction::array(0, new NextAction("illidan stormrage main tank reposition boss", bt::BT_EMERGENCY_1), NULL)));

    triggers.push_back(new TriggerNode(
        "illidan stormrage bot has parasitic shadowfiend",
        NextAction::array(0, new NextAction("illidan stormrage isolate bot with parasite", raidboss::ACTION_RAID + 3), NULL)));

    triggers.push_back(new TriggerNode(
        "illidan stormrage parasitic shadowfiends run wild",
        NextAction::array(0, new NextAction("illidan stormrage set earthbind totem", raidboss::ACTION_RAID + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "illidan stormrage boss summoned flames of azzinoth",
        NextAction::array(0, new NextAction("illidan stormrage assist tanks handle flames of azzinoth", bt::BT_EMERGENCY_1), NULL)));

    triggers.push_back(new TriggerNode(
        "illidan stormrage pets die to fire",
        NextAction::array(0, new NextAction("illidan stormrage control pet aggression", raidboss::ACTION_RAID + 4), NULL)));

    triggers.push_back(new TriggerNode(
        "illidan stormrage grate is safe from flames",
        NextAction::array(0, new NextAction("illidan stormrage position above grate", bt::BT_EMERGENCY_2), NULL)));

    triggers.push_back(new TriggerNode(
        "illidan stormrage bot struck by dark barrage",
        NextAction::array(0, new NextAction("illidan stormrage remove dark barrage", bt::BT_EMERGENCY_6), NULL)));

    triggers.push_back(new TriggerNode(
        "illidan stormrage boss is preparing to land",
        NextAction::array(0, new NextAction("illidan stormrage move away from landing point", bt::BT_EMERGENCY_3), NULL)));

    triggers.push_back(new TriggerNode(
        "illidan stormrage boss deals splash damage",
        NextAction::array(0, new NextAction("illidan stormrage disperse ranged", raidboss::ACTION_RAID + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "illidan stormrage this expansion hates melee",
        NextAction::array(0, new NextAction("illidan stormrage melee go somewhere to not die", raidboss::ACTION_RAID + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "illidan stormrage boss transforms into demon",
        NextAction::array(0, new NextAction("illidan stormrage warlock tank handle demon boss", bt::BT_EMERGENCY_9), NULL)));

    triggers.push_back(new TriggerNode(
        "illidan stormrage boss spawns adds",
        NextAction::array(0, new NextAction("illidan stormrage dps prioritize adds", bt::BT_EMERGENCY_1), NULL)));

    triggers.push_back(new TriggerNode(
        "illidan stormrage need to manage dps timer and rti",
        NextAction::array(0, new NextAction("illidan stormrage manage dps timer and rti", bt::BT_EMERGENCY_11), NULL)));
}

// mod-playerbots resets its timers and trackers out of combat
void BtStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "black temple bot is not in combat",
        NextAction::array(0, new NextAction("black temple erase timers and trackers", bt::BT_EMERGENCY_11), NULL)));
}

// Checked while casting: dodge volcanos, break Fatal Attraction, survive Shadow of Death / Dark Barrage
void BtStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "high warlord naj'entus player is impaled",
        NextAction::array(0, new NextAction("high warlord naj'entus remove impaling spine", bt::BT_EMERGENCY_1), NULL)));

    triggers.push_back(new TriggerNode(
        "supremus volcano is nearby",
        NextAction::array(0, new NextAction("supremus move away from volcanos", bt::BT_EMERGENCY_6), NULL)));

    triggers.push_back(new TriggerNode(
        "teron gorefiend boss is casting shadow of death",
        NextAction::array(0, new NextAction("teron gorefiend avoid shadow of death", bt::BT_EMERGENCY_10), NULL)));

    triggers.push_back(new TriggerNode(
        "mother shahraz bots are linked by fatal attraction",
        NextAction::array(0, new NextAction("mother shahraz run away to break fatal attraction", bt::BT_EMERGENCY_10), NULL)));

    triggers.push_back(new TriggerNode(
        "illidan stormrage bot struck by dark barrage",
        NextAction::array(0, new NextAction("illidan stormrage remove dark barrage", bt::BT_EMERGENCY_6), NULL)));
}

void BtStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new BlackTempleDelayDpsCooldownsMultiplier(ai));
    multipliers.push_back(new HighWarlordNajentusDisableCombatFormationMoveMultiplier(ai));
    multipliers.push_back(new SupremusFocusOnAvoidanceInPhase2Multiplier(ai));
    multipliers.push_back(new SupremusHitboxIsBuggedMultiplier(ai));
    multipliers.push_back(new ShadeOfAkamaMeleeFocusChannelersMultiplier(ai));
    multipliers.push_back(new TeronGorefiendControlMovementMultiplier(ai));
    multipliers.push_back(new TeronGorefiendMarkedBotOnlyMoveToDieMultiplier(ai));
    multipliers.push_back(new TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier(ai));
    multipliers.push_back(new TeronGorefiendDisableAttackingConstructsMultiplier(ai));
    multipliers.push_back(new GurtoggBloodboilControlMovementMultiplier(ai));
    multipliers.push_back(new ReliquaryOfSoulsDontWasteHealingMultiplier(ai));
    multipliers.push_back(new MotherShahrazControlMovementMultiplier(ai));
    multipliers.push_back(new MotherShahrazBotsWithFatalAttractionOnlyRunAwayMultiplier(ai));
    multipliers.push_back(new IllidariCouncilDisableTankActionsMultiplier(ai));
    multipliers.push_back(new IllidariCouncilControlMovementMultiplier(ai));
    multipliers.push_back(new IllidariCouncilControlMisdirectionMultiplier(ai));
    multipliers.push_back(new IllidariCouncilDisableArcaneShotOnZerevorMultiplier(ai));
    multipliers.push_back(new IllidariCouncilDisableIceBlockMultiplier(ai));
    multipliers.push_back(new IllidariCouncilWaitForDpsMultiplier(ai));
    multipliers.push_back(new IllidanStormrageDelayDpsCooldownsMultiplier(ai));
    multipliers.push_back(new IllidanStormrageControlTankActionsMultiplier(ai));
    multipliers.push_back(new IllidanStormrageDisableDefaultTargetingMultiplier(ai));
    multipliers.push_back(new IllidanStormrageControlNonTankMovementMultiplier(ai));
    multipliers.push_back(new IllidanStormrageUseEarthbindTotemMultiplier(ai));
    multipliers.push_back(new IllidanStormrageWaitForDpsMultiplier(ai));
}
