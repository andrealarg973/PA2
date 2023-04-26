#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/select.h>
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
        fprintf(parse_file,"Options: %s\n",command_list->option);
    } else {
        fprintf(parse_file,"Options: \n");
    }
    if(command_list->isRedirection) {
        fprintf(parse_file,"Redirection: %s\n",command_list->redirection);
    } else {
        fprintf(parse_file,"Redirection: -\n");
    }
    if(command_list->isBackground) {
        fprintf(parse_file,"Background Job: y\n");
    } else {
        fprintf(parse_file,"Background Job: n\n");
    }

    fprintf(parse_file,"----------\n");
    //print_command(command_list);
}

void insert_parameters(char **myargs, struct command *command) {
    // 1.1.4 basic execution 
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

void* thread_func(void* arg) {
    int *fd = (int *)arg;

    // to make the thread wait for the output if it tries to read from pipe before it has been written
    fcntl(fd[0], F_SETFL, O_NONBLOCK);  // set the read end of the pipe to non-blocking
    struct timeval tv;
    int retval;
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd[0], &read_fds);
    tv.tv_sec = 0;
    tv.tv_usec = 200000; // time to give to the process to print on pipe (0.2 secs)
    retval = select(fd[0], &read_fds, NULL, NULL, &tv);
    if(retval == -1) { 
        return NULL; // no more data to read after timer, return
    } else {
        
        pthread_mutex_lock(&lock);
        char buf[BUFSIZE];
        memset(buf, 0, BUFSIZE);  // clear the buffer

        printf("---- %ld\n", pthread_self());
        fflush(stdout); 

        while(read(fd[0], buf, BUFSIZE) > 0){

            //write(STDOUT_FILENO, buf, strlen(buf));
            printf("%s", buf);
            fflush(stdout);
        }


        printf("---- %ld\n", pthread_self());
        fflush(stdout);

        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    FILE *commands_file, *parse_file;
    char buffer[BUFSIZE];
    int n;
    char *token;
    struct command command_list[MAX_COMMANDS];
    pthread_t threads[MAX_COMMANDS];
    int threads_count[MAX_COMMANDS];
    int stdin_fd = dup(STDIN_FILENO);
    int stdout_fd = dup(STDOUT_FILENO);
    commands_file = fopen("commands.txt", "r");
    parse_file = fopen("parse.txt", "w");

    int command_count=0;
    int rc;
    int fd[MAX_COMMANDS][2];
    // read the file line by line and parse each command
    while (fgets(buffer, BUFSIZE, commands_file) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0; // remove newline character from the command

        // parse new command
        parseCommand(buffer, parse_file, &command_list[command_count]); 
        //print_command(&command_list[command_count]);     
        pipe(fd[command_count]);
        
        // create new process for the command if command != wait
        if (strcmp(command_list[command_count].name, "wait") != 0) {
            rc = fork();
        }

        if (rc < 0) {
            // fork failed; exit
            fprintf(stderr, "fork failed\n");
            exit(1);
        } else if (rc == 0) { // command execution
            // 1.1.5 implementing redirectioning
            if (command_list[command_count].isRedirection == 1) {
                if (command_list[command_count].redirection[0] == '>') {
                    int new_stdout = open(command_list[command_count].redirection_file, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU); // open output file
                    dup2(new_stdout, STDOUT_FILENO); // redirect output to file
                } else if (command_list[command_count].redirection[0] == '<'){
                    int new_stdin = open(command_list[command_count].redirection_file, O_RDONLY); // open input file
                    dup2(new_stdin, STDIN_FILENO); // redirect input to file
                    dup2(fd[command_count][1], STDOUT_FILENO); // redirect output to fd
                }
            } else {
                // if there is no redirection, send output to pipe, shell will manage it
                dup2(fd[command_count][1], STDOUT_FILENO);
            }

            char *myargs[10];
            insert_parameters(myargs, &command_list[command_count]);
            pthread_mutex_lock(&lock);
            execvp(myargs[0], myargs);
        } else { // shell
            if ((command_list[command_count].isRedirection == 1 && 
                command_list[command_count].redirection[0] == '<') ||
                command_list[command_count].isRedirection == 0) { 
                // if command has no output redirection
                if (strcmp(command_list[command_count].name, "wait") != 0) {
                    threads_count[command_count] = 1;
                    pthread_create(&threads[command_count], NULL, thread_func, (void *) fd[command_count]);
                    // if not a background job, finish it's execution before parsing a new command
                    if (command_list[command_count].isBackground == 0) {
                        threads_count[command_count] = 0;
                        pthread_join(threads[command_count], NULL);
                    }
                }
            }
            // if it isn't a background command, shell wait for child to finish execution
            if (command_list[command_count].isBackground == 0) {
                wait(NULL);
            }

            // wait command implementation:
            // wait for all childs and threads to finish
            if (strcmp(command_list[command_count].name, "wait") == 0) {
                //printf("WAIT DETECTED\n");
                for(int i=0; i<command_count; i++) {
                    wait(NULL);
                    if(threads_count[i] == 1) {
                        threads_count[i] = 0;
                        pthread_join(threads[i], NULL);
                    }
                }
            }
        }
        command_count++;
    }

    // wait for all threads before finishin shell execution
    for(int i=0; i<command_count; i++) {
        if(threads_count[i] == 1) {
            threads_count[i] = 0;
            pthread_join(threads[i], NULL);
        }
    }

    fclose(commands_file);
    fclose(parse_file);

    return 0;
}
