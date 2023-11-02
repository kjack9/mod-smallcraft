#include "GameObjectAI.h"
#include "Log.h"
#include "MoveSplineInit.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Smallcraft.h"
#include "SmartAI.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "../../src/server/scripts/EasternKingdoms/ZulGurub/zulgurub.h"

namespace ac_thekal
{
    #include "../../src/server/scripts/EasternKingdoms/ZulGurub/boss_thekal.cpp"
}

class sc_boss_thekal_DatabaseScript : public DatabaseScript
{
public:
    sc_boss_thekal_DatabaseScript() : DatabaseScript("sc_boss_thekal_DatabaseScript") { }

    void OnAfterDatabaseLoadCreatureTemplates(std::vector<CreatureTemplate*> creatureTemplates) override
    {
        // Zealot Zath (11348) - Tiger Boss Add
        // make kite-able
        creatureTemplates[11348]->MechanicImmuneMask = 536936977; // can't be CC'd, but can be slowed/distracted/rooted/etc
    }

};

void load_sc_boss_thekal()
{
    LOG_DEBUG("module.Smallcraft", "SmallCraft: Vanilla/Zul'Gurub/High Priest Thekal is enabled.");

    new sc_boss_thekal_DatabaseScript();
}