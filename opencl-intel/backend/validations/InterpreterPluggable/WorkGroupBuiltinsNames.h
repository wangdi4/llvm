/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  WorkGroupBuiltinsNames.h

\*****************************************************************************/
#ifndef WORK_GROUP_BUILTINS_NAMES
#define WORK_GROUP_BUILTINS_NAMES

#include <map>
#include <string>
#include "FunctionDescriptor.h"
#include "llvm/IR/Type.h"

namespace Validation{

    ///helper class for finding workgroup builtins
    class WorkGroupBultinsNames{
    public:
        //ctor
        WorkGroupBultinsNames();
        ///@brief determinates whether given mangled function name belongs
        ///to work group builtins names or not.
        ///@param [in] MangledName - mangled name to be processed
        ///@return is MangledName belongs to work group builtins
        bool isWorkGroupBuiltin(std::string MangledName);
        ///@brief constructs mangled name of pre-exec method for given function
        ///@param [in] MangeldName - mangled name of function for which you want to
        ///construct mangled pre-exec method name
        //@return namgled name of pre-exec method
        std::string getMangledPreExecMethodName(std::string MangeldName);
    private:
        typedef std::map<std::string, reflection::FunctionDescriptor> WGBuiltinsNamesMap;
        typedef std::pair<std::string, reflection::FunctionDescriptor> WGBuiltinsNamesDesc;
        WGBuiltinsNamesMap m_MangledNames;
    };
}

#endif
