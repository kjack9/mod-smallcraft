#include "DBCStores.h"
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
#include "../../src/server/scripts/EasternKingdoms/BlackrockMountain/MoltenCore/molten_core.h"

namespace ac
{
    #include "../../src/server/scripts/EasternKingdoms/BlackrockMountain/MoltenCore/boss_magmadar.cpp"
}

namespace sc
{
class boss_magmadar_GlobalScript : public GlobalScript
{
public:
    boss_magmadar_GlobalScript() : GlobalScript("boss_magmadar_GlobalScript") { }

    void OnLoadSpellCustomAttr(SpellInfo* spellInfo) override
    {
        switch (spellInfo->Id)
        {
            // Enrage (19451)
            // Reduce duration to 2.5s
            case (19451):
                LOG_DEBUG("module.SmallCraft", "sc::boss_magmadar_GlobalScript:: Modifying spell {} ({})...", spellInfo->Id, spellInfo->SpellName[0]);
                spellInfo->DurationEntry = sSpellDurationStore.LookupEntry(66); // 2500ms
                break;
            default:
                break;
        }
    }
};

void load_boss_magmadar()
{
    LOG_DEBUG("module.SmallCraft", "SmallCraft: Vanilla/Molten Core/Magmadar is enabled.");

    new boss_magmadar_GlobalScript();
}
} // namespace sc