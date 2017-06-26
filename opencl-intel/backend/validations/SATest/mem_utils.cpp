/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  mem_utils.cpp

\*****************************************************************************/

#include "mem_utils.h"

namespace Validation { 

void * align_malloc(size_t size, size_t alignment)
{
    void * ptr = NULL;
#if defined(_WIN32) && defined(_MSC_VER)
    ptr = _aligned_malloc(size, alignment);
#elif  defined(__linux__) || defined (linux)
   // The value of alignment shall be a multiple of sizeof(void*)
   {
      size_t linuxAlignment;
      size_t remainder = alignment % (sizeof(void*));
      if(remainder) {
        size_t quotient = alignment / (sizeof(void*));
        linuxAlignment = (quotient+1)*(sizeof(void*));
      }
      else {
        linuxAlignment = alignment;
      }
      if (0 != posix_memalign(&ptr, linuxAlignment, size)) 
      {
        ptr = NULL;
      }
   }
#elif defined(__MINGW32__)
    ptr =  __mingw_aligned_malloc(size, alignment);
#else
#error "Please add support OS for aligned malloc" 
#endif
    if( NULL == ptr )
    {
        throw std::bad_alloc();
    }
    return ptr;
}

// This function implementation must ignore NULL ptr value.
void   align_free(void * ptr)
{
#if defined(_WIN32) && defined(_MSC_VER)
    _aligned_free(ptr);
#elif  defined(__linux__) || defined (linux)
    return  free(ptr);
#elif defined(__MINGW32__)
    return __mingw_aligned_free(ptr); 
#else
#error "Please add OS support for aligned free"
#endif
}

} // namespace