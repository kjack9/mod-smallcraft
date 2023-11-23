#include "Config.h"
#include "Log.h"

void AddSmallcraftScripts();

namespace sc
{
// Vanilla raids
void load_instance_ahnqiraj();
void load_instance_molten_core();
void load_instance_zulgurub();
}

// TempSpells
void load_tempspells();

void Addmod_smallcraftScripts()
{
    if (sConfigMgr->GetOption<bool>("Smallcraft.Enable", false))
    {
        AddSmallcraftScripts();
        LOG_INFO("module.SmallCraft", "SmallCraft is enabled.");

        // Vanilla raids
        if (sConfigMgr->GetOption<bool>("Smallcraft.RaidChanges.Vanilla.AhnQiraj", true))
            sc::load_instance_ahnqiraj();

        if (sConfigMgr->GetOption<bool>("Smallcraft.RaidChanges.Vanilla.MoltenCore", true))
            sc::load_instance_molten_core();

        if (sConfigMgr->GetOption<bool>("Smallcraft.RaidChanges.Vanilla.ZulGurub", true))
            sc::load_instance_zulgurub();

        // TempSpells
        if (sConfigMgr->GetOption<bool>("Smallcraft.TempSpells.Enable", true))
            load_tempspells();
    }
}

