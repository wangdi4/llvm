// Copyright (c) 2006-2009 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly
#include "common_clang.h"

#include "llvm/ADT/OwningPtr.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Bitcode/ReaderWriter.h"

#include <string>
#include <vector>

using namespace Intel::OpenCL::ClangFE;

void CommonClangInitialize();

struct CACHED_ARG_INFO
{
    std::string name;
    std::string typeName;
    cl_kernel_arg_address_qualifier adressQualifier;
    cl_kernel_arg_access_qualifier accessQualifier;
    cl_kernel_arg_type_qualifier typeQualifier;
};

class OCLFEKernelArgInfo : public IOCLFEKernelArgInfo
{
public:
    unsigned int getNumArgs() const { return m_argsInfo.size(); }
    const char* getArgName(unsigned int index) const { return m_argsInfo[index].name.c_str(); }
    const char* getArgTypeName(unsigned int index) const { return m_argsInfo[index].typeName.c_str(); }
    cl_kernel_arg_address_qualifier getArgAdressQualifier(unsigned int index) const { return m_argsInfo[index].adressQualifier; }
    cl_kernel_arg_access_qualifier getArgAccessQualifier(unsigned int index) const { return m_argsInfo[index].accessQualifier; }
    cl_kernel_arg_type_qualifier getArgTypeQualifier(unsigned int index) const { return m_argsInfo[index].typeQualifier; }

    void Release() { delete this; }

public:
    void addInfo(const CACHED_ARG_INFO& info)
    {
        m_argsInfo.push_back(info);
    }

private:
    std::vector<CACHED_ARG_INFO>  m_argsInfo;
};

extern "C" CC_DLL_EXPORT int GetKernelArgInfo(const void *pBin,
                                              size_t uiBinarySize,
                                              const char *szKernelName,
                                              IOCLFEKernelArgInfo** ppResult)
{
    // Lazy initialization
    CommonClangInitialize();

    try
    {
        std::string sError;
        llvm::OwningPtr<OCLFEKernelArgInfo> pResult(new OCLFEKernelArgInfo );
        llvm::OwningPtr<llvm::LLVMContext> context( new llvm::LLVMContext() );

        const char* pBinary = (const char*)pBin;

        llvm::OwningPtr<llvm::MemoryBuffer> pBinBuff(llvm::MemoryBuffer::getMemBuffer(llvm::StringRef(pBinary, uiBinarySize), "", false));

        // parseIR takes ownership of the buffer, so release it
        llvm::OwningPtr<llvm::Module> pModule(ParseBitcodeFile(pBinBuff.get(), *context, &sError));

        if (NULL == pModule.get())
        {
            throw std::bad_alloc();
        }

        llvm::NamedMDNode *pKernels = pModule->getNamedMetadata("opencl.kernels");
        if (!pKernels)
        {
            throw  std::string("couldn't find any kernels in the metadata");
        }

        unsigned int uiNumKernels = pKernels->getNumOperands();

        llvm::MDNode* pKernel = NULL;
        bool bFoundKernel = false;

        // go over the kernels and search for ours
        for (unsigned int i = 0; i < uiNumKernels; ++i)
        {
            pKernel = pKernels->getOperand(i);
            llvm::StringRef szCurrKernelName = pKernel->getOperand(0)->getName();

            if (0 == szCurrKernelName.compare(szKernelName))
            {
                // We found our kernel
                bFoundKernel = true;
                break;
            }
        }

        if (!bFoundKernel)
        {
            throw std::string("couldn't find our kernel in the metadata");
        }

        llvm::MDNode* pAddressQualifiers = NULL;
        llvm::MDNode* pAccessQualifiers = NULL;
        llvm::MDNode* pTypeNames = NULL;
        llvm::MDNode* pTypeQualifiers = NULL;
        llvm::MDNode* pArgNames = NULL;

        for (unsigned int i = 0; i < pKernel->getNumOperands(); ++i)
        {
            llvm::MDNode* pTemp = llvm::dyn_cast<llvm::MDNode>(pKernel->getOperand(i));
            if (NULL == pTemp){
                continue;
            }

            llvm::MDString* pName = llvm::dyn_cast<llvm::MDString>(pTemp->getOperand(0));
            if (NULL == pName) {
                continue;
            }

            if (0 == pName->getString().compare("kernel_arg_addr_space")) {
                pAddressQualifiers = pTemp;
                continue;
            }

            if (0 == pName->getString().compare("kernel_arg_access_qual")) {
                pAccessQualifiers = pTemp;
                continue;
            }

            if (0 == pName->getString().compare("kernel_arg_type")) {
                pTypeNames = pTemp;
                continue;
            }

            if (0 == pName->getString().compare("kernel_arg_type_qual")) {
                pTypeQualifiers = pTemp;
                continue;
            }

            if (0 == pName->getString().compare("kernel_arg_name")) {
                pArgNames = pTemp;
                continue;
            }
        }

        // all of the above must be valid
        if ( !( pAddressQualifiers && pAccessQualifiers && pTypeNames && pTypeQualifiers /*&& pArgNames*/ ) ) {
            assert(pAddressQualifiers && "pAddressQualifiers is NULL");
            assert(pAccessQualifiers && "pAccessQualifiers is NULL");
            assert(pTypeNames && "pTypeNames is NULL");
            assert(pTypeQualifiers && "pTypeQualifiers is NULL");
            //assert(pArgNames && "pArgNames is NULL");
            return -3; //CL_FE_INTERNAL_ERROR_OHNO;
        }

        for (unsigned int i = 1; i < pAddressQualifiers->getNumOperands(); ++i)
        {
            // Since the arg info in the metadata have a string field before the operands
            // Now we have an off by one that we need to compensate for
            CACHED_ARG_INFO argInfo;

            // Address qualifier
            llvm::ConstantInt* pAddressQualifier = llvm::dyn_cast<llvm::ConstantInt>(pAddressQualifiers->getOperand(i));
            assert(pAddressQualifier && "pAddressQualifier is not a valid ConstantInt*");

            uint64_t uiAddressQualifier = pAddressQualifier->getZExtValue();
            switch( uiAddressQualifier )
            {
            case 0:
                argInfo.adressQualifier = CL_KERNEL_ARG_ADDRESS_PRIVATE;
                break;
            case 1:
                argInfo.adressQualifier = CL_KERNEL_ARG_ADDRESS_GLOBAL;
                break;
            case 2:
                argInfo.adressQualifier = CL_KERNEL_ARG_ADDRESS_CONSTANT;
                break;
            case 3:
                argInfo.adressQualifier = CL_KERNEL_ARG_ADDRESS_LOCAL;
                break;
            }

            // Access qualifier
            llvm::MDString* pAccessQualifier = llvm::dyn_cast<llvm::MDString>(pAccessQualifiers->getOperand(i));
            assert(pAccessQualifier && "pAccessQualifier is not a valid MDString");

            if (!pAccessQualifier->getString().compare("none")) {
                argInfo.accessQualifier = CL_KERNEL_ARG_ACCESS_NONE;
            } else if (!pAccessQualifier->getString().compare("read_only")) {
                argInfo.accessQualifier = CL_KERNEL_ARG_ACCESS_READ_ONLY;
            } else if (!pAccessQualifier->getString().compare("write_only")) {
                argInfo.accessQualifier = CL_KERNEL_ARG_ACCESS_WRITE_ONLY;
            } else {
                argInfo.accessQualifier = CL_KERNEL_ARG_ACCESS_READ_WRITE;
            }

            // Type qualifier
            llvm::MDString* pTypeQualifier = llvm::dyn_cast<llvm::MDString>(pTypeQualifiers->getOperand(i));
            assert(pTypeQualifier && "pTypeQualifier is not a valid MDString*");
            argInfo.typeQualifier = 0;
            if (pTypeQualifier->getString().find("const") != llvm::StringRef::npos)
            {
                argInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_CONST;
            }
            if (pTypeQualifier->getString().find("restrict") != llvm::StringRef::npos)
            {
                argInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_RESTRICT;
            }
            if (pTypeQualifier->getString().find("volatile") != llvm::StringRef::npos)
            {
                argInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_VOLATILE;
            }
            if (pTypeQualifier->getString().find("pipe") != llvm::StringRef::npos)
            {
                argInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_PIPE;
            }

            // Type name
            llvm::MDString* pTypeName = llvm::dyn_cast<llvm::MDString>(pTypeNames->getOperand(i));
            assert(pTypeName && "pTypeName is not a valid MDString*");

            argInfo.typeName = pTypeName->getString().str();

            if (pArgNames) {
                // Parameter name
                llvm::MDString* pArgName = llvm::dyn_cast<llvm::MDString>(pArgNames->getOperand(i));
                assert(pArgName && "pArgName is not a valid MDString*");

                argInfo.name = pArgName->getString().str();
            }

            pResult->addInfo(argInfo);
        }

        if( ppResult )
        {
            *ppResult = pResult.take();
        }
        return CL_SUCCESS;
    }
    catch( std::bad_alloc& )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    catch( std::string& )
    {
        return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
    }
}
