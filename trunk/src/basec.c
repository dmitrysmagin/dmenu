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

void free_str_arr(char** arr)
{
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

void copyfile(char* from, char* to)  {
    FILE *in, *out;
    char ch;
    
    log_debug("Copying file: %s to %s", from, to);
    
    if((in=fopen(from, "rb")) == NULL) {
        log_error("Cannot open file: %s", from);
    }
    if((out=fopen(to, "wb")) == NULL) {
        log_error("Cannot open file: %s", to);
    }
    
    while(!feof(in)) {
        ch = getc(in);
        if(ferror(in)) {
            log_error("Read Error: %s", from);
            clearerr(in);
            break;
        } else {
            if(!feof(in)) putc(ch, out);
            if(ferror(out)) {
                log_error("Write Error: %s", to);
                clearerr(out);
                break;
            }
        }
    }
    fclose(in);
    
    int fno = fileno(out);
    fsync(fno);
    fclose(out);
}