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

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "common_clang.h"
#include "llvm/ADT/SmallVector.h"
#include <string>

class OCLFEBinaryResult:public Intel::OpenCL::ClangFE::IOCLFEBinaryResult
{
    // IOCLFEBinaryResult
public:

    virtual size_t GetIRSize() const
    {
        return m_IRBuffer.size();
    }

    virtual const void* GetIR() const
    {
        return m_IRBuffer.data();
    }

    virtual const char* GetIRName() const
    {
        return m_IRName.c_str();
    }

    virtual Intel::OpenCL::ClangFE::IR_TYPE GetIRType() const
    {
        return m_type;
    }

    virtual const char* GetErrorLog() const
    {
        return m_log.c_str();
    }

    virtual void Release()
    {
        delete this;
    }
    // OCLFEBinaryResult
public:
    OCLFEBinaryResult():
        m_type(Intel::OpenCL::ClangFE::IR_TYPE_UNKNOWN),
        m_result(CL_SUCCESS)
    {
    }

    llvm::SmallVectorImpl<char>& getIRBufferRef()
    {
        return m_IRBuffer;
    }

    std::string& getLogRef()
    {
        return m_log;
    }

    void setLog( const std::string& log )
    {
        m_log = log;
    }

    void setIRName( const std::string& name)
    {
        m_IRName = name;
    }

    void setIRType( Intel::OpenCL::ClangFE::IR_TYPE type)
    {
        m_type = type;
    }

    void setResult(int result)
    {
        m_result = result;
    }

    int getResult(void) const
    {
        return m_result;
    }

private:
    llvm::SmallVector<char, 4096> m_IRBuffer;
    std::string m_log;
    std::string m_IRName;
    Intel::OpenCL::ClangFE::IR_TYPE m_type;
    int m_result;
};
