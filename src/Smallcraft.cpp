#include <vector>
#include "Config.h"
#include "Chat.h"
#include "Group.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
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

/**
 * @brief Hijack an event from one EventMap and add it to another EventMap.
 *
 * @param eventId The event ID to hijack.
 * @param oldMap The EventMap to hijack the event from.
 * @param newMap The EventMap to hijack the event to.
 * @param newTime The new time to schedule the event for.
 * @param cancelOriginal Whether or not to cancel the original event.
 * @return true If the event was hijacked.
 * @return false If the event was not hijacked.
 */
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

/**
 * @brief Add the scriptName to the the ObjectMgr::ScriptNames store.
 *
 * @param scriptName Name of the SmallCraft script to add.
 * @return uint32 Script ID for use in DB "ScriptName" overrides.
 */
uint32 AddScriptName(std::string scriptName)
{
    // The Script Names store in sObjectMgr is populated from script assignments in the DB.
    // Since we are dynamically overriding those entries, there is no way for the
    // ObjectMgr::LoadScriptNames() function to load our script name entries. So instead we
    // will add them to the store here if they don't already exist.

    if (sObjectMgr->GetScriptId(scriptName) == 0)
    {
        sObjectMgr->GetScriptNames().push_back(scriptName);
    }
    return sObjectMgr->GetScriptId(scriptName);
}

// Add all scripts
void AddSmallcraftScripts()
{
    new Smallcraft_PlayerScript();
}
