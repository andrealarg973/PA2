#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256

int main() {
    FILE *commands_file = fopen("commands.txt", "r");
    FILE *parse_file = fopen("parse.txt", "w");

    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, commands_file) != NULL) {
        // Remove trailing newline character
        char *newline = strchr(line, '\n');
        if (newline != NULL) {
            *newline = '\0';
        }

        // Parse command name, input, option, redirection, and background job
        char command[MAX_LINE_LENGTH] = "";
        char input[MAX_LINE_LENGTH] = "";
        char option[MAX_LINE_LENGTH] = "";
        char redirection = '-';
        char background_job = 'n';

        char *token = strtok(line, " ");
        if (token != NULL) {
            strncpy(command, token, MAX_LINE_LENGTH - 1);
        }

        while ((token = strtok(NULL, " ")) != NULL) {
            if (strcmp(token, ">") == 0 || strcmp(token, "<") == 0) {
                redirection = token[0];

                token = strtok(NULL, " ");
                if (token != NULL) {
                    strncpy(input, token, MAX_LINE_LENGTH - 1);
                }
            } else if (strcmp(token, "&") == 0) {
                background_job = 'y';
                break;
            } else {
                strncat(option, token, MAX_LINE_LENGTH - strlen(option) - 1);
                strncat(option, " ", MAX_LINE_LENGTH - strlen(option) - 1);
            }
        }

        // Write parsed information to parse file
        fprintf(parse_file, "----------\n");
        fprintf(parse_file, "Command : %s\n", command);
        if (strlen(input) > 0) {
            fprintf(parse_file, "Inputs : %s\n", input);
        }
        if (strlen(option) > 0) {
            fprintf(parse_file, "Options : %s\n", option);
        }
        fprintf(parse_file, "Redirection : %c\n", redirection);
        fprintf(parse_file, "Background Job : %c\n", background_job);
        fprintf(parse_file, "----------\n");
    }

    fclose(commands_file);
    fclose(parse_file);
    return 0;
}
