/*
 *  Copyright (C) 2009 sca <scahoo <at> gmail <dot> com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "persistent.h"

persistent_t* g_persistent;

int persistent_init() {
    key_t key = PERSISTENT_ID;
    int   shmid;
    char* shm;
    int   erase_mem = 0;

    shmid = shmget(key, sizeof(persistent_t), 0666);
    if (shmid == -1)
    {
        // This is the first run, allocate shared memory
        shmid     = shmget(key, sizeof(persistent_t), IPC_CREAT | 0666);
        erase_mem = 1;

        if (shmid == -1)
        {
            perror("shmget");
            goto init_failed;
        }
    }

    shm = (char*)shmat(shmid, NULL, 0);
    if (shm == (char*)-1)
    {
        perror("shmat");
        goto init_failed;
    }

    // TODO: Find out wether this is needed.
    if (erase_mem)
        memset(shm, 0, sizeof(persistent_t));

    g_persistent = (persistent_t*)shm;
    
    return 1;
    
init_failed:
    g_persistent = malloc(sizeof(persistent_t));
    if (!g_persistent) exit(1);
    
    memset(g_persistent, 0, sizeof(persistent_t));
    return 0;
}
