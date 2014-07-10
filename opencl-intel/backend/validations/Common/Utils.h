/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  Utils.h

\*****************************************************************************/
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
