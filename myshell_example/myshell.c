/*
 * MyShell Project for SOFE 3950U / CSCI 3020U: Operating Systems
 *
 * Copyright (C) 2015, Muhammad Ahmad, Timothy MacDougall, Devin Westbye
 * All rights reserved.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include "utility.h"
#include "myshell.h"

// Put macros or constants here using #define
#define BUFFER_LEN 256
#define NUM_ENV 2

// Put global environment variables here
typedef struct{
	char name[8];
	char value[256];
} env_var;

env_var env_vars[2];

// Define functions declared in myshell.h here
// Returns the current working directory path
char *get_cwd(){
    return getcwd(NULL, BUFFER_LEN);
}

// Returns the path of the current executable
char *get_executable(char* SHELL){
	SHELL = (char *) calloc(BUFFER_LEN, sizeof(char));
	int n = readlink("/proc/self/exe", SHELL, BUFFER_LEN);
	if(n > 0){
		return SHELL;
	} else {
		return NULL;
	}
}

void set_var(char name[], char value[]){
	for(int i = 0; i < NUM_ENV; i++){
		if(strcmp(&name, &env_vars[i].name) == 0){
			 strcpy(env_vars[i].value, value);
		}	
	}	
}
	

int main(int argc, char *argv[], char *env[]) {
    // Input buffer and and commands
    char buffer[BUFFER_LEN] = { 0 };
    char command[BUFFER_LEN] = { 0 };
    char arg[BUFFER_LEN] = { 0 };
    char delim[BUFFER_LEN];
    char* token = NULL;
    char* SHELL = NULL;

	setenv("SHELL", get_executable(SHELL), 1);
	free(SHELL);

	strcpy(env_vars[0].name, "PWD");
	strcpy(env_vars[0].value, get_cwd());
	strcpy(env_vars[1].name, "SHELL");
	strcpy(env_vars[1].value, get_executable(SHELL));

    // Parse the commands provided using argc and argv
	FILE *in_stream;
	if(argc > 1){
		in_stream = fopen(argv[1], "r");
		if(in_stream == NULL){
			printf("Could not open file %s", argv[1]);
			return EXIT_FAILURE;
		}
		
	} else {
		in_stream = stdin;
		printf("%s> ", get_cwd());
	}

	
    // Perform an infinite loop getting command input from users
    while (fgets(buffer, BUFFER_LEN, in_stream) != NULL) {
        //inputLen = strlen(buffer); // get length of line typed in shell
        strcpy(arg, ""); // reset arg
        strcpy(command, ""); // reset command
        // Perform string tokenization to get the command and argument
        strcpy(delim, " ");
        token = strtok(buffer, delim); // get first word

        if (token[strlen(token)-1] == '\n')
            token[strlen(token)-1] = '\0'; // remove newline character if at end of string
        strcpy(command, token); // copy token to command

        // loop to get second word in buffer
        while(token != NULL) {
            strcpy(delim, "\n");
            token = strtok(NULL, delim);
//puts(token);
            if (token == NULL)
                break;
            strcpy(arg, token); // store second word in arg 
        }

        // Check the command and execute the operations for each command
        // cd command -- change the current directory
        if (strcmp(command, "cd") == 0) {
            if (strcmp(arg, " ") == 0 || strcmp(arg, "") == 0)
                printf("Error: cd: argument required\n");
            cd(arg);
        } else if (strcmp(command, "clr") == 0) {
            clr();
        } else if (strcmp(command, "dir") == 0) {
            dir(arg);
        } else if (strcmp(command, "environ") == 0) {
            environ(env);
        } else if (strcmp(command, "echo") == 0) {
            echo(arg);
        } else if (strcmp(command, "help") == 0) {
            if (strcmp(arg, " ") == 0 || strcmp(arg, "") == 0)
                printf("Error: help: argument must be of format: help [-dlfpcsu] \n");
            help(arg, get_executable(SHELL));
        } else if (strcmp(command, "pause") == 0) {
            pauses();
        } else if (strcmp(command, "") == 0) {
            
        } 
        
        // quit command -- exit the shell
        else if (strcmp(command, "quit") == 0) {
            return EXIT_SUCCESS;
        }

        // Unsupported command
        else {
            char program[BUFFER_LEN];
            char location[BUFFER_LEN];
            int len = sizeof(command) / sizeof(command[0]);

            for (int i = len; i >= 0; i--) {
                if (command[i] == '/') {
                    for (int j = i+1; j < len; j++) {
                        program[j-i-1] = command[j]; // get program name
                    }
                    for (int j = 0; j <= i; j++) {
                        location[j] = command[j]; // get location name
                    }
                    break;
                }
            }

            DIR *directory;
            struct dirent *ep = NULL;
            directory = opendir(location);
            if (directory != NULL) {
                while ((ep = readdir(directory))) {
                    if (strcmp(ep->d_name, program) == 0) {
                        int pid;
                        pid = fork();
                        if (pid < 0) {
                            printf("Error\n");
                            exit(1);
                        } else if (pid == 0) {
                            if (execvpe(program, arg, argv) == -1) { // run the program in the child process
                                printf("Error: could not run program\n");
                                exit(1);
                            }
                        }
                        wait(); // wait for the process to terminate
                    }
                }
            } else {
                printf("Program or program location does not exist\n");
            }
            //fputs("Unsupported command, use help to display the manual\n", stderr);
            free(ep);
        }
		if(argc == 1)
			printf("%s> ", get_cwd());

    }
    free(token); // free the token pointer
    return EXIT_SUCCESS;
}
