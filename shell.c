// PID: 730334448
// I pledge the COMP 211 honor code.

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

#include "shell.h"

const char *BUILT_IN_COMMANDS[] = {"cd", "exit", NULL};
const char *PATH_SEPARATOR = ":";

void parse(char *line, command_t *p_cmd){
    char *token;
    int argc = 0;

    while((token = strtok_r(line, " \n\t\r", &line)) != NULL){
        strcpy(p_cmd->argv[argc], token);
        argc++;
    }

    strcpy(p_cmd->path, p_cmd->argv[0]);
    p_cmd->argc = is_builtin(p_cmd) || find_fullpath(p_cmd) ? argc : ERROR;
} // end parse function

bool find_fullpath(command_t *p_cmd){
    // Path environmental variables. Looks something like:
    // /usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/mnt/learncli/bin
    char path[300];
    strcpy(path, getenv("PATH"));

    char *token = strtok(path, PATH_SEPARATOR);

    // Combines '/' to the command
    char slash[30];
    strcpy(slash, "/");
    strcat(slash, p_cmd->path);

    while(token != NULL){
        // Debug:
        //printf("%s\n", token);

        struct stat buffer;
        int exists;
        char temp[100];
        strcpy(temp, token);
        strcat(temp, slash);

        exists = stat(temp, &buffer);

        if(exists == 0 && (S_IFREG & buffer.st_mode)){
            strcpy(p_cmd->path, temp);
            return true;
        }

        token = strtok(NULL, PATH_SEPARATOR);
    }

    return false;
} // end find_fullpath function

int execute(command_t *p_cmd){
    int status = SUCCESSFUL;
    int child_process_status;
    pid_t child_pid;

    child_pid = fork();

    if(child_pid == -1) {
        perror("Execute terminated with an error condition!\n");
        status = ERROR;
    } else if(child_pid == 0) {
        p_cmd->argv[p_cmd->argc] = NULL;
        child_process_status = execv(p_cmd->path, p_cmd->argv);
        if(child_process_status == -1) {
            return ERROR;
        }
    } else{
        wait(&child_process_status); 
    }

    return status;
} // end execute function


bool is_builtin(command_t *p_cmd){
    int i = 0;

    while(BUILT_IN_COMMANDS[i] != NULL){
        if(strcmp(p_cmd->path, BUILT_IN_COMMANDS[i]) == 0){
            return true;
        }

        i++;
    }

    return false;
} // end is_builtin function


int do_builtin(command_t *p_cmd){
    struct stat buff;
    int status = ERROR;

    if(p_cmd->argc == 1){

        // -----------------------
        // cd with no arg
        // -----------------------
        // change working directory to that
        // specified in HOME environmental 
        // variable

        status = chdir(getenv("HOME"));
    }else if((stat(p_cmd->argv[1], &buff) == 0 && (S_IFDIR & buff.st_mode))){
        // -----------------------
        // cd with one arg 
        // -----------------------
        // only perform this operation if the requested
        // folder exists

        status = chdir(p_cmd->argv[1]);
    }

    return status;
} // end do_builtin function

void cleanup(command_t *p_cmd){
    int i = 0;

    while(p_cmd->argv[i] != NULL){
        free(p_cmd->argv[i]);
        i++;
    }

    free(p_cmd->argv);
    free(p_cmd->path);	
} // end cleanup function
