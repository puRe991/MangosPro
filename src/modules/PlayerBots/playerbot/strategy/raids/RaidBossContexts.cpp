#include "playerbot/playerbot.h"
#include "RaidBossContexts.h"
#include "RaidBossStrategies.h"
#include "mc/McContext.h"
#include "bwl/BwlContext.h"
#include "onyxia/OnyxiaContext.h"
#include "aq20/Aq20Context.h"
#include "naxx/NaxxContext.h"
#include "karazhan/KarazhanContext.h"
#include "gruul/GruulContext.h"
#include "magtheridon/MagtheridonContext.h"
#include "ssc/SscContext.h"
#include "tk/TkContext.h"
#include "hyjal/HyjalContext.h"
#include "bt/BtContext.h"
#include "za/ZaContext.h"
#include "dungeons/TbcDungeonContext.h"

using namespace ai;

void ai::AddRaidBossContexts(NamedObjectContextList<Strategy>& strategies, NamedObjectContextList<Action>& actions,
                             NamedObjectContextList<Trigger>& triggers, NamedObjectContextList<UntypedValue>& values)
{
    strategies.Add(new RaidBossMapStrategyContext());
    actions.Add(new RaidBossMapActionContext());
    triggers.Add(new RaidBossMapTriggerContext());

    strategies.Add(new McStrategyContext());
    actions.Add(new McActionContext());
    triggers.Add(new McTriggerContext());
    values.Add(new McValueContext());

    strategies.Add(new BwlStrategyContext());
    actions.Add(new BwlActionContext());
    triggers.Add(new BwlTriggerContext());
    values.Add(new BwlValueContext());

    strategies.Add(new OnyxiaStrategyContext());
    actions.Add(new OnyxiaActionContext());
    triggers.Add(new OnyxiaTriggerContext());
    values.Add(new OnyxiaValueContext());

    strategies.Add(new Aq20StrategyContext());
    actions.Add(new Aq20ActionContext());
    triggers.Add(new Aq20TriggerContext());
    values.Add(new Aq20ValueContext());

    strategies.Add(new NaxxStrategyContext());
    actions.Add(new NaxxActionContext());
    triggers.Add(new NaxxTriggerContext());
    values.Add(new NaxxValueContext());

    strategies.Add(new KarazhanStrategyContext());
    actions.Add(new KarazhanActionContext());
    triggers.Add(new KarazhanTriggerContext());
    values.Add(new KarazhanValueContext());

    strategies.Add(new GruulStrategyContext());
    actions.Add(new GruulActionContext());
    triggers.Add(new GruulTriggerContext());
    values.Add(new GruulValueContext());

    strategies.Add(new MagtheridonStrategyContext());
    actions.Add(new MagtheridonActionContext());
    triggers.Add(new MagtheridonTriggerContext());
    values.Add(new MagtheridonValueContext());

    strategies.Add(new SscStrategyContext());
    actions.Add(new SscActionContext());
    triggers.Add(new SscTriggerContext());
    values.Add(new SscValueContext());

    strategies.Add(new TkStrategyContext());
    actions.Add(new TkActionContext());
    triggers.Add(new TkTriggerContext());
    values.Add(new TkValueContext());

    strategies.Add(new HyjalStrategyContext());
    actions.Add(new HyjalActionContext());
    triggers.Add(new HyjalTriggerContext());
    values.Add(new HyjalValueContext());

    strategies.Add(new BtStrategyContext());
    actions.Add(new BtActionContext());
    triggers.Add(new BtTriggerContext());
    values.Add(new BtValueContext());

    strategies.Add(new ZaStrategyContext());
    actions.Add(new ZaActionContext());
    triggers.Add(new ZaTriggerContext());
    values.Add(new ZaValueContext());

    strategies.Add(new TbcDungeonStrategyContext());
    actions.Add(new TbcDungeonActionContext());
    triggers.Add(new TbcDungeonTriggerContext());
    values.Add(new TbcDungeonValueContext());
}
