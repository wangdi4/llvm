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

File Name:  PrintIRPass.h

\*****************************************************************************/
#ifndef __PRINT_IR_PASS__H__
#define __PRINT_IR_PASS__H__

#include "cl_dev_backend_api.h"
#include <llvm/Pass.h>
#include <llvm/Module.h>

#include <vector>
#include <sstream>
#include <string>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class PrintIRPass : public llvm::ModulePass
{
    static char ID; // Pass identification, replacement for typeid
    std::string m_IRDumpOption;
    std::string m_IRDumpFolder;
    bool m_doNotDumpIR;
public:
    /**
     * Options used during dumping IR
     */
    class DumpIRConfig
    {
        const std::vector<int>* m_IRDumpOption;
        bool m_IRDumpOptionAll;
    public:
        /// This is a helper to determine whether the IR option was stated
        bool FindIROption(const std::vector<int>* IROptions, IRDumpOptions option)
        {
            //rely on that IROptions is already sorted
            if (binary_search(IROptions->begin(), IROptions->end(), option))
                return true;
            return false;
        }

        bool FindIROptionAll(const std::vector<int>* IROptions)
        {
            if(FindIROption(IROptions, DUMP_IR_ALL))
                return true;
            return false;
        }

        /// This is a utility to check whether a pass should have IR dumped
        bool ShouldPrintPass(IRDumpOptions option)
        {
          return m_IRDumpOptionAll || FindIROption(m_IRDumpOption,option);
        }
       
        DumpIRConfig(const std::vector<int>* IRDumpOption):
                m_IRDumpOption(IRDumpOption)
            {
                m_IRDumpOptionAll = FindIROptionAll(IRDumpOption);
            }

         ~DumpIRConfig() {}
    };
    PrintIRPass(IRDumpOptions option, dump_IR_options optionLocation,
                            std::string dumpDir);
    // doPassInitialization - prints the IR
    bool runOnModule(llvm::Module &M);
};
    /// createPrintIRPass - Create and return a pass that dumps the module
    /// to the specified ostream.
    llvm::ModulePass *createPrintIRPass(int option, int optionLocation,
                            std::string dumpDir);
}}}

#endif
