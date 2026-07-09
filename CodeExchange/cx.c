/**
 * @file cx.c
 * @brief Program to encode and decode text files in CodeExchange format.
 *        Supports dynamic dual-mode hex RLE, line validation, and global file checks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#define MAX_LINE_LENGTH 80

// Standard 1-byte XOR calculation engine used for both line and file verification
unsigned char calculate_checksum(const char *str, size_t len) {
    unsigned char crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= (unsigned char)str[i];
    }
    return crc;
}

// Appends formatted data directly to a trailing buffer array
void append_to_line_buffer(char *buffer, size_t *buf_len, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(buffer + *buf_len, 1024 - *buf_len, fmt, args);
    if (written > 0) {
        *buf_len += written;
    }
    va_end(args);
}

// Emits runs dynamically using /x (byte hex) or /z (word hex), bounded by line delimiters
void flush_run_to_buffer(char *buffer, size_t *buf_len, char ch, size_t run_count, FILE *output, size_t line_length) {
    if (run_count == 0) return;

    size_t normal_width = (ch == '\n' || ch == '\t' || ch == '\r' || ch == '\\') ? 2 : 1;

    while (run_count > 0) {
        size_t chunk_count = (run_count > 65535) ? 65535 : run_count;
        size_t raw_cost = normal_width * chunk_count;
        size_t rle_cost = 0;
        int use_word_mode = 0;

        if (chunk_count > 255) {
            rle_cost = 2 + 4 + normal_width; // /zXXXXc
            use_word_mode = 1;
        } else {
            rle_cost = 2 + 2 + normal_width; // /xXXc
        }

        // Reserve 4 bytes for the structural suffix format: "|XX"
        size_t max_allowed_width = (line_length > 4) ? (line_length - 4) : line_length;

        if (line_length > 0 && (*buf_len + rle_cost > max_allowed_width)) {
            if (*buf_len > 0) {
                unsigned char crc = calculate_checksum(buffer, *buf_len);
                fprintf(output, "%s|%02X\n", buffer, crc);
                *buf_len = 0;
                continue;
            }
        }

        if (rle_cost < raw_cost) {
            if (use_word_mode) {
                append_to_line_buffer(buffer, buf_len, "/z%04X", (unsigned int)chunk_count);
            } else {
                append_to_line_buffer(buffer, buf_len, "/x%02X", (unsigned int)chunk_count);
            }
            
            if (ch == '\n') append_to_line_buffer(buffer, buf_len, "\\n");
            else if (ch == '\t') append_to_line_buffer(buffer, buf_len, "\\t");
            else if (ch == '\r') append_to_line_buffer(buffer, buf_len, "\\r");
            else if (ch == '\\') append_to_line_buffer(buffer, buf_len, "\\\\");
            else append_to_line_buffer(buffer, buf_len, "%c", ch);

            run_count -= chunk_count;
        } else {
            size_t process_now = chunk_count;
            if (line_length > 0) {
                size_t space_left = (max_allowed_width > *buf_len) ? (max_allowed_width - *buf_len) : 0;
                size_t max_raw_chars = space_left / normal_width;
                if (max_raw_chars == 0) {
                    unsigned char crc = calculate_checksum(buffer, *buf_len);
                    fprintf(output, "%s|%02X\n", buffer, crc);
                    *buf_len = 0;
                    continue;
                }
                if (process_now > max_raw_chars) process_now = max_raw_chars;
            }

            for (size_t i = 0; i < process_now; i++) {
                if (ch == '\n') append_to_line_buffer(buffer, buf_len, "\\n");
                else if (ch == '\t') append_to_line_buffer(buffer, buf_len, "\\t");
                else if (ch == '\r') append_to_line_buffer(buffer, buf_len, "\\r");
                else if (ch == '\\') append_to_line_buffer(buffer, buf_len, "\\\\");
                else append_to_line_buffer(buffer, buf_len, "%c", ch);
            }
            run_count -= process_now;
        }

        if (line_length > 0 && *buf_len >= max_allowed_width) {
            unsigned char crc = calculate_checksum(buffer, *buf_len);
            fprintf(output, "%s|%02X\n", buffer, crc);
            *buf_len = 0;
        }
    }
}

void encode_file(FILE *output, const char *file_path, size_t line_length) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    // Pass 1: Compute full file global checksum over raw source text
    unsigned char global_file_checksum = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        global_file_checksum ^= (unsigned char)ch;
    }
    rewind(file);

    // Write top header metadata frame with the file checksum (FCS)
    fprintf(output, "===== FILE: %s - LL:%zu - FCS:%02X ====\n", file_path, line_length, global_file_checksum);

    char buffer[1024] = {0};
    size_t buf_len = 0;
    int current_ch = EOF;
    size_t run_count = 0;

    while ((ch = fgetc(file)) != EOF) {
        if (run_count == 0) {
            current_ch = ch;
            run_count = 1;
        } else if (ch == current_ch) {
            run_count++;
        } else {
            flush_run_to_buffer(buffer, &buf_len, (char)current_ch, run_count, output, line_length);
            current_ch = ch;
            run_count = 1;
        }
    }
    
    if (run_count > 0) {
        flush_run_to_buffer(buffer, &buf_len, (char)current_ch, run_count, output, line_length);
    }

    if (buf_len > 0) {
        unsigned char crc = calculate_checksum(buffer, buf_len);
        fprintf(output, "%s|%02X\n", buffer, crc);
    }

    fclose(file);
}

void encode_files(FILE *output, char *file_paths[], size_t num_files, size_t line_length) {
    fprintf(output, "CodeExchange format - xc format v2 (Dual-RLE & Line-Validated)\n\n");
    
    for (size_t i = 0; i < num_files; i++) {
        encode_file(output, file_paths[i], line_length);
    }
    fprintf(output, "===== EOF\n");
}

void decode_archive(FILE *input) {
    char line[512];
    int archive_started = 0;
    FILE *current_output = NULL;

    unsigned char running_file_checksum = 0;
    unsigned char expected_file_checksum = 0;
    int has_active_checksum_target = 0;
    char last_decoded_file[256] = {0};

    while (fgets(line, sizeof(line), input)) {
        if (!archive_started) {
            if (strstr(line, "CodeExchange format") != NULL) {
                archive_started = 1;
            }
            continue;
        }

        if (strncmp(line, "===== FILE:", 11) == 0 || strncmp(line, "===== EOF", 9) == 0) {
            // Verify file global hash block before shifting file resource pointers
            if (current_output && has_active_checksum_target) {
                if (running_file_checksum != expected_file_checksum) {
                    fprintf(stderr, "CRITICAL ERROR: Global checksum failed for '%s'! (Header: %02X, Generated: %02X). Data is corrupt!\n", 
                            last_decoded_file, expected_file_checksum, running_file_checksum);
                } else {
                    printf("SUCCESS: Verified integrity of '%s' completely.\n", last_decoded_file);
                }
            }

            if (strncmp(line, "===== EOF", 9) == 0) {
                if (current_output) { fclose(current_output); current_output = NULL; }
                break;
            }

            if (current_output) { fclose(current_output); }
            char file_path[256];
            size_t line_length = 0;
            unsigned int fcs_val = 0;

            if (sscanf(line, "===== FILE: %255s - LL:%zu - FCS:%02X ====", file_path, &line_length, &fcs_val) == 3) {
                strncpy(last_decoded_file, file_path, sizeof(last_decoded_file));
                expected_file_checksum = (unsigned char)fcs_val;
                running_file_checksum = 0;
                has_active_checksum_target = 1;
                current_output = fopen(file_path, "w");
                if (!current_output) {
                    perror("Failed to create output file");
                }
            }
            continue;
        }

        if (current_output) {
            size_t len = strlen(line);
            if (len > 0 && line[len - 1] == '\n') {
                line[len - 1] = '\0';
                len--;
            }

            // Split and verify line context hash
            char *separator_ptr = strrchr(line, '|');
            if (separator_ptr != NULL) {
                char *crc_ptr = separator_ptr + 1;
                *separator_ptr = '\0';
                size_t content_len = separator_ptr - line;

                unsigned char expected_crc = (unsigned char)strtoul(crc_ptr, NULL, 16);
                unsigned char actual_crc = calculate_checksum(line, content_len);

                if (actual_crc != expected_crc) {
                    fprintf(stderr, "LINE ERROR: Row hash validation failed! (Expected %02X, Got %02X). Transmission is damaged.\n", expected_crc, actual_crc);
                }
                len = content_len;
            }

            for (size_t i = 0; i < len; i++) {
                unsigned int count = 0;
                int parsed_rle = 0;

                // Evaluate Type 1 RLE: /xXX (Byte mode)
                if (line[i] == '/' && i + 3 < len && line[i+1] == 'x' && isxdigit((unsigned char)line[i+2]) && isxdigit((unsigned char)line[i+3])) {
                    char hex_str[3] = { line[i+2], line[i+3], '\0' };
                    count = (unsigned int)strtoul(hex_str, NULL, 16);
                    i += 4;
                    parsed_rle = 1;
                }
                // Evaluate Type 2 RLE: /zXXXX (Word mode)
                else if (line[i] == '/' && i + 5 < len && line[i+1] == 'z' && 
                         isxdigit((unsigned char)line[i+2]) && isxdigit((unsigned char)line[i+3]) && 
                         isxdigit((unsigned char)line[i+4]) && isxdigit((unsigned char)line[i+5])) {
                    char hex_str[5] = { line[i+2], line[i+3], line[i+4], line[i+5], '\0' };
                    count = (unsigned int)strtoul(hex_str, NULL, 16);
                    i += 6;
                    parsed_rle = 1;
                }

                if (parsed_rle) {
                    char target = line[i];
                    if (target == '\\' && i + 1 < len) {
                        switch (line[i+1]) {
                            case 'n': target = '\n'; break;
                            case 't': target = '\t'; break;
                            case 'r': target = '\r'; break;
                            case '\\': target = '\\'; break;
                        }
                        i++;
                    }
                    for (unsigned int c = 0; c < count; c++) {
                        fputc(target, current_output);
                        running_file_checksum ^= (unsigned char)target;
                    }
                } 
                else if (line[i] == '\\' && i + 1 < len) {
                    char target = line[i+1];
                    switch (line[i+1]) {
                        case 'n': target = '\n'; break;
                        case 't': target = '\t'; break;
                        case 'r': target = '\r'; break;
                        case '\\': target = '\\'; break;
                    }
                    fputc(target, current_output);
                    running_file_checksum ^= (unsigned char)target;
                    i++;
                } else {
                    fputc(line[i], current_output);
                    running_file_checksum ^= (unsigned char)line[i];
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage:\n  Encode: %s -e [-l line_length] <file1> [file2 ...]\n  Decode: %s -d <archive_file>\n", argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-e") == 0) {
        size_t line_length = MAX_LINE_LENGTH;
        int file_idx = 2;
        if (argc > 3 && strcmp(argv[2], "-l") == 0) {
            line_length = (size_t)strtoul(argv[3], NULL, 10);
            file_idx = 4;
        }
        if (file_idx >= argc) {
            fprintf(stderr, "Error: No input files specified for encoding.\n");
            return EXIT_FAILURE;
        }
        encode_files(stdout, &argv[file_idx], argc - file_idx, line_length);
    } else if (strcmp(argv[1], "-d") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No archive file specified for decoding.\n");
            return EXIT_FAILURE;
        }
        FILE *archive = fopen(argv[2], "r");
        if (!archive) {
            perror("Error opening archive file");
            return EXIT_FAILURE;
        }
        decode_archive(archive);
        fclose(archive);
    } else {
        fprintf(stderr, "Invalid option: %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
