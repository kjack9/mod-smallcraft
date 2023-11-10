#ifndef SMALLCRAFT_H
#define SMALLCRAFT_H
#include <chrono>
#include <map>
#include <set>
#include "SharedDefines.h"
#include "ObjectGuid.h"

// create a class to hold group member data
// some of this is redudant with the `Player` class, but we need it if the player disconnects
class SmallcraftGroupMemberInfo : public DataMap::Base
{
public:
    SmallcraftGroupMemberInfo() {}

    ObjectGuid guid;                // the player's GUID
    Player* player;                 // may be null if the player is offline
    std::string name;               // the player's name
    Classes myClass;                // the player's class
    Powers powerType;               // the player's power type (mana, rage, etc)
    uint8 talentSpec;               // the player's last-known spec
    std::set<uint32> tempSpells;    // spell IDs that have been or will be temporarily added to this player

    bool scheduleUpdate = false;    // whether or not to update this player's information on the next update
};

// create a class to hold group data
class SmallcraftGroupInfo : public DataMap::Base
{
public:
    SmallcraftGroupInfo() {}

    // a map of group member GUIDs to their information
    std::map<ObjectGuid, SmallcraftGroupMemberInfo> members;

    std::set<DispelType> dispelTypesWeHave;
    std::set<DispelType> dispelTypesWeAreMissing;
};

class Smallcraft_PlayerScript {
public:
    Smallcraft_PlayerScript();
};

bool HijackEvent(uint32 eventId, EventMap &oldMap, EventMap &newMap, std::chrono::duration<int64_t, std::milli> newTime = Milliseconds::max(), bool cancelOriginal = true);

uint32 AddScriptName(std::string scriptName);
#define RegisterSmallcraftCreatureAI(ai_name) new GenericCreatureScript<ai_name>(#ai_name); AddScriptName(#ai_name);

#endif // SMALLCRAFT_H
