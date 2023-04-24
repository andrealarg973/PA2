#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>
#include <pthread.h>
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

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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

    fprintf(parse_file,"----------\n");

    token = strtok(command, " ");
    
    int count=0;
    // divide the command in tokens
    while(token != NULL) {
        if (count == 0) { // command name
            strcpy(command_list->name, token);
        }
        if (count == 1 && token[0] != '-') { // input
            strcpy(command_list->input, token);
            command_list->isInput=1;
        }
        if (token[0] == '-') { // option
            strcpy(command_list->option, token);
            command_list->isOption=1;
        }
        if (token[0] == '>' || token[0] == '<') { // redirection
            strcpy(command_list->redirection, token);
            command_list->isRedirection=1;
            token = strtok(NULL, " "); // now the token must be the redirection file
            strcpy(command_list->redirection_file, token);
        }
        if (token[0] == '&') { // background
            command_list->isBackground=1;
        }
        count++;
        token = strtok(NULL, " ");
    }

    // FIX EVERYTHING
    fprintf(parse_file,"Command: %s\n",command_list->name);

    if(command_list->isInput) {
        fprintf(parse_file,"Inputs: %s\n",command_list->input);
    } else {
        fprintf(parse_file,"Inputs: \n");
    }
    if(command_list->isOption) {
        fprintf(parse_file,"Option: %s\n",command_list->option);
    } else {
        fprintf(parse_file,"Option: \n");
    }
    if(command_list->isRedirection) {
        fprintf(parse_file,"Redirection: %s\n",command_list->redirection);
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
    //print_command(command_list);
}

void insert_parameters(char **myargs, struct command *command) {
    // 1.1.4 basic execution 
    if(command->isBackground == 0) {    //  && command->isRedirection == 0
        // can just call execvp and execute
        myargs[0] = strdup(command->name);
        if(command->isInput == 1) {
            myargs[1] = strdup(command->input);
            if(command->isOption == 1) {
                myargs[2] = strdup(command->option);
                myargs[3] = NULL;
            } else {
                myargs[2] = NULL;
            }
        } else {
            if(command->isOption == 1) {
                myargs[1] = strdup(command->option);
                myargs[2] = NULL;
            } else {
                myargs[1] = NULL;
            }
        }
    }
}

void* thread_func(int fd) {

    return NULL;
}

int main(int argc, char *argv[]) {
    FILE *commands_file, *parse_file;
    char buffer[BUFSIZE];
    int n;
    char *token;
    struct command command_list[MAX_COMMANDS];
    pthread_t threads[MAX_COMMANDS];
    // fd[0]: read
    // fd[1]: write
    int fd[2];
    pipe(fd);
    int stdin_fd = dup(STDIN_FILENO);
    int stdout_fd = dup(STDOUT_FILENO);

    commands_file = fopen("commands2.txt", "r");
    parse_file = fopen("parse.txt", "w");

    int command_count=0;
    int rc;
    // read the file line by line and parse each command
    while (fgets(buffer, BUFSIZE, commands_file) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0; // remove newline character from the command

        // parse new command
        parseCommand(buffer, parse_file, &command_list[command_count]);
        //print_command(&command_list[command_count]);
        // create new process for the command
        rc = fork();

        if (rc < 0) {
            // fork failed; exit
            fprintf(stderr, "fork failed\n");
            exit(1);
        } else if (rc == 0) { // command execution
            // 1.1.5 implementing redirectioning
            if (command_list[command_count].isRedirection == 1) {
                // wc commands.txt > output.txt
                //printf("red_file: %s\n", command_list[command_count].redirection_file);
                //printf("CHAR: %c\n", command_list[command_count].redirection[0]);
                if (command_list[command_count].redirection[0] == '>') {
                    //printf("Redirection: >, %s\n", command_list[command_count].redirection_file);
                    int new_stdout = open(command_list[command_count].redirection_file, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU); // open output file
                    dup2(new_stdout, STDOUT_FILENO); // redirect output to file
                    //close(new_stdout);
                } else if (command_list[command_count].redirection[0] == '<'){
                    //printf("Redirection: <, %s\n", command_list[command_count].redirection_file);
                    int new_stdin = open(command_list[command_count].redirection_file, O_RDONLY); // open input file
                    dup2(new_stdin, STDIN_FILENO); // redirect input to file
                    // redirect output to fd

                    //close(new_stdin);
                }
            } 
            /*
            if ((command_list[command_count].isRedirection == 1 && 
                command_list[command_count].redirection[0] == '<') ||
                command_list[command_count].isRedirection == 0) { // no output redirection, synchronization required
                //dup2(fd[1], STDOUT_FILENO); // close STDOUT and redirect to fd
                printf("Solo input\n");

            }*/
            char *myargs[10];
            insert_parameters(myargs, &command_list[command_count]);
            //printf("Args: %s\n", command_list[command_count].redirection_file);
            execvp(myargs[0], myargs);
        } else { // shell
            if ((command_list[command_count].isRedirection == 1 && 
                command_list[command_count].redirection[0] == '<') ||
                command_list[command_count].isRedirection == 0) { 
                // if command has no output redirection

                //pthread_create(&threads[command_count], NULL, thread_func, (void *) &fd[1]);


            }

            // if it isn't a background command, shell wait for child to finish execution
            if (command_list[command_count].isBackground == 0) {
                wait(NULL);
            }
            //printf("PADRE\n");
        }
        /*
            if(command_list[command_count].isBackground == 0 && command_list[command_count].isRedirection == 0) {
                // can just call execvp and execute
                char *myargs[3];
                myargs[0] = strdup(&command_list->name[command_count]);
                if(command_list[command_count].isInput == 1) {
                    myargs[1] = strdup(&command_list->input[command_count]);
                    if(command_list[command_count].isOption == 1) {
                        myargs[2] = strdup(&command_list->option[command_count]);
                        myargs[3] = NULL;
                    } else {
                        myargs[2] = NULL;
                    }
                } else {
                    if(command_list[command_count].isOption == 1) {
                        myargs[1] = strdup(&command_list->option[command_count]);
                        myargs[2] = NULL;
                    } else {
                        myargs[1] = NULL;
                    }
                }

                execvp(myargs[0], myargs);
            }*/



        command_count++;

    }
    printf("\nfine\n");
    // restore default STDIN and STDOUT
    /*if (command_list[command_count].isRedirection == 1) {
        int stdin_fd = dup(STDIN_FILENO);
        int stdout_fd = dup(STDOUT_FILENO);

        dup2(stdin_fd, STDIN_FILENO);
        dup2(stdout_fd, STDOUT_FILENO);
        printf("Restored!\n");
    }*/

    fclose(commands_file);
    fclose(parse_file);

    return 0;
}
