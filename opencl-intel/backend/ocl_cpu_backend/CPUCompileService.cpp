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
#include "CPUCompileService.h"
#include "CPUDeviceBackendFactory.h"
#include "exceptions.h"
#include "elf_binary.h"
#include "cache_binary_handler.h"

#include "llvm/IR/Module.h"

#if defined _M_X64 || defined __x86_64__
#define SPIR_TARGET_TRIPLE "spir64-unknown-unknown"
#else
#define SPIR_TARGET_TRIPLE "spir-unknown-unknown"
#endif

namespace Intel { namespace OpenCL { namespace DeviceBackend {

CPUCompileService::CPUCompileService(const ICompilerConfig& config)
    :m_programBuilder(CPUDeviceBackendFactory::GetInstance(), config)
{
     m_backendFactory = CPUDeviceBackendFactory::GetInstance();
}

cl_dev_err_code CPUCompileService::DumpJITCodeContainer( const ICLDevBackendCodeContainer* pCodeContainer,
                                              const std::string& filename) const
{
    try
    {
        const BitCodeContainer* pContainer = static_cast<const BitCodeContainer*>(pCodeContainer);
        llvm::Module*           pModule    = reinterpret_cast<llvm::Module*>(pContainer->GetModule());
        const CPUCompiler*      pCompiler  = static_cast<const CPUCompiler*>(m_programBuilder.GetCompiler());

        pCompiler->DumpJIT( pModule, filename);
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
    CPUId cpuId = m_programBuilder.GetCompiler()->GetCpuId();

    // check bitOS
    // get bitOS from ELF header
    CLElfLib::E_EH_MACHINE headerBit = static_cast<CLElfLib::E_EH_MACHINE>(reader.GetElfHeader()->Machine);
    valid = cpuId.Is64BitOS() ? (headerBit == CLElfLib::EM_X86_64)
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
    if (*((unsigned int*)binaryVersion) != (unsigned int)OCL_CACHED_BINARY_VERSION) {
        return CL_DEV_INVALID_BINARY;
    }

    // check maximum supported instruction
    // get maximum supported instruction from ELF header
    CLElfLib::E_EH_FLAGS headerFlag = static_cast<CLElfLib::E_EH_FLAGS>(reader.GetElfHeader()->Flags);

    ECPU cpu = DEVICE_INVALID;
    if (headerFlag == CLElfLib::EH_FLAG_AVX512_ICL){
        valid &= cpuId.HasAVX512ICL();
        cpu = CPU_ICL;
    }else if (headerFlag == CLElfLib::EH_FLAG_AVX512_SKX){
        valid &= cpuId.HasAVX512SKX();
        cpu = CPU_SKX;
    }else if (headerFlag == CLElfLib::EH_FLAG_AVX2){
        valid &= cpuId.HasAVX2();
        cpu = CPU_HASWELL;
    }else if (headerFlag == CLElfLib::EH_FLAG_AVX1){
        valid &= cpuId.HasAVX1();
        cpu = CPU_SANDYBRIDGE;
    }else if (headerFlag == CLElfLib::EH_FLAG_SSE4){
        valid &= (cpuId.HasSSE41()||cpuId.HasSSE42());
        cpu = CPU_COREI7;
    }else{
        valid=false;
    }
    if (valid && cpuId.GetCPU() != cpu)
        m_programBuilder.GetCompiler()->SetBuiltinModules(CPUId::GetCPUName(cpu));
    return valid ? CL_DEV_SUCCESS : CL_DEV_INVALID_BINARY;
}
}}}
