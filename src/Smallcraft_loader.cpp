/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "Config.h"

// From SC
void AddSmallcraftScripts();
void load_sc_instance_zulgurub();

// Add all
// additionally replace all '-' in the module folder name with '_' here
void Addmod_smallcraftScripts()
{
    if (sConfigMgr->GetOption<bool>("Smallcraft.Enable", false))
    {
        AddSmallcraftScripts();
        load_sc_instance_zulgurub();
    }
}

