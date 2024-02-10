/**
 * @file main.c
 * @brief Program to encode and decode text files in CodeExchange format.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 80

/**
 * @brief Escapes special characters and prints them to the output file.
 * @param output Output file stream.
 * @param ch Character to be escaped and printed.
 */
void escape_and_print_char(FILE *output, char ch) {
    switch (ch) {
        case '\n':
            fprintf(output, "\\n");
            break;
        case '\t':
            fprintf(output, "\\t");
            break;
        case '\r':
            fprintf(output, "\\r");
            break;
        case '\\':
            fprintf(output, "\\\\");
            break;
        default:
            fputc(ch, output);
            break;
    }
}

/**
 * @brief Encodes a file and writes the encoded content to the output file.
 * @param output Output file stream.
 * @param file_path Path to the file to be encoded.
 * @param line_length Maximum line length for encoded lines.
 */
void encode_file(FILE *output, const char *file_path, size_t line_length) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fprintf(output, "===== FILE: %s - LL:%zu ====\n", file_path, line_length);

    int ch;
    size_t current_length = 0;
    while ((ch = fgetc(file)) != EOF) {
        escape_and_print_char(output, (char)ch);
        current_length++;

        if (line_length > 0 && current_length >= line_length) {
            fprintf(output, "\n");
            current_length = 0;
        }
    }

    fprintf(output, "\n");
    fclose(file);
}

/**
 * @brief Encodes multiple files and writes the encoded content to the output file.
 * @param output Output file stream.
 * @param file_paths Array of file paths to be encoded.
 * @param num_files Number of files to be encoded.
 * @param line_length Maximum line length for encoded lines.
 */
void encode_files(FILE *output, char *file_paths[], size_t num_files, size_t line_length) {
    fprintf(output, "CodeExchange format - xc format\n");
    fprintf(output, "The following archive consists of various text files in a compressed format suitable for text chat exchange.\n");
    fprintf(output, "Each file is individually encoded, with the header naming filename and line length of the escaped encoded file.\n");

    fprintf(output, "\nTo manually decode the files, follow these steps:\n");
    fprintf(output, "1. Open the encoded archive.\n");
    fprintf(output, "2. Identify the file you want to decode by looking at the header, which includes the filename and line length.\n");
    fprintf(output, "3. Manually expand the escaped characters using the following guide:\n");
    fprintf(output, "    - '\\n': Newline\n");
    fprintf(output, "    - '\\t': Tab\n");
    fprintf(output, "    - '\\r': Carriage return\n");
    fprintf(output, "    - '\\': Backslash\n");
    fprintf(output, "    - '\\z=n;': Repeat the previous character 'n' times\n\n");

    for (size_t i = 0; i < num_files; i++) {
        encode_file(output, file_paths[i], line_length);
    }

    fprintf(output, "===== EOF\n", line_length);
}

/**
 * @brief Decodes an encoded file and prints the content to the console.
 * @param file_path Path to the encoded file to be decoded.
 */
void decode_file(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        perror("Error opening file for decoding");
        exit(EXIT_FAILURE);
    }

    char encoded_content[MAX_LINE_LENGTH];
    while (fgets(encoded_content, sizeof(encoded_content), file) != NULL) {
        if (strncmp(encoded_content, "===== FILE:", 11) == 0) {
            printf("Decoded Content from %s:\n", file_path);
            while (fgets(encoded_content, sizeof(encoded_content), file) != NULL && strncmp(encoded_content, "=====", 5) != 0) {
                size_t length = strlen(encoded_content);

                if (length > 0 && encoded_content[length - 1] == '\n') {
                    encoded_content[length - 1] = '\0';
                }

                for (size_t i = 0; i < length; i++) {
                    if (encoded_content[i] == '\\' && i + 1 < length) {
                        switch (encoded_content[i + 1]) {
                            case 'n':
                            case 't':
                            case 'r':
                            case '\\':
                                putchar(encoded_content[i + 1]);
                                break;
                            default:
                                putchar(encoded_content[i]);
                                break;
                        }
                        i++;
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

/**
 * @brief Displays program usage information.
 * @param program_name Name of the program.
 */
void display_usage(const char *program_name) {
    printf("Usage:\n");
    printf("To encode: %s -e file1.c file2.h file3.x ... [-l line_length]\n", program_name);
    printf("To decode: %s -d file.xxx\n", program_name);
}

/**
 * @brief Main function, the entry point of the program.
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return Program exit status.
 */
int main(int argc, char *argv[]) {
    size_t line_length = MAX_LINE_LENGTH; // Default line length
    int curarg = 1;

    if (argc < 3) {
        display_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[curarg], "-l") == 0 && curarg + 1 < argc) {
        if (!isdigit(argv[curarg + 1][0])) {
            display_usage(argv[0]);
            return EXIT_FAILURE;
        }
        line_length = atoi(argv[curarg + 1]);
        curarg += 2; // Skip the next argument (line length value)
    }

    if (strcmp(argv[curarg], "-e") == 0) {
        if (argc < curarg + 3) {
            fprintf(stderr, "Usage:\n");
            fprintf(stderr, "To encode: %s [-l line_length] -e output_file file1.c ...\n", argv[0]);
            return EXIT_FAILURE;
        }
        printf("Output file: %s\n", argv[curarg + 1]);
        FILE *output = fopen(argv[curarg + 1], "w");
        if (!output) {
            perror("Error opening output file");
            return EXIT_FAILURE;
        }

        encode_files(output, argv + curarg + 2, argc - curarg - 2, line_length);
        fclose(output);
    } else if (strcmp(argv[curarg], "-d") == 0) {
        decode_file(argv[curarg]);
    } else {
        fprintf(stderr, "Invalid option. Use -e for encoding or -d for decoding.\n");
        return EXIT_FAILURE;
    }

    return 0;
}
