#include "GameObjectAI.h"
#include "MoveSplineInit.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Smallcraft.h"
#include "SmartAI.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "../../src/server/scripts/EasternKingdoms/ZulGurub/zulgurub.h"

namespace ac_jeklik
{

    #include "../../src/server/scripts/EasternKingdoms/ZulGurub/boss_jeklik.cpp"
}

// High Priestess Jeklik (14517)
struct boss_jeklik : public ac_jeklik::boss_jeklik
{
    boss_jeklik(Creature* creature) : ac_jeklik::boss_jeklik(creature) { }

    void UpdateAI(uint32 diff) override
    {
        // EVENT_SPAWN_FLYING_BATS
        // Reschedule event to 10s
        if
        (
                ac_jeklik::boss_jeklik::events.GetTimeUntilEvent(ac_jeklik::EVENT_SPAWN_FLYING_BATS) > 10s &&
                ac_jeklik::boss_jeklik::events.GetTimeUntilEvent(ac_jeklik::EVENT_SPAWN_FLYING_BATS) != Milliseconds::max()
        )
        {
            LOG_DEBUG("module.Smallcraft", "sc::boss_jeklik::UpdateAI: EVENT_SPAWN_FLYING_BATS rescheduled from {}ms to 10s",
                ac_jeklik::boss_jeklik::events.GetTimeUntilEvent(ac_jeklik:: EVENT_SPAWN_FLYING_BATS).count()
            );
            ac_jeklik::boss_jeklik::events.RescheduleEvent(ac_jeklik::EVENT_SPAWN_FLYING_BATS, 10s);

        }

        // run original UpdateAI
        ac_jeklik::boss_jeklik::UpdateAI(diff);
    }
};

// Gurubashi Bat Rider (14750) - trash and boss summon are same creature ID
struct npc_batrider : public ac_jeklik::npc_batrider
{
    npc_batrider(Creature* creature) : ac_jeklik::npc_batrider(creature) {}

    EventMap sc_events;

    void Reset() override
    {
        sc_events.Reset();
        ac_jeklik::npc_batrider::Reset();
    }

    void UpdateAI(uint32 diff) override
    {
        //
        // Event hijacking
        //

        // only if this bat is in BAT_RIDER_MODE_BOSS
        if (ac_jeklik::npc_batrider::_mode == ac_jeklik::BAT_RIDER_MODE_BOSS)
        {
            // EVENT_BAT_RIDER_THROW_BOMB
            if (HijackEvent(ac_jeklik::EVENT_BAT_RIDER_THROW_BOMB, ac_jeklik::npc_batrider::events, sc_events))
            {
                LOG_DEBUG("module.Smallcraft", "sc::npc_batrider::UpdateAI: EVENT_BAT_RIDER_THROW_BOMB hijacked");
            }
        }

        sc_events.Update(diff);

        //
        // Event Handling
        //
        switch (sc_events.ExecuteEvent())
        {
            // time between bombs increased: 9s - 12s
            case ac_jeklik::EVENT_BAT_RIDER_THROW_BOMB:
                LOG_DEBUG("module.Smallcraft", "sc::npc_batrider::UpdateAI: EVENT_BAT_RIDER_THROW_BOMB");
                DoCastRandomTarget(ac_jeklik::SPELL_THROW_LIQUID_FIRE);
                sc_events.ScheduleEvent(ac_jeklik::EVENT_BAT_RIDER_THROW_BOMB, 9s, 12s);
                break;
            default:
                break;
        }

        // run original UpdateAI
        ac_jeklik::npc_batrider::UpdateAI(diff);
    }
};

class sc_boss_jeklik_GlobalScript : public GlobalScript
{
public:
    sc_boss_jeklik_GlobalScript() : GlobalScript("sc_boss_jeklik_GlobalScript") { }

    void OnLoadSpellCustomAttr(SpellInfo* spellInfo) override
    {
        switch (spellInfo->Id)
        {
            // Jeklik - Bat Aspect Boss
            // Blaze (23972)
            case (23972):
                LOG_DEBUG("module.Smallcraft", "sc::sc_boss_jeklik_GlobalScript:: Correcting spell {} ({})...", spellInfo->Id, spellInfo->SpellName[0]);
                spellInfo->AttributesEx2 |= SPELL_ATTR2_CANT_CRIT;
                break;
            default:
                break;
        }
    }
};

void load_sc_boss_jeklik()
{
    RegisterCreatureAI(boss_jeklik);
    RegisterCreatureAI(npc_batrider);
    new sc_boss_jeklik_GlobalScript();
}

