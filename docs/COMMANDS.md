# SneezyMUD Player Command Reference

This guide organizes commands by function to help you learn what's available.
Commands can be abbreviated (e.g., `n` for `north`, `l` for `look`).
Some commands have aliases - multiple names for the same function.

## Core Game Commands

### Move your character in the specified direction

- **`north`** - *All Players*
- **`south`** - *All Players*
- **`east`** - *All Players*
- **`west`** - *All Players*
- **`up`** - *All Players*
- **`down`** - *All Players*

### Change your character's position (affects combat and resting)

- **`stand`** - *All Players*
- **`sit`** - *All Players*
- **`rest`** - *All Players*
- **`sleep`** - *All Players*
- **`wake`** - *All Players*

### Observe your surroundings

- **`look`** - *All Players*
- **`examine`** - *All Players*
  - Like look, but automatically shows contents of containers and firearm ammo
- **`exits`** - *All Players*

### Interact with doors and entryways

- **`open`** - *All Players*
- **`close`** - *All Players*
- **`lock`** - *All Players*
- **`unlock`** - *All Players*
- **`enter`** - *All Players*
- **`leave`** - *All Players*
- **`pick`** - *Level 1+*

### Manage your inventory

- **`get`** (aliases: `take`) - *All Players*
- **`drop`** - *All Players*
- **`put`** - *All Players*
- **`give`** - *All Players*
- **`junk`** - *Level 1+*
  - Permanently destroy an item and gain 5 XP (cannot junk cursed items)

### Manage worn equipment and wielded weapons

- **`wear`** - *All Players*
- **`wield`** - *All Players*
- **`grab`** (aliases: `hold`) - *All Players*
- **`remove`** - *All Players*

### Eat, drink, and use consumable items

- **`eat`** - *All Players*
- **`drink`** - *All Players*
- **`sip`** - *All Players*
  - Drink a small amount (1/4 of normal) - safer than full drink in combat
- **`taste`** - *All Players*
  - Take a small bite of food without consuming the whole item
- **`quaff`** - *All Players*
  - Drink a potion immediately (up to 3 spells) - can drop in combat
- **`recite`** - *All Players*
  - Read a scroll (up to 3 spells) - can specify target, requires READ_MAGIC for non-casters
- **`use`** - *Level 1+*
  - Use staffs (room effect) or wands (targeted effect) - must be held

### Interact with containers and liquid vessels

- **`pour`** - *All Players*
- **`fill`** - *All Players*

### View information about your character

- **`score`** - *All Players*
- **`inventory`** - *All Players*
- **`equipment`** - *All Players*
- **`attribute`** - *Level 5+*
  - Display your character stats (STR, DEX, CON, INT, WIS, CHA)
- **`allspells`** - *All Players*
  - List all spells you know with their levels

### View information about the game world and other players

- **`who`** - *All Players*
- **`whozone`** - *All Players*
  - Show players in your current zone
- **`where`** - *Level 1+*
  - Show nearby characters in your area
- **`world`** - *All Players*
  - Display server statistics: uptime, rooms, zones, mobs, objects, players
- **`time`** - *All Players*
- **`weather`** - *All Players*
- **`atlas`** - *Level 1+*
  - View world maps (use atlas 1-4 for different volumes)

### Combat commands

- **`kill`** (aliases: `hit`) - *All Players*
- **`assist`** - *All Players*
  - Join combat to help an ally who is already fighting
- **`flee`** - *Level 1+*
- **`consider`** - *All Players*
  - Assess combat difficulty vs NPC based on level difference - shows creature type with relevant skills
- **`wimpy`** - *All Players*
  - Set automatic flee threshold to 20% HP

### Manage your adventuring group

- **`group`** - *Level 1+*
- **`follow`** - *All Players*
  - Follow another character - they become your group leader
- **`split`** - *Level 1+*
  - Divide gold equally among all grouped party members in the same room
- **`report`** - *Level 1+*
  - Broadcast your HP, mana, and movement status to the room - useful for group coordination
- **`order`** - *Level 1+*
  - Command your charmed followers to perform actions

### Talk to other players

- **`say`** (aliases: `'`) - *All Players*
- **`tell`** - *All Players*
- **`shout`** - *Level 5+*
- **`whisper`** - *All Players*
- **`ask`** - *All Players*
- **`grouptell`** (aliases: `gt`) - *All Players*
  - Send message to all grouped party members (alias: gt)

### Express emotions and perform roleplay actions

- **`emote`** (aliases: `:`, `,`) - *Level 1+*

### Report issues to administrators

- **`bug`** - *All Players*
- **`typo`** - *All Players*
- **`idea`** - *All Players*

### Social and roleplay commands

- **`consider`** - *All Players*
  - Assess combat difficulty vs NPC based on level difference - shows creature type with relevant skills
- **`pose`** - *All Players*
  - Set your character's pose/stance shown in room descriptions
- **`title`** - *Level 5+*
  - Set your character title (appears after name)
- **`pray`** - *All Players*
  - Pray at an altar or to your deity

### Improve your character's abilities

- **`practice`** (aliases: `practise`) - *Level 1+*
- **`gain`** - *Level 1+*

### Game system and settings

- **`save`** - *All Players*
- **`quit`** (aliases: `qui`) - *All Players*
- **`color`** - *Level 1+*
  - Toggle ANSI color mode for colored text
- **`terminal`** - *All Players*
  - Set terminal type (ansi/vt100/none)
- **`prompt`** - *All Players*
  - Set custom command prompt
- **`brief`** - *All Players*
  - Toggle brief room descriptions (name only vs full description)
- **`compact`** - *All Players*
  - Toggle compact output mode (reduces spacing)
- **`cls`** - *All Players*
  - Clear your screen

### Get help and information

- **`help`** - *All Players*
- **`news`** - *All Players*
- **`info`** - *All Players*
- **`credits`** - *All Players*
- **`wizlist`** - *All Players*
- **`levels`** - *All Players*
- **`command`** - *All Players*
  - List all commands available to your character

### Other Core Commands

- **`balance`** - *Level 1+*
- **`bet`** - *Level 1+*
  - Place bets in craps or blackjack games
- **`board`** - *Level 1+*
  - Access bulletin boards to read/post messages
- **`buy`** - *All Players*
- **`channel`** - *Level 1+*
- **`check`** - *Level 1+*
  - Check for mail messages
- **`deafen`** - *Level 1+*
- **`deposit`** - *Level 1+*
- **`echo`** - *Level 1+*
- **`edit`** - *Level 1+*
  - Edit your character description
- **`guard`** - *Level 1+*
- **`highfive`** - *Level 1+*
- **`insult`** - *All Players*
- **`list`** - *All Players*
- **`mail`** - *Level 1+*
  - Send and receive mail messages
- **`peek`** - *Level 1+*
  - View your face-down card in blackjack
- **`play`** - *Level 1+*
- **`pull`** - *Level 1+*
- **`read`** - *All Players*
  - Read books, scrolls, signs, and other readable objects
- **`receive`** - *Level 1+*
  - Receive a mail message
- **`rent`** - *Level 1+*
- **`return`** - *All Players*
- **`sell`** - *All Players*
- **`send`** - *Level 1+*
- **`stay`** - *Level 1+*
  - End your turn in blackjack - dealer draws to 17
- **`value`** - *All Players*
- **`withdraw`** - *Level 1+*
- **`write`** - *Level 1+*
  - Write on paper, books, or boards

## Skill Commands (Combat & Class Abilities)

These commands activate special abilities learned through your class.
Use `practice` to see available skills and `gain` to acquire new ones.

### All Players

- **`fire`**
- **`quaff`**
- **`recite`**
### Level 1+

- **`backstab`**
- **`bash`**
- **`bload`**
- **`cast`**
- **`disarm`**
- **`doorbash`**
- **`feign death`**
- **`first aid`**
- **`flee`**
- **`gain`**
- **`glance`**
- **`hide`**
- **`kick`**
- **`lay hands`**
- **`pick`**
- **`practice`**
- **`practise`**
- **`reload`**
- **`rescue`**
- **`search`**
- **`shoot`**
- **`sign`**
- **`sneak`**
- **`springleap`**
- **`spy`**
- **`steal`**
- **`swim`**
- **`track`**
### Level 10+

- **`brew`**
- **`scribe`**
- **`subterfuge`**
### Level 15+

- **`headbutt`**
- **`throw`**
### Level 20+

- **`bodyslam`**
- **`grapple`**
### Level 30+

- **`quivering palm`**
### Brutius

- **`deathstroke`**

## Social Commands (Emotes)

Social commands (also called emotes) express emotions and roleplay actions.
You can also use `emote`, `:`, or `,` followed by any text for custom emotes.
For example: `emote waves cheerfully` or `:waves cheerfully`

`accuse` `applaud` `beam` `beg` `belittle` `bite` 
`bleed` `blush` `bonk` `bounce` `bow` `burp` 
`cackle` `chortle` `chuckle` `clap` `comb` `comfort` 
`cough` `cringe` `cry` `cuddle` `curse` `curtsey` 
`dance` `daydream` `drool` `fart` `flip` `flipoff` 
`fondle` `french` `frown` `fume` `gasp` `giggle` 
`glare` `grin` `groan` `grope` `grovel` `growl` 
`hiccup` `hop` `hug` `kiss` `laugh` `lick` 
`love` `massage` `moan` `moon` `nibble` `nod` 
`nudge` `nuzzle` `pat` `peer` `piledrive` `pimp` 
`pinch` `point` `poke` `ponder` `pout` `puke` 
`punch` `purr` `rip` `ruffle` `scold` `scream` 
`shake` `shiver` `shrug` `sigh` `sing` `slap` 
`smile` `smirk` `snap` `snarl` `sneeze` `snicker` 
`sniff` `snore` `snuggle` `spank` `spit` `squeeze` 
`stare` `steam` `stretch` `strut` `sulk` `tackle` 
`tap` `taunt` `thank` `think` `tickle` `twiddle` 
`wave` `whap` `whine` `whistle` `wiggle` `wink` 
`worship` `yawn` `yodel` 

## Immortal Commands (Level 51+)

These commands are only available to immortals (administrators).
Players do not have access to these commands.

### Low Immortal

- **`bamfin`**
  - Customize your teleport arrival message (use ~N for name, ~H for pronoun, "def" to reset)
- **`bamfout`**
  - Customize your teleport departure message (use ~N for name, ~H for pronoun, "def" to reset)
- **`goto`**
  - Teleport to a room, mobile, or object by number or name
- **`invisible`**
  - Set or toggle immortal invisibility level (only immortals at/above level can see you)
- **`monitor`**
  - Set number of radio channels to monitor (requires radio object)
- **`nohassle`**
  - Toggle immunity to aggressive NPC attacks
- **`noshout`**
  - Toggle hearing shouts, or silence another player (SAINT+)
- **`purge`**
  - Destroy objects/mobiles in room, or purge links/room ranges (higher levels)
- **`rload`**
  - Load room data from disk for a range of room numbers
- **`rsave`**
  - Save room data to disk for a range of room numbers
- **`wizhelp`**
  - Display list of all privileged commands available at your level
- **`wiznet`**
  - Immortal social/emote command
### Creator

- **`at`**
  - Execute a command at a remote location without moving there
- **`flag`**
  - Toggle player flags (killer, outlaw, banished) - cannot flag higher-level players
- **`instazone`**
  - Create zone files from current room/mobile/object state
- **`show`**
  - Display comprehensive game world data (zones, objects, mobiles, rooms)
- **`stat`**
  - Display detailed statistics on characters, objects, or rooms
- **`stealth`**
  - Toggle stealth mode - hide your teleport messages from mortals
- **`string`**
  - Modify string fields on objects and characters (name, descriptions, etc.)
- **`switch`**
  - Transfer your consciousness into an NPC body (use "return" to switch back)
- **`users`**
  - Display all current connections with character names and hostnames
- **`wiznews`**
  - Display the immortal news file
### Demigod

- **`cutlink`**
  - Command exists in table but has no implementation
- **`deathcheck`**
  - Search log files for player deaths
- **`loglist`**
  - Create listing of all files in oldlogs/ directory
- **`restore`**
  - Fully restore HP/mana/movement, set skills to 100% (CREATOR+), max stats (GOD+)
- **`silence`**
  - Toggle global shout silencing - prevents all characters from shouting
- **`snowball`**
  - Social/emote command for immortals
- **`teams`**
  - Display or manage teams
- **`transfer`**
  - Teleport a character to your location (use "transfer all" for all players)
- **`wizlock`**
  - Control game access restriction (all/off/add/rem host/list)
### Lesser God

- **`force`**
  - Force a character to execute a command (cannot force equal/higher level)
- **`load`**
  - Create objects or mobiles from virtual numbers (some objects blocked)
### God

- **`snoop`**
  - Monitor another player's input/output (cannot snoop equal/higher level)
### Silly Lord

- **`@set`**
  - Set character attributes (align, class, exp, level, sex, race, stats, etc.)
- **`log`**
  - Set PLR_LOGGED flag to log all of a player's commands
- **`oset`**
  - Modify object properties in-game (name, type, affects, values, etc.)
- **`reroll`**
  - Deprecated command with no implementation
- **`shutdow`**
  - Intentional typo - reminds you to type "shutdown" fully to prevent accidents
- **`slay`**
  - Immortal kill command for instant NPC/player elimination
### Implementor

- **`advance`**
  - Advance a player's level in a specific class (M/C/W/T)
- **`checklog`**
  - Search old log files for a string pattern
- **`imptest`**
  - Test command for implementors (requires exact test string)
- **`shutdown`**
  - Shut down the MUD server (use "shutdown reboot" for automatic restart)
- **`system`**
  - Send a system announcement message to all players
### Brutius

- **`chpwd`**
  - Change a player's password (also: passwd)
- **`demote`**
  - Reduce the caller's age by one MUD year

## Summary

- **Total commands:** 318
- **Core game commands:** 120
- **Skill commands:** 40
- **Social/emote commands:** 111
- **Immortal commands:** 47

