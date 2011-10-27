/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  mem_utils.h

\*****************************************************************************/
#ifndef MEM_UTILS_H
#define MEM_UTILS_H

#include "auto_ptr_ex.h"
#include <stdlib.h>

namespace Validation { 

//\brief Allocated memory of given size, aligned to the given alignment. Throws bad_alloc on failure
void * align_malloc(size_t size, size_t alignment);
//\brief Frees memory previously allocated by align_malloc
void   align_free(void * ptr);

//\brief auto_ptr_ex destruction policy. Uses align_free to free the pointers
template<class T> struct AlignDP
{
    static void Delete(T* pT) { align_free((void*)pT); }
};
typedef auto_ptr_ex<char, AlignDP<char> > auto_ptr_aligned;


//\brief auto_ptr_ex destruction policy. Uses Release method to free the pointers
template<class T> struct ReleaseDP
{
    static void Delete(T* pT) { pT->Release(); }
};
                  
}
#endif //MEM_UTILS