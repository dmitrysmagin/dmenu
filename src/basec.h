#ifndef __BASEC_H__
#define __BASEC_H__

#include<dirent.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>

#define _log(dst, pre, fmt, args...) fprintf(dst, "%-20s(%4d)[%s]: "fmt "\n", strstr(__FILE__, "src"), __LINE__, pre, ##args)
#define log_error(fmt, args...)      _log(stderr, "err", fmt, ##args); perror(0)
#define log_message(fmt, args...)    _log(stdout, "msg", fmt, ##args)
#if DEBUG==1
#define log_debug(fmt, args...) _log(stdout, "dbg", fmt, ##args)
#else
#define log_debug(args...)
#endif

#define in_bounds(v, l, h) ((v>=l) && (v<h))
#define min(a,b) (a<b?a:b)
#define max(a,b) (a>b?a:b)

#define new_array(t, len)      (t*)malloc(sizeof(t) * len)
#define new_str(len)           new_array(char, len+1)
#define new_item(t)            new_array(t, 1)
#define copy_item(dst, src, t) memcpy(dst,src,sizeof(t))
#define clean_erase(e,f)       if (e) { f(e); e = NULL; } 

#define foreach(arr, fn, len) _foreach((void*)arr, (void*)fn, len)

#define new_str_arr(l) new_array(char*, l)
void free_str_arr(char** arr);
#define append_str(lst, len, val)\
len++;\
lst = realloc(lst, len*sizeof(char*));\
lst[len-1] = val;

#ifdef DINGOO_BUILD
char* strndup(char* s, size_t n);
#endif

int strrpos (char* s, int c);
int bound(int val, int low, int high);
int wrap(int val, int low, int high);
void _foreach(void** array, void* (f)(void*), int len);

#endif