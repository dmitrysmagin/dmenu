/*
 *  Copyright (C) 2009 Rookie1 <mr.rookie1@gmail.com>
 *
 *  Author: <mr.rookie1@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <SDL.h>
#include "filelist.h"
#include "conf.h"
#include "menu.h"
#include "dosd/dosd.h"

void run_internal_command(char* command, char* args, char* workdir);
/*
void fill_fb(Uint16* source)
{
    int fbdev;
    Uint16 *fb;

    fbdev=open("/dev/fb0", O_RDWR);
    fb=(Uint16 *)mmap(0, 320*240*sizeof(Uint16), PROT_WRITE, MAP_SHARED, fbdev, 0);
    memcpy(fb, source, 320*240*sizeof(Uint16));
}
*/
/*
void run_command(char* executable, char* args, char* workdir)
{
    char exe[512];
    char cwd[512];
    int rc;

    int exe_len = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    exe[exe_len] = '\0';
    int cwd_len = readlink("/proc/self/cwd", cwd, sizeof(cwd) - 1);
    cwd[cwd_len] = '\0';

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s %s;cd %s;exec %s", executable, args, cwd, exe);

    rc = chdir(workdir);
    if (rc != 0) {
        printf("Unable to change to directory %s\n", workdir);
        perror(0);
    }

    execlp("/bin/sh", "/bin/sh", "-c", cmd, NULL);

    // it should not return, otherwise it means we are not able to execute the application
    rc = chdir(cwd);
    if (rc != 0) {
        printf("Unable to change to directory %s\n", cwd);
        perror(0);
    }
    execlp(exe, exe, NULL);
}
*/

/* this is a simplified version when init is used to spawn dmenu */
void run_command(char* executable, char* args, char* workdir)
{
    int rc;

    if (executable[0] == '!') {
        run_internal_command(executable, args, workdir);
        return;
    }

    SDL_Quit();
    rc = chdir(workdir);
    if (rc != 0) {
        printf("Unable to change to directory %s\n", workdir);
        perror(0);
    }

    // the system ARG_MAX might be too large. just use a fixed
    // value here.
#define MAX_CMD_LEN 4096
    char *commandline = malloc(MAX_CMD_LEN);

    // commandline = executable + args
    strcpy(commandline, executable);
    if (args) {
        strcat(commandline, " ");
        strcat(commandline, args);
    }

    // build the args list for exec()
    char** args_list = NULL;
    char* token;
    int args_list_size = 0;
    const char delimeter[] = " ";
    while ((token = strsep(&commandline, delimeter))) {
        if (token[0] != '\0') {
            args_list_size ++;
            args_list = realloc(args_list, args_list_size * sizeof(char*));
            args_list[args_list_size-1] = token;
        }
    }

    // add null poiner as the last arg
    args_list_size ++;
    args_list = realloc(args_list, args_list_size * sizeof(char*));
    args_list[args_list_size-1] = NULL;

    /* Call destructors, otherwise open FDs will be leaked to the
       exec()'ed process.
       Yes, this is ugly. If another situation like this arises we should write
       a custom atexit implementation.
    */    
    menu_deinit();
    filelist_deinit();
    conf_unload();
    dosd_deinit();

    // launch the program
    execvp(args_list[0], args_list);

    // it should not return, otherwise it means we are not able to execute the application
    printf("Unable to execute. Command line - ");
    int i=0;
    while (args_list[i]) {
        printf("%s ", args_list[i]);
        i++;
    }
    printf("\n");
    perror(0);
    
    // Exit without calling any atexit() functions
    _exit(1);
}

void run_internal_command(char* command, char* args, char* workdir)
{
    if (strcmp(command, COMMAND_THEMESELECT) == 0) { // internal command - themeselect
        if (args) {
            conf_themeselect(args);
        }
        return;
    }
}

