#ifndef SMALLCRAFT_DEFINES_H
#define SMALLCRAFT_DEFINES_H

#include <vector>
#include <set>
#include <map>
#include "Config.h"
#include "Group.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"

// create an enum for the different player roles
enum PlayerRole
{
    ROLE_NONE,
    ROLE_TANK,
    ROLE_HEALER,
    ROLE_MELEE_DPS,
    ROLE_RANGED_DPS
};

// create a class to store talent spec information
class SmallcraftTalentSpecInfo
{
public:
    // Constructor
    SmallcraftTalentSpecInfo(
        std::string description, std::vector<DispelType> dispelTypes, PlayerRole role)
    {
        this->description = description;
        this->dispelTypes = dispelTypes;
        this->role = role;
    }

    std::string description;
    std::vector<DispelType> dispelTypes;
    PlayerRole role;
};

// create a map between talent specs and the dispel types that they can handle
std::map<uint8, SmallcraftTalentSpecInfo> talentSpecInfo =
{
    {uint8(0),                                  SmallcraftTalentSpecInfo("Unspec'd",        {},                                             ROLE_NONE)},
    {uint8(TALENT_TREE_DEATH_KNIGHT_BLOOD),     SmallcraftTalentSpecInfo("Blood",           {},                                             ROLE_TANK)},
    {uint8(TALENT_TREE_DEATH_KNIGHT_FROST),     SmallcraftTalentSpecInfo("Frost",           {},                                             ROLE_MELEE_DPS)},
    {uint8(TALENT_TREE_DEATH_KNIGHT_UNHOLY),    SmallcraftTalentSpecInfo("Unholy",          {},                                             ROLE_MELEE_DPS)},
    {uint8(TALENT_TREE_DRUID_BALANCE),          SmallcraftTalentSpecInfo("Balance",         {DISPEL_CURSE, DISPEL_POISON},                  ROLE_RANGED_DPS)},
    {uint8(TALENT_TREE_DRUID_FERAL_COMBAT),     SmallcraftTalentSpecInfo("Feral Combat",    {},                                             ROLE_TANK)},
    {uint8(TALENT_TREE_DRUID_RESTORATION),      SmallcraftTalentSpecInfo("Restoration",     {DISPEL_CURSE, DISPEL_POISON},                  ROLE_HEALER)},
    {uint8(TALENT_TREE_HUNTER_BEAST_MASTERY),   SmallcraftTalentSpecInfo("Beast Mastery",   {},                                             ROLE_RANGED_DPS)},
    {uint8(TALENT_TREE_HUNTER_MARKSMANSHIP),    SmallcraftTalentSpecInfo("Marksmanship",    {},                                             ROLE_RANGED_DPS)},
    {uint8(TALENT_TREE_HUNTER_SURVIVAL),        SmallcraftTalentSpecInfo("Survival",        {},                                             ROLE_RANGED_DPS)},
    {uint8(TALENT_TREE_MAGE_ARCANE),            SmallcraftTalentSpecInfo("Arcane",          {DISPEL_CURSE},                                 ROLE_RANGED_DPS)},
    {uint8(TALENT_TREE_MAGE_FIRE),              SmallcraftTalentSpecInfo("Fire",            {DISPEL_CURSE},                                 ROLE_RANGED_DPS)},
    {uint8(TALENT_TREE_MAGE_FROST),             SmallcraftTalentSpecInfo("Frost",           {DISPEL_CURSE},                                 ROLE_RANGED_DPS)},
    {uint8(TALENT_TREE_PALADIN_HOLY),           SmallcraftTalentSpecInfo("Holy",            {DISPEL_DISEASE, DISPEL_MAGIC, DISPEL_POISON},  ROLE_HEALER)},
    {uint8(TALENT_TREE_PALADIN_PROTECTION),     SmallcraftTalentSpecInfo("Protection",      {DISPEL_DISEASE, DISPEL_MAGIC, DISPEL_POISON},  ROLE_TANK)},
    {uint8(TALENT_TREE_PALADIN_RETRIBUTION),    SmallcraftTalentSpecInfo("Retribution",     {},                                             ROLE_MELEE_DPS)},
    {uint8(TALENT_TREE_PRIEST_DISCIPLINE),      SmallcraftTalentSpecInfo("Discipline",      {DISPEL_DISEASE, DISPEL_MAGIC},                 ROLE_HEALER)},
    {uint8(TALENT_TREE_PRIEST_HOLY),            SmallcraftTalentSpecInfo("Holy",            {DISPEL_DISEASE, DISPEL_MAGIC},                 ROLE_HEALER)},
    {uint8(TALENT_TREE_PRIEST_SHADOW),          SmallcraftTalentSpecInfo("Shadow",          {DISPEL_DISEASE, DISPEL_MAGIC},                 ROLE_RANGED_DPS)},
    {uint8(TALENT_TREE_ROGUE_ASSASSINATION),    SmallcraftTalentSpecInfo("Assassination",   {},                                             ROLE_MELEE_DPS)},
    {uint8(TALENT_TREE_ROGUE_COMBAT),           SmallcraftTalentSpecInfo("Combat",          {},                                             ROLE_MELEE_DPS)},
    {uint8(TALENT_TREE_ROGUE_SUBTLETY),         SmallcraftTalentSpecInfo("Subtlety",        {},                                             ROLE_MELEE_DPS)},
    {uint8(TALENT_TREE_SHAMAN_ELEMENTAL),       SmallcraftTalentSpecInfo("Elemental",       {DISPEL_DISEASE, DISPEL_POISON},                ROLE_RANGED_DPS)},
    {uint8(TALENT_TREE_SHAMAN_ENHANCEMENT),     SmallcraftTalentSpecInfo("Enhancement",     {},                                             ROLE_MELEE_DPS)},
    {uint8(TALENT_TREE_SHAMAN_RESTORATION),     SmallcraftTalentSpecInfo("Restoration",     {DISPEL_DISEASE, DISPEL_POISON},                ROLE_HEALER)},
    {uint8(TALENT_TREE_WARLOCK_AFFLICTION),     SmallcraftTalentSpecInfo("Affliction",      {},                                             ROLE_RANGED_DPS)},
    {uint8(TALENT_TREE_WARLOCK_DEMONOLOGY),     SmallcraftTalentSpecInfo("Demonology",      {},                                             ROLE_RANGED_DPS)},
    {uint8(TALENT_TREE_WARLOCK_DESTRUCTION),    SmallcraftTalentSpecInfo("Destruction",     {},                                             ROLE_RANGED_DPS)},
    {uint8(TALENT_TREE_WARRIOR_ARMS),           SmallcraftTalentSpecInfo("Arms",            {},                                             ROLE_MELEE_DPS)},
    {uint8(TALENT_TREE_WARRIOR_FURY),           SmallcraftTalentSpecInfo("Fury",            {},                                             ROLE_MELEE_DPS)},
    {uint8(TALENT_TREE_WARRIOR_PROTECTION),     SmallcraftTalentSpecInfo("Protection",      {},                                             ROLE_TANK)}
};

// Create a map between classes and the dispel types they have regardless of spec
// This is used to help avoid weird situations like granting the mage version of
// "Remove Curse" to a druid who already has its own class version if the druid is a tank.
std::map<Classes, std::set<DispelType>> classToDispelTypes =
{
    {CLASS_DEATH_KNIGHT,    {DISPEL_DISEASE}},
    {CLASS_DRUID,           {DISPEL_CURSE, DISPEL_POISON}},
    {CLASS_HUNTER,          {}},
    {CLASS_MAGE,            {DISPEL_CURSE}},
    {CLASS_PALADIN,         {DISPEL_DISEASE, DISPEL_MAGIC, DISPEL_POISON}},
    {CLASS_PRIEST,          {DISPEL_DISEASE, DISPEL_MAGIC}},
    {CLASS_ROGUE,           {}},
    {CLASS_SHAMAN,          {DISPEL_DISEASE, DISPEL_POISON}},
    {CLASS_WARLOCK,         {}},
    {CLASS_WARRIOR,         {}}
};

// create a map that maps dispel types to a string description
std::map<DispelType, std::string> dispelTypeDescriptions =
{
    {DISPEL_DISEASE, "Disease"},
    {DISPEL_MAGIC, "Magic"},
    {DISPEL_POISON, "Poison"},
    {DISPEL_CURSE, "Curse"}
};

// create a map that maps player roles to a string description
std::map<PlayerRole, std::string> playerRoleDescriptions =
{
    {ROLE_NONE, "None"},
    {ROLE_TANK, "Tank"},
    {ROLE_HEALER, "Healer"},
    {ROLE_MELEE_DPS, "Melee DPS"},
    {ROLE_RANGED_DPS, "Ranged DPS"}
};

// create a map that maps config values to roles
std::map<std::string, PlayerRole> configValueToRole =
{
    {"tank", ROLE_TANK},
    {"healer", ROLE_HEALER},
    {"melee", ROLE_MELEE_DPS},
    {"ranged", ROLE_RANGED_DPS}
};

// create a set of dispel types that we ensure are always available in the group
std::set<DispelType> DesiredDispelTypes =
{
    DISPEL_CURSE,
    DISPEL_DISEASE,
    DISPEL_MAGIC,
    DISPEL_POISON
};

#endif // SMALLCRAFT_DEFINES_H