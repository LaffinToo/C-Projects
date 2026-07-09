/**
 * @file main.c
 * @brief Program to encode and decode text files in CodeExchange format.
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
    fprintf(output, "    - '\\n': Newline\n    - '\\t': Tab\n    - '\\r': Carriage return\n    - '\\\\': Backslash\n\n");

    for (size_t i = 0; i < num_files; i++) {
        encode_file(output, file_paths[i], line_length);
    }

    fprintf(output, "===== EOF\n");
}

void decode_file(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        perror("Error opening file for decoding");
        exit(EXIT_FAILURE);
    }

    char encoded_content[MAX_LINE_LENGTH * 2]; // Increased to handle line overflows safely
    while (fgets(encoded_content, sizeof(encoded_content), file) != NULL) {
        if (strncmp(encoded_content, "===== FILE:", 11) == 0) {
            printf("Decoded Content from %s:\n", file_path);
            
            while (fgets(encoded_content, sizeof(encoded_content), file) != NULL) {
                if (strncmp(encoded_content, "=====", 5) == 0) {
                    break; 
                }

                size_t length = strlen(encoded_content);
                if (length > 0 && encoded_content[length - 1] == '\n') {
                    encoded_content[length - 1] = '\0';
                    length--;
                }

                for (size_t i = 0; i < length; i++) {
                    if (encoded_content[i] == '\\' && (i + 1) < length) {
                        switch (encoded_content[i + 1]) {
                            case 'n':  putchar('\n'); break;
                            case 't':  putchar('\t'); break;
                            case 'r':  putchar('\r'); break;
                            case '\\': putchar('\\'); break;
                            default:   putchar(encoded_content[i]); putchar(encoded_content[i+1]); break;
                        }
                        i++; // Skip the escaped character identifier
                    } else {
                        putchar(encoded_content[i]);
                    }
                }
            }
            printf("\n");
        }
    }
    fclose(file);
}

void display_usage(const char *program_name) {
    printf("Usage:\n");
    printf("  To encode: %s [-l line_length] -e <output_file> <file1> <file2> ...\n", program_name);
    printf("  To decode: %s -d <archive_file>\n", program_name);
}

int main(int argc, char *argv[]) {
    size_t line_length = MAX_LINE_LENGTH;
    int curarg = 1;

    if (argc < 3) {
        display_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Parse line length option if present
    if (strcmp(argv[curarg], "-l") == 0) {
        if (curarg + 1 >= argc || !isdigit((unsigned char)argv[curarg + 1][0])) {
            display_usage(argv[0]);
            return EXIT_FAILURE;
        }
        line_length = (size_t)atoi(argv[curarg + 1]);
        curarg += 2;
    }

    if (curarg >= argc) {
        display_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Parse Action
    if (strcmp(argv[curarg], "-e") == 0) {
        if (argc < curarg + 3) { // Needs -e, output_file, and at least 1 input file
            display_usage(argv[0]);
            return EXIT_FAILURE;
        }
        const char *out_filename = argv[curarg + 1];
        printf("Output file: %s\n", out_filename);
        
        FILE *output = fopen(out_filename, "w");
        if (!output) {
            perror("Error opening output file");
            return EXIT_FAILURE;
        }

        encode_files(output, argv + curarg + 2, argc - curarg - 2, line_length);
        fclose(output);
    } else if (strcmp(argv[curarg], "-d") == 0) {
        if (curarg + 1 >= argc) {
            display_usage(argv[0]);
            return EXIT_FAILURE;
        }
        decode_file(argv[curarg + 1]); // Fixed passing the filename instead of "-d"
    } else {
        fprintf(stderr, "Invalid option. Use -e for encoding or -d for decoding.\n");
        display_usage(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
