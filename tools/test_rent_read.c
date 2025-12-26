/*
 * test_rent_read.c - Test reading a single rent file
 *
 * Build: clang -std=c23 -I../code -o test_rent_read test_rent_read.c
 */

#include <stdio.h>
#include <stdint.h>
#include "structs.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <rent_file>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("Failed to open rent file");
        return 1;
    }

    printf("Reading: %s\n", argv[1]);
    printf("sizeof(struct obj_file_u) = %zu bytes\n", sizeof(struct obj_file_u));

    struct obj_file_u rent;
    size_t bytes_read = fread(&rent, 1, sizeof(rent), fp);
    fclose(fp);

    printf("Bytes read: %zu\n", bytes_read);
    printf("\nRent file data:\n");
    printf("  Owner:        '%s'\n", rent.owner);
    printf("  Gold left:    %d\n", rent.gold_left);
    printf("  Total cost:   %d\n", rent.total_cost);
    printf("  Last update:  %ld\n", (long)rent.last_update);
    printf("  Minimum stay: %ld\n", (long)rent.minimum_stay);
    printf("  Number:       %d\n", rent.number);

    if (rent.number > 0 && rent.number < 100) {
        printf("\nFirst item:\n");
        printf("  Item number:  %d\n", rent.objects[0].item_number);
        printf("  Value:        %d\n", rent.objects[0].value);
        printf("  Name:         '%s'\n", rent.objects[0].name);
    }

    return 0;
}
