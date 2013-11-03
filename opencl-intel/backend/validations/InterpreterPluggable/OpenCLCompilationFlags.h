/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  OpenCLCompilationFlags.h

\*****************************************************************************/

#ifndef OPEN_CL_REF_COMPILATION_FLAGS_H
#define OPEN_CL_REF_COMPILATION_FLAGS_H

#include "llvm/IR/Module.h"

namespace llvm
{
    /// TODO: Add support for all compilation flags
    enum RefCompilationFlags
    {
        CL_STD_20,
        CL_FAST_RELAXED_MATH
    };
    typedef std::vector<RefCompilationFlags> CompilationFlagsList;

    CompilationFlagsList GetCompilationFlags(llvm::Module* module);
}

#endif