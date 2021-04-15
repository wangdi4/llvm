// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

#include "BitCodeContainer.h"
#include "CompilationUtils.h"
#include "CPUCompileService.h"
#include "CPUDeviceBackendFactory.h"
#include "LibraryProgramManager.h"
#include "ObjectCodeContainer.h"
#include "ObjectDump.h"
#include "SystemInfo.h"
#include "exceptions.h"
#include "elf_binary.h"
#include "cache_binary_handler.h"

#include "llvm/IR/Module.h"

#if defined _M_X64 || defined __x86_64__
#define SPIR_TARGET_TRIPLE "spir64-unknown-unknown"
#else
#define SPIR_TARGET_TRIPLE "spir-unknown-unknown"
#endif

using CPUDetect = Intel::OpenCL::Utils::CPUDetect;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

namespace Utils {

static int getAsmDumpFileId() {
  static std::atomic<unsigned> fileId(0);
  fileId++;

  return fileId.load(std::memory_order_relaxed);
}

} // namespace Utils

CPUCompileService::CPUCompileService(std::unique_ptr<ICompilerConfig> config)
    :m_programBuilder(CPUDeviceBackendFactory::GetInstance(), std::move(config))
{
    m_backendFactory = CPUDeviceBackendFactory::GetInstance();
    LibraryProgramManager::getInstance()->createProgram(
        static_cast<CPUDeviceBackendFactory *>(m_backendFactory),
        m_programBuilder);
}

cl_dev_err_code CPUCompileService::DumpJITCodeContainer(
    const ICLDevBackendCodeContainer* codeContainer,
    const ICLDevBackendOptions* options) const
{
    try
    {
        // Load object code from ObjectCodeContainer
        const ObjectCodeContainer* container =
            static_cast<const ObjectCodeContainer*>(codeContainer);
        if (!container)
            return CL_DEV_INVALID_PROGRAM_EXECUTABLE;

        OpenCL::ELFUtils::CacheBinaryReader reader(container->GetCode(),
                                                   container->GetCodeSize());
        if (!reader.IsCachedObject())
            return CL_DEV_INVALID_PROGRAM_EXECUTABLE;
        size_t size = reader.GetSectionSize(
            Intel::OpenCL::ELFUtils::g_objSectionName);
        const char* buffer = (const char*)reader.GetSectionData(
            Intel::OpenCL::ELFUtils::g_objSectionName);
        assert(buffer && "Object buffer is null");
        // Create a copy to ensure objBuffer is properly aligned
        std::unique_ptr<llvm::MemoryBuffer> objBuffer =
            llvm::MemoryBuffer::getMemBufferCopy(llvm::StringRef(buffer, size));

        // Get dump filename
        std::string filename;
        if (options) {
            filename =
                options->GetStringValue(CL_DEV_BACKEND_OPTION_DUMPFILE, "");
        } else {
            // Use default base filename
            filename = Utils::SystemInfo::GetExecutableFilename();
            if (filename.empty())
                filename = "Program";
            filename += std::to_string(Utils::getAsmDumpFileId()) + ".asm";
        }

        // Open file
        std::error_code ec;
        llvm::raw_fd_ostream out(filename.c_str(), ec, llvm::sys::fs::FA_Write);
        if (ec)
            throw Exceptions::CompilerException(
                std::string("Failed to open file for dump: ") + ec.message());

        Utils::ObjectDump &objDump = Utils::ObjectDump::getInstance();
        if (llvm::Error err = objDump.dumpObject(objBuffer.get(), out)) {
            llvm::logAllUnhandledErrors(std::move(err), llvm::errs());
            throw Exceptions::CompilerException("Failed to dump object buffer");
        }

        return CL_DEV_SUCCESS;
    }
    catch( Exceptions::DeviceBackendExceptionBase& e )
    {
        return e.GetErrorCode();
    }
    catch( std::bad_alloc& )
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
}

cl_dev_err_code
CPUCompileService::CheckProgramBinary(const void *pBinary,
                                      size_t uiBinarySize)
{
    // check if it is LLVM BC (such as SPIR 1.2)
    if (!memcmp(_CL_LLVM_BITCODE_MASK_, pBinary, sizeof(_CL_LLVM_BITCODE_MASK_)-1)) {
        std::string strTargetTriple = (m_programBuilder.GetCompiler())->GetBitcodeTargetTriple(pBinary, uiBinarySize);

        if (strTargetTriple.find(SPIR_TARGET_TRIPLE) != 0) {
            assert(strTargetTriple.substr(0, 4) == "spir" && "Unexpected non-spir triple");
            return CL_DEV_INVALID_BINARY;
        }

        return CL_DEV_SUCCESS;
    }

    // check if it is a binary object
    if (_CL_OBJECT_BITCODE_MASK_ != ((const int *)pBinary)[0])
        return CL_DEV_SUCCESS;

    // Check only for cached Binary Object
    OpenCL::ELFUtils::CacheBinaryReader reader(pBinary, uiBinarySize);
    if (!reader.IsCachedObject())
        return CL_DEV_SUCCESS;

    bool valid = true;
    CPUDetect *cpuId = m_programBuilder.GetCompiler()->GetCpuId();

    // check bitOS
    // get bitOS from ELF header
    CLElfLib::E_EH_MACHINE headerBit = static_cast<CLElfLib::E_EH_MACHINE>(reader.GetElfHeader()->Machine);
    valid = cpuId->Is64BitOS() ? (headerBit == CLElfLib::EM_X86_64)
                               : (headerBit == CLElfLib::EM_860);

    // Check version of binary.
    // If binary doesn't contain section with version - return CL_INVALID_BINARY.
    const void* binaryVersion = reader.GetSectionData(Intel::OpenCL::ELFUtils::g_objVerSectionName);
    if (binaryVersion == nullptr) {
        return CL_DEV_INVALID_BINARY;
    }

    // Check does cached binary have up-to-date version. If not - return CL_INVALID_BINARY
    // So there's no backward compatibility between cached binaries with
    // different versions.
    if (*((const unsigned int *)binaryVersion) !=
        (unsigned int)OCL_CACHED_BINARY_VERSION) {
      return CL_DEV_INVALID_BINARY;
    }

    // check maximum supported instruction
    // get maximum supported instruction from ELF header
    CLElfLib::E_EH_FLAGS headerFlag = static_cast<CLElfLib::E_EH_FLAGS>(reader.GetElfHeader()->Flags);
    Intel::OpenCL::Utils::ECPU cpu = Intel::OpenCL::Utils::CPU_UNKNOWN;

    if (headerFlag == CLElfLib::EH_FLAG_AVX512_ICL){
      valid &= cpuId->HasAVX512ICL();
      cpu = Intel::OpenCL::Utils::CPU_ICL;
    }else if (headerFlag == CLElfLib::EH_FLAG_AVX512_SKX){
      valid &= cpuId->HasAVX512SKX();
      cpu = Intel::OpenCL::Utils::CPU_SKX;
    }else if (headerFlag == CLElfLib::EH_FLAG_AVX2){
      valid &= cpuId->HasAVX2();
      cpu = Intel::OpenCL::Utils::CPU_HSW;
    }else if (headerFlag == CLElfLib::EH_FLAG_AVX1){
      valid &= cpuId->HasAVX1();
      cpu = Intel::OpenCL::Utils::CPU_SNB;
    }else if (headerFlag == CLElfLib::EH_FLAG_SSE4){
      valid &= (cpuId->HasSSE41() || cpuId->HasSSE42());
      cpu = Intel::OpenCL::Utils::CPU_COREI7;
    }else{
        valid=false;
    }
    if (valid && cpuId->GetCPU() != cpu)
      m_programBuilder.GetCompiler()->SetBuiltinModules(
          CPUDetect::GetCPUName(cpu));
    return valid ? CL_DEV_SUCCESS : CL_DEV_INVALID_BINARY;
}
}}}
