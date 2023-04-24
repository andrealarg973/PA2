#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>
#define BUFSIZE 1024
#define MAX_COMMANDS 15

struct command {
    char name[10];
    char input[20];
    char option[3];
    char redirection[2];
    char redirection_file[20];
    int isInput;
    int isOption;
    int isRedirection;
    int isBackground;
};

void copy(char* A, char* B) {
    //size_t size = sizeof(A[0]);
    int size = sizeof(A) / sizeof(A[0]);
    //printf("%ld\n", size);
    for(int i=0; i<size; i++) {
        printf("%c ", A[i]);
        B[i] = A[i];
    }
    printf("\n");
}

void print_command(struct command *command_list) {
    printf("Name: %s\n", command_list->name);
    printf("Input: %s\n", command_list->isInput == 1 ? command_list->input : "");
    printf("Option: %s\n", (command_list->isOption == 1 ? command_list->option : ""));
    if(command_list->isRedirection == 1) {
        printf("Redirection: %s\n", command_list->redirection);
        printf("Redirection file: %s\n", command_list->redirection_file);
    } else {
        printf("Redirection: -\n");
    }
    printf("Background: %d\n", command_list->isBackground);
    printf("----------------\n");
}

void parseCommand(char* command, FILE *parse_file, struct command *command_list) {
    char *token;
    command_list->isInput=0;
    command_list->isOption=0;
    command_list->isRedirection=0;
    command_list->isBackground=0;
    //char *name,*input,*option,*redirection,*redirection_file,*background;
    //int isInput=0,isOption=0,isRedirection=0,isBackground=0;

    fprintf(parse_file,"----------\n");

    token = strtok(command, " ");
    
    int count=0;
    // divide the command in tokens
    while(token != NULL) {
        if (count == 0) { // command name
            //copy(token, command_list->name);
            //command_list->name = *token;
            strcpy(command_list->name, token);
            //fprintf(parse_file,"Command: %s\n",token);
            //printf("Command: %s\n", token);
        }
        if (count == 1 && token[0] != '-') { // input
            //copy(token, command_list->input);
            strcpy(command_list->input, token);
            //command_list->input = token;
            command_list->isInput=1;
            //printf("ciao\n");
            //fprintf(parse_file,"Inputs: %s\n",token);
            //printf("Inputs: %s\n", token);
        }
        if (token[0] == '-') { // option
            //copy(token, command_list->option);
            strcpy(command_list->option, token);
            //printf("option: %s\n", command_list->option);
            //command_list->option = token;
            command_list->isOption=1;
            //fprintf(parse_file,"Options: %s\n",token);
            //printf("Options: %s\n", token);
        }
        if (token[0] == '>' || token[0] == '<') { // redirection
            //copy(token, command_list->redirection);
            strcpy(command_list->redirection, token);
            //printf("redirection: %s\n", command_list->redirection);
            //command_list->redirection = token;
            command_list->isRedirection=1;
            //fprintf(parse_file,"Redirection: %s\n",token);
            //printf("Redirection: %s\n", token);
            token = strtok(NULL, " "); // now the token must be the redirection file
            //copy(token, command_list->redirection_file);
            strcpy(command_list->redirection_file, token);
            //printf("red_file: %s\n", command_list->redirection_file);
            //command_list->redirection_file = token;
            //fprintf(parse_file,"Redirection-file: %s\n",token);
            //printf("Redirection-file: %s\n", token);
        }
        if (token[0] == '&') { // background
            //background = token;
            command_list->isBackground=1;
            //fprintf(parse_file,"Background: y\n");
            //printf("Background: y\n");
        }
        /*
         else if (count == 1) {
            if(token[0] != '-') {
                fprintf(parse_file,"Inputs: %s\n",token);
                printf("Inputs: %s\n", token);
            } else {
                fprintf(parse_file,"Options: %s\n",token);
                printf("Options: %s\n", token);
            }
        } else if (count == 2) {
            if(token[0] == '-') {
                fprintf(parse_file,"Options: %s\n",token);
                printf("Options: %s\n", token);
            } else if (token[0] == '>' || token[0] == '<') {

            }
        }*/
        //printf("count: %d, %s\n", count, command_list->option);
        count++;
        token = strtok(NULL, " ");
    }

    
    // FIX EVERYTHING
    fprintf(parse_file,"Command: %s\n",command_list->name);
    //printf("%s\n", command_list->name);
    //command_list->name = name;

    if(command_list->isInput) {
        fprintf(parse_file,"Inputs: %s\n",command_list->input);
        //command_list->input= input;
        //command_list->isInput = 1;
    } else {
        fprintf(parse_file,"Inputs: \n");
    }
    if(command_list->isOption) {
        fprintf(parse_file,"Option: %s\n",command_list->option);
        //command_list->option = option;
        //command_list->isOption = 1;
    } else {
        fprintf(parse_file,"Option: \n");
    }
    if(command_list->isRedirection) {
        fprintf(parse_file,"Redirection: %s\n",command_list->redirection);
        //command_list->redirection = redirection;
        //command_list->isRedirection = 1;
        //command_list->redirection_file = redirection_file;
    } else {
        fprintf(parse_file,"Redirection: -\n");
    }
    if(command_list->isBackground) {
        fprintf(parse_file,"Background: y\n");
        //command_list->isBackground = 1;
    } else {
        fprintf(parse_file,"Background: n\n");
    }

    fprintf(parse_file,"----------\n");
    //fclose(parse_file);
    //print_command(command_list);
    
}

int main(int argc, char *argv[]) {
    FILE *commands_file, *parse_file;
    char buffer[BUFSIZE];
    int n;
    char *token;
    struct command command_list[MAX_COMMANDS];

    commands_file = fopen("commands.txt", "r");
    parse_file = fopen("parse.txt", "w");

    int command_count=0;
    // read the file line by line and parse each command
    while (fgets(buffer, BUFSIZE, commands_file) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0; // remove newline character from the command
        //printf("%s\n",buffer);
        //printf("count: %d\n", command_count);
        parseCommand(buffer, parse_file, &command_list[command_count]);
        print_command(&command_list[command_count]);
        //print_command(&command_list[0]);
        command_count++;

    }

    //print_command(&command_list[0]);
    fclose(commands_file);
    fclose(parse_file);

    return 0;
}