#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "sc_boss_jeklik.cpp"
#include "sc_boss_thekal.cpp"

void load_sc_instance_zulgurub()
{
    load_sc_boss_jeklik();
    load_sc_boss_thekal();
}