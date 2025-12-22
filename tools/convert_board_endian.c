/*
 * convert_board_endian.c - Convert board message files from big-endian to little-endian
 *
 * The original board files were created on a Sun SPARC (big-endian) system.
 * This tool converts them to little-endian format for modern x86 systems.
 *
 * File format:
 *   int msg_num (number of messages)
 *   For each message:
 *     int header_len
 *     char[header_len] header
 *     int body_len
 *     char[body_len] body
 *
 * Usage: convert_board_endian <input_file> <output_file>
 *
 * Build: clang -o convert_board_endian convert_board_endian.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
        fprintf(stderr, "Converts board message file from big-endian to little-endian\n");
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

    // Read message count (big-endian)
    int32_t msg_num_be;
    if (fread(&msg_num_be, sizeof(int32_t), 1, in) != 1) {
        fprintf(stderr, "Failed to read message count\n");
        fclose(in);
        fclose(out);
        return 1;
    }

    int32_t msg_num = bswap32(msg_num_be);
    printf("Message count: %d (from big-endian 0x%08X)\n", msg_num, (uint32_t)msg_num_be);

    if (msg_num < 0 || msg_num > 1000) {
        fprintf(stderr, "Invalid message count %d - file may be corrupted\n", msg_num);
        fclose(in);
        fclose(out);
        return 1;
    }

    // Write message count (little-endian, which is native)
    fwrite(&msg_num, sizeof(int32_t), 1, out);

    // Process each message
    for (int i = 0; i < msg_num; i++) {
        // Read header length
        int32_t header_len_be;
        if (fread(&header_len_be, sizeof(int32_t), 1, in) != 1) {
            fprintf(stderr, "Failed to read header length for message %d\n", i + 1);
            break;
        }
        int32_t header_len = bswap32(header_len_be);

        if (header_len < 0 || header_len > 10000) {
            fprintf(stderr, "Invalid header length %d for message %d\n", header_len, i + 1);
            break;
        }

        // Write header length (little-endian)
        fwrite(&header_len, sizeof(int32_t), 1, out);

        // Copy header text
        char *header = malloc(header_len);
        if (!header) {
            fprintf(stderr, "Failed to allocate memory for header\n");
            break;
        }
        fread(header, 1, header_len, in);
        fwrite(header, 1, header_len, out);

        printf("  Message %d: %.60s%s\n", i + 1, header, header_len > 60 ? "..." : "");
        free(header);

        // Read body length
        int32_t body_len_be;
        if (fread(&body_len_be, sizeof(int32_t), 1, in) != 1) {
            fprintf(stderr, "Failed to read body length for message %d\n", i + 1);
            break;
        }
        int32_t body_len = bswap32(body_len_be);

        if (body_len < 0 || body_len > 100000) {
            fprintf(stderr, "Invalid body length %d for message %d\n", body_len, i + 1);
            break;
        }

        // Write body length (little-endian)
        fwrite(&body_len, sizeof(int32_t), 1, out);

        // Copy body text
        char *body = malloc(body_len);
        if (!body) {
            fprintf(stderr, "Failed to allocate memory for body\n");
            break;
        }
        fread(body, 1, body_len, in);
        fwrite(body, 1, body_len, out);
        free(body);
    }

    fclose(in);
    fclose(out);

    printf("\nConversion complete! Output written to: %s\n", argv[2]);
    return 0;
}
