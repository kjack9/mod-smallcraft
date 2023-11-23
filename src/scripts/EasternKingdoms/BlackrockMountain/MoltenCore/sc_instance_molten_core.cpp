#include "Config.h"
#include "Log.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"

namespace sc
{
// Load function declarations
void load_boss_magmadar();

void load_instance_molten_core()
{
    LOG_DEBUG("module.SmallCraft", "SmallCraft: Vanilla/Molten Core is enabled.");

    if (sConfigMgr->GetOption<bool>("Smallcraft.RaidChanges.Vanilla.MoltenCore.Magmadar", true))
        load_boss_magmadar();

}
} // namespace sc