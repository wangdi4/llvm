/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  WorkItemStorage.h

\*****************************************************************************/

#ifndef  LLI_WORK_ITEM_STORAGE_H
#define LLI_WORK_ITEM_STORAGE_H

#include <vector>
#include <llvm/DerivedTypes.h>
#include "IWorkItemBuiltins.h"

namespace Validation{

/// helper class for storing work-item related stuff for passing between 
/// Open CL Reference runner and Reference WorkItem built-ins
class WorkItemStorage : public OCLBuiltins::IWorkItemBuiltins
{
public:
    /// maximum number of dimensions of workitems
    static const uint32_t MAX_OCL_DIM = 3;

    // ctor
    WorkItemStorage(const uint32_t WorkDim, const std::vector<uint64_t>& GlobalSize,
        const std::vector<uint64_t>& LocalSize, const std::vector<uint64_t>& GlobalOffset)
        : m_WorkDim(WorkDim), m_GlobalSize(GlobalSize), m_GlobalID(MAX_OCL_DIM, 0), 
        m_LocalSize(LocalSize), m_LocalID(MAX_OCL_DIM, 0),
        m_GlobalOffset(GlobalOffset)
        
    {}

    /// IWorkItemBuiltins interface functions
    virtual uint32_t GetWorkDim(){
        return m_WorkDim;
    }
    
    virtual uint64_t GetGlobalSize(const uint32_t dimindx){
        return m_GlobalSize[dimindx];
    }
    
    virtual uint64_t GetLocalSize(const uint32_t dimindx){
        return m_LocalSize[dimindx];
    }
    
    virtual uint64_t GetGlobalIdNoOffset(const uint32_t dimindx){
        return m_GlobalID[dimindx];
    }

    virtual uint64_t GetLocalId(const uint32_t dimindx){
        return m_LocalID[dimindx];
    }
  
    virtual uint64_t GetGlobalOffset(const uint32_t dimindx){
        return m_GlobalOffset[dimindx];
    }

    void SetGlobalID(const uint32_t dimindx, const uint64_t& val){
        m_GlobalID[dimindx] = val;
    }

    void SetLocalID(const uint32_t dimindx, const uint64_t& val){
        m_LocalID[dimindx] = val;
    }

private:
    uint32_t m_WorkDim;
    std::vector<uint64_t> m_GlobalSize;
    std::vector<uint64_t> m_GlobalID;
    std::vector<uint64_t> m_LocalSize;
    std::vector<uint64_t> m_LocalID;
    std::vector<uint64_t> m_GlobalOffset;
};

} // namespace Validation
#endif // LLI_WORK_ITEM_STORAGE_H
