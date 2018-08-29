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

#ifndef __PRINT_IR_PASS__H__
#define __PRINT_IR_PASS__H__

#include "cl_dev_backend_api.h"
#include <llvm/Pass.h>
#include <llvm/IR/Module.h>

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
}}}
#endif
