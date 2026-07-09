/**
 * @file cx.c
 * @brief Program to encode and decode text files in CodeExchange format.
 *        Supports reading polyglot files containing self-extracting shell scripts.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 80

void escape_and_print_char(FILE *output, char ch) {
    switch (ch) {
        case '\n': fprintf(output, "\\n"); break;
        case '\t': fprintf(output, "\\t"); break;
        case '\r': fprintf(output, "\\r"); break;
        case '\\': fprintf(output, "\\\\"); break;
        default:   fputc(ch, output); break;
    }
}

void encode_file(FILE *output, const char *file_path, size_t line_length) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        perror("Error opening file");
        return; // Don't crash entire program if one file fails
    }

    fprintf(output, "===== FILE: %s - LL:%zu ====\n", file_path, line_length);

    int ch;
    size_t current_length = 0;
    while ((ch = fgetc(file)) != EOF) {
        // Approximate tracking: an escaped char prints 2 chars, others print 1
        size_t written = (ch == '\n' || ch == '\t' || ch == '\r' || ch == '\\') ? 2 : 1;
        escape_and_print_char(output, (char)ch);
        current_length += written;

        if (line_length > 0 && current_length >= line_length) {
            fprintf(output, "\n");
            current_length = 0;
        }
    }
    fprintf(output, "\n");
    fclose(file);
}

void encode_files(FILE *output, char *file_paths[], size_t num_files, size_t line_length) {
    fprintf(output, "CodeExchange format - xc format\n");
    fprintf(output, "The following archive consists of various text files in a compressed format suitable for text chat exchange.\n");
    fprintf(output, "\nTo manually decode the files, follow these steps:\n");
    fprintf(output, "    - '\\n': Newline\n");
    fprintf(output, "    - '\\t': Tab\n");
    fprintf(output, "    - '\\r': Carriage return\n");
    fprintf(output, "    - '\\\\': Backslash\n\n");

    for (size_t i = 0; i < num_files; i++) {
        encode_file(output, file_paths[i], line_length);
    }
    fprintf(output, "===== EOF\n");
}

void decode_archive(FILE *input) {
    char line[512];
    int archive_started = 0;
    FILE *current_output = NULL;

    while (fgets(line, sizeof(line), input)) {
        // 1. Skip everything until the official signature is detected
        if (!archive_started) {
            if (strstr(line, "CodeExchange format") != NULL) {
                archive_started = 1;
            }
            continue; 
        }

        // 2. Check for the end of the archive
        if (strncmp(line, "===== EOF", 9) == 0) {
            if (current_output) {
                fclose(current_output);
                current_output = NULL;
            }
            break;
        }

        // 3. Check for a new file block header
        if (strncmp(line, "===== FILE:", 11) == 0) {
            if (current_output) {
                fclose(current_output);
            }

            char file_path[256];
            size_t line_length = 0;
            
            // Extract the filename and line length rule
            if (sscanf(line, "===== FILE: %255s - LL:%zu ====", file_path, &line_length) == 2) {
                current_output = fopen(file_path, "w");
                if (!current_output) {
                    perror("Failed to create output file");
                }
            }
            continue;
        }

        // 4. Decode content lines into the active file stream
        if (current_output) {
            size_t len = strlen(line);
            // Strip trailing newlines added by formatting/wrapping
            if (len > 0 && line[len - 1] == '\n') {
                line[len - 1] = '\0';
                len--;
            }

            for (size_t i = 0; i < len; i++) {
                if (line[i] == '\\' && i + 1 < len) {
                    switch (line[i + 1]) {
                        case 'n': fputc('\n', current_output); break;
                        case 't': fputc('\t', current_output); break;
                        case 'r': fputc('\r', current_output); break;
                        case '\\': fputc('\\', current_output); break;
                        default:
                            // Fallback if it wasn't a standard escape sequence
                            fputc(line[i], current_output);
                            fputc(line[i + 1], current_output);
                            break;
                    }
                    i++; // Skip past the escape target character
                } else {
                    fputc(line[i], current_output);
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
