# ![logo](https://raw.githubusercontent.com/azerothcore/azerothcore.github.io/master/images/logo-github.png) AzerothCore

# SmallCraft<br><sub>Smaller Raids, Better Parties</sub>

## What is this?
SmallCraft helps make raids possible with a small party of  3-5 players. It makes changes to creatures, AI, spells, and other game mechanics in order to make this possible.

## Requirements
SmallCraft assumes that your group has:
- 3 to 5 players
- 1 dedicated tank
- 1 dedicated healer

SmallCraft also assumes that the players in your group consist of:
- At least one mana user
- One player with some sort of long CC (e.g. polymorph, sap, root, etc)
- One player who is able to slow/stun/snare
- One player who is able to do light off-tanking

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
- [Molten Core (MC)](docs/raids/vanilla/mc.md)