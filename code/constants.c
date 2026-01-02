#include "constants.h"

#include "game_constants.h"
#include "structs.h"

const struct QuestItem QuestList[4][IMMORTAL] = {
  {/* magic user */
    {.item = 0, .where = ""},
    {.item = 2,
      .where = "It can be found in the donation room, or on your "
               "head\n\r"},
    {.item = 1410, .where = "Its a heavy bag of white powder\n\r"},
    {.item = 6010,
      .where = "You can make pies out of them, deer seem to like "
               "them too\n\r"},
    {.item = 3013,
      .where = "Its a yummy breakfast food, they go great with "
               "eggs and cheese\n\r"},
    {.item = 20,
      .where = "If you twiddle your thumbs enough, you'll find "
               "one.\n\r"},
    {.item = 24764, .where = "Dead people might wear them on their hands\n\r"},
    {.item = 112,
      .where = "If you found one of these, it would be mighty "
               "strange!\n\r"},
    {.item = 106,
      .where = "Eye of Toad and Toe of Frog, bring me one or the "
               "other\n\r"},
    {.item = 109,
      .where = "A child's favorite place holds the object of my "
               "desire, on the dark river\n\r"},
    /* 10 */
    {.item = 3628, .where = "The latest in New padded footwear\n\r"},
    {.item = 113,
      .where = "A child might play with one, when the skexies "
               "aren't around\n\r"},
    {.item = 19204, .where = "A precious moon in a misty castle\n\r"},
    {.item = 20006,
      .where = "Are you a fly? You might run into one. "
               "Beware..\n\r"},
    {.item = 1109,
      .where = "Little people have tiny weapons.. bring me "
               "one\n\r"},
    {.item = 6203, .where = "IReallyReallyWantACurvedBlade\n\r"},
    {.item = 21007,
      .where = "I want to be taller and younger, find the nasty "
               "children\n\r"},
    {.item = 5228, .where = "Don't you find screaming women so disarming?\n\r"},
    {.item = 7204, .where = "Vaulted lightning\n\r"},
    {.item = 16043,
      .where = "Precious elements can come in dull, dark "
               "mines\n\r"},
    /* 20 */
    {.item = 20007,
      .where = "You'll catch him napping, no guardian of passing "
               "time."},
    {.item = 16903,
      .where = "Nature's mistake, carried by a man on a new moon, "
               "fish on full."},
    {.item = 5226, .where = "Sealed in the hands of a city's failed guardian."},
    {.item = 10900,
      .where = "Anachronistic rectangular receptacle holds "
               "circular plane."}, /* Sentry, TL */
    {.item = 13840,
      .where =
        "What kind of conditioner does one use for asps?"}, /* Medusa, GQ */
    {.item = 7406,
      .where = "If you don't bring a scroll of recall, you might "
               "die a fiery death"},            /* room_data 7284, SM */
    {.item = 120, .where = "Dock down anchor"}, /* Ixitxachitl, NT */
    {.item = 21008, .where = "Very useful, behind a hearth."}, /* Dog, OR */
    {.item = 10002,
      .where = "He didn't put them to sleep with these, the "
               "results were quite deadly"}, /* On Corpse, DCE */
    {.item = 5114,
      .where = "Headgear of a dark elf - she thinks the color "
               "suits her."}, /* Matron, DR */
    {.item = 15805,
      .where = "A single sample of fine plumage, held by a guard "
               "and a ghost."}, /* Guard & Brack, SK */
    {.item = 21141,
      .where = "In the land of the troglodytes there is a "
               "headpiece with unpleasant powers."}, /* Case, TR */
    {.item = 1532,
      .where = "Three witches have the flighty component you "
               "need."}, /* Pot, MT */
    {.item = 3648,
      .where = "Unsummoned, they pose large problems. What you "
               "want is on their queen."}, /* Chieftess, HGS */
    {.item = 5304,
      .where = "A spectral force holds the key to advancement in "
               "a geometric dead end."}, /* Spectre, PY */
    {.item = 5105,
      .where = "What you need is as dark as the heart of the elf "
               "who wields it."}, /* Weaponsmaster, DR */
    {.item = 21011,
      .where = "The key to your current problem is in Orshingal, "
               "on a haughty strutter."}, /* Enfan, OR */
    {.item = 27004,
      .where = "A small explosive pinapple shaped object. Ever "
               "see Monty-Python?, you might find it in the hands "
               "of sorcerous undead men"}, /* ???, Haplo's */
    {.item = 6616,
      .where = "You might smile if drinking a can of this. Look "
               "in Prydain."}, /* ???, PRY */
    {.item = 21125,
      .where = "With enough of this strong amber drink you'd "
               "forget about the nightly ghosts."}, /* Crate, ARO
                                                     */
    /* 40 */
    {.item = 5309,
      .where = "Powerful items of magic follow... first bring me "
               "a medallion of mana\n\r"},
    {.item = 1585,
      .where = "Bubble bubble, toil and trouble, bring me a staff "
               "on the double\n\r"},
    {.item = 21003,
      .where = "I need some good boots, you know how strange it "
               "is to find them\n\r"},
    {.item = 13704, .where = "Watch for a dragon, find his ring\n\r"},
    {.item = 252, .where = "Dead-makers cloaks, buried deep in stone\n\r"},
    {.item = 3670, .where = "bottled mana\n\r"},
    {.item = 1104, .where = "The master of fireworks, take his silver"},
    {.item = 5020,
      .where = "You're not a real mage till you get platinum from "
               "purple\n\r"},
    {.item = 1599, .where = "Grand Major\n\r"},
    {.item = 20002,
      .where = "She's hiding in her room, but she has my "
               "hat!\n\r"}},
  {/* cleric */
    {.item = 0, .where = ""},
    {.item = 1,
      .where = "It can be found in the donation room, or on your "
               "head\n\r"},
    {.item = 1110,
      .where = "White and young, with no corners or sides, a "
               "golden treasure can be found inside\n\r"},
    {.item = 3070,
      .where = "The armorer might have a pair, but they're not "
               "his best on hand\n\r"},
    {.item = 3057, .where = "judicandus dies\n\r"},
    {.item = 6001, .where = "I want clothes i can play chess on\n\r"},
    {.item = 16033, .where = "A goblin's favorite food, the eastern path\n\r"},
    {.item = 107,
      .where = "Every righteous cleric should have one, but few "
               "do\n\r"},
    {.item = 4000, .where = "I have a weakness for cheap rings\n\r"},
    {.item = 3025, .where = "cleaver for hire, just outside of midgaard\n\r"},
    /* 10 */
    {.item = 3649,
      .where = "My wife needs something New, to help keep her "
               "girlish figure\n\r"},
    {.item = 7202,
      .where = "mindflayers have small ones, especially in the "
               "sewers\n\r"},
    {.item = 19203, .where = "the weapon of a traitor, lost in a fog\n\r"},
    {.item = 15814,
      .where = "striped black and white, take what this "
               "gelfling-friend offers\n\r"},
    {.item = 119, .where = "Play with a gypsy child when he asks you to\n\r"},
    {.item = 5012,
      .where = "You might use these to kill a vampire, they are "
               "in the desert\n\r"},
    {.item = 6809,
      .where = "Really cool sunglasses, on a really cool guy, in "
               "a really cool place\n\r"},
    {.item = 17021, .where = "The proof is in the volcano\n\r"},
    {.item = 3648, .where = "Giant women have great fashion sense\n\r"},
    {.item = 27001,
      .where = "Undead have the strangest law enforcement tools, "
               "near a split in the river\n\r"},
    /* 20*/
    {.item = 105,
      .where = "A venomed bite will end your life - you need the "
               "jaws that do it."}, /* Rattler fangs, MidT */
    {.item = 3668,
      .where =
        "Buy some wine from a fortuneteller's brother."}, /* Gypsy Bartender */
    {.item = 1703, .where = "On a cat, but not in the petting zoo."}, /* Bengal
                                                            tiger
                                                            , MZ*/
    {.item = 13758,
      .where = "Held by a doggie with who will bite you and bite "
               "you and bite you."}, /* Cerebus, Hades */
    {.item = 5240,
      .where = "In the old city you'll find the accursed vibrant "
               "stone you require."}, /* Lamia, OT */
    {.item = 5013,
      .where = "Where can you go for directions in the desert? "
               "Try the wet spot."}, /* Oasis Map, GED */
    {.item = 17011,
      .where = "{An unholy symbol on an unholy creature under a "
               "fuming mountain."}, /* Amelia, WPM */
    {.item = 1708,
      .where = "Some liar's got it in a cul-de-sac. The high "
               "priests of Odin know the way."}, /*Liar in room
                                                    10911, TL */
    {.item = 9203,
      .where = "What would you use to swat a very large "
               "mosquito? Get it from the giants."}, /* Giant, HGS
                                                      */
    {.item = 21109,
      .where = "A bow made of dark wood, carried by a "
               "troglodyte."}, /* Trog, CT */
    /* 30 */
    {.item = 15817,
      .where = "In an secret cold place, a dark flower is "
               "carried by a midnight one."}, /* Bechemel, SK */
    {.item = 5239,
      .where = "You'll have to withstand an ambush to get the "
               "rock they prize."}, /* Snake, AM */
    {.item = 6112,
      .where = "If you would be king, you'd need one. With a "
               "wooded worm."}, /* Dragon, DH-D */
    {.item = 1758,
      .where = "Carried by a hag in the dark lake under the "
               "sewers."}, /* Sea Hag, SM */
    {.item = 27411,
      .where = "This Roo's better than you and she has the stick "
               "to prove it."}, /* Queen Roo, LDU */

    {.item = 5317,
      .where = "The dead don't lie still when properly "
               "prepared."}, /* Mummy, PY */
    {.item = 5033,
      .where = "You can get it off a drider, but he won't give "
               "it to you."}, /* Drider, DR */
    {.item = 16615,
      .where = "South of a bay, past a portal, into a tower, be "
               "prepared"}, /* Hoeur, HR */
    {.item = 121,
      .where = "To the far southeast, in the lair of a pair of "
               "arachnids."}, /* Cave Spiders, TR */
    {.item = 13901,
      .where = "On the shore, down the river from the "
               "troll-bridge"}, /* RHY */
    /* 40 */
    {.item = 5104, .where = "Four heads are better than one\n\r"},
    {.item = 15806,
      .where = "You don't stand a ghost of a chance against a "
               "glow of white and a cloak of fire"},
    {.item = 16022, .where = "a Powerful, blunt, and fragile weapon\n\r"},
    {.item = 122, .where = "The sole possession of a devil down-under\n\r"},
    {.item = 7220, .where = "The highest thing at the top of a chain"},
    {.item = 13785, .where = "From the fairest\n\r"},
    {.item = 1597, .where = "Mana in a green ring\n\r"},
    {.item = 1563, .where = "Famous, blue and very very rare"},
    {.item = 5001, .where = "Search for a banded male\n\r"},
    {.item = 20003,
      .where = "Ensnared for power, she holds the helmet of the "
               "wise"}

  },
  {/* warrior */
    {.item = 0, .where = ""},
    {.item = 11,
      .where = "Something you might find in the donation room, "
               "or on your body\n\r"},
    {.item = 16034,
      .where = "Goblins have been known to play with these, "
               "especially in dark caves\n\r"},
    {.item = 6000,
      .where = "A decent weapon, just the right size for a "
               "tree\n\r"},
    {.item = 24760, .where = "Dead men's feet look like this\n\r"},
    {.item = 1413, .where = "You were SUPPOSED to bell the CAT!\n\r"},
    {.item = 18256,
      .where = "In the city of Mordilnia, a shield of "
               "roygiv\n\r"},
    {.item = 8121, .where = "A bag that opens with a ripping sound\n\r"},
    {.item = 108, .where = "Floating for safety on the dark\n\r"},
    {.item = 123,
      .where = "A mule stole my hat, now he fights in front of "
               "an audience\n\r"},
    /* 10 */
    {.item = 3621,
      .where = "Thank goodness when I broke my arm, I still had "
               "my New shield\n\r"},
    {.item = 117,
      .where = "If you get this, someone will be quite howling "
               "angry\n\r"},
    {.item = 7405, .where = "Sewer Secret Light Sources\n\r"},
    {.item = 6205,
      .where = "my eyes just arent as fast to focus as they used "
               "to be\n\r"},
    {.item = 4051,
      .where = "These warriors seem scared, but its all in their "
               "head"}, /* Scarred Warrior, MM */
    {.item = 5219, .where = "Fresh deer.. yum!\n\r"},
    {.item = 16015,
      .where = "An ugly bird in the mountains once told me: 'A "
               "despotic ruler rules with one of these'\n\r"},
    {.item = 1718,
      .where = "Hey, that's not a painting at all! But boy is "
               "she ugly! In the new city."}, /* Mimic, NT */
    {.item = 5032,
      .where = "Bushwhacked, Bushwhacked, West, West, Green. "
               "Start at the obvious\n\r"},
    {.item = 3685,
      .where = "Mightier than a sword, wielded by a four "
               "man\n\r"},
    /* 20 */
    {.item = 5100, .where = "Learn humility: I want a common sword\n\r"},
    {.item = 16902,
      .where = "They'd all be normal in a moonless world. You "
               "need to steal a silver stole"}, /* Werefox, LY */
    {.item = 17022,
      .where = "A lion with a woman's torso holds the book you "
               "need."}, /* Gynosphinx, WPM */
    {.item = 5206,
      .where = "To hold the girth of a corpulent man, it must be "
               "ferrous. In the old city hall."}, /* Cabinet, OT
                                                   */
    {.item = 1737,
      .where = "In the hands of an elf with a green "
               "thumb."}, /* Gardener, NT */
    {.item = 5306, .where = "my mommy gave me a knife, but i lost it\n\r"},
    {.item = 21006,
      .where = "Childlike, maybe, but they're not children. You "
               "need the locked up cloth."}, /* Case, OR */
    {.item = 9204,
      .where = "The largest in the hands of the largest of the "
               "large"}, /* Giant Chief, HGS */
    {.item = 1721,
      .where = "Get the toolbook of the trade from the royal "
               "cook in the new city."}, /* Chef, NT */
    {.item = 16901,
      .where = "Only an elephant's might be as big as this "
               "bores' mouthpiece."}, /* Boarish, LY */
    /* 30 */
    {.item = 6511,
      .where = "A bearded woman might be so engaged, but a "
               "guard's got this one."},
    {.item = 5101,
      .where = "Dark elves can be noble too, but they won't let "
               "you take their arms."}, /* Drow Noble, DR */
    {.item = 1761,
      .where = "In a suspended polygon, in a chest which is "
               "not."}, /* Mimic, PY */
    {.item = 15812,
      .where = "You think that water can't be sharp? Look under "
               "birdland."}, /* Ice Pick, SK */
    {.item = 16046, .where = "A miner's tool in the dwarven mines"}, /* Shovel,
                                                           ?? */
    {.item = 21114,
      .where = "These skeletal beasts will reel you in, you want "
               "the crowbar."}, /* Cave Fisher, MVE */
    {.item = 13762,
      .where = "Once in Hades, the key to getting out lies with "
               "a long dead warrior."}, /* Skeletal Warrior, Hades
                                         */
    {.item = 20005,
      .where = "This usurper will think you very tasty, defeat "
               "him for the thing you need."}, /* Yevaud, AR */
    {.item = 5019,
      .where = "A nasty potion in the hands of an even nastier "
               "desert worm."}, /* Worm, GED */
    {.item = 10002, .where = "These can be found in the zoo, on 'Al'"}, /* On
                                                              Corpse,
                                                              DCE
                                                            */
    /* 40 */
    {.item = 5221,
      .where = "Weapons are the keys to the remaining quests. "
               "First, bring me a Stone golem's sword"},
    {.item = 3090,
      .where = "If you were a royal guard, what weapon would you "
               "prefer?"},
    {.item = 15808, .where = "Weapon of champions"},
    {.item = 13775, .where = "By the light of the moon\n\r"},
    {.item = 21004,
      .where = "By name, you can assume its the largest weapon "
               "in the game\n\r"},
    {.item = 3092,
      .where = "He's always pissed, and so are his guards. take "
               "his weapon and make it yours\n\r"},
    {.item = 5002, .where = "The weapon of the oldest wyrm\n\r"},
    {.item = 5107, .where = "One Two Three Four Five Six\n\r"},
    {.item = 1430, .where = "It rises from the ashes, and guards a tower\n\r"},
    {.item = 5019,
      .where = "You're not a REAL fighter til you've had one of "
               "these, enchanted\n\r"}

  },
  {
    /* thief   */
    {
      .item = 0,
      .where = "",
    },
    {.item = 4,
      .where = "You might find one of these in the donation "
               "room, or in your hand\n\r"},
    {.item = 3071, .where = "They're the best on hand for 5 coins\n\r"},
    {.item = 30,
      .where = "At the wrong end of a nasty spell, or a heavy "
               "hitter\n\r"},
    {.item = 3902, .where = "Michelob or Guiness Stout. which is better?\n\r"},
    {.item = 24767, .where = "I've heard that skeletons love bleach\n\r"},
    {.item = 6006, .where = "Nearly useless in a hearth\n\r"},
    {.item = 4104, .where = "Its what makes kobolds green\n\r"},
    {.item = 42,
      .where = "Do she-devils steal, as they flap their bat "
               "wings?\n\r"},
    {.item = 19202, .where = "Animal light, lost in a fog\n\r"},
    {.item = 3647, .where = "These New boots were made for walking\n\r"},
    {.item = 4101, .where = "Hands only a warrior could love\n\r"},
    {.item = 116, .where = "Near a road to somewhere city\n\r"},
    {.item = 111,
      .where = "Only a fool would look at the end of the "
               "river\n\r"},
    {.item = 15812,
      .where = "I'd love a really cool backstabbing weapon..  "
               "Make sure it doesn't melt\n\r"},
    {.item = 17023,
      .where = "Being charming can be offensive, especially in a "
               "plumed white cap\n\r"},
    {.item = 9205, .where = "You could hide a giant in this stuff\n\r"},
    {.item = 10002,
      .where = "feeling tired and fuzzy?  Exhibit some stealth, "
               "or you just might get eaten\n\r"},
    {.item = 3690,
      .where = "I am an old man, but I will crush you at "
               "chess\n\r"},
    {.item = 5000, .where = "Find a dark dwarf. Pick something silver\n\r"},
    /* 20 */
    {.item = 15802,
      .where = "It's easy work to work a rejected bird for the "
               "means to his former home."}, /* Skexie Reject, SK
                                              */
    {.item = 1750,
      .where = "In the twisted forest of the Graecians a man in "
               "a black cloak has it."}, /* Put on 13731, GRF */
    {.item = 5012,
      .where = "Vampire's bane in a wicker basket near a desert "
               "pool.a"}, /* Basket, GED */
    {.item = 20008,
      .where = "The toothless dragon eats the means to your "
               "advancement."}, /* Young Wormkin, AR */
    {.item = 6810,
      .where = "You are everywhere you look in this frozen "
               "northern maze of ice."}, /* room_data 6854, ART */
    {.item = 255, .where = "Get the happy stick from a desert worm."}, /* Young
                                                             Worm,
                                                             GED
                                                           */
    {.item = 7190,
      .where = "In a secret sewer place a squeaking rodent wears "
               "a trinket."}, /* Rat, SM */
    {.item = 7205,
      .where = "The master flayer under the city has it on him, "
               "but not in use. Steal it!"}, /* Master mind, SM */
    {.item = 7230,
      .where = "You could be stoned for skinning this "
               "subterranean reptilian monster."}, /* Basilisk, SM
                                                    */
    {.item = 3690,
      .where = "An old man at the park might have one, but these "
               "old men are in the new city."}, /* Old man, NT */
    /* 30 */
    {.item = 1729,
      .where = "In the forest north of the new city a traveller "
               "lost his way. It's on him."}, /* Lost Adventurer,
                                                 MT */
    {.item = 1708,
      .where = "It's growing on a cliff face, on the way to the "
               "lost kingdom."}, /* In room 21170, MVE */
    {.item = 1759,
      .where = "The moon's phase can change a man. Find the "
               "badger in a tavern."}, /* Werebadger, LY */
    {.item = 1718,
      .where = "You'll find it in the only ice cave a stone's "
               "throw from a desert."}, /* In room 10010, DCE */
    {.item = 5243,
      .where = "I hope it is clear which stone you will need"}, /* Lamia, OT */
    {.item = 5302,
      .where = "In a hanging desert artifact, the softest golem "
               "has the key to your success."}, /* Clay Golem, PY
                                                 */
    {.item = 21008,
      .where = "If your dog were this ugly, you'd lock him in a "
               "fireplace too!"}, /* Dog, OR */
    {.item = 9206,
      .where = "It can be electrifying scaling a dragon; a big "
               "guy must have done it."}, /* Chieftain, HGS */
    {.item = 6524,
      .where = "The dwarven mazekeeper has the only pair, if you "
               "can find him."}, /* Mazekeeper, Dwarf Mines */
    {.item = 1533,
      .where = "Three witches in the mage's tower have the orb "
               "you need"}, /* Pot, MT */
    /* 40 */
    {.item = 27404, .where = "Hide behind this vest, it won't stop you"},
    {.item = 5113, .where = "The weapon of a maiden, shaped like a goddess"},
    {.item = 21014, .where = "The dagger of a yellow-belly"},
    {.item = 5037,
      .where = "A thief of great reknown, at least he doesn't "
               "use a spoon"},
    {.item = 1101,
      .where = "Elven blade of ancient lore, matches insects "
               "blow for blow"},
    {.item = 27000,
      .where = "It strikes like a rattlesnake, but not as "
               "deadly"},
    {.item = 27409,
      .where = "The weapon of a primitive man, just right for "
               "killing his mortal foe"},
    {.item = 1594, .where = "White wielded by white, glowing white\n\r"},
    {.item = 20001,
      .where = "He judges your soul, wields a weapon that shares "
               "your name"},
    {.item = 13703,
      .where = "Watch for a dragon, he wears that which you "
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
    {.title_m = "Man", .title_f = "Woman", .exp = 0},
    {.title_m = "Apprentice", .title_f = "Apprentice", .exp = 1},
    {.title_m = "Student", .title_f = "Student", .exp = 2500},
    {.title_m = "Scholar", .title_f = "Scholar", .exp = 5000},
    {.title_m = "Trickster", .title_f = "Trickster", .exp = 10000},
    {.title_m = "Medium", .title_f = "Gypsy", .exp = 20000},
    {.title_m = "Scribe", .title_f = "Witch", .exp = 40000},
    {.title_m = "Seer", .title_f = "Seeress", .exp = 60000},
    {.title_m = "Sage", .title_f = "Sage", .exp = 90000},
    {.title_m = "Illusionist", .title_f = "Illusionist", .exp = 135000},
    {.title_m = "Abjurer", .title_f = "Abjuress", .exp = 250000},
    {.title_m = "Invoker", .title_f = "Invoker", .exp = 375000},
    {.title_m = "Enchanter", .title_f = "Enchantress", .exp = 750000},
    {.title_m = "Conjurer", .title_f = "Conjuress", .exp = 1125000},
    {.title_m = "Magician", .title_f = "Magician", .exp = 1500000},
    {.title_m = "SpellMaster", .title_f = "SpellMistress", .exp = 1875000},
    {.title_m = "Savant", .title_f = "Savanti", .exp = 2250000},
    {.title_m = "Magus", .title_f = "Incantrix", .exp = 2625000},
    {.title_m = "Wizard", .title_f = "Wizard", .exp = 3000000},
    {.title_m = "Warlock", .title_f = "War Witch", .exp = 3375000},
    {.title_m = "Sorcerer", .title_f = "Sorceress", .exp = 3750000},
    {.title_m = "Prestidigitator",
      .title_f = "Prestidigitator",
      .exp = 4125000},
    {.title_m = "Geomancer", .title_f = "Geomancer", .exp = 5000000},
    {.title_m = "Spiritman", .title_f = "Spiritman", .exp = 6000000},
    {.title_m = "Necromancer", .title_f = "Necromancer", .exp = 7000000},
    {.title_m = "Mindreader", .title_f = "Mindreader", .exp = 8000000},
    {.title_m = "Possessor", .title_f = "Possessor", .exp = 9000000},
    {.title_m = "Fairy", .title_f = "Fairy", .exp = 10000000},
    {.title_m = "Charmer", .title_f = "Charmstress", .exp = 11000000},
    {.title_m = "Medicaster", .title_f = "Medicastress", .exp = 12000000},
    {.title_m = "Voodoo", .title_f = "Voodoo", .exp = 13000000},
    {.title_m = "Dowser", .title_f = "Dowser", .exp = 14000000},
    {.title_m = "Medicineman", .title_f = "Medicineman", .exp = 15000000},
    {.title_m = "Witch-Doctor", .title_f = "Witch-Doctor", .exp = 16000000},
    {.title_m = "Shaman", .title_f = "Shaman", .exp = 18000000},
    {.title_m = "Hypnotist", .title_f = "Hypnotist", .exp = 20000000},
    {.title_m = "Astrologer", .title_f = "Astrologer", .exp = 22000000},
    {.title_m = "Mastermind", .title_f = "Mastermind", .exp = 24000000},
    {.title_m = "Spellcaster", .title_f = "Spellcaster", .exp = 26000000},
    {.title_m = "Demonstrator", .title_f = "Demonstrator", .exp = 28000000},
    {.title_m = "Fasinator", .title_f = "Fasinator", .exp = 30000000},
    {.title_m = "Fireworker", .title_f = "Fireworker", .exp = 35000000},
    {.title_m = "Summoner", .title_f = "Summoner", .exp = 40000000},
    {.title_m = "Genie", .title_f = "Genie", .exp = 45000000},
    {.title_m = "Flamethrower", .title_f = "Flamethrower", .exp = 50000000},
    {.title_m = "Oracler", .title_f = "Oracler", .exp = 55000000},
    {.title_m = "Soothsayer", .title_f = "Soothsayer", .exp = 60000000},
    {.title_m = "Augur", .title_f = "Augur", .exp = 70000000},
    {.title_m = "Omen", .title_f = "Omen", .exp = 80000000},
    {.title_m = "Sorcerer", .title_f = "Sorcerer", .exp = 90000000},
    {.title_m = "Arch Sorcerer", .title_f = "Arch Sorceress", .exp = 100000000},
    {.title_m = "Immortal Warlock",
      .title_f = "Immortal Enchantress",
      .exp = 150000000},
    {.title_m = "Immortal Warlock",
      .title_f = "Immortal Enchantress",
      .exp = 160000000},
    {.title_m = "Immortal Warlock",
      .title_f = "Immortal Enchantress",
      .exp = 170000000},
    {.title_m = "Immortal Warlock",
      .title_f = "Immortal Enchantress",
      .exp = 180000000},
    {.title_m = "Immortal Warlock",
      .title_f = "Immortal Enchantress",
      .exp = 190000000},
    {.title_m = "Immortal Warlock",
      .title_f = "Immortal Enchantress",
      .exp = 200000000},
    {.title_m = "Avatar of Magic",
      .title_f = "Empress of Magic",
      .exp = 210000000},
    {.title_m = "God of magic",
      .title_f = "Goddess of magic",
      .exp = 220000000},
    {.title_m = "Implementor", .title_f = "Implementrix", .exp = 230000000},
    {.title_m = "Implementor",
      .title_f = "Implementrix",
      .exp = 240000000} /* 60 */
  },

  {{.title_m = "Man", .title_f = "Woman", .exp = 0},
    {.title_m = "Believer", .title_f = "Believer", .exp = 1},
    {.title_m = "Attendant", .title_f = "Attendant", .exp = 1500},
    {.title_m = "Acolyte", .title_f = "Acolyte", .exp = 3000},
    {.title_m = "Novice", .title_f = "Novice", .exp = 6000},
    {.title_m = "Missionary", .title_f = "Missionary", .exp = 13000},
    {.title_m = "Adept", .title_f = "Adept", .exp = 27500},
    {.title_m = "Deacon", .title_f = "Deaconess", .exp = 55000},
    {.title_m = "Vicar", .title_f = "Vicaress", .exp = 110000},
    {.title_m = "Priest", .title_f = "Priestess", .exp = 225000},
    {.title_m = "Minister", .title_f = "Lady Minister", .exp = 450000},
    {.title_m = "Canon", .title_f = "Canon", .exp = 675000},
    {.title_m = "Levite", .title_f = "Levitess", .exp = 900000},
    {.title_m = "Curate", .title_f = "Curess", .exp = 1125000},
    {.title_m = "Monk", .title_f = "Nunne", .exp = 1350000},
    {.title_m = "Healer", .title_f = "Healer", .exp = 1575000},
    {.title_m = "Chaplain", .title_f = "Chaplain", .exp = 1800000},
    {.title_m = "Expositor", .title_f = "Expositress", .exp = 2025000},
    {.title_m = "Bishop", .title_f = "Bishop", .exp = 2250000},
    {.title_m = "Page", .title_f = "Page", .exp = 2475000},
    {.title_m = "Resurrector", .title_f = "Resurrector", .exp = 2700000},
    {.title_m = "Apostle", .title_f = "Apostle", .exp = 3000000},
    {.title_m = "Holy Father", .title_f = "Mother of God", .exp = 3250000},
    {.title_m = "Altarboy", .title_f = "Altargirl", .exp = 3500000},
    {.title_m = "Sanctor", .title_f = "Sanctor", .exp = 3750000},
    {.title_m = "Dean", .title_f = "Dean", .exp = 4000000},
    {.title_m = "Pastor", .title_f = "Pastor", .exp = 5000000},
    {.title_m = "Confessor", .title_f = "Confessor", .exp = 6000000},
    {.title_m = "Divine", .title_f = "Divine", .exp = 7000000},
    {.title_m = "Preacher", .title_f = "Preacher", .exp = 8000000},
    {.title_m = "Pontiff", .title_f = "Pontiff", .exp = 9000000},
    {.title_m = "Rejuvenator", .title_f = "Rejuvenator", .exp = 10000000},
    {.title_m = "Saint", .title_f = "Saint", .exp = 12000000},
    {.title_m = "Worshipper", .title_f = "Worshipper", .exp = 14000000},
    {.title_m = "Animator", .title_f = "Animator", .exp = 15000000},
    {.title_m = "Choirboy", .title_f = "Choirgirl", .exp = 17000000},
    {.title_m = "Guru", .title_f = "Guru", .exp = 18000000},
    {.title_m = "Churchman", .title_f = "Churchwoman", .exp = 20000000},
    {.title_m = "Lifesaver", .title_f = "Lifesaver", .exp = 25000000},
    {.title_m = "Prophet", .title_f = "Prophet", .exp = 30000000},
    {.title_m = "Beadsma", .title_f = "Beadsma", .exp = 35000000},
    {.title_m = "Spiritual Healer",
      .title_f = "Spiritual Healer",
      .exp = 40000000},
    {.title_m = "Repentor", .title_f = "Repentor", .exp = 45000000},
    {.title_m = "Disciple", .title_f = "Disciple", .exp = 50000000},
    {.title_m = "Bishop", .title_f = "Bishop", .exp = 55000000},
    {.title_m = "Arch-bishop", .title_f = "Arch-bishop", .exp = 60000000},
    {.title_m = "Patriarch", .title_f = "Matriarch", .exp = 65000000},
    {.title_m = "Assistant Pope", .title_f = "Assistant Pope", .exp = 70000000},
    {.title_m = "Pope", .title_f = "Pope", .exp = 80000000},
    {.title_m = "Supreme Pope", .title_f = "Supreme Pope", .exp = 90000000},
    {.title_m = "Arch Pope", .title_f = "Arch Pope", .exp = 100000000},
    {.title_m = "Immortal Cardinal",
      .title_f = "Immortal Priestess",
      .exp = 150000000},
    {.title_m = "Immortal Cardinal",
      .title_f = "Immortal Priestess",
      .exp = 160000000},
    {.title_m = "Immortal Cardinal",
      .title_f = "Immortal Priestess",
      .exp = 170000000},
    {.title_m = "Immortal Cardinal",
      .title_f = "Immortal Priestess",
      .exp = 180000000},
    {.title_m = "Immortal Cardinal",
      .title_f = "Immortal Priestess",
      .exp = 190000000},
    {.title_m = "Immortal Cardinal",
      .title_f = "Immortal Priestess",
      .exp = 200000000},
    {.title_m = "Inquisitor", .title_f = "Inquisitress", .exp = 210000000},
    {.title_m = "God", .title_f = "Goddess", .exp = 220000000},
    {.title_m = "Implementor", .title_f = "Implementress", .exp = 230000000},
    {.title_m = "Implementor", .title_f = "Implementress", .exp = 240000000}},

  {{.title_m = "Man", .title_f = "Woman", .exp = 0},
    {.title_m = "Swordpupil", .title_f = "Swordpupil", .exp = 1},
    {.title_m = "Recruit", .title_f = "Recruit", .exp = 2000},
    {.title_m = "Sentry", .title_f = "Sentress", .exp = 4000},
    {.title_m = "Fighter", .title_f = "Fighter", .exp = 8000},
    {.title_m = "Soldier", .title_f = "Soldier", .exp = 16000},
    {.title_m = "Warrior", .title_f = "Warrior", .exp = 32000},
    {.title_m = "Veteran", .title_f = "Veteran", .exp = 64000},
    {.title_m = "Swordsman", .title_f = "Swordswoman", .exp = 125000},
    {.title_m = "Fencer", .title_f = "Fenceress", .exp = 250000},
    {.title_m = "Combatant", .title_f = "Combatrix", .exp = 500000},
    {.title_m = "Hero", .title_f = "Heroine", .exp = 750000},
    {.title_m = "Myrmidon", .title_f = "Myrmidon", .exp = 1000000},
    {.title_m = "Swashbuckler", .title_f = "Swashbuckleress", .exp = 1250000},
    {.title_m = "Mercenary", .title_f = "Mercenaress", .exp = 1500000},
    {.title_m = "Swordmaster", .title_f = "Swordmistress", .exp = 1750000},
    {.title_m = "Lieutenant", .title_f = "Lieutenant", .exp = 2000000},
    {.title_m = "Champion", .title_f = "Lady Champion", .exp = 2250000},
    {.title_m = "Dragoon", .title_f = "Lady Dragoon", .exp = 2500000},
    {.title_m = "Cavalier", .title_f = "Cavalier", .exp = 2750000},
    {.title_m = "Sabreur", .title_f = "Sabreur", .exp = 3000000},
    {.title_m = "Assailant", .title_f = "Assailant", .exp = 3250000},
    {.title_m = "Mini-tank", .title_f = "Mini-tank", .exp = 3500000},
    {.title_m = "Trooper", .title_f = "Trooper", .exp = 3750000},
    {.title_m = "Maimer", .title_f = "Maimer", .exp = 4000000},
    {.title_m = "Butcher", .title_f = "Butcher", .exp = 5000000},
    {.title_m = "Slayer", .title_f = "Slayer", .exp = 6000000},
    {.title_m = "Gladiator", .title_f = "Gladiator", .exp = 7000000},
    {.title_m = "Amazon", .title_f = "Amazon", .exp = 8000000},
    {.title_m = "Private", .title_f = "Private", .exp = 9000000},
    {.title_m = "Bruiser", .title_f = "Bruiser", .exp = 10000000},
    {.title_m = "Officer", .title_f = "Officer", .exp = 12000000},
    {.title_m = "Lancer", .title_f = "Lancer", .exp = 14000000},
    {.title_m = "Muskateer", .title_f = "Muskateer", .exp = 16000000},
    {.title_m = "Gunner", .title_f = "Gunner", .exp = 18000000},
    {.title_m = "Tank", .title_f = "Tank", .exp = 19000000},
    {.title_m = "Destroyer", .title_f = "Destroyer", .exp = 20000000},
    {.title_m = "Sargeant", .title_f = "Sargeant", .exp = 22000000},
    {.title_m = "Guard", .title_f = "Guard", .exp = 24000000},
    {.title_m = "Lieutenant", .title_f = "Lieutenant", .exp = 26000000},
    {.title_m = "Smasher", .title_f = "Smasher", .exp = 28000000},
    {.title_m = "Kickboxer", .title_f = "Kickboxer", .exp = 30000000},
    {.title_m = "Cremator", .title_f = "Cremator", .exp = 32000000},
    {.title_m = "Exterminator", .title_f = "Exterminator", .exp = 34000000},
    {.title_m = "Barbarian", .title_f = "Barbarian", .exp = 36000000},
    {.title_m = "Captain", .title_f = "Captain", .exp = 38000000},
    {.title_m = "Commander", .title_f = "Commander", .exp = 40000000},
    {.title_m = "Knight Apprentice",
      .title_f = "Knight Apprentice",
      .exp = 42000000},
    {.title_m = "White Knight", .title_f = "White Lady", .exp = 44000000},
    {.title_m = "Black Knight", .title_f = "Black Lady", .exp = 47000000},
    {.title_m = "Grand Knight", .title_f = "Grand Lady", .exp = 50000000},
    {.title_m = "Immortal", .title_f = "Immortal", .exp = 150000000},
    {.title_m = "Immortal", .title_f = "Immortal", .exp = 160000000},
    {.title_m = "Immortal", .title_f = "Immortal", .exp = 170000000},
    {.title_m = "Immortal", .title_f = "Immortal", .exp = 180000000},
    {.title_m = "Immortal", .title_f = "Immortal", .exp = 190000000},
    {.title_m = "Immortal", .title_f = "Immortal", .exp = 200000000},
    {.title_m = "Extirpator", .title_f = "Queen", .exp = 210000000},
    {.title_m = "God", .title_f = "Goddess", .exp = 220000000},
    {.title_m = "Implementor", .title_f = "Implementress", .exp = 230000000},
    {.title_m = "Implementor", .title_f = "Implementress", .exp = 240000000}},

  {{.title_m = "Man", .title_f = " Woman", .exp = 0},
    {.title_m = "Pilferer", .title_f = "Pilferess", .exp = 1},
    {.title_m = "Footpad", .title_f = "Footpad", .exp = 1250},
    {.title_m = "Filcher", .title_f = "Filcheress", .exp = 2500},
    {.title_m = "Pick-Pocket", .title_f = "Pick-Pocket", .exp = 5000},
    {.title_m = "Sneak", .title_f = "Sneak", .exp = 10000},
    {.title_m = "Pincher", .title_f = "Pincheress", .exp = 20000},
    {.title_m = "Cut-Purse", .title_f = "Cut-Purse", .exp = 30000},
    {.title_m = "Snatcher", .title_f = "Snatcheress", .exp = 60000},
    {.title_m = "Sharper", .title_f = "Sharper", .exp = 100000},
    {.title_m = "Rogue", .title_f = "Rogue", .exp = 140000},
    {.title_m = "Robber", .title_f = "Robber", .exp = 200000},
    {.title_m = "Magsman", .title_f = "Magswoman", .exp = 340000},
    {.title_m = "Highwayman", .title_f = "Highwaywoman", .exp = 560000},
    {.title_m = "Burglar", .title_f = "Burglaress", .exp = 780000},
    {.title_m = "Thief", .title_f = "Thief", .exp = 1000000},
    {.title_m = "Knifer", .title_f = "Knifer", .exp = 1200000},
    {.title_m = "Quick-Blade", .title_f = "Quick-Blade", .exp = 1400000},
    {.title_m = "Killer", .title_f = "Murderess", .exp = 1600000},
    {.title_m = "Brigand", .title_f = "Brigand", .exp = 1800000},
    {.title_m = "Cut-Throat", .title_f = "Cut-Throat", .exp = 2000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 2200000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 2400000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 2600000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 2800000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 3000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 3200000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 3400000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 3600000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 3800000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 4000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 5000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 6000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 7000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 8000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 9000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 10000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 11000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 12000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 13000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 14000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 17000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 20000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 25000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 30000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 35000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 40000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 50000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 60000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 70000000},
    {.title_m = "Master Thief", .title_f = "Master Thief", .exp = 75000000},
    {.title_m = "Immortal Assasin",
      .title_f = "Immortal Assasin",
      .exp = 150000000},
    {.title_m = "Immortal Assasin",
      .title_f = "Immortal Assasin",
      .exp = 160000000},
    {.title_m = "Immortal Assasin",
      .title_f = "Immortal Assasin",
      .exp = 170000000},
    {.title_m = "Immortal Assasin",
      .title_f = "Immortal Assasin",
      .exp = 180000000},
    {.title_m = "Immortal Assasin",
      .title_f = "Immortal Assasin",
      .exp = 190000000},
    {.title_m = "Immortal Assasin",
      .title_f = "Immortal Assasin",
      .exp = 200000000},
    {.title_m = " Demi God", .title_f = "Demi Goddess", .exp = 210000000},
    {.title_m = " God", .title_f = "Goddess", .exp = 220000000},
    {.title_m = "Implementor", .title_f = "Implementrix", .exp = 230000000},
    {.title_m = "Implementor", .title_f = "Implementrix", .exp = 240000000}},

  {{.title_m = "Man", .title_f = " Woman", .exp = 0},
    {.title_m = "1", .title_f = "1", .exp = 1},
    {.title_m = "2", .title_f = "2", .exp = 1250},
    {.title_m = "3", .title_f = "3", .exp = 2500},
    {.title_m = "4", .title_f = "4", .exp = 5000},
    {.title_m = "5", .title_f = "5", .exp = 10000},
    {.title_m = "6", .title_f = "6", .exp = 20000},
    {.title_m = "7", .title_f = "7", .exp = 30000},
    {.title_m = "8", .title_f = "8", .exp = 60000},
    {.title_m = "9", .title_f = "9", .exp = 100000},
    {.title_m = "10", .title_f = "10", .exp = 140000},
    {.title_m = "11", .title_f = "11", .exp = 200000},
    {.title_m = "12", .title_f = "12", .exp = 340000},
    {.title_m = "13", .title_f = "13", .exp = 560000},
    {.title_m = "14", .title_f = "14", .exp = 780000},
    {.title_m = "15", .title_f = "15", .exp = 1000000},
    {.title_m = "16", .title_f = "16", .exp = 1200000},
    {.title_m = "17", .title_f = "17", .exp = 1400000},
    {.title_m = "18", .title_f = "18", .exp = 1600000},
    {.title_m = "19", .title_f = "19", .exp = 1800000},
    {.title_m = "20", .title_f = "20", .exp = 2000000},
    {.title_m = "21", .title_f = "21", .exp = 2200000},
    {.title_m = "22", .title_f = "22", .exp = 2400000},
    {.title_m = "23", .title_f = "23", .exp = 2600000},
    {.title_m = "24", .title_f = "24", .exp = 2800000},
    {.title_m = "25", .title_f = "25", .exp = 3000000},
    {.title_m = "26", .title_f = "26", .exp = 3200000},
    {.title_m = "27", .title_f = "27", .exp = 3400000},
    {.title_m = "28", .title_f = "28", .exp = 3600000},
    {.title_m = "29", .title_f = "29", .exp = 3800000},
    {.title_m = "30", .title_f = "30", .exp = 4000000},
    {.title_m = "31", .title_f = "31", .exp = 5000000},
    {.title_m = "32", .title_f = "32", .exp = 6000000},
    {.title_m = "33", .title_f = "33", .exp = 7000000},
    {.title_m = "34", .title_f = "34", .exp = 8000000},
    {.title_m = "35", .title_f = "35", .exp = 9000000},
    {.title_m = "36", .title_f = "36", .exp = 10000000},
    {.title_m = "37", .title_f = "37", .exp = 11000000},
    {.title_m = "38", .title_f = "38", .exp = 12000000},
    {.title_m = "39", .title_f = "39", .exp = 13000000},
    {.title_m = "40", .title_f = "40", .exp = 14000000},
    {.title_m = "41", .title_f = "41", .exp = 17000000},
    {.title_m = "42", .title_f = "42", .exp = 20000000},
    {.title_m = "43", .title_f = "43", .exp = 25000000},
    {.title_m = "44", .title_f = "44", .exp = 30000000},
    {.title_m = "45", .title_f = "45", .exp = 35000000},
    {.title_m = "46", .title_f = "46", .exp = 40000000},
    {.title_m = "47", .title_f = "47", .exp = 50000000},
    {.title_m = "48", .title_f = "48", .exp = 60000000},
    {.title_m = "49", .title_f = "49", .exp = 70000000},
    {.title_m = "50", .title_f = "50", .exp = 75000000},
    {.title_m = "", .title_f = "", .exp = 150000000},
    {.title_m = "", .title_f = "", .exp = 160000000},
    {.title_m = "", .title_f = "", .exp = 170000000},
    {.title_m = "", .title_f = "", .exp = 180000000},
    {.title_m = "", .title_f = "", .exp = 190000000},
    {.title_m = "", .title_f = "", .exp = 200000000},
    {.title_m = "", .title_f = "", .exp = 210000000},
    {.title_m = "", .title_f = "", .exp = 220000000},
    {.title_m = "", .title_f = "", .exp = 230000000},
    {.title_m = "", .title_f = "", .exp = 240000000}},

  {{.title_m = "Man", .title_f = " Woman", .exp = 0},
    {.title_m = "1", .title_f = "1", .exp = 1},
    {.title_m = "2", .title_f = "2", .exp = 1250},
    {.title_m = "3", .title_f = "3", .exp = 2500},
    {.title_m = "4", .title_f = "4", .exp = 5000},
    {.title_m = "5", .title_f = "5", .exp = 10000},
    {.title_m = "6", .title_f = "6", .exp = 20000},
    {.title_m = "7", .title_f = "7", .exp = 30000},
    {.title_m = "8", .title_f = "8", .exp = 60000},
    {.title_m = "9", .title_f = "9", .exp = 100000},
    {.title_m = "10", .title_f = "10", .exp = 140000},
    {.title_m = "11", .title_f = "11", .exp = 200000},
    {.title_m = "12", .title_f = "12", .exp = 340000},
    {.title_m = "13", .title_f = "13", .exp = 560000},
    {.title_m = "14", .title_f = "14", .exp = 780000},
    {.title_m = "15", .title_f = "15", .exp = 1000000},
    {.title_m = "16", .title_f = "16", .exp = 1200000},
    {.title_m = "17", .title_f = "17", .exp = 1400000},
    {.title_m = "18", .title_f = "18", .exp = 1600000},
    {.title_m = "19", .title_f = "19", .exp = 1800000},
    {.title_m = "20", .title_f = "20", .exp = 2000000},
    {.title_m = "21", .title_f = "21", .exp = 2200000},
    {.title_m = "22", .title_f = "22", .exp = 2400000},
    {.title_m = "23", .title_f = "23", .exp = 2600000},
    {.title_m = "24", .title_f = "24", .exp = 2800000},
    {.title_m = "25", .title_f = "25", .exp = 3000000},
    {.title_m = "26", .title_f = "26", .exp = 3200000},
    {.title_m = "27", .title_f = "27", .exp = 3400000},
    {.title_m = "28", .title_f = "28", .exp = 3600000},
    {.title_m = "29", .title_f = "29", .exp = 3800000},
    {.title_m = "30", .title_f = "30", .exp = 4000000},
    {.title_m = "31", .title_f = "31", .exp = 5000000},
    {.title_m = "32", .title_f = "32", .exp = 6000000},
    {.title_m = "33", .title_f = "33", .exp = 7000000},
    {.title_m = "34", .title_f = "34", .exp = 8000000},
    {.title_m = "35", .title_f = "35", .exp = 9000000},
    {.title_m = "36", .title_f = "36", .exp = 10000000},
    {.title_m = "37", .title_f = "37", .exp = 11000000},
    {.title_m = "38", .title_f = "38", .exp = 12000000},
    {.title_m = "39", .title_f = "39", .exp = 13000000},
    {.title_m = "40", .title_f = "40", .exp = 14000000},
    {.title_m = "41", .title_f = "41", .exp = 17000000},
    {.title_m = "42", .title_f = "42", .exp = 20000000},
    {.title_m = "43", .title_f = "43", .exp = 25000000},
    {.title_m = "44", .title_f = "44", .exp = 30000000},
    {.title_m = "45", .title_f = "45", .exp = 35000000},
    {.title_m = "46", .title_f = "46", .exp = 40000000},
    {.title_m = "47", .title_f = "47", .exp = 50000000},
    {.title_m = "48", .title_f = "48", .exp = 60000000},
    {.title_m = "49", .title_f = "49", .exp = 70000000},
    {.title_m = "50", .title_f = "50", .exp = 75000000},
    {.title_m = "", .title_f = "", .exp = 150000000},
    {.title_m = "", .title_f = "", .exp = 160000000},
    {.title_m = "", .title_f = "", .exp = 170000000},
    {.title_m = "", .title_f = "", .exp = 180000000},
    {.title_m = "", .title_f = "", .exp = 190000000},
    {.title_m = "", .title_f = "", .exp = 200000000},
    {.title_m = "", .title_f = "", .exp = 210000000},
    {.title_m = "", .title_f = "", .exp = 220000000},
    {.title_m = "", .title_f = "", .exp = 230000000},
    {.title_m = "", .title_f = "", .exp = 240000000}},

  {{.title_m = "Man", .title_f = " Woman", .exp = 0},
    {.title_m = "1", .title_f = "1", .exp = 1},
    {.title_m = "2", .title_f = "2", .exp = 1250},
    {.title_m = "3", .title_f = "3", .exp = 2500},
    {.title_m = "4", .title_f = "4", .exp = 5000},
    {.title_m = "5", .title_f = "5", .exp = 10000},
    {.title_m = "6", .title_f = "6", .exp = 20000},
    {.title_m = "7", .title_f = "7", .exp = 30000},
    {.title_m = "8", .title_f = "8", .exp = 60000},
    {.title_m = "9", .title_f = "9", .exp = 100000},
    {.title_m = "10", .title_f = "10", .exp = 140000},
    {.title_m = "11", .title_f = "11", .exp = 200000},
    {.title_m = "12", .title_f = "12", .exp = 340000},
    {.title_m = "13", .title_f = "13", .exp = 560000},
    {.title_m = "14", .title_f = "14", .exp = 780000},
    {.title_m = "15", .title_f = "15", .exp = 1000000},
    {.title_m = "16", .title_f = "16", .exp = 1200000},
    {.title_m = "17", .title_f = "17", .exp = 1400000},
    {.title_m = "18", .title_f = "18", .exp = 1600000},
    {.title_m = "19", .title_f = "19", .exp = 1800000},
    {.title_m = "20", .title_f = "20", .exp = 2000000},
    {.title_m = "21", .title_f = "21", .exp = 2200000},
    {.title_m = "22", .title_f = "22", .exp = 2400000},
    {.title_m = "23", .title_f = "23", .exp = 2600000},
    {.title_m = "24", .title_f = "24", .exp = 2800000},
    {.title_m = "25", .title_f = "25", .exp = 3000000},
    {.title_m = "26", .title_f = "26", .exp = 3200000},
    {.title_m = "27", .title_f = "27", .exp = 3400000},
    {.title_m = "28", .title_f = "28", .exp = 3600000},
    {.title_m = "29", .title_f = "29", .exp = 3800000},
    {.title_m = "30", .title_f = "30", .exp = 4000000},
    {.title_m = "31", .title_f = "31", .exp = 5000000},
    {.title_m = "32", .title_f = "32", .exp = 6000000},
    {.title_m = "33", .title_f = "33", .exp = 7000000},
    {.title_m = "34", .title_f = "34", .exp = 8000000},
    {.title_m = "35", .title_f = "35", .exp = 9000000},
    {.title_m = "36", .title_f = "36", .exp = 10000000},
    {.title_m = "37", .title_f = "37", .exp = 11000000},
    {.title_m = "38", .title_f = "38", .exp = 12000000},
    {.title_m = "39", .title_f = "39", .exp = 13000000},
    {.title_m = "40", .title_f = "40", .exp = 14000000},
    {.title_m = "41", .title_f = "41", .exp = 17000000},
    {.title_m = "42", .title_f = "42", .exp = 20000000},
    {.title_m = "43", .title_f = "43", .exp = 25000000},
    {.title_m = "44", .title_f = "44", .exp = 30000000},
    {.title_m = "45", .title_f = "45", .exp = 35000000},
    {.title_m = "46", .title_f = "46", .exp = 40000000},
    {.title_m = "47", .title_f = "47", .exp = 50000000},
    {.title_m = "48", .title_f = "48", .exp = 60000000},
    {.title_m = "49", .title_f = "49", .exp = 70000000},
    {.title_m = "50", .title_f = "50", .exp = 75000000},
    {.title_m = "", .title_f = "", .exp = 150000000},
    {.title_m = "", .title_f = "", .exp = 160000000},
    {.title_m = "", .title_f = "", .exp = 170000000},
    {.title_m = "", .title_f = "", .exp = 180000000},
    {.title_m = "", .title_f = "", .exp = 190000000},
    {.title_m = "", .title_f = "", .exp = 200000000},
    {.title_m = "", .title_f = "", .exp = 210000000},
    {.title_m = "", .title_f = "", .exp = 220000000},
    {.title_m = "", .title_f = "", .exp = 230000000},
    {.title_m = "", .title_f = "", .exp = 240000000}},

  {{.title_m = "Man", .title_f = " Woman", .exp = 0},
    {.title_m = "1", .title_f = "1", .exp = 1},
    {.title_m = "2", .title_f = "2", .exp = 1250},
    {.title_m = "3", .title_f = "3", .exp = 2500},
    {.title_m = "4", .title_f = "4", .exp = 5000},
    {.title_m = "5", .title_f = "5", .exp = 10000},
    {.title_m = "6", .title_f = "6", .exp = 20000},
    {.title_m = "7", .title_f = "7", .exp = 30000},
    {.title_m = "8", .title_f = "8", .exp = 60000},
    {.title_m = "9", .title_f = "9", .exp = 100000},
    {.title_m = "10", .title_f = "10", .exp = 140000},
    {.title_m = "11", .title_f = "11", .exp = 200000},
    {.title_m = "12", .title_f = "12", .exp = 340000},
    {.title_m = "13", .title_f = "13", .exp = 560000},
    {.title_m = "14", .title_f = "14", .exp = 780000},
    {.title_m = "15", .title_f = "15", .exp = 1000000},
    {.title_m = "16", .title_f = "16", .exp = 1200000},
    {.title_m = "17", .title_f = "17", .exp = 1400000},
    {.title_m = "18", .title_f = "18", .exp = 1600000},
    {.title_m = "19", .title_f = "19", .exp = 1800000},
    {.title_m = "20", .title_f = "20", .exp = 2000000},
    {.title_m = "21", .title_f = "21", .exp = 2200000},
    {.title_m = "22", .title_f = "22", .exp = 2400000},
    {.title_m = "23", .title_f = "23", .exp = 2600000},
    {.title_m = "24", .title_f = "24", .exp = 2800000},
    {.title_m = "25", .title_f = "25", .exp = 3000000},
    {.title_m = "26", .title_f = "26", .exp = 3200000},
    {.title_m = "27", .title_f = "27", .exp = 3400000},
    {.title_m = "28", .title_f = "28", .exp = 3600000},
    {.title_m = "29", .title_f = "29", .exp = 3800000},
    {.title_m = "30", .title_f = "30", .exp = 4000000},
    {.title_m = "31", .title_f = "31", .exp = 5000000},
    {.title_m = "32", .title_f = "32", .exp = 6000000},
    {.title_m = "33", .title_f = "33", .exp = 7000000},
    {.title_m = "34", .title_f = "34", .exp = 8000000},
    {.title_m = "35", .title_f = "35", .exp = 9000000},
    {.title_m = "36", .title_f = "36", .exp = 10000000},
    {.title_m = "37", .title_f = "37", .exp = 11000000},
    {.title_m = "38", .title_f = "38", .exp = 12000000},
    {.title_m = "39", .title_f = "39", .exp = 13000000},
    {.title_m = "40", .title_f = "40", .exp = 14000000},
    {.title_m = "41", .title_f = "41", .exp = 17000000},
    {.title_m = "42", .title_f = "42", .exp = 20000000},
    {.title_m = "43", .title_f = "43", .exp = 25000000},
    {.title_m = "44", .title_f = "44", .exp = 30000000},
    {.title_m = "45", .title_f = "45", .exp = 35000000},
    {.title_m = "46", .title_f = "46", .exp = 40000000},
    {.title_m = "47", .title_f = "47", .exp = 50000000},
    {.title_m = "48", .title_f = "48", .exp = 60000000},
    {.title_m = "49", .title_f = "49", .exp = 70000000},
    {.title_m = "50", .title_f = "50", .exp = 75000000},
    {.title_m = "", .title_f = "", .exp = 150000000},
    {.title_m = "", .title_f = "", .exp = 160000000},
    {.title_m = "", .title_f = "", .exp = 170000000},
    {.title_m = "", .title_f = "", .exp = 180000000},
    {.title_m = "", .title_f = "", .exp = 190000000},
    {.title_m = "", .title_f = "", .exp = 200000000},
    {.title_m = "", .title_f = "", .exp = 210000000},
    {.title_m = "", .title_f = "", .exp = 220000000},
    {.title_m = "", .title_f = "", .exp = 230000000},
    {.title_m = "", .title_f = "", .exp = 240000000}},

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
  {.tohit = -5, .todam = -4, .carry_w = 0, .wield_w = 0}, /* 0  */
  {.tohit = -5, .todam = -4, .carry_w = 3, .wield_w = 1}, /* 1  */
  {.tohit = -3, .todam = -2, .carry_w = 3, .wield_w = 2},
  {.tohit = -3, .todam = -1, .carry_w = 10, .wield_w = 3}, /* 3  */
  {.tohit = -2, .todam = -1, .carry_w = 25, .wield_w = 4},
  {.tohit = -2, .todam = -1, .carry_w = 55, .wield_w = 5}, /* 5  */
  {.tohit = -1, .todam = 0, .carry_w = 80, .wield_w = 6},
  {.tohit = -1, .todam = 0, .carry_w = 90, .wield_w = 7},
  {.tohit = 0, .todam = 0, .carry_w = 100, .wield_w = 8},
  {.tohit = 0, .todam = 0, .carry_w = 100, .wield_w = 9},
  {.tohit = 0, .todam = 0, .carry_w = 115, .wield_w = 10}, /* 10  */
  {.tohit = 0, .todam = 0, .carry_w = 115, .wield_w = 11},
  {.tohit = 0, .todam = 0, .carry_w = 140, .wield_w = 12},
  {.tohit = 0, .todam = 0, .carry_w = 140, .wield_w = 13},
  {.tohit = 0, .todam = 0, .carry_w = 170, .wield_w = 14},
  {.tohit = 0, .todam = 0, .carry_w = 170, .wield_w = 15}, /* 15  */
  {.tohit = 0, .todam = 1, .carry_w = 195, .wield_w = 16},
  {.tohit = 1, .todam = 1, .carry_w = 220, .wield_w = 18},
  {.tohit = 1, .todam = 2, .carry_w = 255, .wield_w = 20}, /* 18  */
  {.tohit = 3, .todam = 7, .carry_w = 640, .wield_w = 40},
  {.tohit = 3, .todam = 8, .carry_w = 700, .wield_w = 40}, /* 20  */
  {.tohit = 4, .todam = 9, .carry_w = 810, .wield_w = 40},
  {.tohit = 4, .todam = 10, .carry_w = 970, .wield_w = 40},
  {.tohit = 5, .todam = 11, .carry_w = 1130, .wield_w = 40},
  {.tohit = 6, .todam = 12, .carry_w = 1440, .wield_w = 40},
  {.tohit = 7, .todam = 14, .carry_w = 1750, .wield_w = 40}, /* 25            */
  {.tohit = 1, .todam = 3, .carry_w = 280, .wield_w = 22},   /* 18/01-50      */
  {.tohit = 2, .todam = 3, .carry_w = 305, .wield_w = 24},   /* 18/51-75      */
  {.tohit = 2, .todam = 4, .carry_w = 330, .wield_w = 26},   /* 18/76-90      */
  {.tohit = 2, .todam = 5, .carry_w = 380, .wield_w = 28},   /* 18/91-99      */
  {.tohit = 3, .todam = 6, .carry_w = 480, .wield_w = 30}    /* 18/100   (30) */
};

/* [dex] skillapply (thieves only) */
const struct dex_skill_type dex_app_skill[26] = {
  {.p_pocket = -99,
    .p_locks = -99,
    .traps = -90,
    .sneak = -99,
    .hide = -60,
    .volume = 2000}, /* 0 */
  {.p_pocket = -90,
    .p_locks = -90,
    .traps = -60,
    .sneak = -90,
    .hide = -50,
    .volume = 4000}, /* 1 */
  {.p_pocket = -80,
    .p_locks = -80,
    .traps = -40,
    .sneak = -80,
    .hide = -45,
    .volume = 6000},
  {.p_pocket = -70,
    .p_locks = -70,
    .traps = -30,
    .sneak = -70,
    .hide = -40,
    .volume = 8000},
  {.p_pocket = -60,
    .p_locks = -60,
    .traps = -30,
    .sneak = -60,
    .hide = -35,
    .volume = 12000},
  {.p_pocket = -50,
    .p_locks = -50,
    .traps = -20,
    .sneak = -50,
    .hide = -30,
    .volume = 16000}, /* 5 */
  {.p_pocket = -40,
    .p_locks = -40,
    .traps = -20,
    .sneak = -40,
    .hide = -25,
    .volume = 20000},
  {.p_pocket = -30,
    .p_locks = -30,
    .traps = -15,
    .sneak = -30,
    .hide = -20,
    .volume = 24000},
  {.p_pocket = -20,
    .p_locks = -20,
    .traps = -15,
    .sneak = -20,
    .hide = -15,
    .volume = 28000},
  {.p_pocket = -15,
    .p_locks = -10,
    .traps = -10,
    .sneak = -20,
    .hide = -10,
    .volume = 34000},
  {.p_pocket = -10,
    .p_locks = -5,
    .traps = -10,
    .sneak = -15,
    .hide = -5,
    .volume = 40000}, /* 10 */
  {.p_pocket = -5,
    .p_locks = 0,
    .traps = -5,
    .sneak = -10,
    .hide = 0,
    .volume = 46000},
  {.p_pocket = 0,
    .p_locks = 0,
    .traps = 0,
    .sneak = -5,
    .hide = 0,
    .volume = 50000},
  {.p_pocket = 0,
    .p_locks = 0,
    .traps = 0,
    .sneak = 0,
    .hide = 0,
    .volume = 60000},
  {.p_pocket = 0,
    .p_locks = 0,
    .traps = 0,
    .sneak = 0,
    .hide = 0,
    .volume = 70000},
  {.p_pocket = 0,
    .p_locks = 0,
    .traps = 0,
    .sneak = 0,
    .hide = 0,
    .volume = 80000}, /* 15 */
  {.p_pocket = 0,
    .p_locks = 5,
    .traps = 0,
    .sneak = 0,
    .hide = 0,
    .volume = 100000},
  {.p_pocket = 5,
    .p_locks = 10,
    .traps = 0,
    .sneak = 5,
    .hide = 5,
    .volume = 120000},
  {.p_pocket = 10,
    .p_locks = 15,
    .traps = 5,
    .sneak = 10,
    .hide = 10,
    .volume = 150000},
  {.p_pocket = 15,
    .p_locks = 20,
    .traps = 10,
    .sneak = 15,
    .hide = 15,
    .volume = 200000},
  {.p_pocket = 15,
    .p_locks = 20,
    .traps = 10,
    .sneak = 15,
    .hide = 15,
    .volume = 200000}, /* 20 */
  {.p_pocket = 20,
    .p_locks = 25,
    .traps = 10,
    .sneak = 15,
    .hide = 20,
    .volume = 250000},
  {.p_pocket = 20,
    .p_locks = 25,
    .traps = 15,
    .sneak = 20,
    .hide = 20,
    .volume = 300000},
  {.p_pocket = 25,
    .p_locks = 25,
    .traps = 15,
    .sneak = 20,
    .hide = 20,
    .volume = 350000},
  {.p_pocket = 25,
    .p_locks = 30,
    .traps = 15,
    .sneak = 25,
    .hide = 25,
    .volume = 400000},
  {.p_pocket = 25,
    .p_locks = 30,
    .traps = 15,
    .sneak = 25,
    .hide = 25,
    .volume = 1000000} /* 25 */
};

/* [dex] apply (all) */
struct dex_app_type dex_app[26] = {
  {.reaction = -7, .miss_att = -7, .defensive = 60}, /* 0 */
  {.reaction = -6, .miss_att = -6, .defensive = 50}, /* 1 */
  {.reaction = -4, .miss_att = -4, .defensive = 50},
  {.reaction = -3, .miss_att = -3, .defensive = 40},
  {.reaction = -2, .miss_att = -2, .defensive = 30},
  {.reaction = -1, .miss_att = -1, .defensive = 20}, /* 5 */
  {.reaction = 0, .miss_att = 0, .defensive = 10},
  {.reaction = 0, .miss_att = 0, .defensive = 0},
  {.reaction = 0, .miss_att = 0, .defensive = 0},
  {.reaction = 0, .miss_att = 0, .defensive = 0},
  {.reaction = 0, .miss_att = 0, .defensive = 0}, /* 10 */
  {.reaction = 0, .miss_att = 0, .defensive = 0},
  {.reaction = 0, .miss_att = 0, .defensive = 0},
  {.reaction = 0, .miss_att = 0, .defensive = 0},
  {.reaction = 0, .miss_att = 0, .defensive = 0},
  {.reaction = 0, .miss_att = 0, .defensive = -10}, /* 15 */
  {.reaction = 1, .miss_att = 1, .defensive = -20},
  {.reaction = 2, .miss_att = 2, .defensive = -30},
  {.reaction = 2, .miss_att = 2, .defensive = -40},
  {.reaction = 3, .miss_att = 3, .defensive = -40},
  {.reaction = 3, .miss_att = 3, .defensive = -40}, /* 20 */
  {.reaction = 4, .miss_att = 4, .defensive = -50},
  {.reaction = 4, .miss_att = 4, .defensive = -50},
  {.reaction = 4, .miss_att = 4, .defensive = -50},
  {.reaction = 5, .miss_att = 5, .defensive = -60},
  {.reaction = 5, .miss_att = 5, .defensive = -60} /* 25 */
};

/* [con] apply (all) */
struct con_app_type con_app[26] = {
  {.hitp = -4, .shock = 20}, /* 0 */
  {.hitp = -3, .shock = 25}, /* 1 */
  {.hitp = -2, .shock = 30}, {.hitp = -2, .shock = 35},
  {.hitp = -1, .shock = 40}, {.hitp = -1, .shock = 45}, /* 5 */
  {.hitp = -1, .shock = 50}, {.hitp = 0, .shock = 55}, {.hitp = 0, .shock = 60},
  {.hitp = 0, .shock = 65}, {.hitp = 0, .shock = 70}, /* 10 */
  {.hitp = 0, .shock = 75}, {.hitp = 0, .shock = 80}, {.hitp = 0, .shock = 85},
  {.hitp = 0, .shock = 88}, {.hitp = 1, .shock = 90}, /* 15 */
  {.hitp = 2, .shock = 95}, {.hitp = 3, .shock = 97}, {.hitp = 3, .shock = 99},
  {.hitp = 4, .shock = 99}, {.hitp = 5, .shock = 99}, /* 20 */
  {.hitp = 6, .shock = 99}, {.hitp = 6, .shock = 99}, {.hitp = 7, .shock = 99},
  {.hitp = 8, .shock = 99}, {.hitp = 9, .shock = 100} /* 25 */
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
