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

///////////////////////////////////////////////////////////
//  elf_binary.h
///////////////////////////////////////////////////////////

#include "CLElfTypes.h"
#include "elf_binary.h"

namespace Intel{ namespace OpenCL{ namespace ELFUtils{
    bool OCLElfBinaryReader::IsValidOpenCLBinary(const char* pBinary, size_t uiBinarySize)
    {
        if( CLElfLib::CElfReader::IsValidElf64((const char*)pBinary, uiBinarySize))
        {
            ElfReaderPtr pReader(CLElfLib::CElfReader::Create((const char*)pBinary, uiBinarySize));
            switch( pReader->GetElfHeader()->Type)
            {
                case CLElfLib::EH_TYPE_OPENCL_OBJECTS   :
                case CLElfLib::EH_TYPE_OPENCL_LIBRARY   :
                case CLElfLib::EH_TYPE_OPENCL_LINKED_OBJECTS:
                    return true;
            }
        }
        return false;
    }

    OCLElfBinaryReader::OCLElfBinaryReader(const char* pBinary, size_t uiBinarySize)
        :m_pReader(CLElfLib::CElfReader::Create(pBinary, uiBinarySize))
    {
        assert(IsValidOpenCLBinary(pBinary, uiBinarySize) && "invalid opencl binary");
        if( nullptr == m_pReader.get() )
        {
            throw std::bad_alloc();
        }
    }

    void OCLElfBinaryReader::GetIR(char*& pData, size_t&uiSize) const
    {
        if(CLElfLib::SUCCESS != m_pReader->GetSectionData(".ocl.ir", pData, uiSize))
        {
            throw "no .ocl.ir section";
        }
    }

    cl_prog_binary_type OCLElfBinaryReader::GetBinaryType() const
    {
        switch( m_pReader->GetElfHeader()->Type)
        {
            case CLElfLib::EH_TYPE_OPENCL_OBJECTS   :
                return CL_PROG_BIN_COMPILED_LLVM;
            case CLElfLib::EH_TYPE_OPENCL_LIBRARY   :
                return CL_PROG_BIN_LINKED_LLVM;
            case CLElfLib::EH_TYPE_OPENCL_LINKED_OBJECTS:
                return CL_PROG_BIN_EXECUTABLE_LLVM;
        }
        throw "unsupported binary type";
    }
}}} //namespace Intel::OpenCL::ELFUtils
