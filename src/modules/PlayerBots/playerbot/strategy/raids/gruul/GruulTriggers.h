// Ported from mod-playerbots (src/Ai/Raid/Gruul, GPL v2+) and adapted for this core.
#pragma once

#include "playerbot/strategy/Trigger.h"

namespace ai
{
    // General

    class GruulsLairEncounterTrigger : public Trigger
    {
    public:
        GruulsLairEncounterTrigger(PlayerbotAI* ai, std::string name) : Trigger(ai, name, 1) {}
        bool IsActive() final;

    protected:
        virtual bool IsActiveInEncounter() = 0;
    };

    class GruulsLairNoEncounterInProgressTrigger : public Trigger
    {
    public:
        // Throttled: true for all trash and downtime, no urgency for the clean-up.
        GruulsLairNoEncounterInProgressTrigger(PlayerbotAI* ai) : Trigger(ai, "gruul's lair no encounter in progress", 2) {}
        bool IsActive() override;
    };

    // High King Maulgar <Lord of the Ogres>

    class HighKingMaulgarThreeOgresNeedMeleeTanksTrigger : public GruulsLairEncounterTrigger
    {
    public:
        HighKingMaulgarThreeOgresNeedMeleeTanksTrigger(PlayerbotAI* ai) : GruulsLairEncounterTrigger(ai, "high king maulgar three ogres need melee tanks") {}
    protected:
        bool IsActiveInEncounter() override;
    };

    class HighKingMaulgarKroshNeedsMageTankTrigger : public GruulsLairEncounterTrigger
    {
    public:
        HighKingMaulgarKroshNeedsMageTankTrigger(PlayerbotAI* ai) : GruulsLairEncounterTrigger(ai, "high king maulgar krosh needs mage tank") {}
    protected:
        bool IsActiveInEncounter() override;
    };

    class HighKingMaulgarKigglerNeedsMoonkinTankTrigger : public GruulsLairEncounterTrigger
    {
    public:
        HighKingMaulgarKigglerNeedsMoonkinTankTrigger(PlayerbotAI* ai) : GruulsLairEncounterTrigger(ai, "high king maulgar kiggler needs moonkin tank") {}
    protected:
        bool IsActiveInEncounter() override;
    };

    class HighKingMaulgarDeterminingKillOrderTrigger : public GruulsLairEncounterTrigger
    {
    public:
        HighKingMaulgarDeterminingKillOrderTrigger(PlayerbotAI* ai) : GruulsLairEncounterTrigger(ai, "high king maulgar determining kill order") {}
    protected:
        bool IsActiveInEncounter() override;
    };

    class HighKingMaulgarChannelingWhirlwindTrigger : public GruulsLairEncounterTrigger
    {
    public:
        HighKingMaulgarChannelingWhirlwindTrigger(PlayerbotAI* ai) : GruulsLairEncounterTrigger(ai, "high king maulgar channeling whirlwind") {}
    protected:
        bool IsActiveInEncounter() override;
    };

    class HighKingMaulgarShouldStandBackFromKroshTrigger : public GruulsLairEncounterTrigger
    {
    public:
        HighKingMaulgarShouldStandBackFromKroshTrigger(PlayerbotAI* ai) : GruulsLairEncounterTrigger(ai, "high king maulgar should stand back from krosh") {}
    protected:
        bool IsActiveInEncounter() override;
    };

    class HighKingMaulgarWildFelStalkerSpawnedTrigger : public GruulsLairEncounterTrigger
    {
    public:
        HighKingMaulgarWildFelStalkerSpawnedTrigger(PlayerbotAI* ai) : GruulsLairEncounterTrigger(ai, "high king maulgar wild fel stalker spawned") {}
    protected:
        bool IsActiveInEncounter() override;
    };

    class HighKingMaulgarPullingOgreCouncilTrigger : public GruulsLairEncounterTrigger
    {
    public:
        HighKingMaulgarPullingOgreCouncilTrigger(PlayerbotAI* ai) : GruulsLairEncounterTrigger(ai, "high king maulgar pulling ogre council") {}
    protected:
        bool IsActiveInEncounter() override;
    };

    // Gruul the Dragonkiller

    class GruulTheDragonkillerShouldBeTankedTrigger : public GruulsLairEncounterTrigger
    {
    public:
        GruulTheDragonkillerShouldBeTankedTrigger(PlayerbotAI* ai) : GruulsLairEncounterTrigger(ai, "gruul the dragonkiller should be tanked") {}
    protected:
        bool IsActiveInEncounter() override;
    };

    class GruulTheDragonkillerRangedShouldSpreadTrigger : public GruulsLairEncounterTrigger
    {
    public:
        GruulTheDragonkillerRangedShouldSpreadTrigger(PlayerbotAI* ai) : GruulsLairEncounterTrigger(ai, "gruul the dragonkiller ranged should spread") {}
    protected:
        bool IsActiveInEncounter() override;
    };

    class GruulTheDragonkillerInCaveInTrigger : public GruulsLairEncounterTrigger
    {
    public:
        GruulTheDragonkillerInCaveInTrigger(PlayerbotAI* ai) : GruulsLairEncounterTrigger(ai, "gruul the dragonkiller in cave in") {}
    protected:
        bool IsActiveInEncounter() override;
    };

    class GruulTheDragonkillerIncomingShatterTrigger : public GruulsLairEncounterTrigger
    {
    public:
        GruulTheDragonkillerIncomingShatterTrigger(PlayerbotAI* ai) : GruulsLairEncounterTrigger(ai, "gruul the dragonkiller incoming shatter") {}
    protected:
        bool IsActiveInEncounter() override;
    };
}
