/*****************************************************************************\

Copyright (c) Intel Corporation (2011,2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  LinkData.h

\*****************************************************************************/
#ifndef __LINKDATA_H__
#define __LINKDATA_H__

#include <vector>
#include <string>

namespace Intel { namespace OpenCL { namespace ClangFE {
    struct IOCLFEBinaryResult;
}}}

using namespace Intel::OpenCL::ClangFE;

namespace Intel { namespace OpenCL { namespace Frontend {
//
//Description:
//  Represents the data used by a link operation.
class LinkData
{
//typedefs
public:
    typedef std::pair<const void*, size_t> BufferInfo;

//methods
public:
    virtual ~LinkData(){}

    void addInputBuffer(const void* pBuffer, size_t size)
    {
        m_inputBuffers.push_back( BufferInfo(pBuffer, size) );
    }

    size_t inputBuffersCount() const
    {
        return m_inputBuffers.size();
    }

    std::vector<BufferInfo>::const_iterator beginInputBuffers() const
    {
        return m_inputBuffers.begin();
    }

    std::vector<BufferInfo>::const_iterator endInputBuffers() const
    {
        return m_inputBuffers.end();
    }

    void setOptions(const char* pszOptions)
    {
        if( pszOptions )
        {
            m_options = pszOptions;
        }
    }

    void setBinaryResult(IOCLFEBinaryResult* pResult)
    {
        m_pResult = pResult;
    }

    IOCLFEBinaryResult* getBinaryResult() const
    {
        return m_pResult;
    }

private:
    std::vector<BufferInfo> m_inputBuffers;
    IOCLFEBinaryResult* m_pResult;
    std::string m_options;
};
}}}

#endif
