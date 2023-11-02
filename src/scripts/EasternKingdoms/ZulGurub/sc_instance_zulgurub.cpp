#include "Config.h"
#include "Log.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "sc_boss_jeklik.cpp"
#include "sc_boss_thekal.cpp"

void load_sc_instance_zulgurub()
{
    LOG_DEBUG("module.Smallcraft", "SmallCraft: Vanilla/Zul'Gurub is enabled.");

    if (sConfigMgr->GetOption<bool>("Smallcraft.RaidChanges.Vanilla.ZulGurub.Jeklik", true))
        load_sc_boss_jeklik();
    if (sConfigMgr->GetOption<bool>("Smallcraft.RaidChanges.Vanilla.ZulGurub.Thekal", true))
        load_sc_boss_thekal();
}