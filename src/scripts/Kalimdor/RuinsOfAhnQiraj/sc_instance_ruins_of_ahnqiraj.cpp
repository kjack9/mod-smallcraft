#include "Config.h"
#include "Log.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"

#include "sc_boss_kurinnaxx.cpp"

void load_sc_instance_ahnqiraj()
{
    LOG_DEBUG("module.Smallcraft", "SmallCraft: Vanilla/AhnQiraj is enabled.");

    if (sConfigMgr->GetOption<bool>("Smallcraft.RaidChanges.Vanilla.AhnQiraj.Kurinnaxx", true))
    {
        load_sc_boss_kurinnaxx();
    }
}