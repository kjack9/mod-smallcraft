/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "scripts/EasternKingdoms/ZulGurub/sc_boss_jeklik.cpp"

// Add player scripts
class Smallcraft_PlayerScript : public PlayerScript
{
public:
    Smallcraft_PlayerScript() : PlayerScript("Smallcraft_PlayerScript") { }

    void OnLogin(Player* player) override
    {
        if (sConfigMgr->GetOption<bool>("Smallcraft.Enable", false))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Smallcraft is enabled.");
        }
    }
};

// Add database scripts
class Smallcraft_DatabaseScript : public DatabaseScript
{
public:
    Smallcraft_DatabaseScript() : DatabaseScript("Smallcraft_DatabaseScript") { }

    void OnAfterDatabaseLoadCreatureTemplates(std::vector<CreatureTemplate*> creatureTemplates) override
    {
        //
        // ZG
        //

        // Zealot Zath (11348) - Tiger Boss Add
        // make Zath kite-able
        creatureTemplates[11348]->MechanicImmuneMask = 536936977; // can't be CC'd, but can be slowed/distracted/rooted/etc

    }
};

// Add all scripts
void AddSmallcraftScripts()
{
    new Smallcraft_PlayerScript();
    new Smallcraft_DatabaseScript();
    AddSC_boss_jeklikScripts();
}
