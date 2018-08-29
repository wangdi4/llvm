// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

#ifndef __UTILS_H__
#define __UTILS_H__

namespace Validation
{
    // this function is used to calculate ilogb(2m-1) - the number of
    // least significant bits of each mask element for built-ins shuffle and shuffle2
    inline int shuffleGetNumMaskBits(unsigned length) {
        int numBits = 0;
        switch(length) {
            case 1: numBits = 1; break;
            case 2: numBits = 1; break;
            case 3: numBits = 2; break;
            case 4: numBits = 2; break;
            case 8: numBits = 3; break;
            case 16: numBits = 4; break;
            default : throw Exception::InvalidArgument("[shuffleGetNumMaskBits] Wrong vector size"); break;
        }
        return numBits;
    }
} // End of Validation namespace
#endif // __UTILS_H__
