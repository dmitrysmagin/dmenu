/*
 *  Copyright (C) 2009 sca <scahoo <at> gmail <dot> com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __PERSISTENT_H__
#define __PERSISTENT_H__

// Increment this when persistent_t is changed
#define PERSISTENT_ID (0xBFAA0201)

typedef struct {
    int current_menu, current_menuitem;
} persistent_t;

extern persistent_t* g_persistent;

int persistent_init();

#endif//__PERSISTENT_H__
