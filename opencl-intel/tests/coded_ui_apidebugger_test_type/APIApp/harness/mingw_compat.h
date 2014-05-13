
#ifndef MINGW_COMPAT_H
#define MINGW_COMPAT_H

#if defined(__MINGW32__)
char *basename(char *path);
#include <malloc.h>

#if defined(__MINGW64__)
//mingw-w64 doesnot have __mingw_aligned_malloc, instead it has _aligned_malloc
#define __mingw_aligned_malloc _aligned_malloc
#define __mingw_aligned_free _aligned_free
#include <stddef.h>
#endif //(__MINGW64__)

#endif //(__MINGW32__)
#endif // MINGW_COMPAT_H
