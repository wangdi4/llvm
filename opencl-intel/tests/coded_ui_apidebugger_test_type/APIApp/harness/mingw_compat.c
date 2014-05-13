
#if defined(__MINGW32__)

#include "mingw_compat.h"
#include <stdio.h>
#include <string.h>

//This function is unavailable on various mingw compilers,
//especially 64 bit so implementing it here
const char *basename_dot="."; 
char*
basename(char *path)
{
    char *p = path, *b = NULL;
    int len = strlen(path);

    if (path == NULL) {
        return (char*)basename_dot;
    }
    
    // Not absolute path on windows
    if (path[1] != ':') {
        return path;
    } 
    
    // Trim trailing path seperators
    if (path[len - 1]  == '\\' || 
        path[len - 1]  == '/' ) {
        len--;
        path[len] = '\0';
    }
    
    while (len) {
        while((*p != '\\' || *p != '/')  && len) {
            p++;
            len--;
        }
        p++;
        b = p;
     }

     return b;
}

#endif