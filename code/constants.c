#include "constants.h"

#include "game_constants.h"
#include "structs.h"

const struct QuestItem QuestList[4][IMMORTAL] = {
  {/* magic user */
    {0, ""},
    {2,
      "It can be found in the donation room, or on your "
      "head\n\r"},
    {1410, "Its a heavy bag of white powder\n\r"},
    {6010,
      "You can make pies out of them, deer seem to like "
      "them too\n\r"},
    {3013,
      "Its a yummy breakfast food, they go great with "
      "eggs and cheese\n\r"},
    {20,
      "If you twiddle your thumbs enough, you'll find "
      "one.\n\r"},
    {24764, "Dead people might wear them on their hands\n\r"},
    {112,
      "If you found one of these, it would be mighty "
      "strange!\n\r"},
    {106,
      "Eye of Toad and Toe of Frog, bring me one or the "
      "other\n\r"},
    {109,
      "A child's favorite place holds the object of my "
      "desire, on the dark river\n\r"},
    /* 10 */
    {3628, "The latest in New padded footwear\n\r"},
    {113,
      "A child might play with one, when the skexies "
      "aren't around\n\r"},
    {19204, "A precious moon in a misty castle\n\r"},
    {20006,
      "Are you a fly? You might run into one. "
      "Beware..\n\r"},
    {1109,
      "Little people have tiny weapons.. bring me "
      "one\n\r"},
    {6203, "IReallyReallyWantACurvedBlade\n\r"},
    {21007,
      "I want to be taller and younger, find the nasty "
      "children\n\r"},
    {5228, "Don't you find screaming women so disarming?\n\r"},
    {7204, "Vaulted lightning\n\r"},
    {16043,
      "Precious elements can come in dull, dark "
      "mines\n\r"},
    /* 20 */
    {20007,
      "You'll catch him napping, no guardian of passing "
      "time."},
    {16903,
      "Nature's mistake, carried by a man on a new moon, "
      "fish on full."},
    {5226, "Sealed in the hands of a city's failed guardian."},
    {10900,
      "Anachronistic rectangular receptacle holds "
      "circular plane."},                                       /* Sentry, TL */
    {13840, "What kind of conditioner does one use for asps?"}, /* Medusa, GQ */
    {7406,
      "If you don't bring a scroll of recall, you might "
      "die a fiery death"},                   /* room_data 7284, SM */
    {120, "Dock down anchor"},                /* Ixitxachitl, NT */
    {21008, "Very useful, behind a hearth."}, /* Dog, OR */
    {10002,
      "He didn't put them to sleep with these, the "
      "results were quite deadly"}, /* On Corpse, DCE */
    {5114,
      "Headgear of a dark elf - she thinks the color "
      "suits her."}, /* Matron, DR */
    {15805,
      "A single sample of fine plumage, held by a guard "
      "and a ghost."}, /* Guard & Brack, SK */
    {21141,
      "In the land of the troglodytes there is a "
      "headpiece with unpleasant powers."}, /* Case, TR */
    {1532,
      "Three witches have the flighty component you "
      "need."}, /* Pot, MT */
    {3648,
      "Unsummoned, they pose large problems. What you "
      "want is on their queen."}, /* Chieftess, HGS */
    {5304,
      "A spectral force holds the key to advancement in "
      "a geometric dead end."}, /* Spectre, PY */
    {5105,
      "What you need is as dark as the heart of the elf "
      "who wields it."}, /* Weaponsmaster, DR */
    {21011,
      "The key to your current problem is in Orshingal, "
      "on a haughty strutter."}, /* Enfan, OR */
    {27004,
      "A small explosive pinapple shaped object. Ever "
      "see Monty-Python?, you might find it in the hands "
      "of sorcerous undead men"}, /* ???, Haplo's */
    {6616,
      "You might smile if drinking a can of this. Look "
      "in Prydain."}, /* ???, PRY */
    {21125,
      "With enough of this strong amber drink you'd "
      "forget about the nightly ghosts."}, /* Crate, ARO
                                            */
    /* 40 */
    {5309,
      "Powerful items of magic follow... first bring me "
      "a medallion of mana\n\r"},
    {1585,
      "Bubble bubble, toil and trouble, bring me a staff "
      "on the double\n\r"},
    {21003,
      "I need some good boots, you know how strange it "
      "is to find them\n\r"},
    {13704, "Watch for a dragon, find his ring\n\r"},
    {252, "Dead-makers cloaks, buried deep in stone\n\r"},
    {3670, "bottled mana\n\r"},
    {1104, "The master of fireworks, take his silver"},
    {5020,
      "You're not a real mage till you get platinum from "
      "purple\n\r"},
    {1599, "Grand Major\n\r"},
    {20002,
      "She's hiding in her room, but she has my "
      "hat!\n\r"}},
  {/* cleric */
    {0, ""},
    {1,
      "It can be found in the donation room, or on your "
      "head\n\r"},
    {1110,
      "White and young, with no corners or sides, a "
      "golden treasure can be found inside\n\r"},
    {3070,
      "The armorer might have a pair, but they're not "
      "his best on hand\n\r"},
    {3057, "judicandus dies\n\r"},
    {6001, "I want clothes i can play chess on\n\r"},
    {16033, "A goblin's favorite food, the eastern path\n\r"},
    {107,
      "Every righteous cleric should have one, but few "
      "do\n\r"},
    {4000, "I have a weakness for cheap rings\n\r"},
    {3025, "cleaver for hire, just outside of midgaard\n\r"},
    /* 10 */
    {3649,
      "My wife needs something New, to help keep her "
      "girlish figure\n\r"},
    {7202,
      "mindflayers have small ones, especially in the "
      "sewers\n\r"},
    {19203, "the weapon of a traitor, lost in a fog\n\r"},
    {15814,
      "striped black and white, take what this "
      "gelfling-friend offers\n\r"},
    {119, "Play with a gypsy child when he asks you to\n\r"},
    {5012,
      "You might use these to kill a vampire, they are "
      "in the desert\n\r"},
    {6809,
      "Really cool sunglasses, on a really cool guy, in "
      "a really cool place\n\r"},
    {17021, "The proof is in the volcano\n\r"},
    {3648, "Giant women have great fashion sense\n\r"},
    {27001,
      "Undead have the strangest law enforcement tools, "
      "near a split in the river\n\r"},
    /* 20*/
    {105,
      "A venomed bite will end your life - you need the "
      "jaws that do it."}, /* Rattler fangs, MidT */
    {3668,
      "Buy some wine from a fortuneteller's brother."}, /* Gypsy Bartender */
    {1703, "On a cat, but not in the petting zoo."},    /* Bengal
                                                           tiger
                                                           , MZ*/
    {13758,
      "Held by a doggie with who will bite you and bite "
      "you and bite you."}, /* Cerebus, Hades */
    {5240,
      "In the old city you'll find the accursed vibrant "
      "stone you require."}, /* Lamia, OT */
    {5013,
      "Where can you go for directions in the desert? "
      "Try the wet spot."}, /* Oasis Map, GED */
    {17011,
      "{An unholy symbol on an unholy creature under a "
      "fuming mountain."}, /* Amelia, WPM */
    {1708,
      "Some liar's got it in a cul-de-sac. The high "
      "priests of Odin know the way."}, /*Liar in room
                                           10911, TL */
    {9203,
      "What would you use to swat a very large "
      "mosquito? Get it from the giants."}, /* Giant, HGS
                                             */
    {21109,
      "A bow made of dark wood, carried by a "
      "troglodyte."}, /* Trog, CT */
    /* 30 */
    {15817,
      "In an secret cold place, a dark flower is "
      "carried by a midnight one."}, /* Bechemel, SK */
    {5239,
      "You'll have to withstand an ambush to get the "
      "rock they prize."}, /* Snake, AM */
    {6112,
      "If you would be king, you'd need one. With a "
      "wooded worm."}, /* Dragon, DH-D */
    {1758,
      "Carried by a hag in the dark lake under the "
      "sewers."}, /* Sea Hag, SM */
    {27411,
      "This Roo's better than you and she has the stick "
      "to prove it."}, /* Queen Roo, LDU */

    {5317,
      "The dead don't lie still when properly "
      "prepared."}, /* Mummy, PY */
    {5033,
      "You can get it off a drider, but he won't give "
      "it to you."}, /* Drider, DR */
    {16615,
      "South of a bay, past a portal, into a tower, be "
      "prepared"}, /* Hoeur, HR */
    {121,
      "To the far southeast, in the lair of a pair of "
      "arachnids."}, /* Cave Spiders, TR */
    {13901,
      "On the shore, down the river from the "
      "troll-bridge"}, /* RHY */
    /* 40 */
    {5104, "Four heads are better than one\n\r"},
    {15806,
      "You don't stand a ghost of a chance against a "
      "glow of white and a cloak of fire"},
    {16022, "a Powerful, blunt, and fragile weapon\n\r"},
    {122, "The sole possession of a devil down-under\n\r"},
    {7220, "The highest thing at the top of a chain"},
    {13785, "From the fairest\n\r"}, {1597, "Mana in a green ring\n\r"},
    {1563, "Famous, blue and very very rare"},
    {5001, "Search for a banded male\n\r"},
    {20003,
      "Ensnared for power, she holds the helmet of the "
      "wise"}

  },
  {/* warrior */
    {0, ""},
    {11,
      "Something you might find in the donation room, "
      "or on your body\n\r"},
    {16034,
      "Goblins have been known to play with these, "
      "especially in dark caves\n\r"},
    {6000,
      "A decent weapon, just the right size for a "
      "tree\n\r"},
    {24760, "Dead men's feet look like this\n\r"},
    {1413, "You were SUPPOSED to bell the CAT!\n\r"},
    {18256,
      "In the city of Mordilnia, a shield of "
      "roygiv\n\r"},
    {8121, "A bag that opens with a ripping sound\n\r"},
    {108, "Floating for safety on the dark\n\r"},
    {123,
      "A mule stole my hat, now he fights in front of "
      "an audience\n\r"},
    /* 10 */
    {3621,
      "Thank goodness when I broke my arm, I still had "
      "my New shield\n\r"},
    {117,
      "If you get this, someone will be quite howling "
      "angry\n\r"},
    {7405, "Sewer Secret Light Sources\n\r"},
    {6205,
      "my eyes just arent as fast to focus as they used "
      "to be\n\r"},
    {4051,
      "These warriors seem scared, but its all in their "
      "head"}, /* Scarred Warrior, MM */
    {5219, "Fresh deer.. yum!\n\r"},
    {16015,
      "An ugly bird in the mountains once told me: 'A "
      "despotic ruler rules with one of these'\n\r"},
    {1718,
      "Hey, that's not a painting at all! But boy is "
      "she ugly! In the new city."}, /* Mimic, NT */
    {5032,
      "Bushwhacked, Bushwhacked, West, West, Green. "
      "Start at the obvious\n\r"},
    {3685,
      "Mightier than a sword, wielded by a four "
      "man\n\r"},
    /* 20 */
    {5100, "Learn humility: I want a common sword\n\r"},
    {16902,
      "They'd all be normal in a moonless world. You "
      "need to steal a silver stole"}, /* Werefox, LY */
    {17022,
      "A lion with a woman's torso holds the book you "
      "need."}, /* Gynosphinx, WPM */
    {5206,
      "To hold the girth of a corpulent man, it must be "
      "ferrous. In the old city hall."}, /* Cabinet, OT
                                          */
    {1737,
      "In the hands of an elf with a green "
      "thumb."}, /* Gardener, NT */
    {5306, "my mommy gave me a knife, but i lost it\n\r"},
    {21006,
      "Childlike, maybe, but they're not children. You "
      "need the locked up cloth."}, /* Case, OR */
    {9204,
      "The largest in the hands of the largest of the "
      "large"}, /* Giant Chief, HGS */
    {1721,
      "Get the toolbook of the trade from the royal "
      "cook in the new city."}, /* Chef, NT */
    {16901,
      "Only an elephant's might be as big as this "
      "bores' mouthpiece."}, /* Boarish, LY */
    /* 30 */
    {6511,
      "A bearded woman might be so engaged, but a "
      "guard's got this one."},
    {5101,
      "Dark elves can be noble too, but they won't let "
      "you take their arms."}, /* Drow Noble, DR */
    {1761,
      "In a suspended polygon, in a chest which is "
      "not."}, /* Mimic, PY */
    {15812,
      "You think that water can't be sharp? Look under "
      "birdland."},                                 /* Ice Pick, SK */
    {16046, "A miner's tool in the dwarven mines"}, /* Shovel,
                                                       ?? */
    {21114,
      "These skeletal beasts will reel you in, you want "
      "the crowbar."}, /* Cave Fisher, MVE */
    {13762,
      "Once in Hades, the key to getting out lies with "
      "a long dead warrior."}, /* Skeletal Warrior, Hades
                                */
    {20005,
      "This usurper will think you very tasty, defeat "
      "him for the thing you need."}, /* Yevaud, AR */
    {5019,
      "A nasty potion in the hands of an even nastier "
      "desert worm."},                                 /* Worm, GED */
    {10002, "These can be found in the zoo, on 'Al'"}, /* On
                                                          Corpse,
                                                          DCE
                                                        */
    /* 40 */
    {5221,
      "Weapons are the keys to the remaining quests. "
      "First, bring me a Stone golem's sword"},
    {3090,
      "If you were a royal guard, what weapon would you "
      "prefer?"},
    {15808, "Weapon of champions"}, {13775, "By the light of the moon\n\r"},
    {21004,
      "By name, you can assume its the largest weapon "
      "in the game\n\r"},
    {3092,
      "He's always pissed, and so are his guards. take "
      "his weapon and make it yours\n\r"},
    {5002, "The weapon of the oldest wyrm\n\r"},
    {5107, "One Two Three Four Five Six\n\r"},
    {1430, "It rises from the ashes, and guards a tower\n\r"},
    {5019,
      "You're not a REAL fighter til you've had one of "
      "these, enchanted\n\r"}

  },
  {
    /* thief   */
    {
      0,
      "",
    },
    {4,
      "You might find one of these in the donation "
      "room, or in your hand\n\r"},
    {3071, "They're the best on hand for 5 coins\n\r"},
    {30,
      "At the wrong end of a nasty spell, or a heavy "
      "hitter\n\r"},
    {3902, "Michelob or Guiness Stout. which is better?\n\r"},
    {24767, "I've heard that skeletons love bleach\n\r"},
    {6006, "Nearly useless in a hearth\n\r"},
    {4104, "Its what makes kobolds green\n\r"},
    {42,
      "Do she-devils steal, as they flap their bat "
      "wings?\n\r"},
    {19202, "Animal light, lost in a fog\n\r"},
    {3647, "These New boots were made for walking\n\r"},
    {4101, "Hands only a warrior could love\n\r"},
    {116, "Near a road to somewhere city\n\r"},
    {111,
      "Only a fool would look at the end of the "
      "river\n\r"},
    {15812,
      "I'd love a really cool backstabbing weapon..  "
      "Make sure it doesn't melt\n\r"},
    {17023,
      "Being charming can be offensive, especially in a "
      "plumed white cap\n\r"},
    {9205, "You could hide a giant in this stuff\n\r"},
    {10002,
      "feeling tired and fuzzy?  Exhibit some stealth, "
      "or you just might get eaten\n\r"},
    {3690,
      "I am an old man, but I will crush you at "
      "chess\n\r"},
    {5000, "Find a dark dwarf. Pick something silver\n\r"},
    /* 20 */
    {15802,
      "It's easy work to work a rejected bird for the "
      "means to his former home."}, /* Skexie Reject, SK
                                     */
    {1750,
      "In the twisted forest of the Graecians a man in "
      "a black cloak has it."}, /* Put on 13731, GRF */
    {5012,
      "Vampire's bane in a wicker basket near a desert "
      "pool.a"}, /* Basket, GED */
    {20008,
      "The toothless dragon eats the means to your "
      "advancement."}, /* Young Wormkin, AR */
    {6810,
      "You are everywhere you look in this frozen "
      "northern maze of ice."},                       /* room_data 6854, ART */
    {255, "Get the happy stick from a desert worm."}, /* Young
                                                         Worm,
                                                         GED
                                                       */
    {7190,
      "In a secret sewer place a squeaking rodent wears "
      "a trinket."}, /* Rat, SM */
    {7205,
      "The master flayer under the city has it on him, "
      "but not in use. Steal it!"}, /* Master mind, SM */
    {7230,
      "You could be stoned for skinning this "
      "subterranean reptilian monster."}, /* Basilisk, SM
                                           */
    {3690,
      "An old man at the park might have one, but these "
      "old men are in the new city."}, /* Old man, NT */
    /* 30 */
    {1729,
      "In the forest north of the new city a traveller "
      "lost his way. It's on him."}, /* Lost Adventurer,
                                        MT */
    {1708,
      "It's growing on a cliff face, on the way to the "
      "lost kingdom."}, /* In room 21170, MVE */
    {1759,
      "The moon's phase can change a man. Find the "
      "badger in a tavern."}, /* Werebadger, LY */
    {1718,
      "You'll find it in the only ice cave a stone's "
      "throw from a desert."}, /* In room 10010, DCE */
    {5243, "I hope it is clear which stone you will need"}, /* Lamia, OT */
    {5302,
      "In a hanging desert artifact, the softest golem "
      "has the key to your success."}, /* Clay Golem, PY
                                        */
    {21008,
      "If your dog were this ugly, you'd lock him in a "
      "fireplace too!"}, /* Dog, OR */
    {9206,
      "It can be electrifying scaling a dragon; a big "
      "guy must have done it."}, /* Chieftain, HGS */
    {6524,
      "The dwarven mazekeeper has the only pair, if you "
      "can find him."}, /* Mazekeeper, Dwarf Mines */
    {1533,
      "Three witches in the mage's tower have the orb "
      "you need"}, /* Pot, MT */
    /* 40 */
    {27404, "Hide behind this vest, it won't stop you"},
    {5113, "The weapon of a maiden, shaped like a goddess"},
    {21014, "The dagger of a yellow-belly"},
    {5037,
      "A thief of great reknown, at least he doesn't "
      "use a spoon"},
    {1101,
      "Elven blade of ancient lore, matches insects "
      "blow for blow"},
    {27000,
      "It strikes like a rattlesnake, but not as "
      "deadly"},
    {27409,
      "The weapon of a primitive man, just right for "
      "killing his mortal foe"},
    {1594, "White wielded by white, glowing white\n\r"},
    {20001,
      "He judges your soul, wields a weapon that shares "
      "your name"},
    {13703,
      "Watch for a dragon, he wears that which you "
      "seek"},
  },
};

const int rev_dir[] = {2, 3, 0, 1, 5, 4};

const int vol_mult[] = {
  1,                                                          /* 0 */
  8, 8, 1, 1, 1, 6, 2, 2, 1, 1,                               /* 10 */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 30 */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 2, 1, 4,
  4,                                                          /* 55 */
  1, 1, 1, 1, 1,                                              /* 60 */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 80 */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 100 */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 120 */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 140 */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1 /* 165 */
};

const char* const exits[] = {"North", "East ", "South", "West ", "Up   ",
  "Down "};

const char* const dirs[] = {"north", "east", "south", "west", "up", "down",
  "\n"};

const int sharp[] = {
  0, 0, 0, 1, /* Slashing */
  0, 0, 0, 0, /* Bludgeon */
  0, 0, 0, 0, /* Pierce   */
};

const char* const drinks[] = {"water", "beer", "wine", "ale", "dark ale",
  "whisky", "lemonade", "firebreather", "local speciality", "slime mold juice",
  "milk", "tea", "coffee", "blood", "salt water", "coca cola", "\n"};

const char* const drinknames[] = {"water", "beer", "wine", "ale", "ale",
  "whisky", "lemonade", "firebreather", "local", "juice", "milk", "tea",
  "coffee", "blood", "salt", "cola", "\n"};

const struct title_type titles[8][ABS_MAX_LVL] = {
  {
    {"Man", "Woman", 0}, {"Apprentice", "Apprentice", 1},
    {"Student", "Student", 2500}, {"Scholar", "Scholar", 5000},
    {"Trickster", "Trickster", 10000}, {"Medium", "Gypsy", 20000},
    {"Scribe", "Witch", 40000}, {"Seer", "Seeress", 60000},
    {"Sage", "Sage", 90000}, {"Illusionist", "Illusionist", 135000},
    {"Abjurer", "Abjuress", 250000}, {"Invoker", "Invoker", 375000},
    {"Enchanter", "Enchantress", 750000}, {"Conjurer", "Conjuress", 1125000},
    {"Magician", "Magician", 1500000},
    {"SpellMaster", "SpellMistress", 1875000}, {"Savant", "Savanti", 2250000},
    {"Magus", "Incantrix", 2625000}, {"Wizard", "Wizard", 3000000},
    {"Warlock", "War Witch", 3375000}, {"Sorcerer", "Sorceress", 3750000},
    {"Prestidigitator", "Prestidigitator", 4125000},
    {"Geomancer", "Geomancer", 5000000}, {"Spiritman", "Spiritman", 6000000},
    {"Necromancer", "Necromancer", 7000000},
    {"Mindreader", "Mindreader", 8000000}, {"Possessor", "Possessor", 9000000},
    {"Fairy", "Fairy", 10000000}, {"Charmer", "Charmstress", 11000000},
    {"Medicaster", "Medicastress", 12000000}, {"Voodoo", "Voodoo", 13000000},
    {"Dowser", "Dowser", 14000000}, {"Medicineman", "Medicineman", 15000000},
    {"Witch-Doctor", "Witch-Doctor", 16000000}, {"Shaman", "Shaman", 18000000},
    {"Hypnotist", "Hypnotist", 20000000},
    {"Astrologer", "Astrologer", 22000000},
    {"Mastermind", "Mastermind", 24000000},
    {"Spellcaster", "Spellcaster", 26000000},
    {"Demonstrator", "Demonstrator", 28000000},
    {"Fasinator", "Fasinator", 30000000},
    {"Fireworker", "Fireworker", 35000000}, {"Summoner", "Summoner", 40000000},
    {"Genie", "Genie", 45000000}, {"Flamethrower", "Flamethrower", 50000000},
    {"Oracler", "Oracler", 55000000}, {"Soothsayer", "Soothsayer", 60000000},
    {"Augur", "Augur", 70000000}, {"Omen", "Omen", 80000000},
    {"Sorcerer", "Sorcerer", 90000000},
    {"Arch Sorcerer", "Arch Sorceress", 100000000},
    {"Immortal Warlock", "Immortal Enchantress", 150000000},
    {"Immortal Warlock", "Immortal Enchantress", 160000000},
    {"Immortal Warlock", "Immortal Enchantress", 170000000},
    {"Immortal Warlock", "Immortal Enchantress", 180000000},
    {"Immortal Warlock", "Immortal Enchantress", 190000000},
    {"Immortal Warlock", "Immortal Enchantress", 200000000},
    {"Avatar of Magic", "Empress of Magic", 210000000},
    {"God of magic", "Goddess of magic", 220000000},
    {"Implementor", "Implementrix", 230000000},
    {"Implementor", "Implementrix", 240000000} /* 60 */
  },

  {{"Man", "Woman", 0}, {"Believer", "Believer", 1},
    {"Attendant", "Attendant", 1500}, {"Acolyte", "Acolyte", 3000},
    {"Novice", "Novice", 6000}, {"Missionary", "Missionary", 13000},
    {"Adept", "Adept", 27500}, {"Deacon", "Deaconess", 55000},
    {"Vicar", "Vicaress", 110000}, {"Priest", "Priestess", 225000},
    {"Minister", "Lady Minister", 450000}, {"Canon", "Canon", 675000},
    {"Levite", "Levitess", 900000}, {"Curate", "Curess", 1125000},
    {"Monk", "Nunne", 1350000}, {"Healer", "Healer", 1575000},
    {"Chaplain", "Chaplain", 1800000}, {"Expositor", "Expositress", 2025000},
    {"Bishop", "Bishop", 2250000}, {"Page", "Page", 2475000},
    {"Resurrector", "Resurrector", 2700000}, {"Apostle", "Apostle", 3000000},
    {"Holy Father", "Mother of God", 3250000},
    {"Altarboy", "Altargirl", 3500000}, {"Sanctor", "Sanctor", 3750000},
    {"Dean", "Dean", 4000000}, {"Pastor", "Pastor", 5000000},
    {"Confessor", "Confessor", 6000000}, {"Divine", "Divine", 7000000},
    {"Preacher", "Preacher", 8000000}, {"Pontiff", "Pontiff", 9000000},
    {"Rejuvenator", "Rejuvenator", 10000000}, {"Saint", "Saint", 12000000},
    {"Worshipper", "Worshipper", 14000000}, {"Animator", "Animator", 15000000},
    {"Choirboy", "Choirgirl", 17000000}, {"Guru", "Guru", 18000000},
    {"Churchman", "Churchwoman", 20000000},
    {"Lifesaver", "Lifesaver", 25000000}, {"Prophet", "Prophet", 30000000},
    {"Beadsma", "Beadsma", 35000000},
    {"Spiritual Healer", "Spiritual Healer", 40000000},
    {"Repentor", "Repentor", 45000000}, {"Disciple", "Disciple", 50000000},
    {"Bishop", "Bishop", 55000000}, {"Arch-bishop", "Arch-bishop", 60000000},
    {"Patriarch", "Matriarch", 65000000},
    {"Assistant Pope", "Assistant Pope", 70000000}, {"Pope", "Pope", 80000000},
    {"Supreme Pope", "Supreme Pope", 90000000},
    {"Arch Pope", "Arch Pope", 100000000},
    {"Immortal Cardinal", "Immortal Priestess", 150000000},
    {"Immortal Cardinal", "Immortal Priestess", 160000000},
    {"Immortal Cardinal", "Immortal Priestess", 170000000},
    {"Immortal Cardinal", "Immortal Priestess", 180000000},
    {"Immortal Cardinal", "Immortal Priestess", 190000000},
    {"Immortal Cardinal", "Immortal Priestess", 200000000},
    {"Inquisitor", "Inquisitress", 210000000}, {"God", "Goddess", 220000000},
    {"Implementor", "Implementress", 230000000},
    {"Implementor", "Implementress", 240000000}},

  {{"Man", "Woman", 0}, {"Swordpupil", "Swordpupil", 1},
    {"Recruit", "Recruit", 2000}, {"Sentry", "Sentress", 4000},
    {"Fighter", "Fighter", 8000}, {"Soldier", "Soldier", 16000},
    {"Warrior", "Warrior", 32000}, {"Veteran", "Veteran", 64000},
    {"Swordsman", "Swordswoman", 125000}, {"Fencer", "Fenceress", 250000},
    {"Combatant", "Combatrix", 500000}, {"Hero", "Heroine", 750000},
    {"Myrmidon", "Myrmidon", 1000000},
    {"Swashbuckler", "Swashbuckleress", 1250000},
    {"Mercenary", "Mercenaress", 1500000},
    {"Swordmaster", "Swordmistress", 1750000},
    {"Lieutenant", "Lieutenant", 2000000},
    {"Champion", "Lady Champion", 2250000},
    {"Dragoon", "Lady Dragoon", 2500000}, {"Cavalier", "Cavalier", 2750000},
    {"Sabreur", "Sabreur", 3000000}, {"Assailant", "Assailant", 3250000},
    {"Mini-tank", "Mini-tank", 3500000}, {"Trooper", "Trooper", 3750000},
    {"Maimer", "Maimer", 4000000}, {"Butcher", "Butcher", 5000000},
    {"Slayer", "Slayer", 6000000}, {"Gladiator", "Gladiator", 7000000},
    {"Amazon", "Amazon", 8000000}, {"Private", "Private", 9000000},
    {"Bruiser", "Bruiser", 10000000}, {"Officer", "Officer", 12000000},
    {"Lancer", "Lancer", 14000000}, {"Muskateer", "Muskateer", 16000000},
    {"Gunner", "Gunner", 18000000}, {"Tank", "Tank", 19000000},
    {"Destroyer", "Destroyer", 20000000}, {"Sargeant", "Sargeant", 22000000},
    {"Guard", "Guard", 24000000}, {"Lieutenant", "Lieutenant", 26000000},
    {"Smasher", "Smasher", 28000000}, {"Kickboxer", "Kickboxer", 30000000},
    {"Cremator", "Cremator", 32000000},
    {"Exterminator", "Exterminator", 34000000},
    {"Barbarian", "Barbarian", 36000000}, {"Captain", "Captain", 38000000},
    {"Commander", "Commander", 40000000},
    {"Knight Apprentice", "Knight Apprentice", 42000000},
    {"White Knight", "White Lady", 44000000},
    {"Black Knight", "Black Lady", 47000000},
    {"Grand Knight", "Grand Lady", 50000000},
    {"Immortal", "Immortal", 150000000}, {"Immortal", "Immortal", 160000000},
    {"Immortal", "Immortal", 170000000}, {"Immortal", "Immortal", 180000000},
    {"Immortal", "Immortal", 190000000}, {"Immortal", "Immortal", 200000000},
    {"Extirpator", "Queen", 210000000}, {"God", "Goddess", 220000000},
    {"Implementor", "Implementress", 230000000},
    {"Implementor", "Implementress", 240000000}},

  {{"Man", " Woman", 0}, {"Pilferer", "Pilferess", 1},
    {"Footpad", "Footpad", 1250}, {"Filcher", "Filcheress", 2500},
    {"Pick-Pocket", "Pick-Pocket", 5000}, {"Sneak", "Sneak", 10000},
    {"Pincher", "Pincheress", 20000}, {"Cut-Purse", "Cut-Purse", 30000},
    {"Snatcher", "Snatcheress", 60000}, {"Sharper", "Sharper", 100000},
    {"Rogue", "Rogue", 140000}, {"Robber", "Robber", 200000},
    {"Magsman", "Magswoman", 340000}, {"Highwayman", "Highwaywoman", 560000},
    {"Burglar", "Burglaress", 780000}, {"Thief", "Thief", 1000000},
    {"Knifer", "Knifer", 1200000}, {"Quick-Blade", "Quick-Blade", 1400000},
    {"Killer", "Murderess", 1600000}, {"Brigand", "Brigand", 1800000},
    {"Cut-Throat", "Cut-Throat", 2000000},
    {"Master Thief", "Master Thief", 2200000},
    {"Master Thief", "Master Thief", 2400000},
    {"Master Thief", "Master Thief", 2600000},
    {"Master Thief", "Master Thief", 2800000},
    {"Master Thief", "Master Thief", 3000000},
    {"Master Thief", "Master Thief", 3200000},
    {"Master Thief", "Master Thief", 3400000},
    {"Master Thief", "Master Thief", 3600000},
    {"Master Thief", "Master Thief", 3800000},
    {"Master Thief", "Master Thief", 4000000},
    {"Master Thief", "Master Thief", 5000000},
    {"Master Thief", "Master Thief", 6000000},
    {"Master Thief", "Master Thief", 7000000},
    {"Master Thief", "Master Thief", 8000000},
    {"Master Thief", "Master Thief", 9000000},
    {"Master Thief", "Master Thief", 10000000},
    {"Master Thief", "Master Thief", 11000000},
    {"Master Thief", "Master Thief", 12000000},
    {"Master Thief", "Master Thief", 13000000},
    {"Master Thief", "Master Thief", 14000000},
    {"Master Thief", "Master Thief", 17000000},
    {"Master Thief", "Master Thief", 20000000},
    {"Master Thief", "Master Thief", 25000000},
    {"Master Thief", "Master Thief", 30000000},
    {"Master Thief", "Master Thief", 35000000},
    {"Master Thief", "Master Thief", 40000000},
    {"Master Thief", "Master Thief", 50000000},
    {"Master Thief", "Master Thief", 60000000},
    {"Master Thief", "Master Thief", 70000000},
    {"Master Thief", "Master Thief", 75000000},
    {"Immortal Assasin", "Immortal Assasin", 150000000},
    {"Immortal Assasin", "Immortal Assasin", 160000000},
    {"Immortal Assasin", "Immortal Assasin", 170000000},
    {"Immortal Assasin", "Immortal Assasin", 180000000},
    {"Immortal Assasin", "Immortal Assasin", 190000000},
    {"Immortal Assasin", "Immortal Assasin", 200000000},
    {" Demi God", "Demi Goddess", 210000000}, {" God", "Goddess", 220000000},
    {"Implementor", "Implementrix", 230000000},
    {"Implementor", "Implementrix", 240000000}},

  {{"Man", " Woman", 0}, {"1", "1", 1}, {"2", "2", 1250}, {"3", "3", 2500},
    {"4", "4", 5000}, {"5", "5", 10000}, {"6", "6", 20000}, {"7", "7", 30000},
    {"8", "8", 60000}, {"9", "9", 100000}, {"10", "10", 140000},
    {"11", "11", 200000}, {"12", "12", 340000}, {"13", "13", 560000},
    {"14", "14", 780000}, {"15", "15", 1000000}, {"16", "16", 1200000},
    {"17", "17", 1400000}, {"18", "18", 1600000}, {"19", "19", 1800000},
    {"20", "20", 2000000}, {"21", "21", 2200000}, {"22", "22", 2400000},
    {"23", "23", 2600000}, {"24", "24", 2800000}, {"25", "25", 3000000},
    {"26", "26", 3200000}, {"27", "27", 3400000}, {"28", "28", 3600000},
    {"29", "29", 3800000}, {"30", "30", 4000000}, {"31", "31", 5000000},
    {"32", "32", 6000000}, {"33", "33", 7000000}, {"34", "34", 8000000},
    {"35", "35", 9000000}, {"36", "36", 10000000}, {"37", "37", 11000000},
    {"38", "38", 12000000}, {"39", "39", 13000000}, {"40", "40", 14000000},
    {"41", "41", 17000000}, {"42", "42", 20000000}, {"43", "43", 25000000},
    {"44", "44", 30000000}, {"45", "45", 35000000}, {"46", "46", 40000000},
    {"47", "47", 50000000}, {"48", "48", 60000000}, {"49", "49", 70000000},
    {"50", "50", 75000000}, {"", "", 150000000}, {"", "", 160000000},
    {"", "", 170000000}, {"", "", 180000000}, {"", "", 190000000},
    {"", "", 200000000}, {"", "", 210000000}, {"", "", 220000000},
    {"", "", 230000000}, {"", "", 240000000}},

  {{"Man", " Woman", 0}, {"1", "1", 1}, {"2", "2", 1250}, {"3", "3", 2500},
    {"4", "4", 5000}, {"5", "5", 10000}, {"6", "6", 20000}, {"7", "7", 30000},
    {"8", "8", 60000}, {"9", "9", 100000}, {"10", "10", 140000},
    {"11", "11", 200000}, {"12", "12", 340000}, {"13", "13", 560000},
    {"14", "14", 780000}, {"15", "15", 1000000}, {"16", "16", 1200000},
    {"17", "17", 1400000}, {"18", "18", 1600000}, {"19", "19", 1800000},
    {"20", "20", 2000000}, {"21", "21", 2200000}, {"22", "22", 2400000},
    {"23", "23", 2600000}, {"24", "24", 2800000}, {"25", "25", 3000000},
    {"26", "26", 3200000}, {"27", "27", 3400000}, {"28", "28", 3600000},
    {"29", "29", 3800000}, {"30", "30", 4000000}, {"31", "31", 5000000},
    {"32", "32", 6000000}, {"33", "33", 7000000}, {"34", "34", 8000000},
    {"35", "35", 9000000}, {"36", "36", 10000000}, {"37", "37", 11000000},
    {"38", "38", 12000000}, {"39", "39", 13000000}, {"40", "40", 14000000},
    {"41", "41", 17000000}, {"42", "42", 20000000}, {"43", "43", 25000000},
    {"44", "44", 30000000}, {"45", "45", 35000000}, {"46", "46", 40000000},
    {"47", "47", 50000000}, {"48", "48", 60000000}, {"49", "49", 70000000},
    {"50", "50", 75000000}, {"", "", 150000000}, {"", "", 160000000},
    {"", "", 170000000}, {"", "", 180000000}, {"", "", 190000000},
    {"", "", 200000000}, {"", "", 210000000}, {"", "", 220000000},
    {"", "", 230000000}, {"", "", 240000000}},

  {{"Man", " Woman", 0}, {"1", "1", 1}, {"2", "2", 1250}, {"3", "3", 2500},
    {"4", "4", 5000}, {"5", "5", 10000}, {"6", "6", 20000}, {"7", "7", 30000},
    {"8", "8", 60000}, {"9", "9", 100000}, {"10", "10", 140000},
    {"11", "11", 200000}, {"12", "12", 340000}, {"13", "13", 560000},
    {"14", "14", 780000}, {"15", "15", 1000000}, {"16", "16", 1200000},
    {"17", "17", 1400000}, {"18", "18", 1600000}, {"19", "19", 1800000},
    {"20", "20", 2000000}, {"21", "21", 2200000}, {"22", "22", 2400000},
    {"23", "23", 2600000}, {"24", "24", 2800000}, {"25", "25", 3000000},
    {"26", "26", 3200000}, {"27", "27", 3400000}, {"28", "28", 3600000},
    {"29", "29", 3800000}, {"30", "30", 4000000}, {"31", "31", 5000000},
    {"32", "32", 6000000}, {"33", "33", 7000000}, {"34", "34", 8000000},
    {"35", "35", 9000000}, {"36", "36", 10000000}, {"37", "37", 11000000},
    {"38", "38", 12000000}, {"39", "39", 13000000}, {"40", "40", 14000000},
    {"41", "41", 17000000}, {"42", "42", 20000000}, {"43", "43", 25000000},
    {"44", "44", 30000000}, {"45", "45", 35000000}, {"46", "46", 40000000},
    {"47", "47", 50000000}, {"48", "48", 60000000}, {"49", "49", 70000000},
    {"50", "50", 75000000}, {"", "", 150000000}, {"", "", 160000000},
    {"", "", 170000000}, {"", "", 180000000}, {"", "", 190000000},
    {"", "", 200000000}, {"", "", 210000000}, {"", "", 220000000},
    {"", "", 230000000}, {"", "", 240000000}},

  {{"Man", " Woman", 0}, {"1", "1", 1}, {"2", "2", 1250}, {"3", "3", 2500},
    {"4", "4", 5000}, {"5", "5", 10000}, {"6", "6", 20000}, {"7", "7", 30000},
    {"8", "8", 60000}, {"9", "9", 100000}, {"10", "10", 140000},
    {"11", "11", 200000}, {"12", "12", 340000}, {"13", "13", 560000},
    {"14", "14", 780000}, {"15", "15", 1000000}, {"16", "16", 1200000},
    {"17", "17", 1400000}, {"18", "18", 1600000}, {"19", "19", 1800000},
    {"20", "20", 2000000}, {"21", "21", 2200000}, {"22", "22", 2400000},
    {"23", "23", 2600000}, {"24", "24", 2800000}, {"25", "25", 3000000},
    {"26", "26", 3200000}, {"27", "27", 3400000}, {"28", "28", 3600000},
    {"29", "29", 3800000}, {"30", "30", 4000000}, {"31", "31", 5000000},
    {"32", "32", 6000000}, {"33", "33", 7000000}, {"34", "34", 8000000},
    {"35", "35", 9000000}, {"36", "36", 10000000}, {"37", "37", 11000000},
    {"38", "38", 12000000}, {"39", "39", 13000000}, {"40", "40", 14000000},
    {"41", "41", 17000000}, {"42", "42", 20000000}, {"43", "43", 25000000},
    {"44", "44", 30000000}, {"45", "45", 35000000}, {"46", "46", 40000000},
    {"47", "47", 50000000}, {"48", "48", 60000000}, {"49", "49", 70000000},
    {"50", "50", 75000000}, {"", "", 150000000}, {"", "", 160000000},
    {"", "", 170000000}, {"", "", 180000000}, {"", "", 190000000},
    {"", "", 200000000}, {"", "", 210000000}, {"", "", 220000000},
    {"", "", 230000000}, {"", "", 240000000}},

};

const char* const RaceName[] = {"Human", "Human", "Elven", "Dwarven", "Hobbit",
  "Gnome", "Ogre", "Mysterion", "Were", "Dragon", "Undead", "Orcan",
  "Insectoid", "Arachnoid", "Dinosaur", "Icthyiod", "Avian", "Giant",
  "Carnivorous", "Parasitic", "Slime", "Demon", "Snake", "Herbivorous", "Tree",
  "Vegan", "Elemental", "Planar", "Devil", "Ghostly", "Goblin", "Troll",
  "Vegan", "Mindflayer", "Primate", "Enfan", "Golem", "Skexie", "uncertain",
  "unidentifiable", "Patryn", "Labrynthian", "Sartan", "Tytan", "\n"};

const char* const material_types[] = {
  "Undefined",
  "Paper",
  "Cloth",
  "Wax",
  "Glass",
  "Wood",
  "Silk",
  "Foodstuff",
  "Plastic",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "Generic organic",
  "Leather",
  "Toughened leather",
  "Dragon scale",
  "Wool",
  "Fur",
  "Feathered",
  "Water/liquid",
  "Fire/flaming",
  "Earth/stone",
  "Elemental",
  "Ice",
  "Lightning",
  "Chaos",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "Generic mineral",
  "Jeweled",
  "Runed",
  "Crystal",
  "Diamond",
  "Ebony",
  "Emerald",
  "Ivory",
  "Obsidian",
  "Onyx",
  "Opal",
  "Ruby",
  "Sapphire",
  "Marble",
  "Stone",
  "Bone",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "Generic metal",
  "Copper",
  "Scale mail",
  "Banded mail",
  "Chain mail",
  "Plate",
  "Bronze",
  "Brass",
  "Iron",
  "Steel",
  "Mithril",
  "Adamantite",
  "Silver",
  "Gold",
  "Platinum",
  "Titanium",
};

const char* const item_types[] = {"UNDEFINED", "LIGHT", "SCROLL", "WAND",
  "STAFF", "WEAPON", "FIRE WEAPON", "MISSILE", "TREASURE", "ARMOR", "POTION",
  "WORN", "OTHER", "TRASH", "TRAP", "CONTAINER", "NOTE", "LIQUID CONTAINER",
  "KEY", "FOOD", "MONEY", "PEN", "BOAT", "AUDIO", "BOARD", "BOW", "ARROW",
  "\n"};

const char* const extra_bits[] = {"GLOW", "HUM", "LEVEL15", "LEVEL25",
  "LEVEL35", "INVISIBLE", "MAGIC", "NODROP", "BLESS", "ANTI-GOOD", "ANTI-EVIL",
  "ANTI-NEUTRAL", "ANTI-CLERIC", "ANTI-MAGE", "ANTI-THIEF", "ANTI-WARRIOR",
  "BRITTLE", "LEVEL10", "LEVEL20", "LEVEL30", "ANTI_ANTI", "ANTI_PALA",
  "ANTI_RANGER", "ANTI_MONK", "LEVEL40", "\n"};

const char* const affected_bits[] = {"BLIND", "INVISIBLE", "DETECT-EVIL",
  "DETECT-INVISIBLE", "DETECT-MAGIC", "SENSE-LIFE", "HOLD", "SANCTUARY",
  "GROUP", "UNUSED", "CURSE", "FLYING", "POISON", "PROTECT-EVIL", "PARALYSIS",
  "INFRAVISION", "WATER-BREATH", "SLEEP", "DODGE", "SNEAK", "HIDE", "FEAR",
  "CHARM", "FOLLOW", "SAVED_OBJECTS", "TRUE_SIGHT", "BREWING", "FIRESHIELD",
  "SILENT", "", "", "SPY", "\n"};

const char* const apply_types[] = {"NONE", "STR", "DEX", "INT", "WIS", "CON",
  "SEX", "CLASS", "LEVEL", "AGE", "CHAR_WEIGHT", "CHAR_HEIGHT", "MANA", "HIT",
  "MOVE", "GOLD", "EXP", "ARMOR", "HITROLL", "DAMROLL", "SAVING_PARA",
  "SAVING_ROD", "SAVING_PETRI", "SAVING_BREATH", "SAVING_SPELL", "SAVING_ALL",
  "RESISTANCE", "SUSCEPTIBILITY", "IMMUNITY", "SPELL AFFECT", "WEAPON SPELL",
  "EAT SPELL", "BACKSTAB", "KICK", "SNEAK", "HIDE", "BASH", "PICK", "STEAL",
  "TRACK", "HIT-N-DAM", "DOUBLE_ATTACK", "DEATHSTROKE", "PARRY", "THROW",
  "GRAPPLE", "\n"};

/* [ch] strength apply (all) */
const struct str_app_type str_app[31] = {
  {-5, -4, 0, 0},                   /* 0  */
  {-5, -4, 3, 1},                   /* 1  */
  {-3, -2, 3, 2}, {-3, -1, 10, 3},  /* 3  */
  {-2, -1, 25, 4}, {-2, -1, 55, 5}, /* 5  */
  {-1, 0, 80, 6}, {-1, 0, 90, 7}, {0, 0, 100, 8}, {0, 0, 100, 9},
  {0, 0, 115, 10}, /* 10  */
  {0, 0, 115, 11}, {0, 0, 140, 12}, {0, 0, 140, 13}, {0, 0, 170, 14},
  {0, 0, 170, 15},                                   /* 15  */
  {0, 1, 195, 16}, {1, 1, 220, 18}, {1, 2, 255, 20}, /* 18  */
  {3, 7, 640, 40}, {3, 8, 700, 40},                  /* 20  */
  {4, 9, 810, 40}, {4, 10, 970, 40}, {5, 11, 1130, 40}, {6, 12, 1440, 40},
  {7, 14, 1750, 40}, /* 25            */
  {1, 3, 280, 22},   /* 18/01-50      */
  {2, 3, 305, 24},   /* 18/51-75      */
  {2, 4, 330, 26},   /* 18/76-90      */
  {2, 5, 380, 28},   /* 18/91-99      */
  {3, 6, 480, 30}    /* 18/100   (30) */
};

/* [dex] skillapply (thieves only) */
const struct dex_skill_type dex_app_skill[26] = {
  {-99, -99, -90, -99, -60, 2000}, /* 0 */
  {-90, -90, -60, -90, -50, 4000}, /* 1 */
  {-80, -80, -40, -80, -45, 6000}, {-70, -70, -30, -70, -40, 8000},
  {-60, -60, -30, -60, -35, 12000}, {-50, -50, -20, -50, -30, 16000}, /* 5 */
  {-40, -40, -20, -40, -25, 20000}, {-30, -30, -15, -30, -20, 24000},
  {-20, -20, -15, -20, -15, 28000}, {-15, -10, -10, -20, -10, 34000},
  {-10, -5, -10, -15, -5, 40000}, /* 10 */
  {-5, 0, -5, -10, 0, 46000}, {0, 0, 0, -5, 0, 50000}, {0, 0, 0, 0, 0, 60000},
  {0, 0, 0, 0, 0, 70000}, {0, 0, 0, 0, 0, 80000}, /* 15 */
  {0, 5, 0, 0, 0, 100000}, {5, 10, 0, 5, 5, 120000},
  {10, 15, 5, 10, 10, 150000}, {15, 20, 10, 15, 15, 200000},
  {15, 20, 10, 15, 15, 200000}, /* 20 */
  {20, 25, 10, 15, 20, 250000}, {20, 25, 15, 20, 20, 300000},
  {25, 25, 15, 20, 20, 350000}, {25, 30, 15, 25, 25, 400000},
  {25, 30, 15, 25, 25, 1000000} /* 25 */
};

/* [dex] apply (all) */
struct dex_app_type dex_app[26] = {
  {-7, -7, 60},                                                    /* 0 */
  {-6, -6, 50},                                                    /* 1 */
  {-4, -4, 50}, {-3, -3, 40}, {-2, -2, 30}, {-1, -1, 20},          /* 5 */
  {0, 0, 10}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},          /* 10 */
  {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, -10},         /* 15 */
  {1, 1, -20}, {2, 2, -30}, {2, 2, -40}, {3, 3, -40}, {3, 3, -40}, /* 20 */
  {4, 4, -50}, {4, 4, -50}, {4, 4, -50}, {5, 5, -60}, {5, 5, -60}  /* 25 */
};

/* [con] apply (all) */
struct con_app_type con_app[26] = {
  {-4, 20},                                     /* 0 */
  {-3, 25},                                     /* 1 */
  {-2, 30}, {-2, 35}, {-1, 40}, {-1, 45},       /* 5 */
  {-1, 50}, {0, 55}, {0, 60}, {0, 65}, {0, 70}, /* 10 */
  {0, 75}, {0, 80}, {0, 85}, {0, 88}, {1, 90},  /* 15 */
  {2, 95}, {3, 97}, {3, 99}, {4, 99}, {5, 99},  /* 20 */
  {6, 99}, {6, 99}, {7, 99}, {8, 99}, {9, 100}  /* 25 */
};

/* [int] apply (all) */
struct int_app_type int_app[26] = {
  {3}, {5},                     /* 1 */
  {7}, {8}, {9}, {10},          /* 5 */
  {11}, {12}, {13}, {15}, {17}, /* 10 */
  {19}, {22}, {25}, {30}, {35}, /* 15 */
  {40}, {45}, {50}, {53}, {55}, /* 20 */
  {56}, {60}, {70}, {80}, {99}  /* 25 */
};

/* [wis] apply (all) */
struct wis_app_type wis_app[26] = {
  {0},                     /* 0 */
  {0},                     /* 1 */
  {0}, {0}, {0}, {0},      /* 5 */
  {1}, {1}, {1}, {1}, {1}, /* 10 */
  {1}, {2}, {2}, {3}, {3}, /* 15 */
  {3}, {4}, {5},           /* 18 */
  {6}, {6},                /* 20 */
  {6}, {6}, {6}, {6}, {6}  /* 25 */
};
