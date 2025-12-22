/*
 * convert_mail_endian.c - Convert mail file from big-endian to little-endian
 *
 * The original mail file was created on a Sun SPARC (big-endian) system.
 * This tool converts it to little-endian format for modern x86 systems.
 *
 * Usage: convert_mail_endian <input_file> <output_file>
 *
 * Build: clang -std=c23 -I../code -o convert_mail_endian convert_mail_endian.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mail.h"

static int32_t bswap32(int32_t val) {
    uint32_t u = (uint32_t)val;
    return (int32_t)(((u >> 24) & 0xFF) |
                     ((u >> 8) & 0xFF00) |
                     ((u << 8) & 0xFF0000) |
                     ((u << 24) & 0xFF000000));
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        fprintf(stderr, "Converts mail file from big-endian to little-endian\n");
        return 1;
    }

    FILE *in = fopen(argv[1], "rb");
    if (!in) {
        perror("Failed to open input file");
        return 1;
    }

    FILE *out = fopen(argv[2], "wb");
    if (!out) {
        perror("Failed to open output file");
        fclose(in);
        return 1;
    }

    // Get file size
    fseek(in, 0, SEEK_END);
    long file_size = ftell(in);
    fseek(in, 0, SEEK_SET);

    if (file_size % BLOCK_SIZE != 0) {
        fprintf(stderr, "Warning: File size %ld is not a multiple of block size %d\n",
                file_size, BLOCK_SIZE);
    }

    int block_count = 0;
    int header_count = 0;
    int data_count = 0;
    int deleted_count = 0;

    // Process each block
    union {
        header_block_type header;
        data_block_type data;
        char raw[BLOCK_SIZE];
    } block;

    while (fread(&block, BLOCK_SIZE, 1, in) == 1) {
        block_count++;

        // Read the block_type (first 4 bytes) in big-endian
        int32_t block_type_be = block.header.block_type;
        int32_t block_type = bswap32(block_type_be);

        if (block_type == HEADER_BLOCK) {
            // Convert header block integer fields
            block.header.block_type = bswap32(block.header.block_type);
            block.header.next_block = bswap32(block.header.next_block);
            block.header.mail_time = bswap32(block.header.mail_time);
            header_count++;
        } else if (block_type == DELETED_BLOCK) {
            // Convert deleted block
            block.header.block_type = bswap32(block.header.block_type);
            block.header.next_block = bswap32(block.header.next_block);
            block.header.mail_time = bswap32(block.header.mail_time);
            deleted_count++;
        } else {
            // Data block (LAST_BLOCK or continuation pointer)
            block.data.block_type = bswap32(block.data.block_type);
            data_count++;
        }

        fwrite(&block, BLOCK_SIZE, 1, out);
    }

    fclose(in);
    fclose(out);

    printf("Conversion complete!\n");
    printf("  Total blocks: %d\n", block_count);
    printf("  Header blocks: %d\n", header_count);
    printf("  Data blocks: %d\n", data_count);
    printf("  Deleted blocks: %d\n", deleted_count);
    printf("\nOutput written to: %s\n", argv[2]);

    return 0;
}
