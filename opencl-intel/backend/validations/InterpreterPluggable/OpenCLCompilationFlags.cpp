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

File Name:  OpenCLCompilationFlags.cpp

\*****************************************************************************/
#include "OpenCLCompilationFlags.h"
#include "llvm/IR/Metadata.h"

namespace llvm{

    CompilationFlagsList GetCompilationFlags(llvm::Module* module)
    {
        CompilationFlagsList flags;

        assert(module && "Module is not set");
        NamedMDNode* metadata = module->getNamedMetadata("opencl.compiler.options");
        if(metadata != NULL)
        {
            MDNode* flag = metadata->getOperand(0);
            for(uint32_t i =0; i< flag->getNumOperands() && flag; ++i)
            {
                MDString* flagName = dyn_cast<MDString>(flag->getOperand(i));
                if(flagName->getString() == "-cl-fast-relaxed-math")
                {
                    flags.push_back(CL_FAST_RELAXED_MATH);
                }
                else if(flagName->getString() == "-cl-std=CL2.0")
                {
                    flags.push_back(CL_STD_20);
                }
            }
        }
        return flags;
    }

}