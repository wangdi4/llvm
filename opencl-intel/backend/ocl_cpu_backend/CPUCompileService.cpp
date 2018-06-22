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
                                      size_t uiBinarySize) const
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

    // check maximum supported instruction
    // get maximum supported instruction from ELF header
    CLElfLib::E_EH_FLAGS headerFlag = static_cast<CLElfLib::E_EH_FLAGS>(reader.GetElfHeader()->Flags);
    if (cpuId.HasAVX2())
        valid &= (headerFlag == CLElfLib::EH_FLAG_AVX2);
    else if (cpuId.HasAVX1())
        valid &= (headerFlag == CLElfLib::EH_FLAG_AVX1);
    else
        valid &= (headerFlag == CLElfLib::EH_FLAG_SSE4);

    return valid ? CL_DEV_SUCCESS : CL_DEV_INVALID_BINARY;
}
}}}
