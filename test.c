#include <string.h>
#include <stdio.h>

int main () {
   char str[80] = "This is - www.tutorialspoint.com - website";
   const char s[2] = "-";
   //char *token[];
   //token = char[10];
   int n;
   printf("%d", n);
   //printf("%d", token[0]);
   
   return(0);
}
/*
// put all the commands into a buffer
    while ((n = fread(buffer, 1, sizeof(buffer), commands_file)) > 0) {
        printf("---------commands.txt----------\n%s\n------------------------\n", buffer);
    }
   
    // first command
    token = strtok(buffer, "\n");
    printf("---------tokens------\n");
    //parseCommand(token);
   
    // divide the commands in tokens
    int n1 = 0;
    while(token != NULL) {
        // for every command execute the parse
        printf("%d: %s\n", n1,token);
        n1++;
        parseCommand(token, parse_file);

        token = strtok(NULL, "\n");
    }*/