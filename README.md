# ![logo](https://raw.githubusercontent.com/azerothcore/azerothcore.github.io/master/images/logo-github.png) AzerothCore

# SmallCraft<br><sub>Smaller Raids, Better Parties</sub>

## What is this?
SmallCraft helps make raids possible with a small party of 3 to 5 players. It makes changes to creatures, AI, spells, and other game mechanics in order to make this possible.

The SmallCraft philosophy is to replicate the difficulty and mechanics of the original raid encounters as faithfully as possible while still making them reasonable with a small party. This may mean giving players spells, changing the AI of creatures, or even changing the mechanics of the encounter entirely.

## Requirements
SmallCraft assumes that your group has:
- 3 to 5 players
- 1 dedicated tank
- 1 dedicated healer

SmallCraft also assumes that the players in your group consist of:
- At least one mana user
- One player with some sort of long CC (e.g. polymorph, sap, root, etc)
- One player who is able to do AT LEAST one of:
    - cast stun, sleep, OR banish
    - off-tank

Stun/sleep/banish are often used to trigger alternative mechanics to multiple tanks. You may, of course, simply off-tank as normal.

One player may fulfil multiple of these requirements.

## Dependencies
SmallCraft relies on [AutoBalance](https://github.com/azerothcore/mod-autobalance) to scale monsters health, damage, CC duration, and other stats. A recommended configuration file for AutoBalance will be provided in the future.

## Defaults
By default, all features and modifications will be enabled. Individual features can be disabled in the [configuration file](conf/mod_smallcraft.conf.dist).

## Features
### Temporary Spells
SmallCraft can grant temporary spells to party members in order to ensure that raid encounters are able to be completed. As of this writing, dispel spells are the only types of spells that are granted. SmallCraft will do this intelligently based on your party member's specs and roles.

More information about this feature can be found in the [configuration file](conf/mod_smallcraft.conf.dist).

### Raid Changes
SmallCraft will progressively add more raids to its list of supported raids. The following raids are currenty altered by SmallCraft:

#### Vanilla
- [Zul'Gurub (ZG)](docs/raids/vanilla/zg.md)
- [Ruins of Ahn'Qiraj (AQ20)](docs/raids/vanilla/aq20.md)
- [Molten Core (MC)](docs/raids/vanilla/mc.md)