#include "Config.h"
#include "Log.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"

#include "sc_boss_magmadar.cpp"

void load_sc_instance_molten_core()
{
    LOG_DEBUG("module.Smallcraft", "SmallCraft: Vanilla/Molten Core is enabled.");

    if (sConfigMgr->GetOption<bool>("Smallcraft.RaidChanges.Vanilla.MoltenCore.Magmadar", true))
        load_sc_boss_magmadar();

}