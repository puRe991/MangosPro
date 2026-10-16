// Karazhan values for the boss tactics ported from mod-playerbots (GPL v2+, src/Ai/Raid/Kara) and adapted for this core.
#include "playerbot/playerbot.h"
#include "KarazhanValues.h"
#include "KarazhanHelpers.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

using namespace ai;

std::list<ObjectGuid> KarazhanNearbyCreaturesValue::Calculate()
{
    std::list<ObjectGuid> result;
    if (!bot->IsInWorld() || bot->GetMapId() != karazhan::KARAZHAN_MAP_ID)
        return result;

    const float range = 120.0f;
    std::list<Unit*> units;
    MaNGOS::AnyUnitFulfillingConditionInRangeCheck check(bot, [](Unit* unit)
    {
        return unit->GetTypeId() == TYPEID_UNIT && unit->IsAlive() && karazhan::IsTrackedEntry(unit->GetEntry());
    }, range, DIST_CALC_NONE);
    MaNGOS::UnitListSearcher<MaNGOS::AnyUnitFulfillingConditionInRangeCheck> searcher(units, check);
    Cell::VisitAllObjects(bot, searcher, range);

    for (Unit* unit : units)
        result.push_back(unit->GetObjectGuid());

    return result;
}
