
#include "playerbot/playerbot.h"
#include "WarriorActions.h"
#include "playerbot/strategy/generic/TbcRotationRules.h"

using namespace ai;

bool ai::CanUseTbcSlam(PlayerbotAI* ai)
{
#ifdef MANGOSBOT_ONE
    Player* bot = ai->GetBot();
    Item* weapon = bot->GetWeaponForAttack(BASE_ATTACK);
    if (!weapon || weapon->GetProto()->InventoryType != INVTYPE_2HWEAPON ||
        (!bot->HasSpell(12330) && !bot->HasSpell(12862)) ||
        !bot->hasUnitState(UNIT_STAT_MELEE_ATTACKING) || bot->IsMoving())
        return false;

    const uint32 spellId = ai->GetAiObjectContext()->GetValue<uint32>("spell id", "slam")->Get();
    const SpellEntry* spell = sServerFacade.LookupSpellInfo(spellId);
    return spell && tbc::SlamWindow(bot->getAttackTimer(BASE_ATTACK),
        uint32(bot->GetFloatValue(UNIT_FIELD_BASEATTACKTIME)), GetSpellCastTime(spell, bot));
#else
    return true;
#endif
}

