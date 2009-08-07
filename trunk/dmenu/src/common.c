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

void run_internal_command(char* command, char* args, char* workdir);

void fill_fb(Uint16* source)
{
    int fbdev;
    Uint16 *fb;

    fbdev=open("/dev/fb0", O_RDWR);
    fb=(Uint16 *)mmap(0, 320*240*sizeof(Uint16), PROT_WRITE, MAP_SHARED, fbdev, 0);
    memcpy(fb, source, 320*240*sizeof(Uint16));
}

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

    // separate the executable file name from the rest of command line,
    // in case there are arguments specified in the config file
    char* filename;
    const char delimeter[] = " ";
    filename = strsep(&executable, delimeter);

    if (executable && (executable[0] != '\0')) {
        if (args)
            execlp(filename, filename, executable, args, NULL);
        else
            execlp(filename, filename, executable, NULL);
    } else {
        if (args)
            execlp(filename, filename, args, NULL);
        else
            execlp(filename, filename, NULL);
    }

    // it should not return, otherwise it means we are not able to execute the application
    printf("Unable to execute %s\n", executable);
    perror(0);
    exit(1);
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

