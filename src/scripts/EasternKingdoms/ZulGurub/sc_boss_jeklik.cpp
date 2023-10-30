#include "GameObjectAI.h"
#include "MoveSplineInit.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SmartAI.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "WaypointMgr.h"

namespace orig
{
    #include "../../src/server/scripts/EasternKingdoms/ZulGurub/zulgurub.h"
    #include "../../src/server/scripts/EasternKingdoms/ZulGurub/boss_jeklik.cpp"
}

// High Priestess Jeklik (14517)
struct boss_jeklik : public orig::boss_jeklik
{
    boss_jeklik(Creature* creature) : orig::boss_jeklik(creature) { }

    uint8 realBatRidersCount = 0;

    void UpdateAI(uint32 diff) override
    {
        orig::boss_jeklik::UpdateAI(diff);

        // // we want only 1 bat rider
        // if (orig::boss_jeklik::bat == 1)
        // {
        //     orig::boss_jeklik::batRidersCount = 2;
        //     LOG_DEBUG("module.Smallcraft", "sc::boss_jeklik::UpdateAI: Cancelled 2nd bat rider.");
        // }
    }
};

void AddSC_boss_jeklikScripts()
{
    RegisterCreatureAI(boss_jeklik);
}

