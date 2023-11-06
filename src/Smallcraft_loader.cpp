#include "Config.h"
#include "Log.h"

void AddSmallcraftScripts();
void load_sc_instance_zulgurub();
void load_sc_instance_molten_core();
void load_sc_tempspells();

void Addmod_smallcraftScripts()
{
    if (sConfigMgr->GetOption<bool>("Smallcraft.Enable", false))
    {
        AddSmallcraftScripts();
        LOG_INFO("module.Smallcraft", "SmallCraft is enabled.");

        // Vanilla raids

        if (sConfigMgr->GetOption<bool>("Smallcraft.RaidChanges.Vanilla.ZulGurub", true))
            load_sc_instance_zulgurub();

        if (sConfigMgr->GetOption<bool>("Smallcraft.RaidChanges.Vanilla.MoltenCore", true))
            load_sc_instance_molten_core();

        if (sConfigMgr->GetOption<bool>("Smallcraft.TempSpells.Enable", true))
            load_sc_tempspells();
    }
}

