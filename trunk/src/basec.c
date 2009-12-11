/*
*  Copyright (C) 2009 Timothy Soehnlin <timothy.soehnlin@gmail.com>
*
*  Author: <timothy.soehnlin@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*/

#include "basec.h"
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>

void free_str_arr(char** arr) {
    int i =0;
    while (*(arr + i)) i++;
    while (i) free(arr[i--]);
    free(arr);
}

#ifdef DINGOO_BUILD
char* strndup(char* s, size_t n) {
    size_t len = strlen(s);
    len = min(len, n);
    char *new = malloc (len + 1);
    
    if (new == NULL) return NULL;
    
    new[len] = '\0';
    return memcpy (new, s, len);
}
#endif

int stricmp(const char *a, const char *b) {
    char c,d;
    do {
        c = to_lower(*a);
        d = to_lower(*b);
        a++; b++;
    } while ((c) && (c == d));
    return (int) (c - d);
}

int strrpos (char* s, int c) {
    int ret = -1, j = 0;
    do {
        if (*s == c) ret = j;
        j++;
    } while (*s++);
    return ret;
}


int bound(int val, int low, int high) {
    return val < low ? low : ( val > high ? high : val);
}

int wrap(int val, int low, int high) {
    return val < low ? high : (val > high ? low : val);
}

void _foreach(void** array, void* (f)(void*), int len) {
    int i=-1;
    while (++i<len) f(array[i]);
}