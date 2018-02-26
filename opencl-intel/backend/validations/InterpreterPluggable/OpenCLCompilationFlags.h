/*****************************************************************************\

Copyright (c) Intel Corporation (2013-2018).

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

#include <string>

namespace Validation {

class CompilationFlags {

public:

  static unsigned getCLVersionFromMetadata(llvm::Module *M);
  static unsigned getCLVersionFromFlags(const std::string &Flags);

  static bool hasFastRelaxedMathFlag(const std::string &Flags);
  static bool hasUniformWGSizeFlag(const std::string &Flags);
};

}

#endif
