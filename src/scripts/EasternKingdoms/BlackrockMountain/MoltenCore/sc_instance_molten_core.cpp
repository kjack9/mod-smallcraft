#include "Config.h"
#include "Log.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"

// Load function declarations
void load_sc_boss_magmadar();

void load_sc_instance_molten_core()
{
    LOG_DEBUG("module.Smallcraft", "SmallCraft: Vanilla/Molten Core is enabled.");

    if (sConfigMgr->GetOption<bool>("Smallcraft.RaidChanges.Vanilla.MoltenCore.Magmadar", true))
        load_sc_boss_magmadar();

}