/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "SpellInfo.h"

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

bool HijackEvent(uint32 eventId, EventMap &oldMap, EventMap &newMap, std::chrono::duration<int64_t, std::milli> newTime = Milliseconds::max(), bool cancelOriginal = true)
{
    if (oldMap.GetTimeUntilEvent(eventId) != Milliseconds::max())
    {
        if (newTime == Milliseconds::max())
        {
            newTime = oldMap.GetTimeUntilEvent(eventId);
        }

        newMap.ScheduleEvent(eventId, newTime);
        if (cancelOriginal) { oldMap.CancelEvent(eventId); }
        return true;
    }

    return false;
}

// Add all scripts
void AddSmallcraftScripts()
{
    new Smallcraft_PlayerScript();
}