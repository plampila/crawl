#ifdef USE_TILE

#include <stdio.h>
#include "AppHdr.h"
#include "direct.h"
#include "externs.h"
#include "food.h"
#include "itemname.h"
#include "items.h"
#include "itemprop.h"
#include "macro.h"
#include "monstuff.h"
#include "mon-util.h"
#include "player.h"
#include "randart.h"
#include "stuff.h"
#include "terrain.h"
#include "tiles.h"
#include "tiledef-p.h"
#include "view.h"

#ifdef TILEP_DEBUG
#include "tilep-cmt.h"
#endif

/* external */
extern short get_helmet_type( const item_def &item );
extern bool player_is_airborne(void);
extern bool item_cursed( const item_def &item );
extern bool item_ident( const item_def &item, unsigned long flags );

// tile index cache to reduce tileidx() calls
static FixedArray < unsigned short, GXM, GYM > tile_dngn;
// // gv backup
static FixedArray < unsigned char, GXM, GYM > gv_now;
static FixedArray < unsigned char, GXM, GYM > wall_flag;
#define WFLAG_L 1
#define WFLAG_U 2
#define WFLAG_D 4
#define WFLAG_R 8

void TileNewLevel(bool first_time)
{
    GmapInit(false);
    TileLoadWall(false);
    tile_clear_buf();
    if (first_time)
        tile_init_flavor();
}

const int TILE_TODO = TILE_ERROR;

/**** tile index routines ****/

int tile_unseen_flag(const coord_def& gc)
{
    if (!map_bounds(gc))
        return TILE_FLAG_UNSEEN;
    else if (is_terrain_known(gc.x,gc.y)
        && !is_terrain_seen(gc.x,gc.y)
        || is_envmap_detected_item(gc.x,gc.y)
        || is_envmap_detected_mons(gc.x,gc.y))
    {
        return TILE_FLAG_MM_UNSEEN;
    }
    else
    {
        return TILE_FLAG_UNSEEN;
    }
}

int tileidx_monster(int mon_idx)
{
  int ch = TILE_ERROR;
  int mons = menv[mon_idx].type;

  switch(mons)
  {
  case MONS_GIANT_ANT: ch=TILE_MONS_GIANT_ANT; break;
  case MONS_GIANT_BAT: ch=TILE_MONS_GIANT_BAT; break;
  case MONS_CENTAUR: ch=TILE_MONS_CENTAUR; break;
  case MONS_RED_DEVIL: ch=TILE_MONS_RED_DEVIL; break;
  case MONS_ETTIN: ch=TILE_MONS_ETTIN; break;
  case MONS_FUNGUS: ch=TILE_MONS_FUNGUS; break;
  case MONS_GOBLIN: ch=TILE_MONS_GOBLIN; break;
  case MONS_HOUND: ch=TILE_MONS_HOUND; break;
  case MONS_IMP: ch=TILE_MONS_IMP; break;
  case MONS_JACKAL: ch=TILE_MONS_JACKAL; break;
  case MONS_KILLER_BEE: ch=TILE_MONS_KILLER_BEE; break;
  case MONS_KILLER_BEE_LARVA: ch=TILE_MONS_KILLER_BEE_LARVA; break;
  case MONS_MANTICORE: ch=TILE_MONS_MANTICORE; break;
  case MONS_NECROPHAGE: ch=TILE_MONS_NECROPHAGE; break;
  case MONS_ORC: ch=TILE_MONS_ORC; break;
  case MONS_PHANTOM: ch=TILE_MONS_PHANTOM; break;
  case MONS_QUASIT: ch=TILE_MONS_QUASIT; break;
  case MONS_RAT: ch=TILE_MONS_RAT; break;
  case MONS_SCORPION: ch=TILE_MONS_SCORPION; break;
  case MONS_UGLY_THING : ch=TILE_MONS_UGLY_THING ; break;
  case MONS_FIRE_VORTEX: ch=TILE_MONS_FIRE_VORTEX; break;
  case MONS_WORM: ch=TILE_MONS_WORM; break;
  case MONS_ABOMINATION_SMALL: ch=TILE_MONS_ABOMINATION_SMALL; break;
  case MONS_YELLOW_WASP: ch=TILE_MONS_YELLOW_WASP; break;
  case MONS_ZOMBIE_SMALL: ch=TILE_MONS_ZOMBIE_SMALL; break;
  case MONS_ANGEL: ch=TILE_MONS_ANGEL; break;
  case MONS_GIANT_BEETLE: ch=TILE_MONS_GIANT_BEETLE; break;
  case MONS_CYCLOPS: ch=TILE_MONS_CYCLOPS; break;
  case MONS_DRAGON: ch=TILE_MONS_DRAGON; break;
  case MONS_TWO_HEADED_OGRE: ch=TILE_MONS_TWO_HEADED_OGRE; break;
  case MONS_FIEND: ch=TILE_MONS_FIEND; break;
  case MONS_GIANT_SPORE: ch=TILE_MONS_GIANT_SPORE; break;
  case MONS_HOBGOBLIN: ch=TILE_MONS_HOBGOBLIN; break;
  case MONS_ICE_BEAST: ch=TILE_MONS_ICE_BEAST; break;
  case MONS_JELLY: ch=TILE_MONS_JELLY; break;
  case MONS_KOBOLD: ch=TILE_MONS_KOBOLD; break;
  case MONS_LICH: ch=TILE_MONS_LICH; break;
  case MONS_MUMMY: ch=TILE_MONS_MUMMY; break;
  case MONS_GUARDIAN_NAGA: ch=TILE_MONS_GUARDIAN_NAGA; break;
  case MONS_OGRE: ch=TILE_MONS_OGRE; break;
  case MONS_PLANT: ch=TILE_MONS_PLANT; break;
  case MONS_QUEEN_BEE: ch=TILE_MONS_QUEEN_BEE; break;
  case MONS_RAKSHASA: ch=TILE_MONS_RAKSHASA; break;
  case MONS_SNAKE: ch=TILE_MONS_SNAKE; break;
  case MONS_TROLL: ch=TILE_MONS_TROLL; break;
  case MONS_UNSEEN_HORROR: ch=TILE_MONS_UNSEEN_HORROR; break;
  case MONS_VAMPIRE: ch=TILE_MONS_VAMPIRE; break;
  case MONS_WRAITH: ch=TILE_MONS_WRAITH; break;

  case MONS_ABOMINATION_LARGE: 
  {
      // 7 different colours
      struct monsters *mon = &menv[mon_idx];
      ch = TILE_MONS_ABOMINATION_LARGE + ((mon->colour)%7);
      break;
  }

  case MONS_YAK: ch=TILE_MONS_YAK; break;
  case MONS_ZOMBIE_LARGE: ch=TILE_MONS_ZOMBIE_LARGE; break;
  case MONS_ORC_WARRIOR: ch=TILE_MONS_ORC_WARRIOR; break;
  case MONS_KOBOLD_DEMONOLOGIST: ch=TILE_MONS_KOBOLD_DEMONOLOGIST; break;
  case MONS_ORC_WIZARD: ch=TILE_MONS_ORC_WIZARD; break;
  case MONS_ORC_KNIGHT: ch=TILE_MONS_ORC_KNIGHT; break;
  case MONS_WYVERN: ch=TILE_MONS_WYVERN; break;
  case MONS_BIG_KOBOLD: ch=TILE_MONS_BIG_KOBOLD; break;
  case MONS_GIANT_EYEBALL: ch=TILE_MONS_GIANT_EYEBALL; break;
  case MONS_WIGHT: ch=TILE_MONS_WIGHT; break;
  case MONS_OKLOB_PLANT: ch=TILE_MONS_OKLOB_PLANT; break;
  case MONS_WOLF_SPIDER: ch=TILE_MONS_WOLF_SPIDER; break;
  case MONS_SHADOW: ch=TILE_MONS_SHADOW; break;
  case MONS_HUNGRY_GHOST: ch=TILE_MONS_HUNGRY_GHOST; break;
  case MONS_EYE_OF_DRAINING: ch=TILE_MONS_EYE_OF_DRAINING; break;

  case MONS_BUTTERFLY: 
  {
      // 7 different colours
      struct monsters *mon = &menv[mon_idx];
      ch = TILE_MONS_BUTTERFLY + ((mon->colour)%7);
      break;
  }

  case MONS_WANDERING_MUSHROOM: ch=TILE_MONS_WANDERING_MUSHROOM; break;
  case MONS_EFREET: ch=TILE_MONS_EFREET; break;
  case MONS_BRAIN_WORM: ch=TILE_MONS_BRAIN_WORM; break;
  case MONS_GIANT_ORANGE_BRAIN: ch=TILE_MONS_GIANT_ORANGE_BRAIN; break;
  case MONS_BOULDER_BEETLE: ch=TILE_MONS_BOULDER_BEETLE; break;
  case MONS_FLYING_SKULL: ch=TILE_MONS_FLYING_SKULL; break;
  case MONS_HELL_HOUND: ch=TILE_MONS_HELL_HOUND; break;
  case MONS_MINOTAUR: ch=TILE_MONS_MINOTAUR; break;
  case MONS_ICE_DRAGON: ch=TILE_MONS_ICE_DRAGON; break;
  case MONS_SLIME_CREATURE: ch=TILE_MONS_SLIME_CREATURE; break;
  case MONS_FREEZING_WRAITH: ch=TILE_MONS_FREEZING_WRAITH; break;
  case MONS_RAKSHASA_FAKE: ch=TILE_MONS_RAKSHASA_FAKE; break;
  case MONS_GREAT_ORB_OF_EYES: ch=TILE_MONS_GREAT_ORB_OF_EYES; break;
  case MONS_HELLION: ch=TILE_MONS_HELLION; break;
  case MONS_ROTTING_DEVIL: ch=TILE_MONS_ROTTING_DEVIL; break;
  case MONS_TORMENTOR: ch=TILE_MONS_TORMENTOR; break;
  case MONS_REAPER: ch=TILE_MONS_REAPER; break;
  case MONS_SOUL_EATER: ch=TILE_MONS_SOUL_EATER; break;
  case MONS_HAIRY_DEVIL: ch=TILE_MONS_HAIRY_DEVIL; break;
  case MONS_ICE_DEVIL: ch=TILE_MONS_ICE_DEVIL; break;
  case MONS_BLUE_DEVIL: ch=TILE_MONS_BLUE_DEVIL; break;
  case MONS_BEAST: ch=TILE_MONS_BEAST; break;
  case MONS_IRON_DEVIL: ch=TILE_MONS_IRON_DEVIL; break;
  case MONS_GLOWING_SHAPESHIFTER: ch=TILE_MONS_GLOWING_SHAPESHIFTER; break;
  case MONS_SHAPESHIFTER: ch=TILE_MONS_SHAPESHIFTER; break;
  case MONS_GIANT_MITE: ch=TILE_MONS_GIANT_MITE; break;
  case MONS_STEAM_DRAGON: ch=TILE_MONS_STEAM_DRAGON; break;
  case MONS_VERY_UGLY_THING: ch=TILE_MONS_VERY_UGLY_THING; break;
  case MONS_ORC_SORCERER: ch=TILE_MONS_ORC_SORCERER; break;
  case MONS_HIPPOGRIFF: ch=TILE_MONS_HIPPOGRIFF; break;
  case MONS_GRIFFON: ch=TILE_MONS_GRIFFON; break;

  case MONS_HYDRA: 
  {
      // Number of heads
      struct monsters *mon = &menv[mon_idx];
      int  heads = mon->number;
      if (heads > 7) heads = 7;
      ch = TILE_MONS_HYDRA + heads - 1;
      break;
  }

  case MONS_SKELETON_SMALL: ch=TILE_MONS_SKELETON_SMALL; break;
  case MONS_SKELETON_LARGE: ch=TILE_MONS_SKELETON_LARGE; break;
  case MONS_HELL_KNIGHT: ch=TILE_MONS_HELL_KNIGHT; break;
  case MONS_NECROMANCER: ch=TILE_MONS_NECROMANCER; break;
  case MONS_WIZARD: ch=TILE_MONS_WIZARD; break;
  case MONS_ORC_PRIEST: ch=TILE_MONS_ORC_PRIEST; break;
  case MONS_ORC_HIGH_PRIEST: ch=TILE_MONS_ORC_HIGH_PRIEST; break;
  case MONS_HUMAN: ch=TILE_MONS_HUMAN; break;
  case MONS_GNOLL: ch=TILE_MONS_GNOLL; break;
  case MONS_CLAY_GOLEM: ch=TILE_MONS_CLAY_GOLEM; break;
  case MONS_WOOD_GOLEM: ch=TILE_MONS_WOOD_GOLEM; break;
  case MONS_STONE_GOLEM: ch=TILE_MONS_STONE_GOLEM; break;
  case MONS_IRON_GOLEM: ch=TILE_MONS_IRON_GOLEM; break;
  case MONS_CRYSTAL_GOLEM: ch=TILE_MONS_CRYSTAL_GOLEM; break;
  case MONS_TOENAIL_GOLEM: ch=TILE_MONS_TOENAIL_GOLEM; break;
  case MONS_MOTTLED_DRAGON: ch=TILE_MONS_MOTTLED_DRAGON; break;
  case MONS_EARTH_ELEMENTAL: ch=TILE_MONS_EARTH_ELEMENTAL; break;
  case MONS_FIRE_ELEMENTAL: ch=TILE_MONS_FIRE_ELEMENTAL; break;
  case MONS_AIR_ELEMENTAL: ch=TILE_MONS_AIR_ELEMENTAL; break;
  case MONS_ICE_FIEND: ch=TILE_MONS_ICE_FIEND; break;
  case MONS_SHADOW_FIEND: ch=TILE_MONS_SHADOW_FIEND; break;
  case MONS_BROWN_SNAKE: ch=TILE_MONS_BROWN_SNAKE; break;
  case MONS_GIANT_LIZARD: ch=TILE_MONS_GIANT_LIZARD; break;
  case MONS_SPECTRAL_WARRIOR: ch=TILE_MONS_SPECTRAL_WARRIOR; break;
  case MONS_PULSATING_LUMP: ch=TILE_MONS_PULSATING_LUMP; break;
  case MONS_STORM_DRAGON: ch=TILE_MONS_STORM_DRAGON; break;
  case MONS_YAKTAUR: ch=TILE_MONS_YAKTAUR; break;
  case MONS_DEATH_YAK: ch=TILE_MONS_DEATH_YAK; break;
  case MONS_ROCK_TROLL: ch=TILE_MONS_ROCK_TROLL; break;
  case MONS_STONE_GIANT: ch=TILE_MONS_STONE_GIANT; break;
  case MONS_FLAYED_GHOST: ch=TILE_MONS_FLAYED_GHOST; break;
  case MONS_BUMBLEBEE: ch=TILE_MONS_BUMBLEBEE; break;
  case MONS_REDBACK: ch=TILE_MONS_REDBACK; break;
  case MONS_INSUBSTANTIAL_WISP: ch=TILE_MONS_INSUBSTANTIAL_WISP; break;
  case MONS_VAPOUR: ch=TILE_MONS_VAPOUR; break;
  case MONS_OGRE_MAGE: ch=TILE_MONS_OGRE_MAGE; break;
  case MONS_SPINY_WORM: ch=TILE_MONS_SPINY_WORM; break;

  case MONS_DANCING_WEAPON: 
  {
      // Use item tile
      item_def item = mitm[menv[mon_idx].inv[MSLOT_WEAPON]];
      ch = tileidx_item(item);
      break;
  }

  case MONS_TITAN: ch=TILE_MONS_TITAN; break;
  case MONS_GOLDEN_DRAGON: ch=TILE_MONS_GOLDEN_DRAGON; break;
  case MONS_ELF: ch=TILE_MONS_ELF; break;
  case MONS_LINDWURM: ch=TILE_MONS_LINDWURM; break;
  case MONS_ELEPHANT_SLUG: ch=TILE_MONS_ELEPHANT_SLUG; break;
  case MONS_WAR_DOG: ch=TILE_MONS_WAR_DOG; break;
  case MONS_GREY_RAT: ch=TILE_MONS_GREY_RAT; break;
  case MONS_GREEN_RAT: ch=TILE_MONS_GREEN_RAT; break;
  case MONS_ORANGE_RAT: ch=TILE_MONS_ORANGE_RAT; break;
  case MONS_BLACK_SNAKE: ch=TILE_MONS_BLACK_SNAKE; break;
  case MONS_SHEEP: ch=TILE_MONS_SHEEP; break;
  case MONS_GHOUL: ch=TILE_MONS_GHOUL; break;
  case MONS_HOG: ch=TILE_MONS_HOG; break;
  case MONS_GIANT_MOSQUITO: ch=TILE_MONS_GIANT_MOSQUITO; break;
  case MONS_GIANT_CENTIPEDE: ch=TILE_MONS_GIANT_CENTIPEDE; break;
  case MONS_IRON_TROLL: ch=TILE_MONS_IRON_TROLL; break;
  case MONS_NAGA: ch=TILE_MONS_NAGA; break;
  case MONS_FIRE_GIANT: ch=TILE_MONS_FIRE_GIANT; break;
  case MONS_FROST_GIANT: ch=TILE_MONS_FROST_GIANT; break;
  case MONS_FIREDRAKE: ch=TILE_MONS_FIREDRAKE; break;
  case MONS_SHADOW_DRAGON: ch=TILE_MONS_SHADOW_DRAGON; break;
  case MONS_YELLOW_SNAKE: ch=TILE_MONS_YELLOW_SNAKE; break;
  case MONS_GREY_SNAKE: ch=TILE_MONS_GREY_SNAKE; break;
  case MONS_DEEP_TROLL: ch=TILE_MONS_DEEP_TROLL; break;
  case MONS_GIANT_BLOWFLY: ch=TILE_MONS_GIANT_BLOWFLY; break;
  case MONS_RED_WASP: ch=TILE_MONS_RED_WASP; break;
  case MONS_SWAMP_DRAGON: ch=TILE_MONS_SWAMP_DRAGON; break;
  case MONS_SWAMP_DRAKE: ch=TILE_MONS_SWAMP_DRAKE; break;
  case MONS_SOLDIER_ANT: ch=TILE_MONS_SOLDIER_ANT; break;
  case MONS_HILL_GIANT: ch=TILE_MONS_HILL_GIANT; break;
  case MONS_QUEEN_ANT: ch=TILE_MONS_QUEEN_ANT; break;
  case MONS_ANT_LARVA: ch=TILE_MONS_ANT_LARVA; break;
  case MONS_GIANT_FROG: ch=TILE_MONS_GIANT_FROG; break;
  case MONS_GIANT_BROWN_FROG: ch=TILE_MONS_GIANT_BROWN_FROG; break;
  case MONS_SPINY_FROG: ch=TILE_MONS_SPINY_FROG; break;
  case MONS_BLINK_FROG: ch=TILE_MONS_BLINK_FROG; break;
  case MONS_GIANT_COCKROACH: ch=TILE_MONS_GIANT_COCKROACH; break;
  case MONS_SMALL_SNAKE: ch=TILE_MONS_SMALL_SNAKE; break;
  case MONS_SHUGGOTH: ch=TILE_TODO; break;
  case MONS_WOLF: ch=TILE_MONS_WOLF; break;
  case MONS_WARG: ch=TILE_MONS_WARG; break;
  case MONS_BEAR: ch=TILE_MONS_BEAR; break;
  case MONS_GRIZZLY_BEAR: ch=TILE_MONS_GRIZZLY_BEAR; break;
  case MONS_POLAR_BEAR: ch=TILE_MONS_POLAR_BEAR; break;
  case MONS_BLACK_BEAR: ch=TILE_MONS_BLACK_BEAR; break;
  case MONS_SIMULACRUM_SMALL: ch=TILE_MONS_SIMULACRUM_SMALL; break;
  case MONS_SIMULACRUM_LARGE: ch=TILE_MONS_SIMULACRUM_LARGE; break;
  case MONS_WHITE_IMP: ch=TILE_MONS_WHITE_IMP; break;
  case MONS_LEMURE: ch=TILE_MONS_LEMURE; break;
  case MONS_UFETUBUS: ch=TILE_MONS_UFETUBUS; break;
  case MONS_MANES: ch=TILE_MONS_MANES; break;
  case MONS_MIDGE: ch=TILE_MONS_MIDGE; break;
  case MONS_NEQOXEC: ch=TILE_MONS_NEQOXEC; break;
  case MONS_ORANGE_DEMON: ch=TILE_MONS_ORANGE_DEMON; break;
  case MONS_HELLWING: ch=TILE_MONS_HELLWING; break;
  case MONS_SMOKE_DEMON: ch=TILE_MONS_SMOKE_DEMON; break;
  case MONS_YNOXINUL: ch=TILE_MONS_YNOXINUL; break;
  case MONS_EXECUTIONER: ch=TILE_MONS_EXECUTIONER; break;
  case MONS_GREEN_DEATH: ch=TILE_MONS_GREEN_DEATH; break;
  case MONS_BLUE_DEATH: ch=TILE_MONS_BLUE_DEATH; break;
  case MONS_BALRUG: ch=TILE_MONS_BALRUG; break;
  case MONS_CACODEMON: ch=TILE_MONS_CACODEMON; break;
  case MONS_DEMONIC_CRAWLER: ch=TILE_MONS_DEMONIC_CRAWLER; break;
  case MONS_SUN_DEMON: ch=TILE_MONS_SUN_DEMON; break;
  case MONS_SHADOW_IMP: ch=TILE_MONS_SHADOW_IMP; break;
  case MONS_SHADOW_DEMON: ch=TILE_MONS_SHADOW_DEMON; break;
  case MONS_LOROCYPROCA: ch=TILE_MONS_LOROCYPROCA; break;
  case MONS_SHADOW_WRAITH: ch=TILE_MONS_SHADOW_WRAITH; break;
  case MONS_GIANT_AMOEBA: ch=TILE_MONS_GIANT_AMOEBA; break;
  case MONS_GIANT_SLUG: ch=TILE_MONS_GIANT_SLUG; break;
  case MONS_GIANT_SNAIL: ch=TILE_MONS_GIANT_SNAIL; break;
  case MONS_SPATIAL_VORTEX: ch=TILE_MONS_SPATIAL_VORTEX; break;
  case MONS_PIT_FIEND: ch=TILE_MONS_PIT_FIEND; break;
  case MONS_BORING_BEETLE: ch=TILE_MONS_BORING_BEETLE; break;
  case MONS_GARGOYLE: ch=TILE_MONS_GARGOYLE; break;
  case MONS_METAL_GARGOYLE: ch=TILE_MONS_METAL_GARGOYLE; break;
  case MONS_MOLTEN_GARGOYLE: ch=TILE_MONS_MOLTEN_GARGOYLE; break;
  case MONS_PROGRAM_BUG: ch=TILE_MONS_PROGRAM_BUG; break;
  case MONS_MNOLEG: ch=TILE_MONS_MNOLEG; break;
  case MONS_LOM_LOBON: ch=TILE_MONS_LOM_LOBON; break;
  case MONS_CEREBOV: ch=TILE_MONS_CEREBOV; break;
  case MONS_GLOORX_VLOQ: ch=TILE_MONS_GLOORX_VLOQ; break;
  case MONS_MOLLUSC_LORD: ch=TILE_TODO; break;
  case MONS_NAGA_MAGE: ch=TILE_MONS_NAGA_MAGE; break;
  case MONS_NAGA_WARRIOR: ch=TILE_MONS_NAGA_WARRIOR; break;
  case MONS_ORC_WARLORD: ch=TILE_MONS_ORC_WARLORD; break;
  case MONS_DEEP_ELF_SOLDIER: ch=TILE_MONS_DEEP_ELF_SOLDIER; break;
  case MONS_DEEP_ELF_FIGHTER: ch=TILE_MONS_DEEP_ELF_FIGHTER; break;
  case MONS_DEEP_ELF_KNIGHT: ch=TILE_MONS_DEEP_ELF_KNIGHT; break;
  case MONS_DEEP_ELF_MAGE: ch=TILE_MONS_DEEP_ELF_MAGE; break;
  case MONS_DEEP_ELF_SUMMONER: ch=TILE_MONS_DEEP_ELF_SUMMONER; break;
  case MONS_DEEP_ELF_CONJURER: ch=TILE_MONS_DEEP_ELF_CONJURER; break;
  case MONS_DEEP_ELF_PRIEST: ch=TILE_MONS_DEEP_ELF_PRIEST; break;
  case MONS_DEEP_ELF_HIGH_PRIEST: ch=TILE_MONS_DEEP_ELF_HIGH_PRIEST; break;
  case MONS_DEEP_ELF_DEMONOLOGIST: ch=TILE_MONS_DEEP_ELF_DEMONOLOGIST; break;
  case MONS_DEEP_ELF_ANNIHILATOR: ch=TILE_MONS_DEEP_ELF_ANNIHILATOR; break;
  case MONS_DEEP_ELF_SORCERER: ch=TILE_MONS_DEEP_ELF_SORCERER; break;
  case MONS_DEEP_ELF_DEATH_MAGE: ch=TILE_MONS_DEEP_ELF_DEATH_MAGE; break;
  case MONS_BROWN_OOZE: ch=TILE_MONS_BROWN_OOZE; break;
  case MONS_AZURE_JELLY: ch=TILE_MONS_AZURE_JELLY; break;
  case MONS_DEATH_OOZE: ch=TILE_MONS_DEATH_OOZE; break;
  case MONS_ACID_BLOB: ch=TILE_MONS_ACID_BLOB; break;
  case MONS_ROYAL_JELLY: ch=TILE_MONS_ROYAL_JELLY; break;
  case MONS_TERENCE: ch=TILE_MONS_TERENCE; break;
  case MONS_JESSICA: ch=TILE_MONS_JESSICA; break;
  case MONS_IJYB: ch=TILE_MONS_IJYB; break;
  case MONS_SIGMUND: ch=TILE_MONS_SIGMUND; break;
  case MONS_BLORK_THE_ORC: ch=TILE_MONS_BLORK_THE_ORC; break;
  case MONS_EDMUND: ch=TILE_MONS_EDMUND; break;
  case MONS_PSYCHE: ch=TILE_MONS_PSYCHE; break;
  case MONS_EROLCHA: ch=TILE_MONS_EROLCHA; break;
  case MONS_DONALD: ch=TILE_MONS_DONALD; break;
  case MONS_URUG: ch=TILE_MONS_URUG; break;
  case MONS_MICHAEL: ch=TILE_MONS_MICHAEL; break;
  case MONS_JOSEPH: ch=TILE_MONS_JOSEPH; break;
  case MONS_SNORG: ch=TILE_MONS_SNORG; break;
  case MONS_ERICA: ch=TILE_MONS_ERICA; break;
  case MONS_JOSEPHINE: ch=TILE_MONS_JOSEPHINE; break;
  case MONS_HAROLD: ch=TILE_MONS_HAROLD; break;
  case MONS_NORBERT: ch=TILE_MONS_NORBERT; break;
  case MONS_JOZEF: ch=TILE_MONS_JOZEF; break;
  case MONS_AGNES: ch=TILE_MONS_AGNES; break;
  case MONS_MAUD: ch=TILE_MONS_MAUD; break;
  case MONS_LOUISE: ch=TILE_MONS_LOUISE; break;
  case MONS_FRANCIS: ch=TILE_MONS_FRANCIS; break;
  case MONS_FRANCES: ch=TILE_MONS_FRANCES; break;
  case MONS_RUPERT: ch=TILE_MONS_RUPERT; break;
  case MONS_WAYNE: ch=TILE_MONS_WAYNE; break;
  case MONS_DUANE: ch=TILE_MONS_DUANE; break;
  case MONS_XTAHUA: ch=TILE_MONS_XTAHUA; break;
  case MONS_NORRIS: ch=TILE_MONS_NORRIS; break;
  case MONS_FREDERICK: ch=TILE_MONS_FREDERICK; break;
  case MONS_MARGERY: ch=TILE_MONS_MARGERY; break;
  case MONS_POLYPHEMUS: ch=TILE_MONS_POLYPHEMUS; break;
  case MONS_BORIS: ch=TILE_MONS_BORIS; break;
  // Draconians handled above
  case MONS_MURRAY: ch=TILE_MONS_MURRAY; break;
  case MONS_TIAMAT: ch=TILE_MONS_TIAMAT; break;
  case MONS_DEEP_ELF_BLADEMASTER: ch=TILE_MONS_DEEP_ELF_BLADEMASTER; break;
  case MONS_DEEP_ELF_MASTER_ARCHER: ch=TILE_MONS_DEEP_ELF_MASTER_ARCHER; break;

  case MONS_GERYON: ch=TILE_MONS_GERYON; break;
  case MONS_DISPATER: ch=TILE_MONS_DISPATER; break;
  case MONS_ASMODEUS: ch=TILE_MONS_ASMODEUS; break;
  case MONS_ANTAEUS: ch=TILE_MONS_ANTAEUS; break;
  case MONS_ERESHKIGAL: ch=TILE_MONS_ERESHKIGAL; break;

  case MONS_ANCIENT_LICH: ch=TILE_MONS_ANCIENT_LICH; break;
  case MONS_OOZE: ch=TILE_MONS_OOZE; break;
  case MONS_VAULT_GUARD: ch=TILE_MONS_VAULT_GUARD; break;
  case MONS_CURSE_SKULL: ch=TILE_MONS_CURSE_SKULL; break;
  case MONS_VAMPIRE_KNIGHT: ch=TILE_MONS_VAMPIRE_KNIGHT; break;
  case MONS_VAMPIRE_MAGE: ch=TILE_MONS_VAMPIRE_MAGE; break;
  case MONS_SHINING_EYE: ch=TILE_MONS_SHINING_EYE; break;
  case MONS_ORB_GUARDIAN: ch=TILE_MONS_ORB_GUARDIAN; break;
  case MONS_DAEVA: ch=TILE_MONS_DAEVA; break;
  case MONS_SPECTRAL_THING: ch=TILE_MONS_SPECTRAL_THING; break;
  case MONS_GREATER_NAGA: ch=TILE_MONS_GREATER_NAGA; break;
  case MONS_SKELETAL_DRAGON: ch=TILE_MONS_SKELETAL_DRAGON; break;
  case MONS_TENTACLED_MONSTROSITY: ch=TILE_MONS_TENTACLED_MONSTROSITY; break;
  case MONS_SPHINX: ch=TILE_MONS_SPHINX; break;
  case MONS_ROTTING_HULK: ch=TILE_MONS_ROTTING_HULK; break;
  case MONS_GUARDIAN_MUMMY: ch=TILE_MONS_GUARDIAN_MUMMY; break;
  case MONS_GREATER_MUMMY: ch=TILE_MONS_GREATER_MUMMY; break;
  case MONS_MUMMY_PRIEST: ch=TILE_MONS_MUMMY_PRIEST; break;
  case MONS_CENTAUR_WARRIOR: ch=TILE_MONS_CENTAUR_WARRIOR; break;
  case MONS_YAKTAUR_CAPTAIN: ch=TILE_MONS_YAKTAUR_CAPTAIN; break;
  case MONS_KILLER_KLOWN: ch=TILE_MONS_KILLER_KLOWN; break;
  case MONS_ELECTRIC_GOLEM: ch=TILE_MONS_ELECTRIC_GOLEM; break;
  case MONS_BALL_LIGHTNING: ch=TILE_MONS_BALL_LIGHTNING; break;
  case MONS_ORB_OF_FIRE: ch=TILE_MONS_ORB_OF_FIRE; break;
  case MONS_QUOKKA: ch=TILE_MONS_QUOKKA; break;
  case MONS_EYE_OF_DEVASTATION: ch=TILE_MONS_EYE_OF_DEVASTATION; break;
  case MONS_MOTH_OF_WRATH: ch=TILE_MONS_MOTH_OF_WRATH; break;
  case MONS_DEATH_COB: ch=TILE_MONS_DEATH_COB; break;
  case MONS_CURSE_TOE: ch=TILE_MONS_CURSE_TOE; break;
//  case MONS_GOLD_MIMIC: ch=TILE_MONS_GOLD_MIMIC; break;

  case MONS_GOLD_MIMIC:
  case MONS_WEAPON_MIMIC:
  case MONS_ARMOUR_MIMIC:
  case MONS_SCROLL_MIMIC:
  case MONS_POTION_MIMIC:
  {
      // Use item tile
      item_def  item;
      struct monsters *mon = &menv[mon_idx];
      get_mimic_item( mon, item );
      ch = tileidx_item(item);
      break;
  }

  case MONS_HELL_HOG: ch=TILE_MONS_HELL_HOG; break;
  case MONS_SERPENT_OF_HELL: ch=TILE_MONS_SERPENT_OF_HELL; break;
  case MONS_BOGGART: ch=TILE_MONS_BOGGART; break;
  case MONS_QUICKSILVER_DRAGON: ch=TILE_MONS_QUICKSILVER_DRAGON; break;
  case MONS_IRON_DRAGON: ch=TILE_MONS_IRON_DRAGON; break;
  case MONS_SKELETAL_WARRIOR: ch=TILE_MONS_SKELETAL_WARRIOR; break;
  case MONS_PLAYER_GHOST: ch=TILE_MONS_PLAYER_GHOST; break;
  case MONS_PANDEMONIUM_DEMON: ch=TILE_MONS_PANDEMONIUM_DEMON; break;
  case MONS_GIANT_NEWT: ch=TILE_MONS_GIANT_NEWT; break;
  case MONS_GIANT_GECKO: ch=TILE_MONS_GIANT_GECKO; break;
  case MONS_GIANT_IGUANA: ch=TILE_MONS_GIANT_IGUANA; break;
  case MONS_GILA_MONSTER: ch=TILE_MONS_GILA_MONSTER; break;
  case MONS_KOMODO_DRAGON: ch=TILE_MONS_KOMODO_DRAGON; break;
  case MONS_LAVA_WORM: ch=TILE_MONS_LAVA_WORM; break;
  case MONS_LAVA_FISH: ch=TILE_MONS_LAVA_FISH; break;
  case MONS_LAVA_SNAKE: ch=TILE_MONS_LAVA_SNAKE; break;
  case MONS_SALAMANDER: ch=TILE_MONS_SALAMANDER; break;
  case MONS_BIG_FISH: ch=TILE_MONS_BIG_FISH; break;
  case MONS_GIANT_GOLDFISH: ch=TILE_MONS_GIANT_GOLDFISH; break;
  case MONS_ELECTRICAL_EEL: ch=TILE_MONS_ELECTRICAL_EEL; break;
  case MONS_JELLYFISH: ch=TILE_MONS_JELLYFISH; break;
  case MONS_WATER_ELEMENTAL: ch=TILE_MONS_WATER_ELEMENTAL; break;
  case MONS_SWAMP_WORM: ch=TILE_MONS_SWAMP_WORM; break;
  case MONS_ORANGE_STATUE: ch = TILE_DNGN_ORANGE_CRYSTAL_STATUE; break;
  case MONS_SILVER_STATUE: ch = TILE_DNGN_SILVER_STATUE; break;
  case MONS_ICE_STATUE: ch = TILE_DNGN_ICE_STATUE; break;
  case MONS_DEATH_DRAKE: ch=TILE_MONS_DEATH_DRAKE; break;
}
    if(mons_flies(&menv[mon_idx])) ch |= TILE_FLAG_FLYING;
    if(menv[mon_idx].has_ench(ENCH_HELD)) ch |= TILE_FLAG_NET;

    if(mons_friendly(&menv[mon_idx]))
    {
        ch |= TILE_FLAG_PET;
    }
    else if (Options.stab_brand != CHATTR_NORMAL && 
        mons_looks_stabbable(&menv[mon_idx]))
    {
        ch |= TILE_FLAG_STAB;
    }
    else if (Options.may_stab_brand != CHATTR_NORMAL && 
        mons_looks_distracted(&menv[mon_idx]))
    {
        ch |= TILE_FLAG_MAY_STAB;
    }
    return ch;
}

int tileidx_fixed_artifact(int special)
{
  int ch = TILE_ERROR;

  switch(special)
  {
  case SPWPN_SINGING_SWORD: ch=TILE_SPWPN_SINGING_SWORD; break;
  case SPWPN_WRATH_OF_TROG: ch=TILE_SPWPN_WRATH_OF_TROG; break;
  case SPWPN_SCYTHE_OF_CURSES: ch=TILE_SPWPN_SCYTHE_OF_CURSES; break;
  case SPWPN_MACE_OF_VARIABILITY: ch=TILE_SPWPN_MACE_OF_VARIABILITY; break;
  case SPWPN_GLAIVE_OF_PRUNE: ch=TILE_SPWPN_GLAIVE_OF_PRUNE; break;
  case SPWPN_SCEPTRE_OF_TORMENT: ch=TILE_SPWPN_SCEPTRE_OF_TORMENT; break;
  case SPWPN_SWORD_OF_ZONGULDROK: ch=TILE_SPWPN_SWORD_OF_ZONGULDROK; break;
  case SPWPN_SWORD_OF_CEREBOV: ch=TILE_SPWPN_SWORD_OF_CEREBOV; break;
  case SPWPN_STAFF_OF_DISPATER: ch=TILE_SPWPN_STAFF_OF_DISPATER; break;
  case SPWPN_SCEPTRE_OF_ASMODEUS: ch=TILE_SPWPN_SCEPTRE_OF_ASMODEUS; break;
  case SPWPN_SWORD_OF_POWER: ch=TILE_SPWPN_SWORD_OF_POWER; break;
  case SPWPN_KNIFE_OF_ACCURACY: ch=TILE_SPWPN_KNIFE_OF_ACCURACY; break;
  case SPWPN_STAFF_OF_OLGREB: ch=TILE_SPWPN_STAFF_OF_OLGREB; break;
  case SPWPN_VAMPIRES_TOOTH: ch=TILE_SPWPN_VAMPIRES_TOOTH; break;
  case SPWPN_STAFF_OF_WUCAD_MU: ch=TILE_SPWPN_STAFF_OF_WUCAD_MU; break;
  }
  return ch;
}

int tileidx_unrand_artifact(int idx)
{
    switch (idx)
    {
        case 1: return TILE_URAND_BLOODBANE;
        case 2: return TILE_URAND_SHADOWS;
        case 3: return TILE_URAND_FLAMING_DEATH;
        case 4: return TILE_URAND_IGNORANCE;
        case 5: return TILE_URAND_ZIN;
        case 6: return TILE_URAND_AUGMENTATION;
        case 7: return TILE_URAND_BRILLIANCE;
        case 8: return TILE_URAND_THIEF;
        case 9: return TILE_URAND_BULLSEYE;
        case 10: return TILE_URAND_DYROVEPREVA;
        case 11: return TILE_URAND_LEECH;
        case 12: return TILE_URAND_CEKUGOB;
        case 13: return TILE_URAND_MISFORTUNE;
        case 14: return TILE_URAND_CHILLY_DEATH;
        case 15: return TILE_URAND_FOUR_WINDS;
        case 16: return TILE_URAND_MORG;
        case 17: return TILE_URAND_FINISHER;
        case 18: return TILE_URAND_PUNK;
        case 19: return TILE_URAND_KRISHNA;
        case 20: return TILE_URAND_FLASH;
        case 21: return TILE_URAND_SKULLCRUSHER;
        case 22: return TILE_URAND_ASSASSIN;
        case 23: return TILE_URAND_GUARD;
        case 24: return TILE_URAND_JIHAD;
        case 25: return TILE_URAND_LEAR;
        case 26: return TILE_URAND_ZHOR;
        case 27: return TILE_URAND_FIERY_DEVIL;
        case 28: return TILE_URAND_SALAMANDER;
        case 29: return TILE_URAND_WAR;
        case 30: return TILE_URAND_DOOM_KNIGHT;
        case 31: return TILE_URAND_RESISTANCE;
        case 32: return TILE_URAND_FOLLY;
        case 33: return TILE_URAND_BLOODLUST;
        case 34: return TILE_URAND_EOS;
        case 35: return TILE_URAND_SHAOLIN;
        case 36: return TILE_URAND_ROBUSTNESS;
        case 37: return TILE_URAND_EDISON;
        case 38: return TILE_URAND_VOO_DOO;
        case 39: return TILE_URAND_OCTOPUS_KING;
        case 40: return TILE_URAND_DRAGONMASK;
        case 41: return TILE_URAND_ARGA;
        case 42: return TILE_URAND_ELEMENTAL;
        case 43: return TILE_URAND_SNIPER;
        case 44: return TILE_URAND_ERCHIDEL;
        case 45: return TILE_URAND_NIGHT;
        case 46: return TILE_URAND_PLUTONIUM;
        case 47: return TILE_URAND_UNDERADHUNTER;
        case 48: return TILE_URAND_DRAGON_KING;
        case 49: return TILE_URAND_ALCHEMIST;
        case 50: return TILE_URAND_FENCER;
        case 51: return TILE_URAND_MAGE;
        case 52: return TILE_URAND_BLOWGUN;
    }
    return 0;
}

int get_etype(const item_def &item)
{
    int etype;
    switch (item.flags & ISFLAG_COSMETIC_MASK)
    {
        case ISFLAG_EMBROIDERED_SHINY:
            etype = 1;
            break;
        case ISFLAG_RUNED:
            etype = 2;
            break;
        case ISFLAG_GLOWING:
            etype = 3;
            break;
        default:
            etype = is_random_artefact(item) ? 4 : 0;
            break;
    }

    return etype;
}

int tileidx_weapon(const item_def &item)
{
  int ch = TILE_ERROR;
  int race = item.flags & ISFLAG_RACIAL_MASK;
  int etype = get_etype(item);

  static const int etable[4][4] = {
    {0, 0, 0, 0}, // No ego tile
    {0, 1, 1, 1}, // One ego tile
    {0, 1, 1, 2}, // Two ego tile
    {0, 1, 2, 3}
  };

    if (etype > 1)
        etype--;

  switch(item.sub_type)
  {
    case WPN_KNIFE: ch=TILE_WPN_KNIFE; break;
    case WPN_DAGGER: 
      ch=TILE_WPN_DAGGER;
      if (race == ISFLAG_ORCISH) ch = TILE_WPN_DAGGER_ORC;
      if (race == ISFLAG_ELVEN ) ch = TILE_WPN_DAGGER_ELF;
      break;
    case WPN_SHORT_SWORD: 
      ch=TILE_WPN_SHORT_SWORD + etable[1][etype];
      if (race == ISFLAG_ORCISH) ch = TILE_WPN_SHORT_SWORD_ORC;
      if (race == ISFLAG_ELVEN ) ch = TILE_WPN_SHORT_SWORD_ELF;
      break;
    case WPN_QUICK_BLADE: ch=TILE_WPN_QUICK_BLADE; break;

    case WPN_SABRE: 
      ch=TILE_WPN_SABRE + etable[1][etype];
      break;
    case WPN_FALCHION: ch=TILE_WPN_FALCHION; break;
    case WPN_KATANA: 
      ch=TILE_WPN_KATANA + etable[1][etype];
      break;
    case WPN_LONG_SWORD:
      ch=TILE_WPN_LONG_SWORD + etable[1][etype];
      if (race == ISFLAG_ORCISH) ch = TILE_WPN_LONG_SWORD_ORC;
      break;
    case WPN_GREAT_SWORD:
      ch=TILE_WPN_GREAT_SWORD + etable[1][etype];
      if (race == ISFLAG_ORCISH) ch = TILE_WPN_GREAT_SWORD_ORC;
      break;
    case WPN_SCIMITAR:
      ch=TILE_WPN_SCIMITAR + etable[1][etype];
      break;
    case WPN_DOUBLE_SWORD: ch=TILE_WPN_DOUBLE_SWORD; break;
    case WPN_TRIPLE_SWORD: ch=TILE_WPN_TRIPLE_SWORD; break;

    case WPN_HAND_AXE: ch=TILE_WPN_HAND_AXE; break;
    case WPN_WAR_AXE: ch=TILE_WPN_WAR_AXE; break;
    case WPN_BROAD_AXE:
      ch=TILE_WPN_BROAD_AXE + etable[1][etype];
      break;
    case WPN_BATTLEAXE:
      ch=TILE_WPN_BATTLEAXE + etable[1][etype];
      break;
    case WPN_EXECUTIONERS_AXE:
      ch=TILE_WPN_EXECUTIONERS_AXE + etable[1][etype];
      break;

    case WPN_BLOWGUN:
      ch=TILE_WPN_BLOWGUN + etable[1][etype];
      break;
    case WPN_SLING: ch=TILE_WPN_SLING; break;
    case WPN_BOW:
      ch=TILE_WPN_BOW + etable[1][etype];
      break;
    case WPN_CROSSBOW:
      ch=TILE_WPN_CROSSBOW + etable[1][etype];
      break;
    case WPN_HAND_CROSSBOW:
      ch=TILE_WPN_HAND_CROSSBOW + etable[1][etype];
      break;

    case WPN_SPEAR: 
      ch=TILE_WPN_SPEAR + etable[1][etype]; 
      break;
    case WPN_TRIDENT:
      ch=TILE_WPN_TRIDENT + etable[1][etype]; 
      break;
    case WPN_HALBERD:
      ch=TILE_WPN_HALBERD + etable[1][etype]; 
      break;
    case WPN_SCYTHE:
      ch=TILE_WPN_SCYTHE + etable[1][etype]; 
      break;
    case WPN_GLAIVE:
      ch=TILE_WPN_GLAIVE + etable[1][etype]; 
      if (race == ISFLAG_ORCISH) ch = TILE_WPN_GLAIVE_ORC;
      break;

    case WPN_QUARTERSTAFF: ch=TILE_WPN_QUARTERSTAFF; break;

    case WPN_CLUB: ch=TILE_WPN_CLUB; break;
    case WPN_HAMMER: 
      ch=TILE_WPN_HAMMER + etable[1][etype];
      break;
    case WPN_MACE:
      ch=TILE_WPN_MACE + etable[1][etype];
      break;
    case WPN_FLAIL: 
      ch=TILE_WPN_FLAIL + etable[1][etype];
      break;
    case WPN_SPIKED_FLAIL:
      ch=TILE_WPN_SPIKED_FLAIL + etable[1][etype];
      break;
    case WPN_GREAT_MACE: 
      ch=TILE_WPN_GREAT_MACE + etable[1][etype];
      break;
    case WPN_DIRE_FLAIL: 
      ch=TILE_WPN_GREAT_FLAIL + etable[1][etype];
      break;
    case WPN_MORNINGSTAR: 
      ch=TILE_WPN_MORNINGSTAR + etable[1][etype];
      break;
    case WPN_EVENINGSTAR:
      ch=TILE_WPN_EVENINGSTAR + etable[1][etype];
      break;

    case WPN_GIANT_CLUB: ch=TILE_WPN_GIANT_CLUB; break;
    case WPN_GIANT_SPIKED_CLUB: ch=TILE_WPN_GIANT_SPIKED_CLUB; break;

    case WPN_ANCUS: ch=TILE_WPN_ANCUS; break;
    case WPN_WHIP: ch=TILE_WPN_WHIP; break;

    case WPN_DEMON_BLADE: ch=TILE_WPN_DEMON_BLADE; break;
    case WPN_DEMON_WHIP: ch=TILE_WPN_DEMON_WHIP; break;
    case WPN_DEMON_TRIDENT: ch=TILE_WPN_DEMON_TRIDENT; break;
    case WPN_BLESSED_BLADE: ch=TILE_WPN_BLESSED_BLADE; break;
    case WPN_LONGBOW: ch=TILE_WPN_LONGBOW; break;
    case WPN_LAJATANG: ch=TILE_WPN_LAJATANG; break;
    case WPN_BARDICHE: ch=TILE_WPN_LOCHABER_AXE; break;
  }
  return ch;
}

int tileidx_missile(const item_def &item)
{
  int ch = TILE_ERROR;
  int brand = item.special;
  switch(item.sub_type)
  {
    case MI_STONE: ch=TILE_MI_STONE; break;
    case MI_ARROW: ch=TILE_MI_ARROW; break;
    case MI_BOLT: ch=TILE_MI_BOLT; break;
    case MI_DART: 
      ch=TILE_MI_DART; 
      if (brand == SPMSL_POISONED || brand == SPMSL_POISONED_II)
          ch = TILE_MI_DART_P;
      break;
    case MI_NEEDLE: 
      ch=TILE_MI_NEEDLE; 
      if (brand == SPMSL_POISONED || brand == SPMSL_POISONED_II)
          ch = TILE_MI_NEEDLE_P;
      break;
    case MI_LARGE_ROCK: ch=TILE_MI_LARGE_ROCK; break;
    case MI_SLING_BULLET: ch=TILE_MI_SLING_BULLET; break;
    case MI_JAVELIN: ch=TILE_MI_JAVELIN; break;
    case MI_THROWING_NET: ch=TILE_MI_THROWING_NET; break;
  }
  return ch;
}

int tileidx_armour(const item_def &item)
{
  int ch = TILE_ERROR;
  int race = item.flags & ISFLAG_RACIAL_MASK;
  int type=item.sub_type;
  int etype = get_etype(item);

  static const int etable[5][5] = {
    {0, 0, 0, 0, 0}, // No ego tile
    {0, 1, 1, 1, 1}, // One ego tile
    {0, 1, 1, 1, 2}, // Two ego tile
    {0, 1, 1, 2, 3},
    {0, 1, 2, 3, 4}
  };

  switch(type)
  {
    case ARM_ROBE: 
      ch= TILE_ARM_ROBE + etable[2][etype];
      break;
    case ARM_LEATHER_ARMOUR:
      ch=TILE_ARM_LEATHER_ARMOUR  + etable[2][etype];
      if (race == ISFLAG_ORCISH) ch = TILE_ARM_LEATHER_ARMOUR_ORC;
      if (race == ISFLAG_ELVEN ) ch = TILE_ARM_LEATHER_ARMOUR_ELF;
      break;
    case ARM_RING_MAIL:
      ch=TILE_ARM_RING_MAIL + etable[1][etype];
      if (race == ISFLAG_ORCISH) ch = TILE_ARM_RING_MAIL_ORC;
      if (race == ISFLAG_ELVEN ) ch = TILE_ARM_RING_MAIL_ELF;
      if (race == ISFLAG_DWARVEN ) ch = TILE_ARM_RING_MAIL_DWA;
      break;
    case ARM_SCALE_MAIL:
      ch=TILE_ARM_SCALE_MAIL + etable[1][etype];
      if (race == ISFLAG_ELVEN ) ch = TILE_ARM_SCALE_MAIL_ELF;
      break;
    case ARM_CHAIN_MAIL:
      ch=TILE_ARM_CHAIN_MAIL + etable[1][etype];
      if (race == ISFLAG_ELVEN) ch = TILE_ARM_CHAIN_MAIL_ELF;
      if (race == ISFLAG_ORCISH) ch = TILE_ARM_CHAIN_MAIL_ORC;
      break;
    case ARM_SPLINT_MAIL: ch=TILE_ARM_SPLINT_MAIL; break;
    case ARM_BANDED_MAIL: ch=TILE_ARM_BANDED_MAIL; break;
    case ARM_PLATE_MAIL:
      ch=TILE_ARM_PLATE_MAIL;
      if (race == ISFLAG_ORCISH) ch = TILE_ARM_PLATE_MAIL_ORC;
      break;

    case ARM_CRYSTAL_PLATE_MAIL: ch=TILE_ARM_CRYSTAL_PLATE_MAIL; break;

    case ARM_SHIELD: ch=TILE_ARM_SHIELD  + etable[2][etype]; break;

    case ARM_CLOAK: 
      ch=TILE_ARM_CLOAK + etable[3][etype];
      break;

    case ARM_WIZARD_HAT:
      ch=TILE_THELM_WIZARD_HAT + etable[1][etype]; break;
    case ARM_CAP:
      ch=TILE_THELM_CAP; break;
    case ARM_HELMET:
      ch=TILE_THELM_HELM  + etable[3][etype]; break;
    case ARM_GLOVES:
      ch=TILE_ARM_GLOVES  + etable[3][etype];
      break;
    case ARM_BOOTS: 
      ch=TILE_ARM_BOOTS + etable[3][etype];
      break;
    case ARM_BUCKLER: 
      ch=TILE_ARM_BUCKLER + etable[1][etype]; 
      break;
    case ARM_LARGE_SHIELD:
      ch=TILE_ARM_LARGE_SHIELD + etable[2][etype];
      break;
    case ARM_CENTAUR_BARDING: ch=TILE_ARM_CENTAUR_BARDING; break;
    case ARM_NAGA_BARDING: ch=TILE_ARM_NAGA_BARDING; break;

    case ARM_ANIMAL_SKIN: ch=TILE_ARM_ANIMAL_SKIN + etable[1][etype]; break;
    case ARM_TROLL_HIDE: ch=TILE_ARM_TROLL_HIDE; break;
    case ARM_TROLL_LEATHER_ARMOUR: ch=TILE_ARM_TROLL_LEATHER_ARMOUR; break;
    case ARM_DRAGON_HIDE: ch=TILE_ARM_DRAGON_HIDE; break;
    case ARM_DRAGON_ARMOUR: ch=TILE_ARM_DRAGON_ARMOUR; break;
    case ARM_ICE_DRAGON_HIDE: ch=TILE_ARM_ICE_DRAGON_HIDE; break;
    case ARM_ICE_DRAGON_ARMOUR: ch=TILE_ARM_ICE_DRAGON_ARMOUR; break;
    case ARM_STEAM_DRAGON_HIDE: ch=TILE_ARM_STEAM_DRAGON_HIDE; break;
    case ARM_STEAM_DRAGON_ARMOUR: ch=TILE_ARM_STEAM_DRAGON_ARMOUR; break;
    case ARM_MOTTLED_DRAGON_HIDE: ch=TILE_ARM_MOTTLED_DRAGON_HIDE; break;
    case ARM_MOTTLED_DRAGON_ARMOUR: ch=TILE_ARM_MOTTLED_DRAGON_ARMOUR; break;
    case ARM_STORM_DRAGON_HIDE: ch=TILE_ARM_STORM_DRAGON_HIDE; break;
    case ARM_STORM_DRAGON_ARMOUR: ch=TILE_ARM_STORM_DRAGON_ARMOUR; break;
    case ARM_GOLD_DRAGON_HIDE: ch=TILE_ARM_GOLD_DRAGON_HIDE; break;
    case ARM_GOLD_DRAGON_ARMOUR: ch=TILE_ARM_GOLD_DRAGON_ARMOUR; break;
    case ARM_SWAMP_DRAGON_HIDE: ch=TILE_ARM_SWAMP_DRAGON_HIDE; break;
    case ARM_SWAMP_DRAGON_ARMOUR: ch=TILE_ARM_SWAMP_DRAGON_ARMOUR; break;
  }
  return ch;
}

int tileidx_food(const item_def &item)
{
    int ch = TILE_ERROR;
    int type=item.sub_type;
    switch(type)
    {
    case FOOD_MEAT_RATION: ch=TILE_FOOD_MEAT_RATION; break;
    case FOOD_BREAD_RATION: ch=TILE_FOOD_BREAD_RATION; break;
    case FOOD_PEAR: ch=TILE_FOOD_PEAR; break;
    case FOOD_APPLE: ch=TILE_FOOD_APPLE; break;
    case FOOD_CHOKO: ch=TILE_FOOD_CHOKO; break;
    case FOOD_HONEYCOMB: ch=TILE_FOOD_HONEYCOMB; break;
    case FOOD_ROYAL_JELLY: ch=TILE_FOOD_ROYAL_JELLY; break;
    case FOOD_SNOZZCUMBER: ch=TILE_FOOD_SNOZZCUMBER; break;
    case FOOD_PIZZA: ch=TILE_FOOD_PIZZA; break;
    case FOOD_APRICOT: ch=TILE_FOOD_APRICOT; break;
    case FOOD_ORANGE: ch=TILE_FOOD_ORANGE; break;
    case FOOD_BANANA: ch=TILE_FOOD_BANANA; break;
    case FOOD_STRAWBERRY: ch=TILE_FOOD_STRAWBERRY; break;
    case FOOD_RAMBUTAN: ch=TILE_FOOD_RAMBUTAN; break;
    case FOOD_LEMON: ch=TILE_FOOD_LEMON; break;
    case FOOD_GRAPE: ch=TILE_FOOD_GRAPE; break;
    case FOOD_SULTANA: ch=TILE_FOOD_SULTANA; break;
    case FOOD_LYCHEE: ch=TILE_FOOD_LYCHEE; break;
    case FOOD_BEEF_JERKY: ch=TILE_FOOD_BEEF_JERKY; break;
    case FOOD_CHEESE: ch=TILE_FOOD_CHEESE; break;
    case FOOD_SAUSAGE: ch=TILE_FOOD_SAUSAGE; break;

    case FOOD_CHUNK: 
        ch=TILE_FOOD_CHUNK; 
        if (item.special < 100) ch = TILE_FOOD_CHUNK_ROTTEN;
        break;
    }

    return ch;
}

int tileidx_corpse(int mon)
{
  int ch = TILE_ERROR;
  switch(mon)
  {
  case MONS_GIANT_ANT: ch=TILE_CORPSE_GIANT_ANT; break;
  case MONS_GIANT_BAT: ch=TILE_CORPSE_GIANT_BAT; break;
  case MONS_CENTAUR: ch=TILE_CORPSE_CENTAUR; break;
  case MONS_GOBLIN: ch=TILE_CORPSE_GOBLIN; break;
  case MONS_HOUND: ch=TILE_CORPSE_HOUND; break;
  case MONS_JACKAL: ch=TILE_CORPSE_JACKAL; break;
  case MONS_KILLER_BEE: ch=TILE_CORPSE_KILLER_BEE; break;
  case MONS_KILLER_BEE_LARVA: ch=TILE_CORPSE_KILLER_BEE_LARVA; break;
  case MONS_MANTICORE: ch=TILE_CORPSE_MANTICORE; break;
  case MONS_NECROPHAGE: ch=TILE_CORPSE_NECROPHAGE; break;
  case MONS_ORC: ch=TILE_CORPSE_ORC; break;
  case MONS_RAT: ch=TILE_CORPSE_RAT; break;
  case MONS_SCORPION: ch=TILE_CORPSE_SCORPION; break;
  case MONS_UGLY_THING: ch=TILE_CORPSE_UGLY_THING; break;
  case MONS_WORM: ch=TILE_CORPSE_WORM; break;
  case MONS_YELLOW_WASP: ch=TILE_CORPSE_YELLOW_WASP; break;
  case MONS_GIANT_BEETLE: ch=TILE_CORPSE_GIANT_BEETLE; break;
  case MONS_CYCLOPS: ch=TILE_CORPSE_CYCLOPS; break;
  case MONS_DRAGON: ch=TILE_CORPSE_DRAGON; break;
  case MONS_TWO_HEADED_OGRE: ch=TILE_CORPSE_TWO_HEADED_OGRE; break;
  case MONS_HOBGOBLIN: ch=TILE_CORPSE_HOBGOBLIN; break;
  case MONS_KOBOLD: ch=TILE_CORPSE_KOBOLD; break;
  case MONS_GUARDIAN_NAGA: ch=TILE_CORPSE_GUARDIAN_NAGA; break;
  case MONS_OGRE: ch=TILE_CORPSE_OGRE; break;
  case MONS_QUEEN_BEE: ch=TILE_CORPSE_QUEEN_BEE; break;
  case MONS_SNAKE: ch=TILE_CORPSE_SNAKE; break;
  case MONS_TROLL: ch=TILE_CORPSE_TROLL; break;
  case MONS_YAK: ch=TILE_CORPSE_YAK; break;
  case MONS_WYVERN: ch=TILE_CORPSE_WYVERN; break;
  case MONS_GIANT_EYEBALL: ch=TILE_CORPSE_GIANT_EYEBALL; break;
  case MONS_WOLF_SPIDER: ch=TILE_CORPSE_WOLF_SPIDER; break;
  case MONS_EYE_OF_DRAINING: ch=TILE_CORPSE_EYE_OF_DRAINING; break;
  case MONS_BUTTERFLY: ch=TILE_CORPSE_BUTTERFLY; break;
  case MONS_BRAIN_WORM: ch=TILE_CORPSE_BRAIN_WORM; break;
  case MONS_GIANT_ORANGE_BRAIN: ch=TILE_CORPSE_GIANT_ORANGE_BRAIN; break;
  case MONS_BOULDER_BEETLE: ch=TILE_CORPSE_BOULDER_BEETLE; break;
  case MONS_MINOTAUR: ch=TILE_CORPSE_MINOTAUR; break;
  case MONS_ICE_DRAGON: ch=TILE_CORPSE_ICE_DRAGON; break;
  case MONS_GREAT_ORB_OF_EYES: ch=TILE_CORPSE_GREAT_ORB_OF_EYES; break;
  case MONS_GLOWING_SHAPESHIFTER: ch=TILE_CORPSE_GLOWING_SHAPESHIFTER; break;
  case MONS_SHAPESHIFTER: ch=TILE_CORPSE_SHAPESHIFTER; break;
  case MONS_GIANT_MITE: ch=TILE_CORPSE_GIANT_MITE; break;
  case MONS_STEAM_DRAGON: ch=TILE_CORPSE_STEAM_DRAGON; break;
  case MONS_VERY_UGLY_THING: ch=TILE_CORPSE_VERY_UGLY_THING; break;
//  case MONS_ORC_SORCERER: ch=TILE_CORPSE_ORC_SORCERER; break;
  case MONS_HIPPOGRIFF: ch=TILE_CORPSE_HIPPOGRIFF; break;
  case MONS_GRIFFON: ch=TILE_CORPSE_GRIFFON; break;
  case MONS_HYDRA: ch=TILE_CORPSE_HYDRA; break;
  case MONS_HELL_KNIGHT: ch=TILE_CORPSE_HELL_KNIGHT; break;
  case MONS_NECROMANCER: ch=TILE_CORPSE_NECROMANCER; break;
  case MONS_WIZARD: ch=TILE_CORPSE_WIZARD; break;
//  case MONS_ORC_PRIEST: ch=TILE_CORPSE_ORC_PRIEST; break;
//  case MONS_ORC_HIGH_PRIEST: ch=TILE_CORPSE_ORC_HIGH_PRIEST; break;
  case MONS_HUMAN: ch=TILE_CORPSE_HUMAN; break;
  case MONS_GNOLL: ch=TILE_CORPSE_GNOLL; break;
  case MONS_MOTTLED_DRAGON: ch=TILE_CORPSE_MOTTLED_DRAGON; break;
  case MONS_BROWN_SNAKE: ch=TILE_CORPSE_BROWN_SNAKE; break;
  case MONS_GIANT_LIZARD: ch=TILE_CORPSE_GIANT_LIZARD; break;
  case MONS_STORM_DRAGON: ch=TILE_CORPSE_STORM_DRAGON; break;
  case MONS_YAKTAUR: ch=TILE_CORPSE_YAKTAUR; break;
  case MONS_DEATH_YAK: ch=TILE_CORPSE_DEATH_YAK; break;
  case MONS_ROCK_TROLL: ch=TILE_CORPSE_ROCK_TROLL; break;
  case MONS_STONE_GIANT: ch=TILE_CORPSE_STONE_GIANT; break;
  case MONS_BUMBLEBEE: ch=TILE_CORPSE_BUMBLEBEE; break;
  case MONS_REDBACK: ch=TILE_CORPSE_REDBACK; break;
  case MONS_SPINY_WORM: ch=TILE_CORPSE_SPINY_WORM; break;
  case MONS_TITAN: ch=TILE_CORPSE_TITAN; break;
  case MONS_GOLDEN_DRAGON: ch=TILE_CORPSE_GOLDEN_DRAGON; break;
  case MONS_ELF: ch=TILE_CORPSE_ELF; break;
  case MONS_LINDWURM: ch=TILE_CORPSE_LINDWURM; break;
  case MONS_ELEPHANT_SLUG: ch=TILE_CORPSE_ELEPHANT_SLUG; break;
  case MONS_WAR_DOG: ch=TILE_CORPSE_WAR_DOG; break;
  case MONS_GREY_RAT: ch=TILE_CORPSE_GREY_RAT; break;
  case MONS_GREEN_RAT: ch=TILE_CORPSE_GREEN_RAT; break;
  case MONS_ORANGE_RAT: ch=TILE_CORPSE_ORANGE_RAT; break;
  case MONS_BLACK_SNAKE: ch=TILE_CORPSE_BLACK_SNAKE; break;
  case MONS_SHEEP: ch=TILE_CORPSE_SHEEP; break;
  case MONS_GHOUL: ch=TILE_CORPSE_GHOUL; break;
  case MONS_HOG: ch=TILE_CORPSE_HOG; break;
  case MONS_GIANT_MOSQUITO: ch=TILE_CORPSE_GIANT_MOSQUITO; break;
  case MONS_GIANT_CENTIPEDE: ch=TILE_CORPSE_GIANT_CENTIPEDE; break;
  case MONS_IRON_TROLL: ch=TILE_CORPSE_IRON_TROLL; break;
  case MONS_NAGA: ch=TILE_CORPSE_NAGA; break;
  case MONS_FIRE_GIANT: ch=TILE_CORPSE_FIRE_GIANT; break;
  case MONS_FROST_GIANT: ch=TILE_CORPSE_FROST_GIANT; break;
  case MONS_FIREDRAKE: ch=TILE_CORPSE_FIREDRAKE; break;
  case MONS_SHADOW_DRAGON: ch=TILE_CORPSE_SHADOW_DRAGON; break;
  case MONS_YELLOW_SNAKE: ch=TILE_CORPSE_YELLOW_SNAKE; break;
  case MONS_GREY_SNAKE: ch=TILE_CORPSE_GREY_SNAKE; break;
  case MONS_DEEP_TROLL: ch=TILE_CORPSE_DEEP_TROLL; break;
  case MONS_GIANT_BLOWFLY: ch=TILE_CORPSE_GIANT_BLOWFLY; break;
  case MONS_RED_WASP: ch=TILE_CORPSE_RED_WASP; break;
  case MONS_SWAMP_DRAGON: ch=TILE_CORPSE_SWAMP_DRAGON; break;
  case MONS_SWAMP_DRAKE: ch=TILE_CORPSE_SWAMP_DRAKE; break;
  case MONS_SOLDIER_ANT: ch=TILE_CORPSE_SOLDIER_ANT; break;
  case MONS_HILL_GIANT: ch=TILE_CORPSE_HILL_GIANT; break;
  case MONS_QUEEN_ANT: ch=TILE_CORPSE_QUEEN_ANT; break;
  case MONS_ANT_LARVA: ch=TILE_CORPSE_ANT_LARVA; break;
  case MONS_GIANT_FROG: ch=TILE_CORPSE_GIANT_FROG; break;
  case MONS_GIANT_BROWN_FROG: ch=TILE_CORPSE_GIANT_BROWN_FROG; break;
  case MONS_SPINY_FROG: ch=TILE_CORPSE_SPINY_FROG; break;
  case MONS_BLINK_FROG: ch=TILE_CORPSE_BLINK_FROG; break;
  case MONS_GIANT_COCKROACH: ch=TILE_CORPSE_GIANT_COCKROACH; break;
  case MONS_SMALL_SNAKE: ch=TILE_CORPSE_SMALL_SNAKE; break;
  case MONS_GIANT_AMOEBA: ch=TILE_CORPSE_GIANT_AMOEBA; break;
  case MONS_GIANT_SLUG: ch=TILE_CORPSE_GIANT_SLUG; break;
  case MONS_GIANT_SNAIL: ch=TILE_CORPSE_GIANT_SNAIL; break;
  case MONS_BORING_BEETLE: ch=TILE_CORPSE_BORING_BEETLE; break;
//  case MONS_NAGA_MAGE: ch=TILE_CORPSE_NAGA_MAGE; break;
//  case MONS_NAGA_WARRIOR: ch=TILE_CORPSE_NAGA_WARRIOR; break;
//  case MONS_ORC_WARLORD: ch=TILE_CORPSE_ORC_WARLORD; break;
//  case MONS_DEEP_ELF_SOLDIER: ch=TILE_CORPSE_DEEP_ELF_SOLDIER; break;
//  case MONS_DEEP_ELF_FIGHTER: ch=TILE_CORPSE_DEEP_ELF_FIGHTER; break;
//  case MONS_DEEP_ELF_KNIGHT: ch=TILE_CORPSE_DEEP_ELF_KNIGHT; break;
//  case MONS_DEEP_ELF_MAGE: ch=TILE_CORPSE_DEEP_ELF_MAGE; break;
//  case MONS_DEEP_ELF_SUMMONER: ch=TILE_CORPSE_DEEP_ELF_SUMMONER; break;
//  case MONS_DEEP_ELF_CONJURER: ch=TILE_CORPSE_DEEP_ELF_CONJURER; break;
//  case MONS_DEEP_ELF_PRIEST: ch=TILE_CORPSE_DEEP_ELF_PRIEST; break;
//  case MONS_DEEP_ELF_HIGH_PRIEST: ch=TILE_CORPSE_DEEP_ELF_HIGH_PRIEST; break;
//  case MONS_DEEP_ELF_DEMONOLOGIST: ch=TILE_CORPSE_DEEP_ELF_DEMONOLOGIST; break;
//  case MONS_DEEP_ELF_ANNIHILATOR: ch=TILE_CORPSE_DEEP_ELF_ANNIHILATOR; break;
//  case MONS_DEEP_ELF_SORCERER: ch=TILE_CORPSE_DEEP_ELF_SORCERER; break;
//  case MONS_DEEP_ELF_DEATH_MAGE: ch=TILE_CORPSE_DEEP_ELF_DEATH_MAGE; break;
  case MONS_GREATER_NAGA: ch=TILE_CORPSE_GREATER_NAGA; break;
//  case MONS_CENTAUR_WARRIOR: ch=TILE_CORPSE_CENTAUR_WARRIOR; break;
//  case MONS_YAKTAUR_CAPTAIN: ch=TILE_CORPSE_YAKTAUR_CAPTAIN; break;
  case MONS_QUOKKA: ch=TILE_CORPSE_QUOKKA; break;
//  case MONS_SHUGGOTH: ch=TILE_CORPSE_SHUGGOTH; break;
  case MONS_WOLF: ch=TILE_CORPSE_WOLF; break;
  case MONS_WARG: ch=TILE_CORPSE_WARG; break;
  case MONS_BEAR: ch=TILE_CORPSE_BEAR; break;
  case MONS_GRIZZLY_BEAR: ch=TILE_CORPSE_GRIZZLY_BEAR; break;
  case MONS_POLAR_BEAR: ch=TILE_CORPSE_POLAR_BEAR; break;
  case MONS_BLACK_BEAR: ch=TILE_CORPSE_BLACK_BEAR; break;
  case MONS_GIANT_NEWT: ch=TILE_CORPSE_GIANT_NEWT; break;
  case MONS_GIANT_GECKO: ch=TILE_CORPSE_GIANT_GECKO; break;
  case MONS_GIANT_IGUANA: ch=TILE_CORPSE_GIANT_IGUANA; break;
  case MONS_GILA_MONSTER: ch=TILE_CORPSE_GILA_MONSTER; break;
  case MONS_KOMODO_DRAGON: ch=TILE_CORPSE_KOMODO_DRAGON; break;

  case MONS_DRACONIAN: ch=TILE_CORPSE_DRACONIAN_BROWN; break;
  case MONS_BLACK_DRACONIAN: ch=TILE_CORPSE_DRACONIAN_BLACK; break;
  case MONS_YELLOW_DRACONIAN: ch=TILE_CORPSE_DRACONIAN_YELLOW; break;
  case MONS_GREEN_DRACONIAN: ch=TILE_CORPSE_DRACONIAN_GREEN; break;
  case MONS_MOTTLED_DRACONIAN: ch=TILE_CORPSE_DRACONIAN_MOTTLED; break;
  case MONS_PALE_DRACONIAN: ch=TILE_CORPSE_DRACONIAN_PALE; break;
  case MONS_PURPLE_DRACONIAN: ch=TILE_CORPSE_DRACONIAN_PURPLE; break;
  case MONS_RED_DRACONIAN: ch=TILE_CORPSE_DRACONIAN_RED; break;
  case MONS_WHITE_DRACONIAN: ch=TILE_CORPSE_DRACONIAN_WHITE; break;

  case MONS_DEATH_DRAKE: ch=TILE_CORPSE_DEATH_DRAKE; break;
  }
  return ch;
}

int tileidx_misc(int type)
{
  int ch = TILE_ERROR;
  switch(type)
  {
    case MISC_BOTTLED_EFREET: ch = TILE_MISC_BOTTLED_EFREET; break;
    case MISC_CRYSTAL_BALL_OF_SEEING: ch = TILE_MISC_CRYSTAL_BALL_OF_SEEING; break;
    case MISC_AIR_ELEMENTAL_FAN: ch = TILE_MISC_AIR_ELEMENTAL_FAN; break;
    case MISC_LAMP_OF_FIRE: ch = TILE_MISC_LAMP_OF_FIRE; break;
    case MISC_STONE_OF_EARTH_ELEMENTALS: ch = TILE_MISC_STONE_OF_EARTH_ELEMENTALS; break;
    case MISC_LANTERN_OF_SHADOWS: ch = TILE_MISC_LANTERN_OF_SHADOWS; break;
    case MISC_HORN_OF_GERYON: ch = TILE_MISC_HORN_OF_GERYON; break;
    case MISC_BOX_OF_BEASTS: ch = TILE_MISC_BOX_OF_BEASTS; break;
    case MISC_CRYSTAL_BALL_OF_ENERGY: ch = TILE_MISC_CRYSTAL_BALL_OF_ENERGY; break;
    case MISC_EMPTY_EBONY_CASKET: ch = TILE_MISC_EMPTY_EBONY_CASKET; break;
    case MISC_CRYSTAL_BALL_OF_FIXATION: ch = TILE_MISC_CRYSTAL_BALL_OF_FIXATION; break;
    case MISC_DISC_OF_STORMS: ch = TILE_MISC_DISC_OF_STORMS; break;

    case MISC_DECK_OF_ESCAPE: ch = TILE_MISC_DECK_OF_WONDERS; break;
    case MISC_DECK_OF_DESTRUCTION: ch = TILE_MISC_DECK_OF_WONDERS; break;
    case MISC_DECK_OF_DUNGEONS: ch = TILE_MISC_DECK_OF_WONDERS; break;
    case MISC_DECK_OF_SUMMONING: ch = TILE_MISC_DECK_OF_SUMMONINGS; break;
    case MISC_DECK_OF_WONDERS: ch = TILE_MISC_DECK_OF_WONDERS; break;
    case MISC_DECK_OF_PUNISHMENT: ch = TILE_MISC_DECK_OF_WONDERS; break;
    case MISC_DECK_OF_WAR: ch = TILE_MISC_DECK_OF_WONDERS; break;
    case MISC_DECK_OF_CHANGES: ch = TILE_MISC_DECK_OF_WONDERS; break;
    case MISC_DECK_OF_DEFENSE: ch = TILE_MISC_DECK_OF_WONDERS; break;

    case MISC_RUNE_OF_ZOT: ch = TILE_MISC_RUNE_OF_ZOT; break;
  }
  return ch;
}

/*****************************************************/
int tileidx_item(const item_def &item)
{
    int clas=item.base_type;
    int type=item.sub_type;
    int special=item.special;
    int color=item.colour;

    id_arr& id = get_typeid_array();

    switch (clas)
    {
	case OBJ_WEAPONS:
        if(is_fixed_artefact(item))
            return tileidx_fixed_artifact(special); 
        else
        if (is_unrandom_artefact( item ))
            return tileidx_unrand_artifact(find_unrandart_index(item));
        else
            return tileidx_weapon(item);
	break;

	case OBJ_MISSILES:
	return tileidx_missile(item);
	break;

        case OBJ_ARMOUR:
        if (is_unrandom_artefact( item ))
            return tileidx_unrand_artifact(find_unrandart_index(item));
        else
            return tileidx_armour(item);
	break;

        case OBJ_WANDS:
#if 1 //ID Item
        if (id[ IDTYPE_WANDS ][type] == ID_KNOWN_TYPE
            ||  (item.flags &ISFLAG_KNOW_TYPE ))
             return TILE_WAND_FLAME + type;
        else
#endif
             return TILE_WAND_OFFSET + special % 12;
	break;

	case OBJ_FOOD:
	return tileidx_food(item);
	break;

        case OBJ_SCROLLS:
#if 1 //ID Item
        if (id[ IDTYPE_SCROLLS ][type] == ID_KNOWN_TYPE
            ||  (item.flags &ISFLAG_KNOW_TYPE ))
             return TILE_SCR_IDENTIFY + type;
#endif
        return TILE_SCROLL;
	break;

        case OBJ_GOLD:
	return TILE_GOLD;
	break;

        case OBJ_JEWELLERY:

        if (type < AMU_RAGE)
        { // rings
           if(is_random_artefact( item ))
	       return TILE_RING_RANDOM_OFFSET + color - 1;
           else 
	       return TILE_RING_NORMAL_OFFSET + special % 13;
        } else { // amu
        if (is_unrandom_artefact( item ))
            return tileidx_unrand_artifact(find_unrandart_index(item));
        else
           if(is_random_artefact( item ))
               return TILE_AMU_RANDOM_OFFSET + color - 1;
           else
               return TILE_AMU_NORMAL_OFFSET + special % 13;
        } 
	break;

        case OBJ_POTIONS:
        if (id[ IDTYPE_POTIONS ][type] == ID_KNOWN_TYPE
            ||  (item.flags &ISFLAG_KNOW_TYPE ))
         return TILE_POT_HEALING + type;
         else
         return TILE_POTION_OFFSET + special % 14;
	 break;

        case OBJ_BOOKS:
        type= special % 10;
        if(type<2) return TILE_BOOK_PAPER_OFFSET + color;
        if(type==2) return TILE_BOOK_LEATHER_OFFSET + special/10;
        if(type==3) return TILE_BOOK_METAL_OFFSET + special/10;
        if(type==4) return TILE_BOOK_PAPYRUS;
	break;

        case OBJ_STAVES:
	return TILE_STAFF_OFFSET + special % 10;
	break;

        case OBJ_CORPSES:
        if (item.sub_type == CORPSE_SKELETON)
            return TILE_FOOD_BONE;
        else
            return tileidx_corpse(item.plus);
	break;

        case OBJ_ORBS:
	return TILE_ORB;
	break;

        case OBJ_MISCELLANY:
	return tileidx_misc(type);
	break;

        default:
	break;
    }
    return TILE_ERROR;
}

/*
  Determine Octant of missile direction
  ---> X+
 |
 |  701
 Y  6O2
 +  543

  the octant boundary slope tan(pi/8)=sqrt(2)-1 = 0.414 is approximated by 2/5

*/
static int tile_bolt_dir(int dx, int dy)
{
    int ax = abs(dx);
    int ay = abs(dy);

    if ( 5*ay < 2*ax )
    {
	return (dx > 0)? 2:6;
    }
    else if ( 5*ax < 2*ay)
    {
        return ( dy > 0 )? 4:0;
    }
    else 
    {
        if (dx>0)
	    return (dy>0)? 3:1;
	else
	    return (dy>0)? 5:7;
    }
}

int tileidx_item_throw(const item_def &item, int dx, int dy)
{
    if (item.base_type == OBJ_MISSILES)
    {
        int ch = -1;
        int dir = tile_bolt_dir(dx, dy);

        // Thrown items with multiple directions
        switch(item.sub_type)
        {
            case MI_ARROW:
                ch = TILE_MI_ARROW0;
                break;
            case MI_BOLT:
                ch = TILE_MI_BOLT0;
                break;
            case MI_DART:
                ch = TILE_MI_DART0;
                break;
            case MI_NEEDLE:
                ch = TILE_MI_NEEDLE0;
                break;
            case MI_JAVELIN:
                ch = TILE_MI_JAVELIN0;
                break;
            case MI_THROWING_NET:
                ch = TILE_MI_THROWING_NET0;
            default:
                break;
        }
        if (ch != -1)
            return ch + dir;

        // Thrown items with a single direction
        switch(item.sub_type)
        {
            case MI_STONE:
                ch = TILE_MI_STONE0;
                break;
            case MI_SLING_BULLET:
                ch = TILE_MI_SLING_BULLET0;
                break;
            case MI_LARGE_ROCK:
                ch = TILE_MI_LARGE_ROCK0;
                break;
            case MI_THROWING_NET:
                ch = TILE_MI_THROWING_NET0;
                break;
            default:
                break;
        }
        if (ch != -1)
            return ch;
    }

    // If not a special case, just return the default tile.
    return tileidx_item(item);
}

int tileidx_feature(int object){
    int ch = TILE_ERROR;
    switch (object)
    {
    case DNGN_UNSEEN: ch=TILE_DNGN_UNSEEN; break;
    case DNGN_ROCK_WALL:
    case DNGN_PERMAROCK_WALL:
    case DNGN_SECRET_DOOR:
        ch=TILE_DNGN_ROCK_WALL_OFS; break;

    case DNGN_STONE_WALL: ch=TILE_DNGN_STONE_WALL; break;
    case DNGN_CLOSED_DOOR: ch=TILE_DNGN_CLOSED_DOOR; break;
    case DNGN_METAL_WALL: ch=TILE_DNGN_METAL_WALL; break;
    case DNGN_GREEN_CRYSTAL_WALL: ch=TILE_DNGN_GREEN_CRYSTAL_WALL; break;
    case DNGN_ORCISH_IDOL: ch=TILE_DNGN_ORCISH_IDOL; break;
    case DNGN_WAX_WALL: ch=TILE_DNGN_WAX_WALL; break;
    //case DNGN_LAST_SOLID_CH: ch=TILE_DNGN_LAST_SOLID_CH; break;
    case DNGN_GRANITE_STATUE: ch=TILE_DNGN_GRANITE_STATUE; break;
    //  case DNGN_STATUE_39: ch=TILE_DNGN_STATUE_39; break;
    case DNGN_LAVA: ch=TILE_DNGN_LAVA; break;
    case DNGN_DEEP_WATER: ch=TILE_DNGN_DEEP_WATER; break;
    case DNGN_SHALLOW_WATER: ch=TILE_DNGN_SHALLOW_WATER; break;
    //  case DNGN_WATER_STUCK: ch=TILE_DNGN_WATER_STUCK; break;

    case DNGN_FLOOR: 
    case DNGN_UNDISCOVERED_TRAP:
        ch=TILE_DNGN_FLOOR; break;
    case DNGN_FLOOR_SPECIAL:
        ch=TILE_DNGN_FLOOR_SPECIAL; break;

    case DNGN_ENTER_HELL: ch=TILE_DNGN_ENTER_HELL; break;
    case DNGN_OPEN_DOOR: ch=TILE_DNGN_OPEN_DOOR; break;
    //  case DNGN_BRANCH_STAIRS: ch=TILE_DNGN_BRANCH_STAIRS; break;
    case DNGN_TRAP_MECHANICAL: ch=TILE_DNGN_TRAP_MECHANICAL; break;
    case DNGN_TRAP_MAGICAL: ch=TILE_DNGN_TRAP_MAGICAL; break;
//    case DNGN_TRAP_III: ch=TILE_DNGN_TRAP_III; break;
    case DNGN_ENTER_SHOP: ch=TILE_DNGN_ENTER_SHOP; break;
    case DNGN_ENTER_LABYRINTH: ch=TILE_DNGN_ENTER_LABYRINTH; break;
    case DNGN_STONE_STAIRS_DOWN_I: 
    case DNGN_STONE_STAIRS_DOWN_II: 
    case DNGN_STONE_STAIRS_DOWN_III: ch=TILE_DNGN_STONE_STAIRS_DOWN; break;
    case DNGN_ROCK_STAIRS_DOWN: ch=TILE_DNGN_ROCK_STAIRS_DOWN; break;
    case DNGN_STONE_STAIRS_UP_I:
    case DNGN_STONE_STAIRS_UP_II:
    case DNGN_STONE_STAIRS_UP_III: ch=TILE_DNGN_STONE_STAIRS_UP; break;
    case DNGN_ROCK_STAIRS_UP: ch=TILE_DNGN_ROCK_STAIRS_UP; break;
    case DNGN_ENTER_DIS: ch=TILE_DNGN_ENTER_DIS; break;
    case DNGN_ENTER_GEHENNA: ch=TILE_DNGN_ENTER_GEHENNA; break;
    case DNGN_ENTER_COCYTUS: ch=TILE_DNGN_ENTER_COCYTUS; break;
    case DNGN_ENTER_TARTARUS: ch=TILE_DNGN_ENTER_TARTARUS; break;
    case DNGN_ENTER_ABYSS: ch=TILE_DNGN_ENTER_ABYSS; break;
    case DNGN_EXIT_ABYSS:
    case DNGN_EXIT_HELL:
        ch=TILE_DNGN_EXIT_ABYSS; break;
    case DNGN_STONE_ARCH: ch=TILE_DNGN_STONE_ARCH; break;
    case DNGN_ENTER_PANDEMONIUM: ch=TILE_DNGN_ENTER_PANDEMONIUM; break;
    case DNGN_EXIT_PANDEMONIUM: ch=TILE_DNGN_EXIT_PANDEMONIUM; break;
    case DNGN_TRANSIT_PANDEMONIUM: ch=TILE_DNGN_TRANSIT_PANDEMONIUM; break;
    // case DNGN_BUILDER_SPECIAL_WALL: ch=TILE_DNGN_BUILDER_SPECIAL_WALL; break;
    // case DNGN_BUILDER_SPECIAL_FLOOR: ch=TILE_DNGN_BUILDER_SPECIAL_FLOOR; break;

    case DNGN_ENTER_ORCISH_MINES:
    case DNGN_ENTER_HIVE:
    case DNGN_ENTER_LAIR:
    case DNGN_ENTER_SLIME_PITS:
    case DNGN_ENTER_VAULTS:
    case DNGN_ENTER_CRYPT:
    case DNGN_ENTER_HALL_OF_BLADES:
    case DNGN_ENTER_ZOT:
    case DNGN_ENTER_TEMPLE:
    case DNGN_ENTER_SNAKE_PIT:
    case DNGN_ENTER_ELVEN_HALLS:
    case DNGN_ENTER_TOMB:
    case DNGN_ENTER_SWAMP:
    case DNGN_ENTER_SHOALS:
    case DNGN_ENTER_RESERVED_2:
    case DNGN_ENTER_RESERVED_3:
    case DNGN_ENTER_RESERVED_4:
        ch=TILE_DNGN_ENTER; break;

    case DNGN_RETURN_FROM_ORCISH_MINES:
    case DNGN_RETURN_FROM_HIVE:
    case DNGN_RETURN_FROM_LAIR:
    case DNGN_RETURN_FROM_SLIME_PITS:
    case DNGN_RETURN_FROM_VAULTS:
    case DNGN_RETURN_FROM_CRYPT:
    case DNGN_RETURN_FROM_HALL_OF_BLADES:
    case DNGN_RETURN_FROM_ZOT:
    case DNGN_RETURN_FROM_TEMPLE:
    case DNGN_RETURN_FROM_SNAKE_PIT:
    case DNGN_RETURN_FROM_ELVEN_HALLS:
    case DNGN_RETURN_FROM_TOMB:
    case DNGN_RETURN_FROM_SWAMP:
    case DNGN_RETURN_FROM_SHOALS:
    case DNGN_RETURN_RESERVED_2:
    case DNGN_RETURN_RESERVED_3:
    case DNGN_RETURN_RESERVED_4:
        ch=TILE_DNGN_RETURN; break;

    case DNGN_ENTER_PORTAL_VAULT:
    case DNGN_EXIT_PORTAL_VAULT:
        ch=TILE_DNGN_TRANSIT_PANDEMONIUM; break;

    case DNGN_ALTAR_ZIN: ch=TILE_DNGN_ALTAR_ZIN; break;
    case DNGN_ALTAR_SHINING_ONE: ch=TILE_DNGN_ALTAR_SHINING_ONE; break;
    case DNGN_ALTAR_KIKUBAAQUDGHA: ch=TILE_DNGN_ALTAR_KIKUBAAQUDGHA; break;
    case DNGN_ALTAR_YREDELEMNUL: ch=TILE_DNGN_ALTAR_YREDELEMNUL; break;
    case DNGN_ALTAR_XOM: ch=TILE_DNGN_ALTAR_XOM; break;
    case DNGN_ALTAR_VEHUMET: ch=TILE_DNGN_ALTAR_VEHUMET; break;
    case DNGN_ALTAR_OKAWARU: ch=TILE_DNGN_ALTAR_OKAWARU; break;
    case DNGN_ALTAR_MAKHLEB: ch=TILE_DNGN_ALTAR_MAKHLEB; break;
    case DNGN_ALTAR_SIF_MUNA: ch=TILE_DNGN_ALTAR_SIF_MUNA; break;
    case DNGN_ALTAR_TROG: ch=TILE_DNGN_ALTAR_TROG; break;
    case DNGN_ALTAR_NEMELEX_XOBEH: ch=TILE_DNGN_ALTAR_NEMELEX_XOBEH; break;
    case DNGN_ALTAR_ELYVILON: ch=TILE_DNGN_ALTAR_ELYVILON; break;
    case DNGN_ALTAR_LUGONU: ch=TILE_DNGN_ALTAR_LUGONU; break;
    case DNGN_ALTAR_BEOGH: ch=TILE_DNGN_ALTAR_BEOGH; break;

    case DNGN_BLUE_FOUNTAIN: ch=TILE_DNGN_BLUE_FOUNTAIN; break;
    case DNGN_DRY_FOUNTAIN_I: 
    case DNGN_DRY_FOUNTAIN_II:
    case DNGN_DRY_FOUNTAIN_III:
    case DNGN_DRY_FOUNTAIN_IV:
    case DNGN_DRY_FOUNTAIN_V:
    case DNGN_DRY_FOUNTAIN_VI:
    case DNGN_DRY_FOUNTAIN_VII:
    case DNGN_DRY_FOUNTAIN_VIII:
    case DNGN_PERMADRY_FOUNTAIN :
        ch=TILE_DNGN_DRY_FOUNTAIN; break;
    case DNGN_SPARKLING_FOUNTAIN: ch=TILE_DNGN_SPARKLING_FOUNTAIN; break;
    }
    
    return ch;
}

int tileidx_cloud(int type, int decay){
    int ch = TILE_ERROR;
    int dur = decay/20;
    if (dur>2) dur = 2;

    switch (type)
    {
        case CLOUD_FIRE:
            ch = TILE_CLOUD_FIRE_0 + dur;
            break;

        case CLOUD_COLD:
            ch = TILE_CLOUD_COLD_0 + dur;
            break;

        case CLOUD_STINK:
        case CLOUD_POISON:
            ch = TILE_CLOUD_POISON_0 + dur;
            break;

        case CLOUD_BLUE_SMOKE:
            ch = TILE_CLOUD_BLUE_SMOKE;
            break;

        case CLOUD_PURP_SMOKE:
            ch = TILE_CLOUD_PURP_SMOKE;
            break;

        case CLOUD_MIASMA:
            ch = TILE_CLOUD_MIASMA;
            break;

        case CLOUD_BLACK_SMOKE:
            ch = TILE_CLOUD_BLACK_SMOKE;
            break;

        default:
            ch = TILE_CLOUD_GREY_SMOKE;
            break;
    }
    return (ch | TILE_FLAG_FLYING);
}

/**********************************************************/

int tileidx_player(int job){

    int ch = TILE_PLAYER;

    // Handle shapechange first
    switch (you.symbol)
    {
        case 's': ch = TILE_MONS_WOLF_SPIDER;break;
        case 'I': ch = TILE_MONS_ICE_BEAST;break;
        case '8': ch = TILE_MONS_STONE_GOLEM;break;
        case 'D': ch = TILE_MONS_DRAGON;break;
        case 'L': ch = TILE_MONS_LICH;break;
        case '#': ch = TILE_MONS_VAPOUR;break;
        case 'S': ch = TILE_MONS_LAVA_SNAKE;break;
        case 'b': ch = TILE_MONS_GIANT_BAT;break;
    }

    if (player_is_airborne())
	ch |= TILE_FLAG_FLYING;

    if (you.attribute[ATTR_HELD])
        ch |= TILE_FLAG_NET;

    return ch;
}


int tileidx_unseen(int ch, const coord_def& gc)
{
    int res = TILE_ERROR;
    ch &= 0xff;
    if (ch<32) ch=32;

    if ( (ch>='@' && ch<='Z') || (ch>='a' && ch<='z') || ch=='&'
        || (ch>='1' && ch<='5') || ch == ';')
        return TILE_UNSEEN_MONSTER | tile_unseen_flag(gc);

    switch (ch)
    {
        //blank, walls, and floors first, since they are frequent
        case ' ': res = TILE_DNGN_UNSEEN; break;
        case 127: //old
        case 176:
        case 177: res = TILE_DNGN_ROCK_WALL_OFS; break;

	case 130:
        case ',':
        case '.':
        case 249:
        case 250: res = TILE_DNGN_FLOOR; break;

	// old;
        case 137: res = TILE_DNGN_WAX_WALL; break;
        case 138: res = TILE_DNGN_STONE_WALL; break;
        case 139: res = TILE_DNGN_METAL_WALL; break;
        case 140: res = TILE_DNGN_GREEN_CRYSTAL_WALL; break;

        // others
        case '!': res = TILE_POTION_OFFSET + 13; break;
        case '"': res = TILE_AMU_NORMAL_OFFSET + 2; break;
        case '#': res = TILE_CLOUD_GREY_SMOKE; break;
        case '$': res = TILE_GOLD; break;
        case '%': res = TILE_FOOD_MEAT_RATION; break;
        case 142: res = TILE_UNSEEN_CORPSE; break;

        case '\'':
        case 134: res = TILE_DNGN_OPEN_DOOR; break;
        case '(':
        case ')': res = TILE_UNSEEN_WEAPON; break;
        case '*': res = TILE_DNGN_ROCK_WALL_OFS ; break;
        case '+': res =  TILE_BOOK_PAPER_OFFSET + 15; break;

        case '/': res = TILE_WAND_OFFSET; break;
        case '8': res = TILE_DNGN_SILVER_STATUE; break;
        case '<': res = TILE_DNGN_STONE_STAIRS_UP; break;
        case '=': res = TILE_RING_NORMAL_OFFSET + 1; break;
        case '>': res = TILE_DNGN_STONE_STAIRS_DOWN; break;
        case '?': res = TILE_SCROLL; break;
        case '[':
        case ']': res = TILE_UNSEEN_ARMOUR; break;
        case '\\': res = TILE_STAFF_OFFSET; break;
        case '^': res = TILE_DNGN_TRAP_MAGICAL; break;
        case '_':
        case 131: res = TILE_UNSEEN_ALTAR; break;
        case '~': res = TILE_UNSEEN_ITEM; break;
        case '{':
        case 135: res = TILE_DNGN_DEEP_WATER; break;
        case 133: res = TILE_DNGN_BLUE_FOUNTAIN; break;
        case '}': res = TILE_MISC_CRYSTAL_BALL_OF_SEEING; break;
        case 128: //old 
        case 254: res = TILE_DNGN_CLOSED_DOOR; break;
        case 129: res = TILE_DNGN_RETURN; break;
        case 132: res = TILE_UNSEEN_ENTRANCE; break;
        case 136: res = TILE_DNGN_ENTER; break;
        case 141: res = TILE_DNGN_LAVA; break;
    }
    //if(res == TILE_ERROR)printf("undefined mapchar %d [%c]\n",ch,ch);
    return res | tile_unseen_flag(gc);
}

int tileidx_bolt(const bolt &bolt)
{
    int col = bolt.colour;
    return tileidx_zap(col);
}

int tileidx_zap(int color)
{
    int col = color;
    if (col > 8) col -= 8;
    if (col < 1) col = 7;
    return TILE_SYM_BOLT_OFS -1 + col;
}

// Convert normal tile to 3D tile if it exists
// Plus modify wall tile index depending on
//  1: floor/wall flavor in 2D mode
//  2: connectivity in 3D mode
void finalize_tile(short unsigned int *tile, unsigned char wflag,
    char wall_flv, char floor_flv, char special_flv)
{
    int orig = (*tile) & TILE_FLAG_MASK;
    int flag = (*tile) & (~TILE_FLAG_MASK);

    // Hack: Swap rock/stone in crypt and tomb, because there are
    //       only stone walls.
    if ((you.where_are_you == BRANCH_CRYPT || you.where_are_you == BRANCH_TOMB)
            && orig == TILE_DNGN_STONE_WALL)
            orig = TILE_DNGN_ROCK_WALL_OFS;

    // If there are special tiles for this level, then use them.
    // Otherwise, we'll fall through to the next case and replace
    // special tiles with normal floor.
    if (orig == TILE_DNGN_FLOOR_SPECIAL && 
        get_num_floor_special_flavors() > 0)
    {
        (*tile) = get_floor_special_tile_idx() + special_flv;
        ASSERT(special_flv >= 0 && 
            special_flv < get_num_floor_special_flavors());
    }
    else if (orig == TILE_DNGN_FLOOR || orig == TILE_DNGN_FLOOR_SPECIAL)
    {
        (*tile) = get_floor_tile_idx() + floor_flv;
    }
    else if (orig == TILE_DNGN_ROCK_WALL_OFS)
    {
        (*tile) = get_wall_tile_idx() + wall_flv;
    }
    else if (orig == TILE_DNGN_SHALLOW_WATER ||
        orig == TILE_DNGN_DEEP_WATER ||
        orig == TILE_DNGN_LAVA)
    {
        // These types always have four flavors...
        (*tile) = orig + (floor_flv % 4);
    }

    (*tile) |= flag;
}

void tilep_calc_flags(int parts[], int flag[])
{
    int i;

    for(i=0;i<TILEP_PARTS_TOTAL;i++) flag[i]=TILEP_FLAG_NORMAL;

    if (parts[TILEP_PART_HELM]-1 >= TILEP_HELM_HELM_OFS)
        flag[TILEP_PART_HAIR]=TILEP_FLAG_HIDE;

    if (parts[TILEP_PART_HELM]-1 >= TILEP_HELM_FHELM_OFS)
        flag[TILEP_PART_BEARD]=TILEP_FLAG_HIDE;

    if(parts[TILEP_PART_BASE]== TILEP_BASE_NAGA ||
       parts[TILEP_PART_BASE]== TILEP_BASE_NAGA+1)
    {
        flag[TILEP_PART_BOOTS]=flag[TILEP_PART_LEG]=TILEP_FLAG_HIDE;
        flag[TILEP_PART_BODY] = TILEP_FLAG_CUT_NAGA;
    }

    if(parts[TILEP_PART_BASE]== TILEP_BASE_CENTAUR ||
       parts[TILEP_PART_BASE]== TILEP_BASE_CENTAUR+1)
    {
        flag[TILEP_PART_BOOTS]=flag[TILEP_PART_LEG]=TILEP_FLAG_HIDE;
        flag[TILEP_PART_BODY] = TILEP_FLAG_CUT_CENTAUR;
    }

    if (parts[TILEP_PART_BASE] == TILEP_BASE_MERFOLK_WATER ||
        parts[TILEP_PART_BASE] == TILEP_BASE_MERFOLK_WATER +1)
    {
        flag[TILEP_PART_BOOTS] = TILEP_FLAG_HIDE;
        flag[TILEP_PART_LEG] = TILEP_FLAG_HIDE;
        flag[TILEP_PART_SHADOW] = TILEP_FLAG_HIDE;
    }
}

/*
 * Set default parts of each race 
 * body + optional beard, hair, etc 
 */

int draconian_color(int race, int level)
{
    if (level < 0) // hack:monster
    {
        switch(race)
        {
        case MONS_DRACONIAN:        return 0;
        case MONS_BLACK_DRACONIAN:  return 1;
        case MONS_YELLOW_DRACONIAN: return 2;
        case MONS_GREEN_DRACONIAN:  return 4;
        case MONS_MOTTLED_DRACONIAN:return 5;
        case MONS_PALE_DRACONIAN:   return 6;
        case MONS_PURPLE_DRACONIAN: return 7;
        case MONS_RED_DRACONIAN:    return 8;
        case MONS_WHITE_DRACONIAN:  return 9;
        }
    }
    if (level < 7)
        return 0;
    switch(race)
    {
    case SP_BLACK_DRACONIAN:   return 1;
    case SP_GOLDEN_DRACONIAN:  return 2;
    case SP_GREY_DRACONIAN:    return 3;
    case SP_GREEN_DRACONIAN:   return 4;
    case SP_MOTTLED_DRACONIAN: return 5;
    case SP_PALE_DRACONIAN:    return 6;
    case SP_PURPLE_DRACONIAN:  return 7;
    case SP_RED_DRACONIAN:     return 8;
    case SP_WHITE_DRACONIAN:   return 9;
    }
    return 0;
}

void tilep_race_default(int race, int gender, int level, int *parts)
{
    int result;
    int hair;
    int beard=0;

    if(gender==TILEP_GENDER_MALE)
        hair = TILEP_HAIR_SHORT_BLACK;
    else
        hair = TILEP_HAIR_LONG_BLACK;

    switch(race)
    {
        case SP_HUMAN:
            result = TILEP_BASE_HUMAN; 
            break;
        case SP_ELF:
        case SP_HIGH_ELF:
        case SP_GREY_ELF:
        case SP_SLUDGE_ELF:
            result = TILEP_BASE_ELF;
            hair = TILEP_HAIR_ELF_YELLOW;
            break;
        case SP_DEEP_ELF:
            result = TILEP_BASE_DEEP_ELF;
            hair = TILEP_HAIR_ELF_WHITE;
            break;
        case SP_HILL_DWARF:
        case SP_MOUNTAIN_DWARF:
            result = TILEP_BASE_DWARF;
            if(gender==TILEP_GENDER_MALE)
            {
                hair = TILEP_HAIR_SHORT_RED;
                beard = TILEP_BEARD_LONG_RED;
            }
            else
            {
                hair = TILEP_HAIR_LONG_RED;
                beard = TILEP_BEARD_SHORT_RED;
            }
            break;
        case SP_HALFLING:
            result = TILEP_BASE_HALFLING;
            break;
        case SP_HILL_ORC:
            result = TILEP_BASE_ORC;
            hair = 0;
            break;
        case SP_KOBOLD:
            result = TILEP_BASE_KOBOLD;
            hair = 0;
            break;
        case SP_MUMMY:
            result = TILEP_BASE_MUMMY;
            hair = 0;
            break;
        case SP_NAGA:
            result = TILEP_BASE_NAGA;
            break;
        case SP_GNOME:
            result = TILEP_BASE_GNOME;
            break;
        case SP_OGRE:
            result = TILEP_BASE_OGRE;
            break;
        case SP_TROLL:
            result = TILEP_BASE_TROLL;
            hair = 0;
            break;
        case SP_OGRE_MAGE:
            result = TILEP_BASE_OGRE_MAGE;
            break;

        case SP_BASE_DRACONIAN:
        case SP_RED_DRACONIAN:
        case SP_WHITE_DRACONIAN:
        case SP_GREEN_DRACONIAN:
        case SP_GOLDEN_DRACONIAN:
        case SP_GREY_DRACONIAN:
        case SP_BLACK_DRACONIAN:
        case SP_PURPLE_DRACONIAN:
        case SP_MOTTLED_DRACONIAN:
        case SP_PALE_DRACONIAN:
            hair = 0;
            if (you.mutation[MUT_BIG_WINGS])
            {
                parts[TILEP_PART_DRCWING] = 1 + draconian_color(race, level);
            }
            result = TILEP_BASE_DRACONIAN + draconian_color(race, level)*2;
            break;

        case SP_CENTAUR:
            result = TILEP_BASE_CENTAUR;
            break;
        case SP_DEMIGOD:
            result = TILEP_BASE_DEMIGOD;
            break;
        case SP_SPRIGGAN:
            result = TILEP_BASE_SPRIGGAN;
            break;
        case SP_MINOTAUR:
            result = TILEP_BASE_MINOTAUR;
            hair = 0;
            break;
        case SP_DEMONSPAWN:
            result = TILEP_BASE_DEMONSPAWN;
            hair = 0;
            break;
        case SP_GHOUL:
            result = TILEP_BASE_GHOUL;
            hair = 0;
            break;
        case SP_KENKU:
            result = TILEP_BASE_KENKU;
            break;
        case SP_MERFOLK:
            result = player_in_water() ? 
                TILEP_BASE_MERFOLK_WATER :
                TILEP_BASE_MERFOLK;
            break;
        case SP_VAMPIRE:
            result = TILEP_BASE_VAMPIRE;
            break;
        default:
            result = TILEP_BASE_HUMAN;
            break;
    }

    if(gender==TILEP_GENDER_MALE) result++;
    parts[TILEP_PART_BASE]=result;
    parts[TILEP_PART_HAIR]=hair;
    parts[TILEP_PART_BEARD]=beard;
    parts[TILEP_PART_SHADOW]= 1;
}

void tilep_job_default(int job, int gender, int *parts)
{
    parts[TILEP_PART_CLOAK]= 0;
    parts[TILEP_PART_BOOTS]= 0;
    parts[TILEP_PART_LEG]= 0;
    parts[TILEP_PART_BODY]= 0;
    parts[TILEP_PART_ARM]= 0;
    parts[TILEP_PART_HAND1]=0;
    parts[TILEP_PART_HAND2]= 0;
    parts[TILEP_PART_HELM]= 0;

    
    switch(job)
    {
        case JOB_FIGHTER:
            parts[TILEP_PART_BODY]=TILEP_SHOW_EQUIP;
            parts[TILEP_PART_LEG]=TILEP_LEG_METAL_SILVER;
            parts[TILEP_PART_HAND2]=TILEP_SHOW_EQUIP;
            parts[TILEP_PART_HAND1]=TILEP_SHOW_EQUIP;
            break;

        case JOB_CRUSADER:
            parts[TILEP_PART_BODY]=TILEP_BODY_SHIRT_WHITE3;
            parts[TILEP_PART_LEG]=TILEP_LEG_SKIRT_OFS;
            parts[TILEP_PART_HELM]=TILEP_HELM_HELM_OFS;
            parts[TILEP_PART_ARM]=TILEP_ARM_GLOVE_GRAY;
            parts[TILEP_PART_BOOTS]=TILEP_BOOTS_MIDDLE_GRAY;
            parts[TILEP_PART_CLOAK]=TILEP_CLOAK_BLUE;
            parts[TILEP_PART_HAND1]=TILEP_SHOW_EQUIP;
            parts[TILEP_PART_HAND2]=TILEP_SHOW_EQUIP;
            break;

        case JOB_PALADIN:
            parts[TILEP_PART_BODY]=TILEP_BODY_ROBE_WHITE;
            parts[TILEP_PART_LEG]=TILEP_LEG_PANTS_BROWN;
            parts[TILEP_PART_HELM]=TILEP_HELM_HELM_OFS;
            parts[TILEP_PART_ARM]=TILEP_ARM_GLOVE_GRAY;
            parts[TILEP_PART_BOOTS]=TILEP_BOOTS_MIDDLE_GRAY;
            parts[TILEP_PART_CLOAK]=TILEP_CLOAK_BLUE;
            parts[TILEP_PART_HAND2]=TILEP_SHOW_EQUIP;
            parts[TILEP_PART_HAND1]=TILEP_SHOW_EQUIP;
            break;

        case JOB_DEATH_KNIGHT:
            parts[TILEP_PART_BODY]=TILEP_BODY_SHIRT_BLACK3;
            parts[TILEP_PART_LEG]=TILEP_LEG_METAL_GRAY;
            parts[TILEP_PART_HELM]=TILEP_HELM_FHELM_OFS;
            parts[TILEP_PART_ARM]=TILEP_ARM_GLOVE_BLACK;
            parts[TILEP_PART_CLOAK]=TILEP_CLOAK_YELLOW;
            parts[TILEP_PART_HAND1]=TILEP_SHOW_EQUIP;
            parts[TILEP_PART_HAND2]=TILEP_HAND2_BOOK_BLACK;
            break;

        case JOB_CHAOS_KNIGHT:
            parts[TILEP_PART_BODY]=TILEP_BODY_BELT1;
            parts[TILEP_PART_LEG]=TILEP_LEG_METAL_GRAY;
            parts[TILEP_PART_HELM]=TILEP_HELM_FHELM_PLUME;
            parts[TILEP_PART_BOOTS]=TILEP_BOOTS_SHORT_BROWN;
            parts[TILEP_PART_HAND1]=TILEP_SHOW_EQUIP;
            parts[TILEP_PART_HAND2]=TILEP_SHOW_EQUIP;
            break;

        case JOB_BERSERKER:
            parts[TILEP_PART_BODY]=TILEP_BODY_ANIMAL_SKIN;
            parts[TILEP_PART_LEG]=TILEP_LEG_BELT_REDBROWN;
            parts[TILEP_PART_HAND1]=TILEP_SHOW_EQUIP;
            parts[TILEP_PART_HAND2]=TILEP_SHOW_EQUIP;
            break;

        case JOB_REAVER:
            parts[TILEP_PART_BODY]=TILEP_BODY_ROBE_BLACK_GOLD;
            parts[TILEP_PART_LEG]=TILEP_LEG_PANTS_BROWN;
            parts[TILEP_PART_HAND2]=TILEP_HAND2_BOOK_RED_DIM;
            parts[TILEP_PART_HAND1]=TILEP_SHOW_EQUIP;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_SHORT_BROWN;
            break;

        case JOB_THIEF:
            parts[TILEP_PART_HELM] = TILEP_HELM_HOOD_YBROWN;
            parts[TILEP_PART_BODY]=TILEP_BODY_LEATHER_JACKET;
            parts[TILEP_PART_LEG]=TILEP_LEG_PANTS_SHORT_GRAY;
            parts[TILEP_PART_HAND1]= TILEP_HAND1_SWORD_THIEF;
            parts[TILEP_PART_ARM]=TILEP_ARM_GLOVE_WRIST_PURPLE;
            parts[TILEP_PART_CLOAK]=TILEP_CLOAK_LBROWN;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_MIDDLE_BROWN2;
            break;

        case JOB_STALKER:
            parts[TILEP_PART_HELM] = TILEP_HELM_HOOD_GREEN;
            parts[TILEP_PART_BODY]=TILEP_BODY_LEATHER_JACKET;
            parts[TILEP_PART_LEG]=TILEP_LEG_PANTS_SHORT_GRAY;
            parts[TILEP_PART_HAND1]= TILEP_HAND1_SWORD_THIEF;
            parts[TILEP_PART_HAND2]= TILEP_HAND2_BOOK_GREEN_DIM;
            parts[TILEP_PART_ARM]=TILEP_ARM_GLOVE_WRIST_PURPLE;
            parts[TILEP_PART_CLOAK]=TILEP_CLOAK_GREEN;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_MIDDLE_BROWN2;
            break;

        case JOB_ASSASSIN:
            parts[TILEP_PART_HELM] = TILEP_HELM_MASK_NINJA_BLACK;
            parts[TILEP_PART_BODY]=TILEP_BODY_SHIRT_BLACK3;
            parts[TILEP_PART_LEG]=TILEP_LEG_PANTS_BLACK;
            parts[TILEP_PART_HAND1]= TILEP_HAND1_SWORD_THIEF;
            parts[TILEP_PART_ARM]=TILEP_ARM_GLOVE_BLACK;
            parts[TILEP_PART_CLOAK]=TILEP_CLOAK_BLACK;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_SHORT_BROWN2;
            break;


        case JOB_WIZARD:
            parts[TILEP_PART_BODY]= TILEP_BODY_GANDALF_G;
            parts[TILEP_PART_HAND1]= TILEP_HAND1_GANDALF;
            parts[TILEP_PART_HAND2]= TILEP_HAND2_BOOK_CYAN_DIM;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_SHORT_BROWN;
            parts[TILEP_PART_HELM] = TILEP_HELM_GANDALF;
            break;

        case JOB_PRIEST:
            parts[TILEP_PART_BODY]= TILEP_BODY_ROBE_WHITE;
            parts[TILEP_PART_ARM] = TILEP_ARM_GLOVE_WHITE;
            parts[TILEP_PART_HAND1]=TILEP_SHOW_EQUIP;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_SHORT_BROWN;
            break;

        case JOB_HEALER:
            parts[TILEP_PART_BODY]= TILEP_BODY_ROBE_WHITE;
            parts[TILEP_PART_ARM] = TILEP_ARM_GLOVE_WHITE;
            parts[TILEP_PART_HAND1]= 38;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_SHORT_BROWN;
            parts[TILEP_PART_HELM] = TILEP_HELM_FHELM_HEALER;
            break;

        case JOB_NECROMANCER:
            parts[TILEP_PART_BODY]= TILEP_BODY_ROBE_BLACK;
            parts[TILEP_PART_HAND1]= TILEP_HAND1_STAFF_SKULL;
            parts[TILEP_PART_HAND2]= TILEP_HAND2_BOOK_BLACK;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_SHORT_BROWN;
            break;

        case JOB_FIRE_ELEMENTALIST:
            parts[TILEP_PART_BODY]= TILEP_BODY_ROBE_RED;
            parts[TILEP_PART_HAND1]= TILEP_HAND1_GANDALF;
            parts[TILEP_PART_HAND2]= TILEP_HAND2_BOOK_RED_DIM;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_SHORT_BROWN;
            break;

        case JOB_ICE_ELEMENTALIST:
            parts[TILEP_PART_BODY]= TILEP_BODY_ROBE_BLUE;
            parts[TILEP_PART_HAND1]= TILEP_HAND1_GANDALF;
            parts[TILEP_PART_HAND2]= TILEP_HAND2_BOOK_BLUE_DIM;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_SHORT_BROWN;
            break;

        case JOB_AIR_ELEMENTALIST:
            parts[TILEP_PART_BODY]= TILEP_BODY_ROBE_CYAN;
            parts[TILEP_PART_HAND1]= TILEP_HAND1_GANDALF;
            parts[TILEP_PART_HAND2]= TILEP_HAND2_BOOK_CYAN_DIM;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_SHORT_BROWN;
            break;

        case JOB_EARTH_ELEMENTALIST:
            parts[TILEP_PART_BODY]= TILEP_BODY_ROBE_YELLOW;
            parts[TILEP_PART_HAND1]= TILEP_HAND1_GANDALF;
            parts[TILEP_PART_HAND2]= TILEP_HAND2_BOOK_YELLOW_DIM;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_SHORT_BROWN;
            break;

        case JOB_VENOM_MAGE:
            parts[TILEP_PART_BODY]= TILEP_BODY_ROBE_GREEN;
            parts[TILEP_PART_HAND1]= TILEP_HAND1_GANDALF;
            parts[TILEP_PART_HAND2]= TILEP_HAND2_BOOK_GREEN_DIM;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_SHORT_BROWN;
            break;

        case JOB_TRANSMUTER:
            parts[TILEP_PART_BODY]= TILEP_BODY_ROBE_RAINBOW;
            parts[TILEP_PART_HAND1]= TILEP_HAND1_STAFF_MAGE;
            parts[TILEP_PART_HAND2]= TILEP_HAND2_BOOK_MAGENTA_DIM;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_SHORT_BROWN;
            break;

        case JOB_CONJURER:
            parts[TILEP_PART_BODY]= TILEP_BODY_ROBE_MAGENTA;
            parts[TILEP_PART_HELM] = TILEP_HELM_GANDALF;
            parts[TILEP_PART_HAND1]= TILEP_HAND1_STAFF_MAGE;
            parts[TILEP_PART_HAND2]= TILEP_HAND2_BOOK_RED_DIM;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_SHORT_BROWN;
            break;

        case JOB_ENCHANTER:
            parts[TILEP_PART_BODY]= TILEP_BODY_ROBE_YELLOW;
            parts[TILEP_PART_HELM] = TILEP_HELM_GANDALF;
            parts[TILEP_PART_HAND1]= TILEP_HAND1_STAFF_MAGE;
            parts[TILEP_PART_HAND2]= TILEP_HAND2_BOOK_BLUE_DIM;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_SHORT_BROWN;
            break;

        case JOB_SUMMONER:
            parts[TILEP_PART_BODY]= TILEP_BODY_ROBE_BROWN;
            parts[TILEP_PART_HELM] = TILEP_HELM_GANDALF;
            parts[TILEP_PART_HAND1]= TILEP_HAND1_STAFF_MAGE;
            parts[TILEP_PART_HAND2]= TILEP_HAND2_BOOK_YELLOW_DIM;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_SHORT_BROWN;
            break;

        case JOB_WARPER:
            parts[TILEP_PART_BODY]= TILEP_BODY_ROBE_BROWN;
            parts[TILEP_PART_HELM] = TILEP_HELM_GANDALF;
            parts[TILEP_PART_HAND1]= 42;
            parts[TILEP_PART_HAND2]= TILEP_HAND2_BOOK_WHITE;
            parts[TILEP_PART_BOOTS] = TILEP_BOOTS_SHORT_BROWN;
            parts[TILEP_PART_CLOAK]=TILEP_CLOAK_RED;
            break;

        case JOB_HUNTER:
            parts[TILEP_PART_BODY]=TILEP_BODY_LEATHER_ARMOUR2;
            parts[TILEP_PART_LEG]=TILEP_LEG_PANTS_BROWN;
            parts[TILEP_PART_HAND1]=TILEP_HAND1_BOW;
            parts[TILEP_PART_ARM] = 7;
            parts[TILEP_PART_BOOTS]= TILEP_BOOTS_MIDDLE_BROWN;
            break;

        case JOB_GLADIATOR:
            parts[TILEP_PART_HAND1]=TILEP_SHOW_EQUIP;
            parts[TILEP_PART_HAND2]= 10;

            if(gender==TILEP_GENDER_MALE)
            {
                parts[TILEP_PART_BODY]= TILEP_BODY_BELT1;
                parts[TILEP_PART_LEG]= TILEP_LEG_BELT_GRAY;
                parts[TILEP_PART_BOOTS] = TILEP_BOOTS_MIDDLE_GRAY;
            }
            else
            {
                parts[TILEP_PART_BODY]= TILEP_BODY_BIKINI_RED;
                parts[TILEP_PART_LEG]= TILEP_LEG_BIKINI_RED;
                parts[TILEP_PART_BOOTS] = TILEP_BOOTS_LONG_RED;
            }
            break;

        case JOB_MONK:
            parts[TILEP_PART_BODY]= TILEP_BODY_MONK_BLACK;
            parts[TILEP_PART_HAND1]=TILEP_SHOW_EQUIP;
            parts[TILEP_PART_HAND2]=TILEP_SHOW_EQUIP;
            break;

        case JOB_WANDERER:
            parts[TILEP_PART_BODY]= TILEP_BODY_SHIRT_HAWAII;
            parts[TILEP_PART_LEG]= TILEP_LEG_PANTS_SHORT_BROWN;
            parts[TILEP_PART_HAND1]=TILEP_SHOW_EQUIP;
            parts[TILEP_PART_HAND2]=TILEP_SHOW_EQUIP;
            parts[TILEP_PART_BOOTS] = 11;
            break;



    }
}

/*
 * Patrs index to string
 */
void tilep_part_to_str(int number, char *buf)
{
    //special
    if (number == TILEP_SHOW_EQUIP)
    {
        buf[0] = buf[1] = buf[2] ='*';
    }
    else
    {
        //normal 2 digits
        buf[0] = '0' + (number/100) % 10;
        buf[1] = '0' + (number/ 10) % 10;
        buf[2] = '0' +  number % 10;
    }
    buf[3] ='\0';
}

/*
 * Parts string to index
 */
int tilep_str_to_part(char *str)
{
    //special
    if (str[0]=='*') return TILEP_SHOW_EQUIP;
    //normal 2 digits
    return atoi(str);
}

const int parts_saved[] ={
    TILEP_PART_SHADOW,
    TILEP_PART_BASE,
    TILEP_PART_CLOAK,
    TILEP_PART_BOOTS,
    TILEP_PART_LEG,
    TILEP_PART_BODY,
    TILEP_PART_ARM,
    TILEP_PART_HAND1,
    TILEP_PART_HAND2,
    TILEP_PART_HAIR,
    TILEP_PART_BEARD,
    TILEP_PART_HELM,
    -1
};

/*
 * scan input line from dolls.txt
 */
void tilep_scan_parts(char *fbuf, int *parts)
{
    char  ibuf[8];

    int gcount = 0;
    int ccount = 0;
    for(int i = 0; parts_saved[i] != -1; i++)
    {
        int idx;
        ccount = 0;
	int p = parts_saved[i];

        while ( (fbuf[gcount] != ':') && (fbuf[gcount] != '\n')
		&& (ccount<4) && (gcount<48) )
        {
            ibuf[ccount] = fbuf[gcount];
            ccount ++;
            gcount ++;
        }

        ibuf[ccount] = '\0';
        gcount ++;

        idx = tilep_str_to_part(ibuf);
        if (p == TILEP_PART_BASE)
        {
            int p0 = (parts[p]-1) & (0xfe);
            if (((1-idx) & 1) == 1) p0++;
            parts[p] = p0 + 1;
        }
        else if (idx == TILEP_SHOW_EQUIP)
            parts[p] = TILEP_SHOW_EQUIP;
        else if (idx < 0) // no negative value
            parts[p] = 0;
        else if (idx > tilep_parts_total[p]) // bound it
            parts[p] = tilep_parts_total[p];
        else
            parts[p] = idx;
    }
}

/*
 * format-print parts
 */
void tilep_print_parts(char *fbuf, int *parts)
{
    int i;
    char *ptr = fbuf;
    for(i = 0; parts_saved[i] != -1; i++)
    {
	int p = parts_saved[i];
        if (p == TILEP_PART_BASE) // 0:female  1:male
        {
            sprintf(ptr, "%03d", parts[p]%2);
            ptr += 3;
        }
        else
        {
            tilep_part_to_str(parts[p], ptr);
            ptr += 3;
        }
        *ptr = ':';
        ptr++;
    }
    ptr--; // erase the last ':'
    *ptr = 0;
}

/*
 * return equip-dependent parts index
 */
int tilep_equ_weapon(const item_def &item)
{
    if (item.base_type == OBJ_STAVES) return TILEP_HAND1_STAFF_MAGE;

    if (item.base_type ==OBJ_MISCELLANY)
    {
        switch(item.sub_type)
        {
        case MISC_BOTTLED_EFREET: return TILEP_HAND1_BOTTLE;
        case MISC_AIR_ELEMENTAL_FAN: return TILEP_HAND1_FAN;
        case MISC_STONE_OF_EARTH_ELEMENTALS: return TILEP_HAND1_STONE;
        case MISC_DISC_OF_STORMS: return TILEP_HAND1_DISC;

        case MISC_CRYSTAL_BALL_OF_SEEING: 
        case MISC_CRYSTAL_BALL_OF_ENERGY: 
        case MISC_CRYSTAL_BALL_OF_FIXATION: return TILEP_HAND1_CRYSTAL;

        case MISC_LAMP_OF_FIRE: return TILEP_HAND1_LANTERN;
        case MISC_LANTERN_OF_SHADOWS: return TILEP_HAND1_BONE_LANTERN;
        case MISC_HORN_OF_GERYON: return TILEP_HAND1_HORN;

        case MISC_BOX_OF_BEASTS: 
        case MISC_EMPTY_EBONY_CASKET: return TILEP_HAND1_BOX;

        case MISC_DECK_OF_ESCAPE:
        case MISC_DECK_OF_DESTRUCTION:
        case MISC_DECK_OF_DUNGEONS:
        case MISC_DECK_OF_SUMMONING:
        case MISC_DECK_OF_WONDERS:
        case MISC_DECK_OF_PUNISHMENT:
        case MISC_DECK_OF_WAR:
        case MISC_DECK_OF_CHANGES:
        case MISC_DECK_OF_DEFENSE: return TILEP_HAND1_DECK;
        }
    }

    if (item.base_type != OBJ_WEAPONS) return 0;

    if (is_fixed_artefact( item ))
    {
        switch(item.special)
        {
            case SPWPN_SINGING_SWORD: return TILEP_HAND1_SINGING_SWORD;
            case SPWPN_WRATH_OF_TROG: return TILEP_HAND1_AXE_TROG;
            case SPWPN_SCYTHE_OF_CURSES: return TILEP_HAND1_FINISHER;
            case SPWPN_MACE_OF_VARIABILITY: return TILEP_HAND1_MACE_OF_VARIABILITY;
            case SPWPN_GLAIVE_OF_PRUNE: return TILEP_HAND1_GLAIVE_OF_PRUNE;
            case SPWPN_SCEPTRE_OF_TORMENT: return TILEP_HAND1_MACE_RUBY;
            case SPWPN_SWORD_OF_ZONGULDROK: return TILEP_HAND1_ZONGULDROK;
            case SPWPN_SWORD_OF_CEREBOV: return TILEP_HAND1_SWORD_TWIST;
            case SPWPN_STAFF_OF_DISPATER: return TILEP_HAND1_DISPATER;
            case SPWPN_SCEPTRE_OF_ASMODEUS: return TILEP_HAND1_ASMODEUS;
            case SPWPN_SWORD_OF_POWER: break;
            case SPWPN_KNIFE_OF_ACCURACY: break;
            case SPWPN_STAFF_OF_OLGREB: return TILEP_HAND1_OLGREB;
            case SPWPN_VAMPIRES_TOOTH: break;
            case SPWPN_STAFF_OF_WUCAD_MU: break;
        }
    }

    if (is_unrandom_artefact( item ))
    {
        int x = find_unrandart_index(item);
        switch(x+1)
        {
            // Bloodbane
            case 2: return TILEP_HAND1_BLOODBANE;
            // Flaming Death
            case 4: return TILEP_HAND1_FLAMING_DEATH;
            //mace of Brilliance
            case 8: return TILEP_HAND1_MACE_OF_VARIABILITY;
            //demon blade Leech
            case 12: return TILEP_HAND1_LEECH;
            //dagger of Chilly Death
            case 15: return TILEP_HAND1_CHILLY_DEATH;
            //dagger \"Morg\"
            case 17: return TILEP_HAND1_MORG;
            //scythe \"Finisher
            case 18: return TILEP_HAND1_FINISHER;
            //sling \"Punk
            case 19: return TILEP_HAND1_PUNK;
            //bow of Krishna
            case 20: return TILEP_HAND1_KRISHNA;
            //giant club \"Skullcrusher
            case 22: break;
            //glaive of the Guard
            case 24: break;
            //sword of Jihad
            case 25: return TILEP_HAND1_JIHAD;
            //crossbow \"Fiery Devil
            case 28: return TILEP_HAND1_FIERY_DEVIL;
            //sword of Doom Knight
            case 31: return TILEP_HAND1_DOOM_KNIGHT;
            //Eos
            case 35: break;
            //spear of Voo-Doo
            case 39: return TILEP_HAND1_VOODOO;
            //trident of the Octopus king
            case 40: break;
            //mithril axe \"Arga
            case 42: return TILEP_HAND1_ARGA;
            //Elemental Staff
            case 43: return TILEP_HAND1_ELEMENTAL_STAFF;
            //hand crossbow \"Sniper
            case 44: return TILEP_HAND1_SNIPER;
            //bow \"Erchidel
            case 45: return TILEP_HAND1_GREAT_BOW;
            //plutonium sword
            case 47: return TILEP_HAND1_PLUTONIUM_SWORD;
            //mace \"Undeadhunter
            case 48: return TILEP_HAND1_LARGE_MACE;
        }
    }

    switch (item.sub_type)
    {
  // Blunt
  case WPN_CLUB: return TILEP_HAND1_CLUB_SLANT;
  case WPN_MACE: return TILEP_HAND1_MACE;
  case WPN_GREAT_MACE: return TILEP_HAND1_GREAT_MACE;
  case WPN_FLAIL: return TILEP_HAND1_FRAIL;
  case WPN_SPIKED_FLAIL: return TILEP_HAND1_SPIKED_FRAIL;
  case WPN_DIRE_FLAIL: return TILEP_HAND1_GREAT_FRAIL;
  case WPN_MORNINGSTAR: return TILEP_HAND1_MORNINGSTAR;
  case WPN_EVENINGSTAR: return TILEP_HAND1_EVENINGSTAR;
  case WPN_GIANT_CLUB: return TILEP_HAND1_GIANT_CLUB_SLANT;
  case WPN_GIANT_SPIKED_CLUB: return TILEP_HAND1_GIANT_CLUB_SPIKE_SLANT;
  case WPN_ANCUS: return TILEP_HAND1_MACE;
  case WPN_WHIP: return TILEP_HAND1_WHIP;
  case WPN_DEMON_WHIP: return TILEP_HAND1_BLACK_WHIP;

  // Edge
  case WPN_KNIFE: return TILEP_HAND1_DAGGER_SLANT;
  case WPN_DAGGER: return TILEP_HAND1_DAGGER_SLANT;
  case WPN_SHORT_SWORD: return TILEP_HAND1_SHORT_SWORD_SLANT;
  case WPN_LONG_SWORD: return TILEP_HAND1_LONG_SWORD_SLANT;
  case WPN_GREAT_SWORD: return TILEP_HAND1_GREAT_SWORD_SLANT;
  case WPN_SCIMITAR: return TILEP_HAND1_SCIMITAR;
  case WPN_FALCHION: return TILEP_HAND1_FALCHION;
  case WPN_SABRE: return TILEP_HAND1_SABRE;
  case WPN_DEMON_BLADE: return TILEP_HAND1_SWORD_BLACK;
  case WPN_QUICK_BLADE: return TILEP_HAND1_DAGGER;
  case WPN_KATANA: return TILEP_HAND1_KATANA_SLANT;
  case WPN_DOUBLE_SWORD: return TILEP_HAND1_DOUBLE_SWORD;
  case WPN_TRIPLE_SWORD: return TILEP_HAND1_TRIPLE_SWORD;
  case WPN_BLESSED_BLADE: return TILEP_HAND1_BLESSED_BLADE;

  // Axe
  case WPN_HAND_AXE: return TILEP_HAND1_HAND_AXE;
  case WPN_BATTLEAXE: return TILEP_HAND1_BATTLEAXE;
  case WPN_BROAD_AXE: return TILEP_HAND1_BROAD_AXE;
  case WPN_WAR_AXE: return TILEP_HAND1_WAR_AXE;
  case WPN_EXECUTIONERS_AXE: return TILEP_HAND1_EXECUTIONERS_AXE;
  case WPN_BARDICHE: return TILEP_HAND1_GLAIVE3;

  //Pole
  case WPN_SPEAR: return TILEP_HAND1_SPEAR;
  case WPN_HALBERD: return TILEP_HAND1_HALBERD;
  case WPN_GLAIVE: return TILEP_HAND1_GLAIVE;
  case WPN_QUARTERSTAFF: return TILEP_HAND1_QUARTERSTAFF1;
  case WPN_SCYTHE: return TILEP_HAND1_SCYTHE;
  case WPN_HAMMER: return TILEP_HAND1_HAMMER;
  case WPN_DEMON_TRIDENT: return TILEP_HAND1_DEMON_TRIDENT;
  case WPN_TRIDENT: return TILEP_HAND1_TRIDENT2;
  case WPN_LAJATANG: return TILEP_HAND1_D_GLAIVE;

  //Ranged
  case WPN_SLING: return TILEP_HAND1_SLING;
  case WPN_BOW: return TILEP_HAND1_BOW2;
  case WPN_CROSSBOW: return TILEP_HAND1_CROSSBOW;
  case WPN_HAND_CROSSBOW: return TILEP_HAND1_CROSSBOW;
  case WPN_BLOWGUN: return TILEP_HAND1_BLOWGUN;
  case WPN_LONGBOW: return TILEP_HAND1_BOW3;
        default: return 0;
    }
}

int tilep_equ_armour(const item_def &item)
{
    if (item.base_type !=OBJ_ARMOUR) return 0;

    if (is_unrandom_artefact( item ))
    {
        int x = find_unrandart_index(item);
        switch(x+1)
        {
            // Holy Armour of Zin
            case 6: return TILEP_BODY_ARMOR_MUMMY;
            // robe of Augmentation
            case 7: return TILEP_BODY_ROBE_RED2;
            // robe of Misfortune
            case 14: return TILEP_BODY_ARWEN;
            // Lear's chain mail
            case 26: return TILEP_BODY_LEARS_CHAIN_MAIL;
            // skin of Zhor
            case 27: return TILEP_BODY_ZHOR; break;
            // salamander hide armour
            case 29: return TILEP_BODY_LEATHER_RED;
            // robe of Folly
            case 33: return TILEP_BODY_ROBE_BLACK;
            // Edison's patent armour
            case 38: return TILEP_BODY_EDISON;
            // robe of Night
            case 46: return TILEP_BODY_ROBE_OF_NIGHT;
            // armour of the Dragon King
            case 49: break;
        }
    }

    switch (item.sub_type)
    {

    case ARM_ROBE: 
        switch(item.colour)
        {
            // We've got a zillion robes; let's use 'em!
            case BLACK:       return TILEP_BODY_ROBE_BLACK_RED;
            case BLUE:        return TILEP_BODY_ROBE_BLUE;
            case LIGHTBLUE:   return TILEP_BODY_ROBE_BLUE_WHITE;
            case GREEN:       return TILEP_BODY_ROBE_GREEN;
            case LIGHTGREEN:  return TILEP_BODY_ROBE_BLUE_GREEN;
            case CYAN:        return TILEP_BODY_ROBE_WHITE_GREEN;
            case LIGHTCYAN:   return TILEP_BODY_ROBE_CYAN;
            case RED:         return TILEP_BODY_ROBE_RED;
            case LIGHTRED:    return TILEP_BODY_ROBE_RED_GOLD;
            case MAGENTA:     return TILEP_BODY_ROBE_MAGENTA;
            case LIGHTMAGENTA:return TILEP_BODY_ROBE_RED3;
            case BROWN:       return TILEP_BODY_ROBE_BROWN;
            case YELLOW:      return TILEP_BODY_ROBE_YELLOW;
            case LIGHTGREY:   return TILEP_BODY_ROBE_GRAY2;
            case DARKGREY:    return TILEP_BODY_GANDALF_G;
            case WHITE:       return TILEP_BODY_ROBE_WHITE;
            default:          return 0;
        }
 
    case ARM_LEATHER_ARMOUR: return TILEP_BODY_LEATHER_ARMOUR3;
    case ARM_RING_MAIL: return TILEP_BODY_RINGMAIL;
    case ARM_CHAIN_MAIL: return TILEP_BODY_CHAINMAIL;
    case ARM_SCALE_MAIL: return TILEP_BODY_SCALEMAIL;
    case ARM_SPLINT_MAIL: return TILEP_BODY_BANDED;
    case ARM_BANDED_MAIL: return TILEP_BODY_BANDED;
    case ARM_PLATE_MAIL: return TILEP_BODY_PLATE_BLACK;
    case ARM_CRYSTAL_PLATE_MAIL: return TILEP_BODY_CRYSTAL_PLATE;

    case ARM_DRAGON_HIDE: return TILEP_BODY_DRAGONSC_GREEN;
    case ARM_ICE_DRAGON_HIDE: return TILEP_BODY_DRAGONSC_CYAN;
    case ARM_STEAM_DRAGON_HIDE: return TILEP_BODY_DRAGONSC_WHITE;
    case ARM_MOTTLED_DRAGON_HIDE: return TILEP_BODY_DRAGONSC_MAGENTA;
    case ARM_STORM_DRAGON_HIDE: return TILEP_BODY_DRAGONSC_BLUE;
    case ARM_GOLD_DRAGON_HIDE: return TILEP_BODY_DRAGONSC_GOLD;
    case ARM_SWAMP_DRAGON_HIDE: return TILEP_BODY_DRAGONSC_BROWN;

    case ARM_DRAGON_ARMOUR: return TILEP_BODY_DRAGONARM_GREEN;
    case ARM_ICE_DRAGON_ARMOUR: return TILEP_BODY_DRAGONARM_CYAN;
    case ARM_STEAM_DRAGON_ARMOUR: return TILEP_BODY_DRAGONARM_WHITE;
    case ARM_MOTTLED_DRAGON_ARMOUR: return TILEP_BODY_DRAGONARM_MAGENTA;
    case ARM_STORM_DRAGON_ARMOUR: return TILEP_BODY_DRAGONARM_BLUE;
    case ARM_GOLD_DRAGON_ARMOUR: return TILEP_BODY_DRAGONARM_GOLD;
    case ARM_SWAMP_DRAGON_ARMOUR: return TILEP_BODY_DRAGONARM_BROWN;

    case ARM_ANIMAL_SKIN: return TILEP_BODY_ANIMAL_SKIN;
    case ARM_TROLL_HIDE: 
    case ARM_TROLL_LEATHER_ARMOUR: return TILEP_BODY_TROLL_HIDE;

    default: return 0;
    }
}

int tilep_equ_shield(const item_def &item)
{
    if (you.equip[EQ_SHIELD] == -1) return 0;
    if (item.base_type !=OBJ_ARMOUR) return 0;

    if (is_unrandom_artefact( item ))
    {
        int x = find_unrandart_index(item);
        switch(x+1)
        {
            // shield of Ignorance
            case 5: return TILEP_HAND2_SHIELD_SHAMAN;
            // Bullseye
            case 10: return TILEP_HAND2_BULLSEYE;
            // shield of Resistance
            case 32: return TILEP_HAND2_SHIELD_OF_RESISTANCE;
        }
    }

    switch (item.sub_type)
    {
        case ARM_SHIELD: return TILEP_HAND2_SHIELD_KNIGHT_BLUE;
        case ARM_BUCKLER: return TILEP_HAND2_SHIELD_ROUND_SMALL;
        case ARM_LARGE_SHIELD: return TILEP_HAND2_SHIELD_LONG_RED;
        default: return 0;
    }
}

int tilep_equ_cloak(const item_def &item)
{
    if (you.equip[EQ_CLOAK] == -1) return 0;
    if (item.base_type !=OBJ_ARMOUR) return 0;
    if (item.sub_type != ARM_CLOAK) return 0;
    switch (item.colour)
    {
         case BLACK:
         case BLUE:
         case LIGHTBLUE:   return TILEP_CLOAK_BLUE;
         case GREEN:
         case LIGHTGREEN:  return TILEP_CLOAK_GREEN;
         case CYAN:
         case LIGHTCYAN:   return TILEP_CLOAK_CYAN;
         case RED:
         case LIGHTRED:    return TILEP_CLOAK_RED;
         case MAGENTA:
         case LIGHTMAGENTA:return TILEP_CLOAK_MAGENTA;
         case BROWN:       return TILEP_CLOAK_LBROWN;
         case YELLOW:      return TILEP_CLOAK_YELLOW;
         case LIGHTGREY:   return TILEP_CLOAK_GRAY;
         case DARKGREY:    return TILEP_CLOAK_BLACK;
         case WHITE:       return TILEP_CLOAK_WHITE;
         default:          return 0;
    }
}

int tilep_equ_helm(const item_def &item)
{
    if (you.equip[EQ_HELMET] == -1)
        return 0;
    if (item.base_type != OBJ_ARMOUR)
        return 0;

    if (is_unrandom_artefact(item))
    {
        int idx = find_unrandart_index(item);
        switch (idx + 1)
        {
            case 11:
                // crown of Dyrovepreva
                return TILEP_HELM_DYROVEPREVA;
            case 41:
                // mask of the Dragon
                return TILEP_HELM_ART_DRAGONHELM;
            case 50:
                // hat of the Alchemist
                return TILEP_HELM_TURBAN_PURPLE;
        }

        // Although there shouldn't be any, just in case
        // unhandled artefacts fall through to defaults...
    }

    int etype = get_etype(item);
    int helmet_desc = get_helmet_desc(item);
    switch (item.sub_type)
    {
        case ARM_CAP:
            switch (item.colour)
            {
                case BLACK:
                case BLUE:
                case LIGHTBLUE:
                    return TILEP_HELM_FEATHER_BLUE;
                case GREEN:
                case LIGHTGREEN:
                case CYAN:
                case LIGHTCYAN:
                    return TILEP_HELM_FEATHER_GREEN;
                case RED:
                case LIGHTRED:
                case MAGENTA:
                case LIGHTMAGENTA:
                    return TILEP_HELM_FEATHER_RED;
                case BROWN:
                case YELLOW:
                    return TILEP_HELM_FEATHER_YELLOW;
                case LIGHTGREY:
                case DARKGREY:
                case WHITE:
                    return TILEP_HELM_FEATHER_WHITE;
            }
            return 0;

        case ARM_WIZARD_HAT:
            switch (item.colour)
            {
                case MAGENTA:
                case LIGHTMAGENTA:
                case BLACK:
                    return TILEP_HELM_WIZARD_BLACKRED;
                case BLUE:
                case LIGHTBLUE:
                    return TILEP_HELM_WIZARD_BLUE;
                case GREEN:
                case LIGHTGREEN:
                    return TILEP_HELM_WIZARD_DARKGREEN;
                case CYAN:
                    return TILEP_HELM_WIZARD_PURPLE;
                case LIGHTCYAN:
                    return TILEP_HELM_WIZARD_BLUEGREEN;
                case RED:
                case LIGHTRED:
                    return TILEP_HELM_WIZARD_RED;
                case BROWN:
                    return TILEP_HELM_WIZARD_BROWN;
                case YELLOW:
                    return TILEP_HELM_WIZARD_BLACKGOLD;
                case LIGHTGREY:
                case DARKGREY:
                case WHITE:
                    return TILEP_HELM_WIZARD_WHITE;
            }
            return 0;

        // Why are there both helms and helmets? -Enne
        // It'd be like having catsup and ketchup in the same game.
        case ARM_HELM:
        case ARM_HELMET:
            switch (helmet_desc)
            {
                case THELM_DESC_PLAIN:
                    switch (etype)
                    {
                        default:
                            return TILEP_HELM_CHAIN;
                        case 1:
                            return TILEP_HELM_HELM_GIMLI;
                        case 2:
                            return TILEP_HELM_HELM_OFS; // urgh
                        case 3:
                            return TILEP_HELM_FHELM_GRAY2;
                        case 4:
                            return TILEP_HELM_FHELM_BLACK;
                    }
                case THELM_DESC_WINGED:
                    return TILEP_HELM_YELLOW_WING;
                case THELM_DESC_HORNED:
                    switch (etype)
                    {
                        default:
                            return TILEP_HELM_FHELM_HORN2;
                        case 1:
                            return TILEP_HELM_BLUE_HORN_GOLD;
                        case 2:
                            return TILEP_HELM_FHELM_HORN_GRAY;
                        case 3:
                            return TILEP_HELM_FHELM_HORN_YELLOW;
                        case 4:
                            return TILEP_HELM_FHELM_OFS; // urgh
                    }
                case THELM_DESC_CRESTED:
                    return TILEP_HELM_FHELM_ISILDUR;
                case THELM_DESC_PLUMED:
                    return TILEP_HELM_FHELM_PLUME;
                case THELM_DESC_SPIKED:
                    return TILEP_HELM_FHELM_EVIL;
                case THELM_DESC_VISORED:
                    return TILEP_HELM_FHELM_GRAY3;
                case THELM_DESC_JEWELLED:
                    return TILEP_HELM_FULL_GOLD;
            }
    }

    return 0;
}

int tilep_equ_gloves(const item_def &item)
{
    if (you.equip[EQ_GLOVES] == -1)
        return 0;
    if (item.base_type != OBJ_ARMOUR)
        return 0;
    if (item.sub_type != ARM_GLOVES)
        return 0;

    if (is_unrandom_artefact(item))
    {
        int idx = find_unrandart_index(item);
        switch (idx + 1)
        {
            case 30: // gauntlets of War (thick brown)
                return TILEP_ARM_GLOVE_BLACK;
            case 51: // fencer's gloves (white)
                return TILEP_ARM_GLOVE_WHITE;
        }
    }

    int etype = get_etype(item);
    switch (etype)
    {
        default:
        case 0:
            switch (item.colour)
            {
                case LIGHTBLUE:
                    return TILEP_ARM_GLOVE_BLUE;
                default:
                case BROWN:
                    return TILEP_ARM_GLOVE_BROWN;
            }
        case 1:
        case 2:
        case 3:
        case 4:
            switch (item.colour)
            {
                case LIGHTBLUE:
                default:
                    return TILEP_ARM_GLOVE_CHUNLI;
                case BROWN:
                    return TILEP_ARM_GLOVE_GRAYFIST;
            }
    }

    return 0;
}

int tilep_equ_boots(const item_def &item)
{
    if (you.equip[EQ_BOOTS] == -1)
        return 0;
    if (item.base_type != OBJ_ARMOUR)
        return 0;

    if (item.sub_type == ARM_NAGA_BARDING)
        return TILEP_BOOTS_NAGA_BARDING;
    if (item.sub_type == ARM_CENTAUR_BARDING)
        return TILEP_BOOTS_CENTAUR_BARDING;

    if (item.sub_type != ARM_BOOTS)
        return 0;

    if (is_unrandom_artefact(item))
    {
        int idx = find_unrandart_index(item);
        switch (idx + 1)
        {
            case 23: // boots of the assassin
                return TILEP_BOOTS_MIDDLE_GRAY;
        }
    }

    int etype = get_etype(item);
    switch (etype)
    {
        default:
        case 0:
            return TILEP_BOOTS_MIDDLE_BROWN3;
        case 1:
        case 2:
            switch (item.colour)
            {
                case BROWN:
                default:
                    return TILEP_BOOTS_MESH_RED;
                case BLUE:
                    return TILEP_BOOTS_MESH_BLUE;
            }
        case 3:
        case 4:
            switch (item.colour)
            {
                case BROWN:
                    return TILEP_BOOTS_LONG_RED;
                default:
                case BLUE:
                    return TILEP_BOOTS_BLUE_GOLD;
            }
    }

    return 0;
}

#ifdef TILEP_DEBUG
/*
 *   Debugging routines
 */

const char *get_ctg_name(int part)
{
    return tilep_parts_name[part];
}

int get_ctg_idx(char *name)
{
    int i;

    for(i=0;i<TILEP_PARTS_TOTAL;i++)
        if(strcmp(name, tilep_parts_name[i])==0) return i;
    return 0;
}

const char *get_parts_name(int part, int idx)
{
    static char tmp[10];
    const char *ptr = tilep_comment[ tilep_comment_ofs[part] -1 + idx ];
    if(idx==0) return "";
    if(ptr[0]==0)
    {
        sprintf(tmp,"%02d",idx);
        return tmp;
    }
    return ptr;
}

// �p�[�c�̖��O�𐔎��ɕϊ�
int get_parts_idx(int part, char *name)
{
    int res = atoi(name);
    int i;

    for(i=0;i<tilep_parts_total[part];i++)
        if(strcmp(name, tilep_comment[ tilep_comment_ofs[part]+i])==0)
            return i+1;
    return res;
}
#endif /* TILEP_DEBUG */

/*
 * Tile data preparation
 */

// for clarity
enum SpecialIdx
{
    SPECIAL_N = 0,
    SPECIAL_NE = 1,
    SPECIAL_E = 2,
    SPECIAL_SE = 3,
    SPECIAL_S = 4,
    SPECIAL_SW = 5,
    SPECIAL_W = 6,
    SPECIAL_NW = 7,
    SPECIAL_FULL = 8
};

int jitter(SpecialIdx i)
{
    return (i + random_range(-1, 1) + 8) % 8;
}

void tile_init_flavor()
{
    for (int x = 0; x < GXM; x++)
    {
        for (int y = 0; y < GYM; y++)
        {
            int max_wall_flavor = get_num_wall_flavors() - 1;
            int max_floor_flavor = get_num_floor_flavors() - 1;
            int wall_flavor = random_range(0, max_wall_flavor);
            int floor_flavor = random_range(0, max_floor_flavor);

            env.tile_flavor[x][y].floor = floor_flavor;
            env.tile_flavor[x][y].wall = wall_flavor;

            if (grd[x][y] == DNGN_FLOOR_SPECIAL)
            {
                int left_grd = (x > 0) ? grd[x-1][y] : DNGN_ROCK_WALL;
                int right_grd = (x < GXM - 1) ? grd[x+1][y] : DNGN_ROCK_WALL;
                int up_grd = (y > 0) ? grd[x][y-1] : DNGN_ROCK_WALL;
                int down_grd = (y < GYM - 1) ? grd[x][y+1] : DNGN_ROCK_WALL;

                // The special tiles contains part floor and part special, so
                // if there are adjacent floor or special tiles, we should
                // do our best to "connect" them appropriately.  If there are
                // are other tiles there (walls, doors, whatever...) then it
                // doesn't matter.
                bool l_nrm = left_grd == DNGN_FLOOR;
                bool r_nrm = right_grd == DNGN_FLOOR;
                bool u_nrm = up_grd == DNGN_FLOOR;
                bool d_nrm = down_grd == DNGN_FLOOR;
                bool l_spc = left_grd == DNGN_FLOOR_SPECIAL;
                bool r_spc = right_grd == DNGN_FLOOR_SPECIAL;
                bool u_spc = up_grd == DNGN_FLOOR_SPECIAL;
                bool d_spc = down_grd == DNGN_FLOOR_SPECIAL;

                if (l_nrm && r_nrm || u_nrm && d_nrm)
                {
                    // Not much to do here...
                    env.tile_flavor[x][y].special = SPECIAL_FULL;
                }
                else if (l_nrm)
                {
                    if (u_nrm)
                        env.tile_flavor[x][y].special = SPECIAL_NW;
                    else if (d_nrm)
                        env.tile_flavor[x][y].special = SPECIAL_SW;
                    else if (u_spc && d_spc)
                        env.tile_flavor[x][y].special = SPECIAL_W;
                    else if (u_spc && r_spc)
                        env.tile_flavor[x][y].special = SPECIAL_SW;
                    else if (d_spc && r_spc)
                        env.tile_flavor[x][y].special = SPECIAL_NW;
                    else if (u_spc)
                        env.tile_flavor[x][y].special = coinflip() ?
                            SPECIAL_W : SPECIAL_SW;
                    else if (d_spc)
                        env.tile_flavor[x][y].special = coinflip() ?
                            SPECIAL_W : SPECIAL_NW;
                    else
                        env.tile_flavor[x][y].special = jitter(SPECIAL_W);
                }
                else if (r_nrm)
                {
                    if (u_nrm)
                        env.tile_flavor[x][y].special = SPECIAL_NE;
                    else if (d_nrm)
                        env.tile_flavor[x][y].special = SPECIAL_SE;
                    else if (u_spc && d_spc)
                        env.tile_flavor[x][y].special = SPECIAL_E;
                    else if (u_spc && l_spc)
                        env.tile_flavor[x][y].special = SPECIAL_SE;
                    else if (d_spc && l_spc)
                        env.tile_flavor[x][y].special = SPECIAL_NE;
                    else if (u_spc)
                        env.tile_flavor[x][y].special = coinflip() ?
                            SPECIAL_E : SPECIAL_SE;
                    else if (d_spc)
                        env.tile_flavor[x][y].special = coinflip() ?
                            SPECIAL_E : SPECIAL_NE;
                    else
                        env.tile_flavor[x][y].special = jitter(SPECIAL_E);
                }
                else if (u_nrm)
                {
                    if (r_spc && l_spc)
                        env.tile_flavor[x][y].special = SPECIAL_N;
                    else if (r_spc && d_spc)
                        env.tile_flavor[x][y].special = SPECIAL_NW;
                    else if (l_spc && d_spc)
                        env.tile_flavor[x][y].special = SPECIAL_NE;
                    else if (r_spc)
                        env.tile_flavor[x][y].special = coinflip() ?
                            SPECIAL_N : SPECIAL_NW;
                    else if (l_spc)
                        env.tile_flavor[x][y].special = coinflip() ?
                            SPECIAL_N : SPECIAL_NE;
                    else
                        env.tile_flavor[x][y].special = jitter(SPECIAL_N);
                }
                else if (d_nrm)
                {
                    if (r_spc && l_spc)
                        env.tile_flavor[x][y].special = SPECIAL_S;
                    else if (r_spc && u_spc)
                        env.tile_flavor[x][y].special = SPECIAL_SW;
                    else if (l_spc && u_spc)
                        env.tile_flavor[x][y].special = SPECIAL_SE;
                    else if (r_spc)
                        env.tile_flavor[x][y].special = coinflip() ?
                            SPECIAL_S : SPECIAL_SW;
                    else if (l_spc)
                        env.tile_flavor[x][y].special = coinflip() ?
                            SPECIAL_S : SPECIAL_SE;
                    else
                        env.tile_flavor[x][y].special = jitter(SPECIAL_S);
                }
                else if (u_spc && d_spc)
                {
                    // We know this value is already initialized and
                    // is necessarily in bounds.
                    char t = env.tile_flavor[x][y-1].special;
                    if (t == SPECIAL_NE || t == SPECIAL_E)
                        env.tile_flavor[x][y].special = SPECIAL_E;
                    else if (t == SPECIAL_NW || t == SPECIAL_W)
                        env.tile_flavor[x][y].special = SPECIAL_W;
                    else
                        env.tile_flavor[x][y].special = SPECIAL_FULL;
                }
                else if (r_spc && l_spc)
                {
                    // We know this value is already initialized and
                    // is necessarily in bounds.
                    char t = env.tile_flavor[x-1][y].special;
                    if (t == SPECIAL_NW || t == SPECIAL_N)
                        env.tile_flavor[x][y].special = SPECIAL_N;
                    else if (t == SPECIAL_SW || t == SPECIAL_S)
                        env.tile_flavor[x][y].special = SPECIAL_S;
                    else
                        env.tile_flavor[x][y].special = SPECIAL_FULL;
                }
                else if (u_spc && l_spc)
                {
                    env.tile_flavor[x][y].special = SPECIAL_SE;
                }
                else if (u_spc && r_spc)
                {
                    env.tile_flavor[x][y].special = SPECIAL_SW;
                }
                else if (d_spc && l_spc)
                {
                    env.tile_flavor[x][y].special = SPECIAL_NE;
                }
                else if (d_spc && r_spc)
                {
                    env.tile_flavor[x][y].special = SPECIAL_NW;
                }
                else
                {
                    env.tile_flavor[x][y].special = SPECIAL_FULL;
                }
            }
            else
            {
                env.tile_flavor[x][y].special = 0;
            }
        }
    }

    // Second pass for clean up.  The only bad part about the above
    // algorithm is that it could turn a block of floor like this:
    //
    // N4NN
    // 3125
    // NN6N
    //
    // (KEY: N = normal floor, # = special floor)
    //
    // Into these flavors:
    // 1 - SPECIAL_S
    // 2 - SPECIAL_N
    // 3-6, not important
    //
    // Generally the tiles don't fit with a north to the right or left
    // of a south tile.  What we really want to do is to separate the
    // two regions, by making 1 a SPECIAL_SE and 2 a SPECIAL_NW tile.
    for (int x = 0; x < GXM - 1; x++)
    {
        for (int y = 0; y < GYM - 1; y++)
        {
            if (grd[x][y] != DNGN_FLOOR_SPECIAL)
                continue;
            if (env.tile_flavor[x][y].special != SPECIAL_N &&
                env.tile_flavor[x][y].special != SPECIAL_S &&
                env.tile_flavor[x][y].special != SPECIAL_E &&
                env.tile_flavor[x][y].special != SPECIAL_W)
            {
                continue;
            }

            int right_flavor = x < GXM - 1 ? env.tile_flavor[x+1][y].special :
                SPECIAL_FULL;
            int down_flavor = y < GYM - 1 ? env.tile_flavor[x][y+1].special :
                SPECIAL_FULL;
            int this_flavor = env.tile_flavor[x][y].special;

            if (this_flavor == SPECIAL_N && right_flavor == SPECIAL_S)
            {
                env.tile_flavor[x][y].special = SPECIAL_NE;
                env.tile_flavor[x+1][y].special = SPECIAL_SW;
            }
            else if (this_flavor == SPECIAL_S && right_flavor == SPECIAL_N)
            {
                env.tile_flavor[x][y].special = SPECIAL_SE;
                env.tile_flavor[x+1][y].special = SPECIAL_NW;
            }
            else if (this_flavor == SPECIAL_E && down_flavor == SPECIAL_W)
            {
                env.tile_flavor[x][y].special = SPECIAL_SE;
                env.tile_flavor[x][y+1].special = SPECIAL_NW;
            }
            else if (this_flavor == SPECIAL_W && down_flavor == SPECIAL_E)
            {
                env.tile_flavor[x][y].special = SPECIAL_NE;
                env.tile_flavor[x][y+1].special = SPECIAL_SW;
            }
        }
    }
}

void tile_clear_buf()
{
    for (int y = 0; y < GYM; y++)
    {
        for (int x = 0; x < GXM; x++)
        {
            unsigned char this_is_wall = 128;
            gv_now[x][y]=0;
            tile_dngn[x][y]=TILE_DNGN_UNSEEN;
            if (env.map[x][y].glyph() != 0)
            {
                int g = grd[x][y];
                if (g >= DNGN_ORCISH_IDOL && g != DNGN_OPEN_DOOR)
                    this_is_wall = 0;
            }

            wall_flag[x][y] = this_is_wall;
        }
    }

    for (int y = 0; y < GYM; y++)
    {
        for (int x = 0; x < GXM; x++)
        {
            unsigned char w = wall_flag[x][y]&0x80;
            if (w)
            {
                if (x != 0)     wall_flag[x-1][y] |= WFLAG_R;
                if (x != GXM-1) wall_flag[x+1][y] |= WFLAG_L;
                if (y != 0)     wall_flag[x][y-1] |= WFLAG_D;
                if (y != GYM-1) wall_flag[x][y+1] |= WFLAG_U;
            }
            wall_flag[x][y] &= 0xf;
        }
    }
}

/* called from view.cc */
void tile_draw_floor()
{
    int cx, cy;
    for (cy = 0; cy < env.tile_fg.height(); cy++)
    {
        for (cx = 0; cx < env.tile_fg.width(); cx++)
        {
            const coord_def ep(cx+1, cy+1);
            const coord_def gc = view2grid(show2view(ep));
            int bg = TILE_DNGN_UNSEEN | tile_unseen_flag(gc);

            int object = env.show(ep);
            if (in_bounds(gc) && object != 0)
            {
                if (object != gv_now[gc.x][gc.y])
                {
                    if (object == DNGN_SECRET_DOOR)
                        object = (int)grid_secret_door_appearance(gc.x, gc.y);

                    tile_dngn[gc.x][gc.y] = tileidx_feature(object);
                    gv_now[gc.x][gc.y] = object;

                    unsigned char wall = 0xff;
                    if (object >= DNGN_ORCISH_IDOL && object != DNGN_OPEN_DOOR)
                        wall = 0;

                    if (gc.x != 0)
                    {
                        wall_flag[gc.x-1][gc.y] = (wall_flag[gc.x-1][gc.y] & 
                            (~WFLAG_R)) | (wall & WFLAG_R);
                    }
                    if (gc.x != GXM-1)
                    {
                        wall_flag[gc.x+1][gc.y] = (wall_flag[gc.x+1][gc.y] & 
                            (~WFLAG_L)) | (wall & WFLAG_L);
                    }
                    if (gc.y != 0)
                    {
                        wall_flag[gc.x][gc.y-1] = (wall_flag[gc.x][gc.y-1] & 
                            (~WFLAG_D)) | (wall & WFLAG_D);
                    }
                    if (gc.y != GYM-1)
                    {
                        wall_flag[gc.x][gc.y+1] = (wall_flag[gc.x][gc.y+1] & 
                            (~WFLAG_U)) | (wall & WFLAG_U);
                    }
                }
                bg = tile_dngn[gc.x][gc.y];
            }
            // init tiles
            env.tile_bg[ep.x-1][ep.y-1] = bg;
            env.tile_fg[ep.x-1][ep.y-1] = 0;
        }
    }
}

// Called from item() in view.cc
void tile_place_item(int x, int y, int idx)
{
    int t = tileidx_item(mitm[idx]);
    if (mitm[idx].link != NON_ITEM)
        t |= TILE_FLAG_S_UNDER;
    env.tile_fg[x-1][y-1]=t;

    if (item_needs_autopickup(mitm[idx]))
        env.tile_bg[x-1][y-1] |= TILE_FLAG_CURSOR3;
}

void tile_place_item_bk(int gx, int gy, int idx)
{
    int t = tileidx_item(mitm[idx]);
    env.tile_bk_fg[gx][gy] = t;

    if (item_needs_autopickup(mitm[idx]))
        env.tile_bk_bg[gx][gy] |= TILE_FLAG_CURSOR3;
}

void tile_place_tile_bk(int gx, int gy, int tileidx)
{
    env.tile_bk_fg[gx][gy] = tileidx;
}

// Called from item() in view.cc
void tile_place_item_marker(int x, int y, int idx)
{
    env.tile_fg[x-1][y-1] |= TILE_FLAG_S_UNDER;

    if (item_needs_autopickup(mitm[idx]))
        env.tile_bg[x-1][y-1] |= TILE_FLAG_CURSOR3;
}

// Called from monster_grid() in view.cc
void tile_place_monster(int gx, int gy, int idx, bool foreground)
{
    if (idx == NON_MONSTER)
    {
        return;
    }

    const coord_def gc(gx, gy);
    const coord_def ep = view2show(grid2view(gc));

    int t = tileidx_monster(idx);
    int t0 = t & TILE_FLAG_MASK;
    int flag = t & (~TILE_FLAG_MASK);
    int mon_wep = menv[idx].inv[MSLOT_WEAPON];

    if (menv[idx].type >=  MONS_DRACONIAN && 
	    menv[idx].type <= MONS_DRACONIAN_SCORCHER)
    {
	int race = draco_subspecies(&menv[idx]);
	int cls = menv[idx].type;
	int eq = 0;
	if (mon_wep != NON_ITEM &&
		(cls == race || cls == MONS_DRACONIAN_KNIGHT))
            eq = tilep_equ_weapon(mitm[mon_wep]);
        t = flag | TileMcacheFind(cls, eq, race);
    }
    else if (mon_wep != NON_ITEM)
    {
        int eq = tilep_equ_weapon(mitm[mon_wep]);
        switch(t0)
        {
            // 3D chars
            case TILE_MONS_VAULT_GUARD:

            case TILE_MONS_BLORK_THE_ORC:
            case TILE_MONS_ORC:
            case TILE_MONS_ORC_KNIGHT:
            case TILE_MONS_ORC_WARLORD:
            case TILE_MONS_ORC_WARRIOR:
            case TILE_MONS_URUG:

            case TILE_MONS_GOBLIN:
            case TILE_MONS_IJYB:
            case TILE_MONS_HOBGOBLIN:
            case TILE_MONS_GNOLL:
            case TILE_MONS_BOGGART:

            case TILE_MONS_KOBOLD:
            case TILE_MONS_KOBOLD_DEMONOLOGIST:
            case TILE_MONS_BIG_KOBOLD:

            case TILE_MONS_DEEP_ELF_FIGHTER:
            case TILE_MONS_DEEP_ELF_SOLDIER:
            case TILE_MONS_DEEP_ELF_KNIGHT:
            case TILE_MONS_DEEP_ELF_BLADEMASTER:
            case TILE_MONS_DEEP_ELF_MASTER_ARCHER:
            case TILE_MONS_DEEP_ELF_MAGE:
            case TILE_MONS_DEEP_ELF_SUMMONER:
            case TILE_MONS_DEEP_ELF_CONJURER:
            case TILE_MONS_DEEP_ELF_PRIEST:
            case TILE_MONS_DEEP_ELF_HIGH_PRIEST:
            case TILE_MONS_DEEP_ELF_DEMONOLOGIST:
            case TILE_MONS_DEEP_ELF_ANNIHILATOR:
            case TILE_MONS_DEEP_ELF_SORCERER:
            case TILE_MONS_DEEP_ELF_DEATH_MAGE:

            case TILE_MONS_MIDGE:
            case TILE_MONS_IMP:

            case TILE_MONS_NAGA:
            case TILE_MONS_GREATER_NAGA:
            case TILE_MONS_NAGA_WARRIOR:
            case TILE_MONS_NAGA_MAGE:

            case TILE_MONS_OGRE_MAGE:
            case TILE_MONS_RED_DEVIL:
            case TILE_MONS_WIZARD:
            case TILE_MONS_HUMAN:
            case TILE_MONS_ELF:

            case TILE_MONS_ANGEL:

            case TILE_MONS_HELL_KNIGHT:

            case TILE_MONS_NORRIS:
            case TILE_MONS_MAUD:
            case TILE_MONS_DUANE:
            case TILE_MONS_EDMUND:
            case TILE_MONS_FRANCES:
            case TILE_MONS_HAROLD:
            case TILE_MONS_JOSEPH:
            case TILE_MONS_JOZEF:
            case TILE_MONS_RUPERT:
            case TILE_MONS_TERENCE:
            case TILE_MONS_WAYNE:
            case TILE_MONS_FREDERICK:

            case TILE_MONS_RAKSHASA:
            case TILE_MONS_RAKSHASA_FAKE:

            case TILE_MONS_VAMPIRE_KNIGHT:

            case TILE_MONS_SKELETAL_WARRIOR:

            if (eq != 0 )
                t = flag | TileMcacheFind(t0, eq);
            break;

        }
    }

    if (foreground)
    {
        env.tile_fg[ep.x-1][ep.y-1] = t;
    }
    else
    {
        env.tile_bk_fg[gc.x][gc.y] = t0;
    }
}

void tile_place_cloud(int x, int y, int type, int decay)
{
    env.tile_fg[x-1][y-1]= tileidx_cloud(type, decay);
}

unsigned int tileRayCount = 0;
FixedVector<coord_def, 30> tileRays;

void tile_place_ray(const coord_def& gc)
{
    // Record rays for later.  The curses version just applies
    // rays directly to the screen.  The tiles version doesn't have
    // (nor want) such direct access.  So, it batches up all of the
    // rays and applies them in viewwindow(...).
    if (tileRayCount < tileRays.size() - 1)
    {
        tileRays[tileRayCount++] = view2show(grid2view(gc));
    }
}

void tile_draw_rays(bool resetCount)
{
    for (unsigned int i = 0; i < tileRayCount; i++)
    {
        env.tile_bg[tileRays[i].x-1][tileRays[i].y-1] |= TILE_FLAG_RAY;
    }

    if (resetCount)
        tileRayCount = 0;
}

void tile_finish_dngn(short unsigned int *tileb, int cx, int cy)
{
    int x, y;
    int count = 0;

    for (y=0; y < crawl_view.viewsz.y; y++)
    {
        for (x=0; x < crawl_view.viewsz.x; x++)
        {
            const int gx = view2gridX(x);
            const int gy = view2gridY(y);
            int wall = 0;

            char wall_flv = 0;
            char floor_flv = 0;
            char special_flv = 0;
            if (map_bounds( gx, gy ))
            {
                wall = wall_flag[gx][gy];
                wall_flv = env.tile_flavor[gx][gy].wall;
                floor_flv = env.tile_flavor[gx][gy].floor;
                special_flv = env.tile_flavor[gx][gy].special;
            }
            finalize_tile(&tileb[count], wall, 
                wall_flv, floor_flv, special_flv);
            finalize_tile(&tileb[count+1], wall,
                wall_flv, floor_flv, special_flv);
            count += 2;
        }
    }
}

void tile_draw_dungeon(short unsigned int *tileb)
{
    tile_finish_dngn(tileb, you.x_pos, you.y_pos);
    TileDrawDungeon(tileb);    
}

#define swapint(a, b) {int tmp = a; a = b; b = tmp;}

// Item is unided(1) or tried(2) or id'ed (0)
//  Note that Jewellries are never "tried"
static int item_unid_type(const item_def &item)
{
    int t = item.base_type;
    int s = item.sub_type;
    int id0 = 0;

    id_arr& id = get_typeid_array();

    if ((item.flags &ISFLAG_KNOW_TYPE) != 0)
        return 0;

    switch (t)
    {
        case OBJ_SCROLLS:
          id0 = id[ IDTYPE_SCROLLS ][s];
          if (id0 == ID_TRIED_TYPE)
             return 2;
          else
          if (id0 != ID_KNOWN_TYPE)
             return 1;
          else
             return 0;

        case OBJ_WANDS:
          id0 = id[ IDTYPE_WANDS ][s];
          if (id0 == ID_TRIED_TYPE)
             return 2;
          else
          if (id0 != ID_KNOWN_TYPE)
             return 1;
          else
             return 0;

        case OBJ_POTIONS:
          id0 = id[ IDTYPE_POTIONS ][s];
          if (id0 == ID_TRIED_TYPE)
             return 2;
          else
          if (id0 != ID_KNOWN_TYPE)
             return 1;
          else
             return 0;

        case OBJ_JEWELLERY:
          id0 = id[ IDTYPE_JEWELLERY ][s];
          if (id0 != ID_KNOWN_TYPE)
             return 1;
          else
             return 0;
    }
    return 0;
}

// Helper routine: sort floor item index and pack into idx
int pack_floor_item(int *idx, int *flag, int *isort, int max)
{
    int n = 0;
    static int isort_weapon2[NUM_WEAPONS];
    static int isort_armour2[NUM_ARMOURS];

    static const int isort_weapon[NUM_WEAPONS] = 
    {
        WPN_WHIP, WPN_CLUB, WPN_HAMMER, WPN_MACE,
        WPN_FLAIL, WPN_DEMON_WHIP,
        WPN_ANCUS, WPN_MORNINGSTAR, WPN_EVENINGSTAR,
        WPN_SPIKED_FLAIL, WPN_GREAT_MACE, WPN_DIRE_FLAIL,
        WPN_GIANT_CLUB,  WPN_GIANT_SPIKED_CLUB,

        WPN_KNIFE, WPN_DAGGER, WPN_SHORT_SWORD, WPN_SABRE, WPN_QUICK_BLADE,
        WPN_FALCHION, WPN_LONG_SWORD, WPN_SCIMITAR, WPN_KATANA,
        WPN_DEMON_BLADE, WPN_DOUBLE_SWORD, WPN_GREAT_SWORD, WPN_TRIPLE_SWORD,

        WPN_HAND_AXE, WPN_WAR_AXE, WPN_BROAD_AXE,
        WPN_BATTLEAXE, WPN_EXECUTIONERS_AXE,

        WPN_SPEAR, WPN_TRIDENT, WPN_HALBERD, WPN_SCYTHE,
        WPN_GLAIVE, WPN_DEMON_TRIDENT,

        WPN_QUARTERSTAFF,

        WPN_SLING, WPN_BOW, WPN_CROSSBOW, WPN_HAND_CROSSBOW
    };
    static const int isort_armour[NUM_ARMOURS] = 
    {
        ARM_ROBE, 
        ARM_ANIMAL_SKIN,
        ARM_LEATHER_ARMOUR,
        ARM_TROLL_LEATHER_ARMOUR,
        ARM_RING_MAIL, ARM_SCALE_MAIL, ARM_CHAIN_MAIL, 
        ARM_SPLINT_MAIL, ARM_BANDED_MAIL, ARM_PLATE_MAIL,
        ARM_CRYSTAL_PLATE_MAIL,
        ARM_SWAMP_DRAGON_ARMOUR,
        ARM_MOTTLED_DRAGON_ARMOUR,
        ARM_STEAM_DRAGON_ARMOUR,
        ARM_DRAGON_ARMOUR,
        ARM_ICE_DRAGON_ARMOUR,
        ARM_STORM_DRAGON_ARMOUR,
        ARM_GOLD_DRAGON_ARMOUR,
        ARM_TROLL_HIDE,
        ARM_SWAMP_DRAGON_HIDE,
        ARM_MOTTLED_DRAGON_HIDE,
        ARM_STEAM_DRAGON_HIDE,
        ARM_DRAGON_HIDE,
        ARM_ICE_DRAGON_HIDE,
        ARM_STORM_DRAGON_HIDE,
        ARM_GOLD_DRAGON_HIDE,
        ARM_CLOAK,
        ARM_BUCKLER, ARM_SHIELD, ARM_LARGE_SHIELD,
        ARM_HELMET, ARM_GLOVES, ARM_BOOTS
    };

    int i;
    for (i=0;i<NUM_WEAPONS;i++)
        isort_weapon2[isort_weapon[i]] = i;
    for (i=0;i<NUM_ARMOURS;i++)
        isort_armour2[isort_armour[i]] = i;

    int o = igrd[you.x_pos][you.y_pos];
    if (o == NON_ITEM) return 0;

    while (o != NON_ITEM)
    {
        int id0 = item_unid_type(mitm[o]);
        int next = mitm[o].link;
        int typ = mitm[o].base_type;

        if (n >= max) break;

        idx[n] = o;
        isort[n] = typ * 256 * 3;
        if (typ == OBJ_WEAPONS)
        {
            isort[n] += 3 * isort_weapon2[ mitm[o].sub_type];
        }
        if (typ == OBJ_ARMOUR)
        {
            isort[n] += 3 * isort_armour2[ mitm[o].sub_type ];
        }
        flag[n] = 0;

        if (item_ident( mitm[o], ISFLAG_KNOW_CURSE ) &&
            item_cursed(mitm[o]))
        {
            flag[n] |= TILEI_FLAG_CURSE;
        }

        if (id0 != 0)
        {
            isort[n] += id0;
            if (id0 == 2)
                flag[n] = TILEI_FLAG_TRIED;
        }
        flag[n] |= TILEI_FLAG_FLOOR;

        // Simple Bubble sort
        int k = n;
        while( (k > 0) && (isort[k-1] > isort[k]))
        {
            swapint(idx[k-1], idx[k]);
            swapint(isort[k-1], isort[k]);
            swapint(flag[k-1], flag[k]);
            k--;
        }
        n++;
        o = next;
    }
    return n;
}

// Helper routine: Calculate tile index and quantity data to be displayed
void finish_inven_data(int n, int *tiles, int *num, int *idx, int *iflag)
{
    int i;

    for(i = 0; i < n; i++)
    {
        int q = -1;
        int j = idx[i];
        item_def *itm;

        if (j==-1)
        {
            num[i]=-1;
            tiles[i]= 0;
            continue;
        }

        if (iflag[i] & TILEI_FLAG_FLOOR)
            itm = &mitm[j];
        else
            itm = &you.inv[j];

        int type = itm->base_type;

        if (type == OBJ_FOOD || type == OBJ_SCROLLS
             || type == OBJ_POTIONS || type == OBJ_MISSILES)
            q = itm->quantity;
        if (q==1) q = -1;

        if (type == OBJ_WANDS && 
              (itm->flags & ISFLAG_KNOW_PLUSES )!= 0)
            q = itm->plus;

        tiles[i] = tileidx_item(*itm);
        num[i] = q;
    }
}

// Display Inventory/floor items
#include "guic.h"
extern TileRegionClass *region_item;
extern TileRegionClass *region_item2;
extern WinClass *win_main;

void tile_draw_inv(int item_type, int flag)
{
    #define MAXINV 200
    int tiles[MAXINV];
    int num[MAXINV];
    int idx[MAXINV];
    int iflag[MAXINV];
    int isort[MAXINV];
    int n = 0;
    int i, j;

    if (flag == -1)
    {
        flag = (win_main->active_layer == 0) ? REGION_INV1 : REGION_INV2;
    }

    TileRegionClass *r = (flag == REGION_INV1) ? region_item:region_item2;
    int numInvTiles = r->mx * r->my;
    if (numInvTiles > MAXINV)
        numInvTiles = MAXINV;

    // item.base_type <-> char conversion table
    const static char *obj_syms = ")([/%#?=!#+\\0}x";

    const static char *syms_table[] =
    {
        ")\\", // weapons and staves
        "(",   // missile
        "[",   // armour
        "/",   // wands
        "%",   // foods
        "#",   //  none
        "?+",  // scrolls and books
        "=",   // rings/amulets
        "!",   // potions
        "#",   //  none
        "?+",  // books/scrolls
        ")\\", // weapons and staves
        "0",
        "}",
        "x"
    };

    const char *item_chars = Options.show_items;

    int eq_flag[ENDOFPACK];
    int empty = 0;

    for (j = 0; j < ENDOFPACK; j++)
    {
        eq_flag[j] =
            (you.inv[j].quantity != 0 && is_valid_item( you.inv[j])) ? 1:0;
        empty += 1-eq_flag[j];
    }

    for (j=0; j<NUM_EQUIP; j++)
    {
        int eq = you.equip[j];
        if (eq >= 0 && eq < ENDOFPACK)
            eq_flag[eq] = 2;
    }

    if (item_type >= 0)
        item_chars = syms_table[item_type];
    else
    if (item_type == -2)
        item_chars = obj_syms;
    else
    if (item_type == -3)
        item_chars = ".";

    if (item_chars[0] == 0) return;

    for (i=0; i < (int)strlen(item_chars); i++)
    {
        int top = n;
        char ic = item_chars[i];

        if (n >= numInvTiles) break;

        // Items on the floor
        if (ic == '.')
        {
            n += pack_floor_item(&idx[n], &iflag[n], &isort[n], numInvTiles - n);
            continue;
        }

        // empty slots
        if (ic == '_')
        {
            for (j = 0; j < empty && n < numInvTiles; j++)
            {
                idx[n] = -1;
                iflag[n] = 0;
                n++;
            }
            continue;
        }

        // convert item char to item type
        int type = -1;
        for (j=0; j < (int)strlen(obj_syms); j++)
        {
            if (obj_syms[j] == ic)
            {
                type = j;
                break;
            }
        }

        if (type == -1) continue;

        for (j = 0; j < ENDOFPACK && n < numInvTiles; j++)
        {
            if (you.inv[j].base_type==type && eq_flag[j] != 0)
            {
                int sval = NUM_EQUIP + you.inv[j].sub_type;
                int base = 0;
                int id0 = item_unid_type(you.inv[j]);

                idx[n] = j;
                iflag[n] = 0;

                if (type == OBJ_JEWELLERY && sval >= AMU_RAGE)
                {
                    base = 1000;
                    sval = base + sval;
                }

                if (id0 == 2)
                {
                     iflag[n] |= TILEI_FLAG_TRIED;
                     // To the tail
                     sval = base + 980;
                }
                else if (id0 == 1)
                {
                     // To the tail
                     sval = base + 990;
                }

                // Equipment first
                if (eq_flag[j]==2)
                {
                     //sval = base;
                     iflag[n] |= TILEI_FLAG_EQUIP;
                }

                if (item_cursed(you.inv[j]) &&
                    item_ident( you.inv[j], ISFLAG_KNOW_CURSE ))
                {
                    iflag[n] |= TILEI_FLAG_CURSE;
                }

                if (flag == REGION_INV2)
                    sval = j;

                isort[n] = sval;

                int k = n;
                while( (k > top) && (isort[k-1] > isort[k]))
                {
                    swapint(idx[k-1], idx[k]);
                    swapint(isort[k-1], isort[k]);
                    swapint(iflag[k-1], iflag[k]);
                    k--;
                }

                n++;
            } // type == base
        } // j
    }  // i

    finish_inven_data(n, tiles, num, idx, iflag);

    for(i = n; i < numInvTiles; i++)
    {
        tiles[i] = 0;
        num[i] = 0;
        idx[i] = -1;
        iflag[i] = 0;
    }
    TileDrawInvData(n, flag, tiles, num, idx, iflag);
}

#endif
