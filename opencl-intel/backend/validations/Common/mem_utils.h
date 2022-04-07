// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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
    static void Delete(T* pT)
    {
        if (NULL != pT)
        {
            pT->Release();
        }
    }
};
}
#endif //MEM_UTILS
