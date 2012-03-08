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

File Name:  CPUCompileService.cpp

\*****************************************************************************/

#include "exceptions.h"
#include "CPUCompileService.h"
#include "ProgramBuilder.h"
#include "Program.h"
#include "BitCodeContainer.h"
#include "plugin_manager.h"
#include "llvm/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "BitCodeContainer.h"
#include "CPUDeviceBackendFactory.h"

#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetData.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/MemoryObject.h"
#include "llvm/MC/MCDisassembler.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/PassManager.h"
#include "llvm/ADT/Triple.h"

#include <sstream>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

DEFINE_EXCEPTION(IOError)

CPUCompileService::CPUCompileService(const CompilerConfig& config)
:m_programBuilder(CPUDeviceBackendFactory::GetInstance(), config) 
{
    m_backendFactory = CPUDeviceBackendFactory::GetInstance(); 
}

void CPUCompileService::DumpJITCodeContainer( const ICLDevBackendCodeContainer* pCodeContainer,
                    const std::string dumpJIT,
                    const std::string baseDirectory) const
{
    const BitCodeContainer* pContainer = static_cast<const BitCodeContainer*>(pCodeContainer);
    llvm::Module* pModule = (llvm::Module*)pContainer->GetModule();
    llvm::Triple triple(pModule->getTargetTriple());
    std::string err;
    const llvm::Target *target = llvm::TargetRegistry::lookupTarget(triple.getTriple(), err);
    std::string FeaturesStr;
    std::string MCPU;
    TargetMachine* TM = target->createTargetMachine(triple.getTriple(),
                                          MCPU, FeaturesStr);
    
    // Build up all of the passes that we want to do to the module.
    PassManager PM;

    // Create the output file.
    std::string fileName;
    if( !llvm::sys::path::is_absolute(dumpJIT) && !baseDirectory.empty())
    {
        llvm::sys::Path absFilePath(baseDirectory.c_str(), baseDirectory.size());
        if(false == absFilePath.appendComponent(dumpJIT))
                throw Exceptions::IOError("GetDataFilePath::Inexistent path created with \
                                         fileName=" + dumpJIT +
                                         " and baseDirectory=" +  baseDirectory);
        fileName = absFilePath.str();
    }
    else
    {
        fileName = dumpJIT;
        llvm::SmallString<128> fName(fileName);
        llvm::sys::fs::make_absolute(fName);
    }

    std::string errorInfo;
    llvm::raw_fd_ostream out(fileName.c_str(), errorInfo,
                llvm::raw_fd_ostream::F_Binary);
    if (!errorInfo.empty()) { return; }
    llvm::formatted_raw_ostream FOS(out);

    TM->addPassesToEmitFile(PM, FOS, TargetMachine::CGFT_AssemblyFile, CodeGenOpt::Default);
    PM.run(*pModule);
}

}}}
