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

File Name:  IWorkItemBuiltins.h

\*****************************************************************************/
#ifndef I_WORK_ITEM_BUILTINS
#define I_WORK_ITEM_BUILTINS

#include <cassert>
#include <llvm/IR/DerivedTypes.h>

namespace Validation {
namespace OCLBuiltins {

/// Interface for setting/getting work-group built-in variables
class IWorkGroupBuiltins
{
public:

    ///Report that someone is referencing to instance of class
    ///if object locked by someone for first time - initialize it
    ///@param [in] string lock  built-in who is referencing to instance
    ///@param [in] GenericValue init initialize with this value on very first run
    virtual void AddRef(const std::string& lock, const llvm::GenericValue init)=0;

    ///Report that reference to instance of supreclass is no longer needed.
    virtual void DecRef()=0;

    ///Get reference to accumulative value. Needed to collect data
    ///for work-group built-ins work_group_any, work_group_all
    ///@return reference to llvm::GenericValue accumulative value for
    ///work_group_all and work_group_all built-ins
    virtual llvm::GenericValue& GetValueForWorkGroupAllAnyBuiltin()=0;

    ///Get reference to accumulative value. Needed to collect data
    ///for work-group built-ins work_group_broadcast
    ///@return reference to llvm::GenericValue accumulative value for
    ///work_group_broadcast builtins
    virtual llvm::GenericValue& GetValueForBroadcastBuiltin()=0;

    ///Get reference to accumulative value. Needed to collect data
    ///for work-group built-ins work_group_reduce_<op>
    ///@return reference to llvm::GenericValue accumulative value for
    ///work_group_reduce_<op> built-ins
    virtual llvm::GenericValue& GetValueForReduceBuiltin()=0;

    ///Get reference to accumulative value. Needed to collect data
    ///for work-group built-ins work_group_prefixsum_<inclusive/exclusive>_<op>
    ///@return reference to llvm::GenericValue accumulative value for
    ///work_group_prefixsum built-ins
    virtual llvm::GenericValue& GetValueForPrefixSumBuiltin()=0;
};

/// Interface for setting/getting work-item built-in variables
/// This interface is intended to pass work-item variables between OpenCL ReferenceRunner layer
/// and work-item built-in implementation in OpenCL Reference.
/// OpenCL 1.1 sec 6.11.1
class IWorkItemBuiltins
{
public:
    
    /// Get number of dimensions in use for get_work_dim() built-in
    /// @return uint32_t type
    virtual uint32_t GetWorkDim() = 0;
    
    /// Get global size for get_global_size() built-in
    /// @param [in]     dimindx dimension number
    virtual uint64_t GetGlobalSize(const uint32_t dimindx) = 0;
    
    /// Get local size for get_local_size() built-in
    /// @param [in]     dimindx dimension number
    virtual uint64_t GetLocalSize(const uint32_t dimindx) = 0;
    
    /// Get global ID without global offset (!!!)  for get_global_id() built-in
    /// global offset should be added inside built-in implementation
    /// @param [in]     dimindx dimension number
    virtual uint64_t GetGlobalIdNoOffset(const uint32_t dimindx) = 0;

    /// Get local ID for get_local_id() built-in
    /// @param [in]     dimindx dimension number
    virtual uint64_t GetLocalId(const uint32_t dimindx) = 0;

    /// Get enqueued local ID for get_enqueued_local_size() built-in
    /// @param [in]     dimindx dimension number
    virtual uint64_t getEnqueuedLocalSize(const uint32_t dimindx) = 0;
  
    /// Get global offset for get_global_offset() built-in
    /// this offset will be added to global ID
    /// @param [in]     dimindx dimension number
    virtual uint64_t GetGlobalOffset(const uint32_t dimindx) = 0;

};

/// singleton class to set interface for workitem built-ins to obtain workgroup IDs
class WorkItemInterfaceSetter
{
public:
    /// obtain pointer to object
    static WorkItemInterfaceSetter* inst() {
        if(!m_pInst) m_pInst = new WorkItemInterfaceSetter;
        assert(m_pInst !=NULL);
        return m_pInst;
    }

    /// Set interface to work-items
    void SetWorkItemInterface(IWorkItemBuiltins * p){
        assert(p);
        m_pWorkItemBuiltins = p;
    }

    /// Set interface to work-group
    void SetWorkGroupInterface(IWorkGroupBuiltins * p){
        assert(p);
        m_pWorkGroupBuiltins = p;
    }

    /// Get interface to work-items
    IWorkItemBuiltins * GetWorkItemInterface(){
        assert(m_pWorkItemBuiltins);
        return m_pWorkItemBuiltins;
    }

    /// Get interface to work-group
    IWorkGroupBuiltins * GetWorkGroupInterface(){
        assert(m_pWorkGroupBuiltins);
        return m_pWorkGroupBuiltins;
    }

protected:
    /// hide ctor and dtor
    WorkItemInterfaceSetter() : m_pWorkItemBuiltins(NULL), m_pWorkGroupBuiltins(NULL) {};
    ~WorkItemInterfaceSetter(){};
private:
    /// hide ctors 
    WorkItemInterfaceSetter(WorkItemInterfaceSetter const&);
    WorkItemInterfaceSetter& operator=(WorkItemInterfaceSetter const&);
    /// static pointer to instance
    static WorkItemInterfaceSetter* m_pInst;
    /// workgroup interface
    IWorkItemBuiltins * m_pWorkItemBuiltins;
    IWorkGroupBuiltins * m_pWorkGroupBuiltins;
};

} // namespace Validation
} // namespace OCLBuiltins 

#endif // I_WORK_ITEM_BUILTINS
