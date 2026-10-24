
#include "playerbot/playerbot.h"
#include "DruidValues.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

class TankWithoutHotPredicate : public FindPlayerPredicate, public PlayerbotAIAware
{
public:
	explicit TankWithoutHotPredicate(PlayerbotAI* ai, const std::string& spell) :
		FindPlayerPredicate(), PlayerbotAIAware(ai), spell(spell)  {}

	bool Check(Unit* unit) override
	{
		if (unit && unit->IsPlayer() && sServerFacade.IsAlive(unit) && ai->IsTank((Player*)unit))
		{
			if (spell == "lifebloom")
				return !ai->HasAura("lifebloom", unit, true, true, -1, false, 2000, 8);

			return !ai->HasAura(spell, unit, false, true, -1, false, 1000);
		}

		return false;
	}

private:
	std::string spell;
};

ObjectGuid PartyTankWithoutLifebloomValue::Calculate()
{
	// The qualifier names the HoT; the lifebloom value is registered without one
	TankWithoutHotPredicate predicate(ai, getQualifier().empty() ? "lifebloom" : getQualifier());
	Unit* tank = FindPartyMember(predicate);
	return tank ? tank->GetObjectGuid() : ObjectGuid();
}