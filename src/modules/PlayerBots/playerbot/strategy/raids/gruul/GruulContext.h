#pragma once
#include "playerbot/strategy/NamedObjectContext.h"
#include "GruulStrategy.h"
#include "GruulTriggers.h"
#include "GruulActions.h"
#include "GruulValues.h"

// Gruul's Lair (map 565) - strategy "boss gruul"
// Ported from mod-playerbots (src/Ai/Raid/Gruul, GPL v2+) and adapted for this core.
namespace ai
{
    class GruulStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        GruulStrategyContext()
        {
            creators["boss gruul"] = [](PlayerbotAI* ai) { return new GruulStrategy(ai); };
        }
    };

    class GruulActionContext : public NamedObjectContext<Action>
    {
    public:
        GruulActionContext()
        {
            // General
            creators["gruul's lair reset encounter states"] = [](PlayerbotAI* ai) { return new GruulsLairResetEncounterStatesAction(ai); };

            // High King Maulgar
            creators["high king maulgar prepare"] = [](PlayerbotAI* ai) { return new HighKingMaulgarPrepareAction(ai); };
            creators["high king maulgar melee tanks position bosses"] = [](PlayerbotAI* ai) { return new HighKingMaulgarMeleeTanksPositionBossesAction(ai); };
            creators["high king maulgar mage tank attack krosh"] = [](PlayerbotAI* ai) { return new HighKingMaulgarMageTankAttackKroshAction(ai); };
            creators["high king maulgar moonkin tank attack kiggler"] = [](PlayerbotAI* ai) { return new HighKingMaulgarMoonkinTankAttackKigglerAction(ai); };
            creators["high king maulgar assign dps priority"] = [](PlayerbotAI* ai) { return new HighKingMaulgarAssignDpsPriorityAction(ai); };
            creators["high king maulgar run away from whirlwind"] = [](PlayerbotAI* ai) { return new HighKingMaulgarRunAwayFromWhirlwindAction(ai); };
            creators["high king maulgar back away from krosh"] = [](PlayerbotAI* ai) { return new HighKingMaulgarBackAwayFromKroshAction(ai); };
            creators["high king maulgar banish fel stalker"] = [](PlayerbotAI* ai) { return new HighKingMaulgarBanishFelStalkerAction(ai); };
            creators["high king maulgar misdirect ogres to tanks"] = [](PlayerbotAI* ai) { return new HighKingMaulgarMisdirectOgresToTanksAction(ai); };

            // Gruul the Dragonkiller
            creators["gruul the dragonkiller tanks position boss"] = [](PlayerbotAI* ai) { return new GruulTheDragonkillerTanksPositionBossAction(ai); };
            creators["gruul the dragonkiller spread ranged"] = [](PlayerbotAI* ai) { return new GruulTheDragonkillerSpreadRangedAction(ai); };
            creators["gruul the dragonkiller get out of cave in"] = [](PlayerbotAI* ai) { return new GruulTheDragonkillerGetOutOfCaveInAction(ai); };
            creators["gruul the dragonkiller shatter spread"] = [](PlayerbotAI* ai) { return new GruulTheDragonkillerShatterSpreadAction(ai); };
        }
    };

    class GruulTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        GruulTriggerContext()
        {
            // General
            creators["gruul's lair no encounter in progress"] = [](PlayerbotAI* ai) { return new GruulsLairNoEncounterInProgressTrigger(ai); };

            // High King Maulgar
            creators["high king maulgar three ogres need melee tanks"] = [](PlayerbotAI* ai) { return new HighKingMaulgarThreeOgresNeedMeleeTanksTrigger(ai); };
            creators["high king maulgar krosh needs mage tank"] = [](PlayerbotAI* ai) { return new HighKingMaulgarKroshNeedsMageTankTrigger(ai); };
            creators["high king maulgar kiggler needs moonkin tank"] = [](PlayerbotAI* ai) { return new HighKingMaulgarKigglerNeedsMoonkinTankTrigger(ai); };
            creators["high king maulgar determining kill order"] = [](PlayerbotAI* ai) { return new HighKingMaulgarDeterminingKillOrderTrigger(ai); };
            creators["high king maulgar channeling whirlwind"] = [](PlayerbotAI* ai) { return new HighKingMaulgarChannelingWhirlwindTrigger(ai); };
            creators["high king maulgar should stand back from krosh"] = [](PlayerbotAI* ai) { return new HighKingMaulgarShouldStandBackFromKroshTrigger(ai); };
            creators["high king maulgar wild fel stalker spawned"] = [](PlayerbotAI* ai) { return new HighKingMaulgarWildFelStalkerSpawnedTrigger(ai); };
            creators["high king maulgar pulling ogre council"] = [](PlayerbotAI* ai) { return new HighKingMaulgarPullingOgreCouncilTrigger(ai); };

            // Gruul the Dragonkiller
            creators["gruul the dragonkiller should be tanked"] = [](PlayerbotAI* ai) { return new GruulTheDragonkillerShouldBeTankedTrigger(ai); };
            creators["gruul the dragonkiller ranged should spread"] = [](PlayerbotAI* ai) { return new GruulTheDragonkillerRangedShouldSpreadTrigger(ai); };
            creators["gruul the dragonkiller in cave in"] = [](PlayerbotAI* ai) { return new GruulTheDragonkillerInCaveInTrigger(ai); };
            creators["gruul the dragonkiller incoming shatter"] = [](PlayerbotAI* ai) { return new GruulTheDragonkillerIncomingShatterTrigger(ai); };
        }
    };

    class GruulValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        GruulValueContext()
        {
            creators["high king maulgar wild fel stalkers"] = [](PlayerbotAI* ai) { return new HighKingMaulgarWildFelStalkersValue(ai); };
            creators["high king maulgar krosh mage tank"] = [](PlayerbotAI* ai) { return new HighKingMaulgarKroshMageTankValue(ai); };
            creators["high king maulgar kiggler moonkin tank"] = [](PlayerbotAI* ai) { return new HighKingMaulgarKigglerMoonkinTankValue(ai); };
        }
    };
}
