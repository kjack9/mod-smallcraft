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

namespace sc
{
/**
 * @brief Check if a spell has an aura of the specified type.
 *
 * @param spellInfo The SpellInfo to check.
 * @param auraType The AuraType to check for.
 * @param log Whether or not to log debug messages.
 * @return true If the spell has an aura of the specified type.
 * @return false If the spell does not have an aura of the specified type.
 */
bool SpellHasAuraWithType(SpellInfo const* spellInfo, AuraType auraType, bool log = false)
{
    // if the spell is not defined, return false
    if (!spellInfo)
    {
        if (log) { LOG_DEBUG("module.AutoBalance_DamageHealingCC", "AutoBalance_UnitScript::_isAuraWithEffectType: SpellInfo is null, returning false."); }
        return false;
    }

    // if the spell doesn't have any effects, return false
    if (!spellInfo->GetEffects().size())
    {
        if (log) { LOG_DEBUG("module.AutoBalance_DamageHealingCC", "AutoBalance_UnitScript::_isAuraWithEffectType: SpellInfo has no effects, returning false."); }
        return false;
    }

    // iterate through the spell effects
    for (SpellEffectInfo effect : spellInfo->GetEffects())
    {
        // if the effect is not an aura, continue to next effect
        if (!effect.IsAura())
        {
            if (log) { LOG_DEBUG("module.AutoBalance_DamageHealingCC", "AutoBalance_UnitScript::_isAuraWithEffectType: SpellInfo has an effect that is not an aura, continuing to next effect."); }
            continue;
        }

        if (effect.ApplyAuraName == auraType)
        {
            // if the effect is an aura of the target type, return true
            LOG_DEBUG("module.AutoBalance_DamageHealingCC", "AutoBalance_UnitScript::_isAuraWithEffectType: SpellInfo has an aura of the target type, returning true.");
            return true;
        }
    }

    // if no aura effect of type auraType was found, return false
    if (log) { LOG_DEBUG("module.AutoBalance_DamageHealingCC", "AutoBalance_UnitScript::_isAuraWithEffectType: SpellInfo has no aura of the target type, returning false."); }
    return false;
}

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
 * @brief Send a chat message from a creature.
 *
 * @param creature The creature to send the message from.
 * @param message The message to send.
 * @param msgType The type of message to send.
 * @param distance The distance to send the message.
 */
void Talk(Creature* creature, std::string message, ChatMsg msgType, float distance = 1000.0f)
{
    creature->Talk(std::string_view(message), msgType, LANG_UNIVERSAL, distance, nullptr);
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

// void AddSpellScript(uint32 spellId, std::string scriptName)
// {
//     // get the spell script store from sObjectMgr
//     SpellScriptsContainer& spellScriptStore = sObjectMgr->GetSpellScripts();

//     // add the spell script to the ScriptNames store
//     uint32 scriptId = AddScriptName(scriptName);

//     // if the spell script store already has an entry for this spell ID, replace it
//     auto it = spellScriptStore.find(spellId);
//     if (it != spellScriptStore.end())
//     {
//         it->second = scriptId;
//     }
//     else
//     {
//         // otherwise, add a new entry
//         spellScriptStore.insert(std::make_pair(spellId, scriptId));
//     }
// }

} // namespace sc

// Add all scripts
void AddSmallcraftScripts()
{
    new Smallcraft_PlayerScript();
}
