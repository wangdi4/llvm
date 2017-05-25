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

File Name:  PrintIRPass.cpp

\*****************************************************************************/

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
    llvm::raw_fd_ostream FDTemp(fileName.str(), ErrorInfo,
                sys::fs::F_RW);
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
