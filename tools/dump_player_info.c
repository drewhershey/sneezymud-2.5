/*
 * dump_player_info.c - Analyze player file and rent files
 *
 * This tool reads the binary player file and associated rent files to
 * determine if there are any meaningful characters (level > 1, equipment).
 *
 * Usage: dump_player_info [lib_directory]
 *   Default lib_directory is "../lib"
 *
 * Build: clang -std=c23 -I../code -o dump_player_info dump_player_info.c
 */

#include <ctype.h>
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "compat_types.h"
#include "structs.h"

/* Statistics tracking */
struct stats {
  int total_players;
  int level_1;
  int level_2_to_10;
  int level_11_plus;
  int with_equipment;
  int with_gold;
  int active_1993;
  int active_1994_plus;
  time_t newest_login;
  char newest_name[20];
};

static void print_player_info(struct char_file_u* player, int has_rent,
                              int rent_items, int rent_gold, struct stats* s) {
  /* Convert compat types to native */
  time_t birth = COMPAT_TO_TIME(player->birth);
  time_t last_logon = COMPAT_TO_TIME(player->last_logon);

  /* Find highest level across all classes */
  int max_level = 0;
  for (int i = 0; i < 8; i++) {
    if (player->level[i] > max_level) {
      max_level = player->level[i];
    }
  }

  /* Update statistics */
  s->total_players++;
  if (max_level <= 1) {
    s->level_1++;
  } else if (max_level <= 10) {
    s->level_2_to_10++;
  } else {
    s->level_11_plus++;
  }

  if (has_rent && rent_items > 0) {
    s->with_equipment++;
  }
  if (has_rent && rent_gold > 0) {
    s->with_gold++;
  }

  /* Track login dates */
  if (last_logon > s->newest_login) {
    s->newest_login = last_logon;
    strncpy(s->newest_name, player->name, sizeof(s->newest_name) - 1);
    s->newest_name[sizeof(s->newest_name) - 1] = '\0';
  }

  struct tm* tm_info = localtime(&last_logon);
  if (tm_info && tm_info->tm_year + 1900 == 1993) {
    s->active_1993++;
  } else if (tm_info && tm_info->tm_year + 1900 >= 1994) {
    s->active_1994_plus++;
  }

  /* Print detailed info for interesting characters */
  if (max_level > 1 || rent_items > 0 || rent_gold > 100) {
    printf("\n%-20s Level: %-2d", player->name, max_level);

    if (last_logon > 0) {
      char time_buf[64];
      strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);
      printf("  Last login: %s", time_buf);
    }

    if (has_rent) {
      printf("\n  Rent: %d items, %d gold", rent_items, rent_gold);
    }

    /* Show affected_by if any */
    int has_affects = 0;
    for (int i = 0; i < MAX_AFFECT; i++) {
      if (player->affected[i].type != 0) {
        has_affects = 1;
        break;
      }
    }
    if (has_affects) {
      printf("\n  Has active effects");
    }
  }
}

static int read_rent_file(const char* filename, int* items, int* gold) {
  FILE* fp = fopen(filename, "rb");
  if (!fp) {
    return 0;
  }

  /* Read rent file the same way reception.c does - field by field */
  char owner[20];
  int gold_left, total_cost, number;
  compat_long last_update, minimum_stay;

  if (fread(&owner, sizeof(owner), 1, fp) != 1 ||
      fread(&gold_left, sizeof(gold_left), 1, fp) != 1 ||
      fread(&total_cost, sizeof(total_cost), 1, fp) != 1 ||
      fread(&last_update, sizeof(last_update), 1, fp) != 1 ||
      fread(&minimum_stay, sizeof(minimum_stay), 1, fp) != 1 ||
      fread(&number, sizeof(number), 1, fp) != 1) {
    fclose(fp);
    return 0;
  }

  fclose(fp);

  *items = number;
  *gold = gold_left;
  return 1;
}

int main(int argc, char* argv[]) {
  const char* lib_dir = argc > 1 ? argv[1] : "../lib";

  /* Open player file */
  char player_file[256];
  snprintf(player_file, sizeof(player_file), "%s/players", lib_dir);

  FILE* fp = fopen(player_file, "rb");
  if (!fp) {
    fprintf(stderr, "Failed to open %s\n", player_file);
    return 1;
  }

  /* Get file size to determine number of players */
  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  size_t player_count = file_size / sizeof(struct char_file_u);
  printf("Player file: %s\n", player_file);
  printf("File size: %ld bytes\n", file_size);
  printf("Record size: %zu bytes\n", sizeof(struct char_file_u));
  printf("Player count: %zu\n", player_count);
  printf("\n%s\n", "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "="
         "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=");

  struct stats stats = {0};
  struct char_file_u player;

  /* Read each player record */
  while (fread(&player, sizeof(player), 1, fp) == 1) {
    /* Convert player name to lowercase for rent file lookup */
    char lowercase_name[20];
    int i;
    for (i = 0; player.name[i] && i < 19; i++) {
      lowercase_name[i] = (char)tolower((unsigned char)player.name[i]);
    }
    lowercase_name[i] = '\0';

    /* Check for rent file */
    char rent_file[256];
    snprintf(rent_file, sizeof(rent_file), "%s/rent/%s", lib_dir,
             lowercase_name);

    int has_rent = 0;
    int rent_items = 0;
    int rent_gold = 0;

    /* Try both with and without .objs extension */
    has_rent = read_rent_file(rent_file, &rent_items, &rent_gold);
    if (!has_rent) {
      snprintf(rent_file, sizeof(rent_file), "%s/rent/%s.objs", lib_dir,
               lowercase_name);
      has_rent = read_rent_file(rent_file, &rent_items, &rent_gold);
    }

    print_player_info(&player, has_rent, rent_items, rent_gold, &stats);
  }

  fclose(fp);

  /* Print summary statistics */
  printf("\n\n%s\n", "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "="
         "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=");
  printf("SUMMARY STATISTICS\n");
  printf("%s\n", "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "="
         "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=");
  printf("\nTotal players:           %d\n", stats.total_players);
  printf("\nLevel distribution:\n");
  printf("  Level 1:               %d (%.1f%%)\n", stats.level_1,
         100.0 * stats.level_1 / stats.total_players);
  printf("  Level 2-10:            %d (%.1f%%)\n", stats.level_2_to_10,
         100.0 * stats.level_2_to_10 / stats.total_players);
  printf("  Level 11+:             %d (%.1f%%)\n", stats.level_11_plus,
         100.0 * stats.level_11_plus / stats.total_players);

  printf("\nEquipment:\n");
  printf("  Players with items:    %d\n", stats.with_equipment);
  printf("  Players with gold:     %d\n", stats.with_gold);

  printf("\nActivity:\n");
  printf("  Last login in 1993:    %d\n", stats.active_1993);
  printf("  Last login 1994+:      %d\n", stats.active_1994_plus);

  if (stats.newest_login > 0) {
    struct tm* tm_info = localtime(&stats.newest_login);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("\nNewest login: %s by %s\n", time_buf, stats.newest_name);
  }

  /* Recommendation */
  printf("\n%s\n", "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "="
         "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=");
  printf("RECOMMENDATION\n");
  printf("%s\n", "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "="
         "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=");

  if (stats.level_11_plus > 0 || stats.with_equipment > 0) {
    printf("\nFound %d characters level 11+ and %d with equipment.\n",
           stats.level_11_plus, stats.with_equipment);
    printf("RECOMMENDATION: Keep these files for historical/nostalgic value.\n");
  } else if (stats.level_2_to_10 > 5) {
    printf(
        "\nFound %d characters level 2-10, but none with significant "
        "progress.\n",
        stats.level_2_to_10);
    printf("RECOMMENDATION: These are mostly test/trial characters.\n");
    printf("                Safe to delete if not needed for testing.\n");
  } else {
    printf("\nMostly level 1 characters with no equipment.\n");
    printf("RECOMMENDATION: Safe to delete - no significant data.\n");
  }

  printf("\n");
  return 0;
}
