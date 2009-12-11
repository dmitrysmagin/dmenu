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
#define PERSISTENT_ID (0xBFAA0202)

typedef struct {
    char current_menu_title[100], 
         current_menu_item_title[100], 
         current_menu_subitem_title[100];
} persistent_t;

extern persistent_t* g_persistent;

int persistent_init();
void persistent_store_menu_position();
void persistent_restore_menu_position();

#endif//__PERSISTENT_H__
