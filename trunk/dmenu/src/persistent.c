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
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "persistent.h"

char *shm;
persistent_t g_persistent;

int persistent_init() {
    key_t key = PERSISTENT_ID;
    int   shmid;
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
            return 0; // Doesn't work...
        }
    }

    shm = (char*)shmat(shmid, NULL, 0);
    if (shm == (char*)-1)
    {
        perror("shmat");
        return 0;
    }

    if (erase_mem)
        memset(shm, 0, sizeof(persistent_t));

    return 1;
}

void persistent_read()
{
    if (shm == (char*)-1) return;
    memcpy(&g_persistent, shm, sizeof(persistent_t));
}

void persistent_write()
{
    if (shm == (char*)-1) return;
    memcpy(shm, &g_persistent, sizeof(persistent_t));
}
