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

#include "PrintIRPass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {
char PrintIRPass::ID=0;

PrintIRPass::PrintIRPass(IRDumpOptions option,
    dump_IR_options optionLocation, std::string dumpDir):
    ModulePass(ID), m_IRDumpFolder(dumpDir), m_doNotDumpIR(false)
{
    std::string passName;
    switch(option)
    {
    case DUMP_IR_TARGERT_DATA :
        passName = "target_data";
        break;
    case DUMP_IR_VECTORIZER :
        passName = "vectorizer";
        break;
    default:
        assert(false && "option is not a valid IRDumpOptions");
        m_doNotDumpIR = true;
        return ;
    }
    std::string passLocation;
    switch(optionLocation)
    {
    case OPTION_IR_DUMPTYPE_BEFORE :
        passLocation = "_before";
        break;
    case OPTION_IR_DUMPTYPE_AFTER :
        passLocation = "_after";
        break;
    }
    m_IRDumpOption = passName + passLocation;
}

bool PrintIRPass::runOnModule(llvm::Module &M)
{
    if(m_doNotDumpIR)
        return false;

    using namespace llvm;
    // Create the output file.
    std::stringstream fileName;
    fileName << m_IRDumpFolder.c_str() << "/dump." << m_IRDumpOption.c_str() 
                                       << ".ll" << std::ends;
    std::error_code ErrorInfo;
    llvm::raw_fd_ostream FDTemp(fileName.str(), ErrorInfo, sys::fs::FA_Write);
    if (ErrorInfo) {
       return false;
    }
    M.print(FDTemp, nullptr);
    return true;
}

}}}

extern "C" llvm::ModulePass *createPrintIRPass(int option, int optionLocation, std::string dumpDir) 
{
  return new Intel::OpenCL::DeviceBackend::PrintIRPass(
    (Intel::OpenCL::DeviceBackend::IRDumpOptions)option,
    (Intel::OpenCL::DeviceBackend::dump_IR_options)optionLocation,
    dumpDir);
}
