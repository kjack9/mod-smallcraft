#ifndef SMALLCRAFT_TEMPSPELLS_H
#define SMALLCRAFT_TEMPSPELLS_H

#include <vector>
#include <map>
#include "Group.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"

#include "SmallcraftDefines.h"

// create a map that maps dispel types to spell IDs set in the config
std::map<DispelType, std::string> dispelTypeToConfigOptionString =
{
    {DISPEL_CURSE,    "Smallcraft.TempSpells.DispelCurse.SpellID"},
    {DISPEL_DISEASE,  "Smallcraft.TempSpells.DispelDisease.SpellID"},
    {DISPEL_MAGIC,    "Smallcraft.TempSpells.DispelMagic.SpellID"},
    {DISPEL_POISON,   "Smallcraft.TempSpells.DispelPoison.SpellID"}
};

class Smallcraft_TempSpells_GroupScript : public GroupScript
{
public:
    Smallcraft_TempSpells_GroupScript() : GroupScript("Smallcraft_TempSpells_GroupScript") {}
    void OnAddMember(Group* group, ObjectGuid guid) override;
    void OnRemoveMember(Group* group, ObjectGuid guid, RemoveMethod method, ObjectGuid kicker, const char* reason) override;
};

class Smallcraft_TempSpells_PlayerScript : public PlayerScript
{
public:
    Smallcraft_TempSpells_PlayerScript() : PlayerScript("Smallcraft_TempSpells_PlayerScript") {}
    void OnLogin(Player* player) override;
    void OnLogout(Player* player) override;
    void OnUpdate(Player* player, uint32 p_time) override;
};

class Smallcraft_TempSpells_AllMapScript : public AllMapScript
{
public:
    Smallcraft_TempSpells_AllMapScript() : AllMapScript("Smallcraft_TempSpells_AllMapScript") {}
    void OnPlayerEnterAll(Map* map, Player* player) override;
    void OnPlayerLeaveAll(Map* map, Player* player) override;
private:
    void _handleEnterLeaveAll(Map* map, Player* player, bool entering);
};

class Smallcraft_TempSpells
{
public:
    Smallcraft_TempSpells() {}
    static bool UpdateGroupMembers(Group* group);
    static bool AnalyzeGroup(Group* group, bool force = false);
    static void UpdateGroup(Group* group);
    static void UpdatePlayers(Group* group);
    static void UpdatePlayer(Player* player);
private:
    static std::vector<SmallcraftGroupMemberInfo*> _getMembersForTempSpell(Group* group);
};

#endif // SMALLCRAFT_TEMPSPELLS_H