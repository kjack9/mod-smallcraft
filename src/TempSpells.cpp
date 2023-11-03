#include "boost/algorithm/string.hpp"
#include "Config.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "SmartEnum.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SmartEnum.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

#include "Smallcraft.h"
#include "TempSpells.h"

/***********************************\
* Smallcraft_TempSpells_GroupScript *
\***********************************/

/**
 * @brief Called after a member is added to a group.
 *
 * @param group The group the member was added to.
 * @param guid The GUID of the member that was added.
 */
void Smallcraft_TempSpells_GroupScript::OnAddMember(Group* group, ObjectGuid guid)
{
    Player* addedPlayer = ObjectAccessor::FindPlayer(guid);

    LOG_DEBUG("module.Smallcraft", "Smallcraft:: ----------------------------------------------------");
    LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:OnAddMember():: {} added to {}'s group.",
        addedPlayer ? addedPlayer->GetName() : "Unknown",
        group->GetLeaderName()
    );

    Smallcraft_TempSpells::UpdateGroupMembers(group);
    Smallcraft_TempSpells::AnalyzeGroup(group, true);
    Smallcraft_TempSpells::UpdateGroup(group);

}

/**
 * @brief Called after a member is removed from or leaves a group.
 *
 * @param group The group the member was removed from.
 * @param guid The GUID of the member that was removed.
 */
void Smallcraft_TempSpells_GroupScript::OnRemoveMember(Group* group, ObjectGuid guid, RemoveMethod /*method*/, ObjectGuid /*kicker*/, const char* /*reason*/)
{
    Player* removedPlayer = ObjectAccessor::FindPlayer(guid);

    LOG_DEBUG("module.Smallcraft", "Smallcraft:: ----------------------------------------------------");
    LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:OnRemoveMember():: {} leaves {}'s group.",
        removedPlayer ? removedPlayer->GetName() : "Unknown",
        group->GetLeaderName()
    );

    // get (or create) the group's SmallcraftInfo
    SmallcraftGroupInfo* groupInfo = group->CustomData.GetDefault<SmallcraftGroupInfo>("SmallcraftGroupInfo");

    // if the removed member is in the member list, remove them
    if (groupInfo->members.find(guid) != groupInfo->members.end())
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:OnRemoveMember():: {} is in the member list. Removing them.",
            removedPlayer ? removedPlayer->GetName() : "Unknown"
        );
        groupInfo->members.erase(guid);
    }

    Smallcraft_TempSpells::UpdateGroupMembers(group);
    Smallcraft_TempSpells::AnalyzeGroup(group, true);
    Smallcraft_TempSpells::UpdateGroup(group);
}

/************************************\
* Smallcraft_TempSpells_PlayerScript *
\************************************/

/**
 * @brief Called after a player logs in.
 *
 * @param player The player that logged in.
 */
void Smallcraft_TempSpells_PlayerScript::OnLogin(Player* player)
{
    // if the player is gone, return
    if (!player)
        return;

    // get the player's group
    Group* group = player->GetGroup();

    // if the player is in a group, update the group's member list
    if (group)
    {

        LOG_DEBUG("module.Smallcraft", "Smallcraft:: ----------------------------------------------------");
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_PlayerScript:OnLogin():: {} logged in.",
            player->GetName()
        );

        // if the group composition or talent specs change, analyze (force) and update the group
        if (Smallcraft_TempSpells::UpdateGroupMembers(group))
        {
            Smallcraft_TempSpells::AnalyzeGroup(group, true);
            Smallcraft_TempSpells::UpdateGroup(group);
        }
    }
}

/**
 * @brief Called after a player changes their spec using the dual spec feature.
 *
 * @param player The player that changed their spec.
 * @param newSlot The new spec slot.
 */
void Smallcraft_TempSpells_PlayerScript::OnAfterSpecSlotChanged(Player* player, uint8 newSlot)
{
    // if the player is gone, return
    if (!player)
        return;

    // get the player's group
    Group* group = player->GetGroup();

    // if the player is in a group, update the group's member list
    if (group)
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft:: ----------------------------------------------------");
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_PlayerScript:OnAfterSpecSlotChanged():: {} changed their spec slot to slot {}.",
            player->GetName(),
            newSlot
        );

        Smallcraft_TempSpells::UpdateGroupMembers(group);
        Smallcraft_TempSpells::AnalyzeGroup(group, true);
        Smallcraft_TempSpells::UpdateGroup(group);
    }
}

/**
 * @brief Called after a player is resurrected.
 *
 * @param player The player that was resurrected.
 * @param restore_percent The percentage of health and mana the player was restored to.
 * @param applySickness Whether or not the player was given resurrection sickness.
 */
void Smallcraft_TempSpells_PlayerScript::OnPlayerResurrect(Player* player, float /*restore_percent*/, bool /*applySickness*/)
{
    // if the player is gone, return
    if (!player)
        return;

    // get the player's group
    Group* group = player->GetGroup();

    // if the player is in a group, update the group's member list
    if (group)
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft:: ----------------------------------------------------");
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_PlayerScript:OnPlayerResurrect():: {} was resurrected. Updating player.",
            player->GetName()
        );

        Smallcraft_TempSpells::UpdatePlayer(player);
    }
}


/************************************\
* Smallcraft_TempSpells_AllMapScript *
\************************************/

/**
 * @brief Called after a player enters a map.
 *
 * @param map The map the player entered.
 * @param player The player that entered the map.
 */
void Smallcraft_TempSpells_AllMapScript::OnPlayerEnterAll(Map* map, Player* player)
{
    _handleEnterLeaveAll(map, player, true);
}

/**
 * @brief Called after a player leaves a map.
 *
 * @param map The map the player left.
 * @param player The player that left the map.
 */
void Smallcraft_TempSpells_AllMapScript::OnPlayerLeaveAll(Map* map, Player* player)
{
    _handleEnterLeaveAll(map, player, false);
}

/**
 * @brief Handles enter and leave events for all players and maps.
 *
 * @param map The map the player entered or left.
 * @param player The player that entered or left.
 * @param entering True if entering, False if leaving.
 */
void Smallcraft_TempSpells_AllMapScript::_handleEnterLeaveAll(Map* map, Player* player, bool entering)
{
    std::string enterLeave = entering ? "enters" : "leaves";

    // only if the player and map are valid
    if (!player || !map)
        return;

    LOG_DEBUG("module.Smallcraft", "Smallcraft:: ----------------------------------------------------");

    LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_AllMapScript:_handleEnterLeaveAll({}):: {} {} {}.",
        enterLeave,
        player->GetName(),
        enterLeave,
        map->GetMapName()
    );

    // get the player's group
    Group* group = player->GetGroup();

    if (!group)
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_AllMapScript:_handleEnterLeaveAll({}):: {} is not in a group.",
            enterLeave,
            player->GetName()
        );

        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_AllMapScript:_handleEnterLeaveAll({}):: Remove {}'s temp spells.",
            enterLeave,
            player->GetName()
        );

        return;
    }

    // if the group composition or talent specs change, analyze (force) and update the group
    if (Smallcraft_TempSpells::UpdateGroupMembers(group))
    {
        // if the analysis indicates the group needs to be updated, update it
        if (Smallcraft_TempSpells::AnalyzeGroup(group, true))
        {
            Smallcraft_TempSpells::UpdateGroup(group);
        }
    }

    Smallcraft_TempSpells::UpdatePlayer(player);
}

/************************\
 * Smallcraft_TempSpells *
\************************/

/**
 * @brief Updates the internal tracking list that contains group members.
 *
 * @param group The group to update
 */
bool Smallcraft_TempSpells::UpdateGroupMembers(Group* group)
{
    // get (or create) the group's SmallcraftInfo
    SmallcraftGroupInfo* groupInfo = group->CustomData.GetDefault<SmallcraftGroupInfo>("SmallcraftGroupInfo");

    // tracking bool to see if the group list was updated
    bool groupListUpdated = false;

    // iterate through the group members and add them to the internal tracking list as appropriate
    // do not re-add members that are already in the list
    group->DoForAllMembers([&](Player* player)
    {
        ObjectGuid guid = player->GetGUID();

        // if the member is not already in the member list, add them
        if (groupInfo->members.find(guid) == groupInfo->members.end())
        {
            SmallcraftGroupMemberInfo newMemberInfo;
            newMemberInfo.guid = guid;
            newMemberInfo.player = player;
            newMemberInfo.myClass = (Classes)(player->getClass());
            newMemberInfo.powerType = (Powers)(player->getPowerType());
            newMemberInfo.name = player->GetName();
            newMemberInfo.talentSpec = (uint8)player->GetSpec(player->GetActiveSpec());

            LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_updateGroupMembers:: Adding {} ({} {})",
                newMemberInfo.name,
                talentSpecInfo.find(newMemberInfo.talentSpec) != talentSpecInfo.end() ? talentSpecInfo.at(newMemberInfo.talentSpec).description : "Unknown",
                EnumUtils::ToString((Classes)(newMemberInfo.myClass)).Title
            );

            groupInfo->members[guid] = newMemberInfo;

            groupListUpdated = true;
        }
        // this member is already in the member list, let's make sure they haven't changed
        else
        {
            // get the member's current SmallcraftGroupMemberInfo
            SmallcraftGroupMemberInfo* memberInfo = &groupInfo->members[guid];

            // see if the member's talentSpec has changed
            uint32 newTalentSpec = (uint8)player->GetSpec(player->GetActiveSpec());
            if (memberInfo->talentSpec != newTalentSpec)
            {
                LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_updateGroupMembers:: {}'s talent spec has changed from {} ({}) to {} ({}).",
                    memberInfo->name,
                    talentSpecInfo.find(memberInfo->talentSpec) != talentSpecInfo.end() ? talentSpecInfo.at(memberInfo->talentSpec).description : "Unknown",
                    memberInfo->talentSpec,
                    talentSpecInfo.find(newTalentSpec) != talentSpecInfo.end() ? talentSpecInfo.at(newTalentSpec).description : "Unknown",
                    newTalentSpec
                );

                memberInfo->talentSpec = newTalentSpec;
                groupListUpdated = true;
            }

            // see if this member is scheduled for an update
            if (memberInfo->scheduleUpdate)
            {
                LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_updateGroupMembers:: {} is scheduled for an update.",
                    memberInfo->name
                );
                groupListUpdated = true;
            }
        }
    });

    if (!groupListUpdated)
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_updateGroupMembers:: {}'s group is unchanged.",
            group->GetLeaderName()
        );
    }

    return groupListUpdated;
}

/**
 * @brief Analyzes the group to see what dispel types need to be granted to group members.
 *
 * @param group The group to analyze.
 * @return Whether or not the group needs to be updated.
 */
bool Smallcraft_TempSpells::AnalyzeGroup(Group* group, bool force)
{
    LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:AnalyzeGroup()");
    LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:AnalyzeGroup:: Group Leader = {}", group->GetLeaderName());

    // get (or create) the group's SmallcraftInfo
    SmallcraftGroupInfo* groupInfo = group->CustomData.GetDefault<SmallcraftGroupInfo>("SmallcraftGroupInfo");

    // count the number of online players
    uint8_t numOnlinePlayers = 0;
    group->DoForAllMembers([&numOnlinePlayers](Player* /*player*/)
    {
        // DoForAllMembers skips offline members, so we can just increment the counter
        numOnlinePlayers++;
    });

    LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:AnalyzeGroup:: Group Members | Tracked / Online / Total = {} / {} / {}",
        groupInfo->members.size(),
        numOnlinePlayers,
        group->GetMembersCount()
    );

    // reset the old lists of dispel types
    std::set<DispelType> oldDispelTypesWeHave = groupInfo->dispelTypesWeHave;
    groupInfo->dispelTypesWeHave.clear();
    groupInfo->dispelTypesWeAreMissing.clear();

    // Iterate the players in the member list. Add the dispels each player can do to the groups information.
    for (auto& member : groupInfo->members)
    {
        SmallcraftGroupMemberInfo* memberInfo = &member.second;

        std::string myDispelTypesString = "";
        if (memberInfo->talentSpec)
        {
            for (DispelType dispelType : talentSpecInfo.at(memberInfo->talentSpec).dispelTypes)
            {
                myDispelTypesString += dispelTypeDescriptions.at(dispelType) + " ";
            }

            // add the dispel types for this spec to the set of dispels we have, then deduplicate
            for (DispelType dispelType : talentSpecInfo.at(memberInfo->talentSpec).dispelTypes)
            {
                groupInfo->dispelTypesWeHave.insert(dispelType);
            }
        }

        // log output the dispel types this member has
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:AnalyzeGroup:: {} | is a {} {} ({}). They are {}. Their spec can cast dispel types: {}",
            memberInfo->name,
            talentSpecInfo.find(memberInfo->talentSpec)->second.description,
            EnumUtils::ToString((Classes)(memberInfo->myClass)).Title,
            playerRoleDescriptions.at(talentSpecInfo.find(memberInfo->talentSpec)->second.role),
            memberInfo->player->IsInWorld() ? "ONLINE" : "OFFLINE",
            myDispelTypesString
        );

        // if the player is scheduled for an update, force update
        if (memberInfo->scheduleUpdate)
        {
            LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:AnalyzeGroup:: {} is scheduled for an update.",
                memberInfo->name
            );
            force = true;
        }
    }

    std::string groupDispelTypesString = "";
    for (DispelType dispelType : groupInfo->dispelTypesWeHave)
    {
        groupDispelTypesString += dispelTypeDescriptions.at(dispelType) + " ";
    }

    // log output the dispel types the group has
    LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:AnalyzeGroup:: The group has: {}",
        groupDispelTypesString
    );

    std::string groupMissingDispelTypesString = "";
    for (DispelType dispelType : DesiredDispelTypes)
    {
        if (groupInfo->dispelTypesWeHave.find(dispelType) == groupInfo->dispelTypesWeHave.end())
        {
            groupInfo->dispelTypesWeAreMissing.insert(dispelType);
            groupMissingDispelTypesString += dispelTypeDescriptions.at(dispelType) + " ";
        }
    }

    // log output the dispel types the group is missing
    LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:AnalyzeGroup:: The group is missing: {}",
        groupMissingDispelTypesString
    );

    // if the force flag is set, return true
    if (force)
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:AnalyzeGroup:: The group will be updated (forced).");
        return true;
    }
    // if the group's missing dispel types list has changed, return true
    // if the group has no dispel types, return true
    else if ((groupInfo->dispelTypesWeHave.size() == 0) || (oldDispelTypesWeHave != groupInfo->dispelTypesWeHave))
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:AnalyzeGroup:: The group needs to be updated.");
        return true;
    }
    else
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:AnalyzeGroup:: The group does NOT need to be updated.");
        return false;
    }
}

void Smallcraft_TempSpells::UpdateGroup(Group* group)
{
    LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_updateGroup()");

    // get (or create) the group's SmallcraftInfo
    SmallcraftGroupInfo* groupInfo = group->CustomData.GetDefault<SmallcraftGroupInfo>("SmallcraftGroupInfo");

    // reset the existing temp spells on all members
    for (auto& member : groupInfo->members)
    {
        SmallcraftGroupMemberInfo* memberInfo = &member.second;
        memberInfo->tempSpells.clear();
        LOG_TRACE("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_updateGroup:: {}'s temp spell list has been reset.",
            memberInfo->name
        );
    }

    // iterate through the dispel types we need (dispelTypesWeAreMissing) and determine which player(s) should receive the temp spell
    for (DispelType dispelType : groupInfo->dispelTypesWeAreMissing)
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_updateGroup:: The group needs a {} dispel.",
            dispelTypeDescriptions.at(dispelType)
        );

        // get the selected members for the dispel type
        std::vector<SmallcraftGroupMemberInfo*> selectedMembers = _getMembersForTempSpell(group, dispelType);

        // log output the selected members
        if (selectedMembers.size() > 0)
        {
            std::string selectedMembersString = "";
            for (SmallcraftGroupMemberInfo* selectedMember : selectedMembers)
            {
                selectedMembersString += selectedMember->name + " ";
            }
            LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_updateGroup:: Selected members for dispel type {}: {}",
                dispelTypeDescriptions.at(dispelType),
                selectedMembersString
            );
        }
        else
        {
            LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_updateGroup:: No suitable members could be found for dispel type {}.",
                dispelTypeDescriptions.at(dispelType)
            );
        }

        // add the appropriate spell ID to the candidate's tempSpells list
        for (SmallcraftGroupMemberInfo* selectedMember : selectedMembers)
        {
            uint32 newSpellId = sConfigMgr->GetOption<uint32>(dispelTypeToConfigOptionString.at(dispelType), 0);
            if (newSpellId)
            {
                LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_updateGroup:: Add spell {} ({}) to {}'s temp spells.",
                    newSpellId,
                    sSpellMgr->GetSpellInfo(newSpellId)->SpellName[0],
                    selectedMember->name
                );
                selectedMember->tempSpells.insert(newSpellId);
            }
            else
            {
                LOG_ERROR("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_updateGroup:: Invalid spell ID for dispel type {}.",
                    dispelTypeDescriptions.at(dispelType)
                );
            }
        }
    }

    // Update players
    UpdatePlayers(group);
}

/**
 * @brief Update the spellbooks of the players in the group. May add or remove spells.
 *
 * @param group The group to update.
 */
void Smallcraft_TempSpells::UpdatePlayers(Group* group)
{
    // iterate all group members
    group->DoForAllMembers([&](Player* player)
    {
        UpdatePlayer(player);
    });
}


void Smallcraft_TempSpells::UpdatePlayer(Player* player)
{
    // get the player's group and member info
    SmallcraftGroupInfo* groupInfo;
    SmallcraftGroupMemberInfo* memberInfo;

    // if the player is gone, do nothing
    if (!player || !player->GetMap())
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer(Unknown?):: is inaccessible or otherwise not in the world.");
        return;
    }

    Map* map = player->GetMap();

    // if the player isn't in a group
    if (!player->GetGroup())
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: is not in a group.",
            player->GetName()
        );

        memberInfo = new SmallcraftGroupMemberInfo();
    }
    // player exists, is in a group, and is in an instance that matches the configured settings
    else if
    (
        (
            (map->IsDungeon() && !map->IsRaid() && sConfigMgr->GetOption<bool>("Smallcraft.TempSpells.Enable.Dungeons", false)) ||
            (map->IsRaid() && sConfigMgr->GetOption<bool>("Smallcraft.TempSpells.Enable.Raids", false)) ||
            (map->IsBattleground() && sConfigMgr->GetOption<bool>("Smallcraft.TempSpells.Enable.Battlegrounds", false))
        )

    )
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: is in a group and in an instance that matches the configured settings.",
            player->GetName()
        );

        Group* group = player->GetGroup();
        groupInfo = group->CustomData.GetDefault<SmallcraftGroupInfo>("SmallcraftGroupInfo");
        memberInfo = &groupInfo->members[player->GetGUID()];

        // if this group doesn't contain info about them, something has gone wrong
        if (!memberInfo)
        {
            LOG_ERROR("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: is not in the group's member list.",
                player->GetName()
            );
            return;
        }
    }
    else
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: is in a group but not in an instance that matches the configured settings.",
            player->GetName()
        );

        memberInfo = new SmallcraftGroupMemberInfo();
    }

    // if the player is dead, we don't want to change their spells
    // this allows them to release and run back to the dungeon without their spells being shuffled around every time they die
    if (!player->IsAlive() && player->GetGroup())
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: is not alive. No changes while dead and in a group.",
            player->GetName()
        );

        // schedule an update for next time
        memberInfo->scheduleUpdate = true;
        return;
    }
    else
    {
        // clear the schedule update flag
        memberInfo->scheduleUpdate = false;
    }

    // connect to the Characters DB and retrieve a uint32 set of all the spell IDs that have been granted for this player's GUID
    std::set<uint32> alreadyGrantedSpellIDs;
    std::string queryString = "SELECT SpellID FROM custom_smallcraft_tempspells WHERE guid = " + std::to_string(player->GetGUID().GetRawValue());
    LOG_TRACE("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: Query: {}",
        player->GetName(),
        queryString
    );

    QueryResult result = CharacterDatabase.Query(queryString);

    // if the query returned a result, iterate through the results and add them to the alreadyGrantedSpellIDs set
    if (result)
    {
        LOG_TRACE("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: Query returned {} results.",
            player->GetName(),
            result->GetRowCount()
        );
        do
        {
            Field* fields = result->Fetch();
            alreadyGrantedSpellIDs.insert(fields[0].Get<uint32>());
        } while (result->NextRow());
    }
    else
    {
        LOG_TRACE("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: Query returned no results.",
            player->GetName()
        );
    }

    if (alreadyGrantedSpellIDs.size())
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: has {} spells in the database.",
            player->GetName(),
            alreadyGrantedSpellIDs.size()
        );

        // create a string that contains all the spell IDs in the alreadyGrantedSpellIDs set delimited by space
        std::string alreadyGrantedSpellIDsString = "";
        for (uint32 spellID : alreadyGrantedSpellIDs)
        {
            alreadyGrantedSpellIDsString += std::to_string(spellID) + " ";
        }

        LOG_TRACE("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: already granted spells: {}",
            player->GetName(),
            alreadyGrantedSpellIDsString
        );
    }
    else
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: has no spells in the database.",
            player->GetName()
        );
    }

    std::set<uint32> spellsToRemove = alreadyGrantedSpellIDs;
    std::set<uint32> spellsToAdd = memberInfo->tempSpells;

    // remove any overlap between the two sets from the spells to remove
    for (uint32 spellID : alreadyGrantedSpellIDs)
    {
        if (spellsToAdd.find(spellID) != spellsToAdd.end())
        {
            spellsToRemove.erase(spellID);
        }
    }

    // if there are any spells to remove, remove them
    if (spellsToRemove.size())
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: has {} spells to remove.",
            player->GetName(),
            spellsToRemove.size()
        );

        // notify the player that they are having spells removed
        if (spellsToRemove.size() == 1)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("|cffc3dbff [Smallcraft]|r|cffffff00 A spell fades away from your spellbook... |r");
        }
        else
        {
            ChatHandler(player->GetSession()).PSendSysMessage("|cffc3dbff [Smallcraft]|r|cffffff00 Several spells fade away from your spellbook... |r");
        }

        // create a string that contains all the spell IDs in the spellsToRemove set delimited by space
        std::string spellsToRemoveString = "";
        for (uint32 spellID : spellsToRemove)
        {
            spellsToRemoveString += std::to_string(spellID) + " ";
        }

        LOG_TRACE("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: spells to remove: {}",
            player->GetName(),
            spellsToRemoveString
        );

        // iterate through the spellsToRemove set and remove each spell
        for (uint32 spellID : spellsToRemove)
        {
            if (player->HasSpell(spellID))
            {
                LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: remove spell {} ({}) from spellbook.",
                    player->GetName(),
                    spellID,
                    sSpellMgr->GetSpellInfo(spellID)->SpellName[0]
                );
                player->removeSpell(spellID, 255, false);
            }
            else
            {
                LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: does not have spell {} ({}) in their spellbook.",
                    player->GetName(),
                    spellID,
                    sSpellMgr->GetSpellInfo(spellID)->SpellName[0]
                );
            }
        }

        // only if there are spells to remove
        if (spellsToRemove.size())
        {
            // Delete the spells in spellsToRemove from the database
            std::string spellsToRemoveStringForQuery = "";
            for (uint32 spellID : spellsToRemove)
            {
                spellsToRemoveStringForQuery += std::to_string(spellID) + ",";
            }
            spellsToRemoveStringForQuery.pop_back();
            std::string deleteSQLString = "DELETE FROM custom_smallcraft_tempspells WHERE guid = " + std::to_string(player->GetGUID().GetRawValue()) + " AND SpellID IN (" + spellsToRemoveStringForQuery + ")";
            LOG_TRACE("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: Delete query: {}",
                player->GetName(),
                deleteSQLString
            );
            CharacterDatabase.Execute(deleteSQLString);
        }
    }
    else
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: has no spells to remove.",
            player->GetName()
        );
    }

    // remove from spellsToAdd any spell that the player already knows
    for (uint32 spellID : memberInfo->tempSpells)
    {
        if (player->HasSpell(spellID))
        {
            // we don't want to remove spells later that we didn't add, so just skip this spell entirely
            spellsToAdd.erase(spellID);
            LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: already has spell {} ({}) in their spellbook.",
                player->GetName(),
                spellID,
                sSpellMgr->GetSpellInfo(spellID)->SpellName[0]
            );
        }
    }

    // if there are any spells to add, add them
    if (spellsToAdd.size())
    {
        // notify the player that they are having spells added
        if (spellsToAdd.size() == 1)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("|cffc3dbff [Smallcraft]|r|cffffff00 A new spell appears in your spellbook!|r");
        }
        else
        {
            ChatHandler(player->GetSession()).PSendSysMessage("|cffc3dbff [Smallcraft]|r|cffffff00 Several new spells appear in your spellbook!|r");
        }

        // iterate through the spellsToAdd set and add each spell
        for (uint32 spellID : spellsToAdd)
        {
            if (!player->HasSpell(spellID))
            {
                LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: add spell {} ({}) to spellbook.",
                    player->GetName(),
                    spellID,
                    sSpellMgr->GetSpellInfo(spellID)->SpellName[0]
                );
                player->learnSpell(spellID, false);
            }
        }

        // Add the spells in spellsToAdd to the database
        std::string spellsToAddStringForQuery = "";
        for (uint32 spellID : spellsToAdd)
        {
            spellsToAddStringForQuery += "(" + std::to_string(player->GetGUID().GetRawValue()) + "," + std::to_string(spellID) + "),";
        }
        spellsToAddStringForQuery.pop_back();
        std::string insertSQLString = "INSERT INTO custom_smallcraft_tempspells (guid, SpellID) VALUES " + spellsToAddStringForQuery;
        LOG_TRACE("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: Insert query: {}",
            player->GetName(),
            insertSQLString
        );
        CharacterDatabase.Execute(insertSQLString);
    }
    else {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells::UpdatePlayer({}):: has no spells to add.",
            player->GetName()
        );
    }
}
/**
 * @brief Returns the next player(s) that should receive a temp spell.
 *
 * @param group The group to analyze.
 * @return std::vector<SmallcraftGroupMemberInfo*> A vector of SmallcraftGroupMemberInfo pointers for the members who should receive the temp spell.
 */
std::vector<SmallcraftGroupMemberInfo*> Smallcraft_TempSpells::_getMembersForTempSpell(Group* group, DispelType dispelType)
{
    // load (or create) the group's SmallcraftInfo
    SmallcraftGroupInfo* groupInfo = group->CustomData.GetDefault<SmallcraftGroupInfo>("SmallcraftGroupInfo");

    // a vector of potential candidates for the dispel type we need
    std::vector<SmallcraftGroupMemberInfo*> potentialCandidates;

    // load the potentialCandidates with all group members
    for (auto& member : groupInfo->members)
    {
        potentialCandidates.push_back(&member.second);
    }

    LOG_TRACE("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_getMembersForTempSpell:: All group members:");

    // output the name, class, spec, and role of each member
    for (SmallcraftGroupMemberInfo* potentialCandidate : potentialCandidates)
    {
        LOG_TRACE("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_getMembersForTempSpell:: {} | {} {} | {} | {}",
            potentialCandidate->name,
            talentSpecInfo.find(potentialCandidate->talentSpec) != talentSpecInfo.end() ? talentSpecInfo.at(potentialCandidate->talentSpec).description : "Unknown",
            EnumUtils::ToString((Classes)(potentialCandidate->myClass)).Title,
            playerRoleDescriptions.at(talentSpecInfo.at(potentialCandidate->talentSpec).role),
            potentialCandidate->player->IsInWorld() ? "ONLINE" : "OFFLINE"
        );
    }

    // iterate through the potentialCandidates and remove any that are a class with this dispel type in classToDispelTypes
    for (auto it = potentialCandidates.begin(); it != potentialCandidates.end(); )
    {
        if (classToDispelTypes.at((*it)->myClass).find(dispelType) != classToDispelTypes.at((*it)->myClass).end())
        {
            LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_getMembersForTempSpell:: Remove {} (class {} has their own {} dispel)",
                (*it)->name,
                EnumUtils::ToString((Classes)((*it)->myClass)).Title,
                dispelTypeDescriptions.at(dispelType)
            );
            it = potentialCandidates.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // iterate through the potentialCandidates and remove any that don't use mana
    for (auto it = potentialCandidates.begin(); it != potentialCandidates.end(); )
    {
        if ((*it)->powerType != Powers::POWER_MANA)
        {
            LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_getMembersForTempSpell:: Remove {} (does not use mana)",
                (*it)->name
            );
            it = potentialCandidates.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // get the config value for role preference
    // this is a comma-deliminated string with values in (ranged,melee,dps,healer,tank)
    std::string rolePrefString = sConfigMgr->GetOption<std::string>("Smallcraft.TempSpells.RolePreference", "");

    // convert the string to a vector of strings
    // if there are extraneous spaces, remove them
    std::vector<std::string> rolePrefStringVector;
    boost::split(rolePrefStringVector, rolePrefString, boost::is_any_of(","));
    for (std::string& rolePref : rolePrefStringVector)
    {
        boost::trim(rolePref);
    }

    // if any of the preferences are invalid, log an error and return
    for (std::string rolePref : rolePrefStringVector)
    {
        if (configValueToRole.find(rolePref) == configValueToRole.end())
        {
            LOG_ERROR("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_getMembersForTempSpell:: Invalid role preference: {}",
                rolePref
            );

            // return an empty vector
            return std::vector<SmallcraftGroupMemberInfo*>();
        }
    }

    // convert the vector of strings to a vector of PlayerRole using configValueToRole
    std::vector<PlayerRole> rolePrefVector;
    for (std::string rolePref : rolePrefStringVector)
    {
        rolePrefVector.push_back(configValueToRole.at(rolePref));
    }

    // debug log the role preferences in order
    std::string rolePrefDebugString = "";
    for (std::string rolePref : rolePrefStringVector) {
        rolePrefDebugString += rolePref + " ";
    }
    LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_getMembersForTempSpell:: Role preferences: {}",
        rolePrefDebugString
    );

    // iterate through the role preferences and attempt to find a candidate that matches each role in order
    // if a role is found, save it as selectedRole and break out of the loop
    PlayerRole selectedRole = ROLE_NONE;
    for (PlayerRole rolePref : rolePrefVector)
    {
        LOG_TRACE("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_getMembersForTempSpell:: Attempting to find a candidate for role {}",
            playerRoleDescriptions.at(rolePref)
        );
        for (auto it = potentialCandidates.begin(); it != potentialCandidates.end(); )
        {
            LOG_TRACE("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_getMembersForTempSpell:: Checking {}",
                (*it)->name
            );

            if (talentSpecInfo.at((*it)->talentSpec).role == rolePref)
            {
                LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_getMembersForTempSpell:: Selected role is {}",
                    playerRoleDescriptions.at(rolePref)
                );
                selectedRole = rolePref;
                break;
            }
            else
            {
                ++it;
            }
        }
        if (selectedRole != ROLE_NONE)
        {
            LOG_TRACE("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_getMembersForTempSpell:: Found a candidate for role {}",
                playerRoleDescriptions.at(selectedRole)
            );
            break;
        }
    }

    // if the selected role is not ROLE_NONE, remove any candidates that do not match the selected role
    if (selectedRole != ROLE_NONE)
    {
        LOG_TRACE("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_getMembersForTempSpell:: Remove any candidates that do not match role {}",
            playerRoleDescriptions.at(selectedRole)
        );
        for (auto it = potentialCandidates.begin(); it != potentialCandidates.end(); )
        {
            if (talentSpecInfo.at((*it)->talentSpec).role != selectedRole)
            {
                LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_getMembersForTempSpell:: Remove {} (does not match selected role)",
                    (*it)->name
                );
                it = potentialCandidates.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    // iterate through the potentialCandidates
    // determine the smallest number of dispel types that any candidate has
    // consider each entry in tempSpells as one dispel type
    uint8 smallestNumDispelTypes = 255;
    for (auto it = potentialCandidates.begin(); it != potentialCandidates.end(); ++it)
    {
        uint8 numDispelTypes = talentSpecInfo.at((*it)->talentSpec).dispelTypes.size();
        numDispelTypes += (*it)->tempSpells.size();
        if (numDispelTypes < smallestNumDispelTypes)
        {
            smallestNumDispelTypes = numDispelTypes;
        }
    }

    LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_getMembersForTempSpell:: Smallest number of dispel types is {}",
        smallestNumDispelTypes
    );

    // remove any candidates that have more dispel types than the smallest number of dispel types
    for (auto it = potentialCandidates.begin(); it != potentialCandidates.end(); )
    {
        uint8_t numDispelTypes = talentSpecInfo.at((*it)->talentSpec).dispelTypes.size();
        numDispelTypes += (*it)->tempSpells.size();
        if (numDispelTypes > smallestNumDispelTypes)
        {
            LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_getMembersForTempSpell:: Remove {} (has {} dispel types, greater than {})",
                (*it)->name,
                numDispelTypes,
                smallestNumDispelTypes
            );
            it = potentialCandidates.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // if there are no candidates left, return an empty vector
    if (potentialCandidates.size() == 0)
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_getMembersForTempSpell:: No candidates remain.");
        return std::vector<SmallcraftGroupMemberInfo*>();
    }
    // if there is only one candidate, return the list
    else if (potentialCandidates.size() == 1)
    {
        LOG_DEBUG("module.Smallcraft", "Smallcraft_TempSpells_GroupScript:_getMembersForTempSpell:: Only one candidate remains: {}",
            potentialCandidates[0]->name
        );
        return potentialCandidates;
    }
    // otherwise, return the list with up to the configured number of candidates
    else
    {
        uint8 maxTiedPlayers = sConfigMgr->GetOption<uint8>("Smallcraft.TempSpells.TieCount", 1);

        // order the potentialCandidates vector by GUID
        // then select and return the first maxTiedPlayers up to the size of the vector
        #include <algorithm> // for std::sort

        // sort potentialCandidates by GUID
        std::sort(potentialCandidates.begin(), potentialCandidates.end(),
                  [](SmallcraftGroupMemberInfo* a, SmallcraftGroupMemberInfo* b) {
                      return a->guid < b->guid;
                  });

        // select and return the first maxTiedPlayers up to the size of the vector
        uint8_t numCandidates = std::min(static_cast<uint8_t>(potentialCandidates.size()), maxTiedPlayers);
        std::vector<SmallcraftGroupMemberInfo*> selectedMembers(potentialCandidates.begin(), potentialCandidates.begin() + numCandidates);
        return selectedMembers;
    }
}

void load_sc_tempspells()
{
    LOG_DEBUG("module.Smallcraft", "SmallCraft: Temp Spells is enabled.");
    new Smallcraft_TempSpells_GroupScript();
    new Smallcraft_TempSpells_PlayerScript();
    new Smallcraft_TempSpells_AllMapScript();
    new Smallcraft_TempSpells();
}
