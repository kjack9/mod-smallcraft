#include "Config.h"
#include "Log.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"

namespace sc
{
// Load function declarations
void load_sc_boss_jeklik();
void load_sc_boss_thekal();
void load_boss_arlokk();

void load_sc_instance_zulgurub()
{
    LOG_DEBUG("module.SmallCraft", "SmallCraft: Vanilla/Zul'Gurub is enabled.");

    if (sConfigMgr->GetOption<bool>("Smallcraft.RaidChanges.Vanilla.ZulGurub.Jeklik", true))
        load_sc_boss_jeklik();
    if (sConfigMgr->GetOption<bool>("Smallcraft.RaidChanges.Vanilla.ZulGurub.Thekal", true))
        load_sc_boss_thekal();
    if (sConfigMgr->GetOption<bool>("Smallcraft.RaidChanges.Vanilla.ZulGurub.Arlokk", true))
        load_boss_arlokk();
}
} // namespace sc