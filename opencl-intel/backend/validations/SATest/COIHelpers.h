/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  COIHelpers.h

\*****************************************************************************/
#ifndef COI_HELPERS_H
#define COI_HELPERS_H

#include "OpenCLRunConfiguration.h"

// COI library headers
#include <source/COIProcess_source.h>
#include <source/COIPipeline_source.h>
#include <source/COIEngine_source.h>
#include "source/COIBuffer_source.h"
#include "source/COIEvent_source.h"

#include <vector>
#include <string>

namespace Validation
{
#define CHECK_COI_RESULT(_COIFUNC)                                                  \
    {                                                                               \
    COIRESULT result = _COIFUNC;                                                    \
    if (COI_SUCCESS != result)                                                      \
    {                                                                               \
        throw Exception::COIUsageException(std::string(#_COIFUNC" retruned ") +     \
            std::string(COIResultGetName(result)));                                 \
    }                                                                               \
}

#define CHECK_COI_RESULT_NO_THROW(_COIFUNC)                                         \
{                                                                                   \
    COIRESULT result = _COIFUNC;                                                    \
    if (COI_SUCCESS != result)                                                      \
    {                                                                               \
        std::cerr << #_COIFUNC" retruned " << COIResultGetName(result) << "\n";     \
    }                                                                               \
}

    // Warning! This wrapper is tunned to be used in OpenCLMICBackendRunner only 
    // and it supports only limited usage scenarios.
    class COIProcessAndPipelineWrapper
    {
    public:
        COIProcessAndPipelineWrapper():m_created(false){}
        void Create(COIENGINE engine, const BERunOptions *pRunConfig);
        ~COIProcessAndPipelineWrapper(void);
        COIPROCESS&  GetProcessHandler()  {return m_process;}
        COIPIPELINE& GetPipelineHandler() {return m_pipeline;}
        COI_ISA_TYPE GetCOIISAType(std::string cpuArch);
    private:
        bool m_created;
        COIPROCESS  m_process;
        COIPIPELINE m_pipeline;
        COILIBRARY  m_library; // SVML built-ins library
    };

    // Warning! This wrapper is tunned to be used in OpenCLMICBackendRunner only 
    // and it supports only limited usage scenarios. For instance there is no way to map more then one buffer.
    class COIBuffersWrapper
    {
    public:
        COIBuffersWrapper(){}
        void AddBuffer( size_t bufferSize, const COIPROCESS process, COI_ACCESS_FLAGS access, uint32_t flags = 0, void* pInitData = NULL );
        void CreateBufferFromMemory( size_t bufferSize, const COIPROCESS process, COI_ACCESS_FLAGS access, void* pData );
        void Map( COI_MAP_TYPE mapType, int numOfDepends, COIEVENT* dependencies, void** data, size_t id );
        void UnMap( );
        size_t GetNumberOfBuffers() const {return m_buffers.size();}
        ~COIBuffersWrapper( void );
        COIBUFFER* GetBufferHandler(size_t id) {return &(m_buffers[id]);}
        COI_ACCESS_FLAGS* GetBufferAccessFlags(size_t id) {return &(m_flags[id]);}
    private:
        // TODO: implement correct copy constructor.
        // HACK!!! Prohibit copying because if it's copied after AddBuffer call Destroy method will be called twice.
        COIBuffersWrapper(const COIBuffersWrapper&){}

        std::vector<COIBUFFER> m_buffers;
        std::vector<COI_ACCESS_FLAGS> m_flags;
        COIMAPINSTANCE m_map;
    };
}

#endif // COI_HELPERS_H
