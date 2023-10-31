#include "GameObjectAI.h"
#include "MoveSplineInit.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Smallcraft.h"
#include "SmartAI.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TaskScheduler.h"

namespace ac
{
    #include "../../src/server/scripts/EasternKingdoms/ZulGurub/zulgurub.h"
    #include "../../src/server/scripts/EasternKingdoms/ZulGurub/boss_jeklik.cpp"
}

// High Priestess Jeklik (14517)
struct boss_jeklik : public ac::boss_jeklik
{
    boss_jeklik(Creature* creature) : ac::boss_jeklik(creature) { }

    void UpdateAI(uint32 diff) override
    {
        // EVENT_SPAWN_FLYING_BATS
        // Reschedule event to 10s
        if
        (
                ac::boss_jeklik::events.GetTimeUntilEvent(ac::EVENT_SPAWN_FLYING_BATS) > 10s &&
                ac::boss_jeklik::events.GetTimeUntilEvent(ac::EVENT_SPAWN_FLYING_BATS) != Milliseconds::max()
        )
        {
            LOG_DEBUG("module.Smallcraft", "sc::boss_jeklik::UpdateAI: EVENT_SPAWN_FLYING_BATS rescheduled from {}ms to 10s",
                ac::boss_jeklik::events.GetTimeUntilEvent(ac:: EVENT_SPAWN_FLYING_BATS).count()
            );
            ac::boss_jeklik::events.RescheduleEvent(ac::EVENT_SPAWN_FLYING_BATS, 10s);

        }

        // run original UpdateAI
        ac::boss_jeklik::UpdateAI(diff);
    }
};

// Gurubashi Bat Rider (14750) - trash and boss summon are same creature ID
struct npc_batrider : public ac::npc_batrider
{
    npc_batrider(Creature* creature) : ac::npc_batrider(creature) {}

    EventMap sc_events;

    void Reset() override
    {
        sc_events.Reset();
        ac::npc_batrider::Reset();
    }

    void UpdateAI(uint32 diff) override
    {
        //
        // Event hijacking
        //

        // only if this bat is in BAT_RIDER_MODE_BOSS
        if (ac::npc_batrider::_mode == ac::BAT_RIDER_MODE_BOSS)
        {
            // EVENT_BAT_RIDER_THROW_BOMB
            if (HijackEvent(ac::EVENT_BAT_RIDER_THROW_BOMB, ac::npc_batrider::events, sc_events))
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
            case ac::EVENT_BAT_RIDER_THROW_BOMB:
                LOG_DEBUG("module.Smallcraft", "sc::npc_batrider::UpdateAI: EVENT_BAT_RIDER_THROW_BOMB");
                DoCastRandomTarget(ac::SPELL_THROW_LIQUID_FIRE);
                sc_events.ScheduleEvent(ac::EVENT_BAT_RIDER_THROW_BOMB, 9s, 12s);
                break;
            default:
                break;
        }

        // run original UpdateAI
        ac::npc_batrider::UpdateAI(diff);
    }
};

void load_sc_boss_jeklik()
{
    RegisterCreatureAI(boss_jeklik);
    RegisterCreatureAI(npc_batrider);
}

