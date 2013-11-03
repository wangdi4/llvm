/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  WorkGroupStorage.h

\*****************************************************************************/

#ifndef  WORK_GROUP_STORAGE_H
#define WORK_GROUP_STORAGE_H

#include <vector>
#include <llvm/IR/DerivedTypes.h>
#include "IWorkItemBuiltins.h"
#include <llvm/ADT/APInt.h>

namespace Validation{

/// helper class for storing work-group related stuff for passing between 
/// Open CL Reference runner and Reference WorkGroup built-ins
class WorkGroupStorage : public OCLBuiltins::IWorkGroupBuiltins
{
public:
    /// ctor
    WorkGroupStorage(): m_RefCounter(0), m_LockedBy(std::string()){
    }

    ///Get reference to accumulative value. Needed to collect data
    ///for work-group built-ins. specific function for work_group_all.
    ///work_group_any builtins
    ///@return reference to llvm::GenericValue
    virtual llvm::GenericValue& GetValueForWorkGroupAllAnyBuiltin(){
        return m_Predicate;
    }

    ///Get reference to accumulative value. Needed to collect data
    ///for work-group built-ins. specific function for work_group_broadcast builtin
    ///@return reference to llvm::GenericValue
    virtual llvm::GenericValue& GetValueForBroadcastBuiltin(){
        return m_BroadcastedValue;
    }

    ///Get reference to accumulative value. Needed to collect data
    ///for work-group built-ins. specific function for work_group_reduce builtin
    ///@return reference to llvm::GenericValue
    virtual llvm::GenericValue& GetValueForReduceBuiltin(){
        return m_ReduceValue;
    }

    ///Get reference to accumulative value. Needed to collect data
    ///for work-group built-ins. specific function for work_group_prefixsum builtins
    ///@return reference to llvm::GenericValue
    virtual llvm::GenericValue& GetValueForPrefixSumBuiltin(){
        return m_PrefixSumValue;
    }

    ///Report that someone is referencing to instance of class
    ///@param [in] string lock  built-in who is referencing to instance
    ///@param [in] GenericValue init initialize with this value on very first run
    virtual void AddRef(const std::string& lock, const llvm::GenericValue init){
        if(m_LockedBy!=std::string()&&m_RefCounter)
            assert(lock==m_LockedBy&&"Two different work group builtins \
                                     are executing in the same time");
        if(!m_RefCounter)
            initializeWithValue(init);

        m_LockedBy = lock;
        ++m_RefCounter;
    }

    ///Report that reference to instance of supreclass is no longer needed.
    ///reinitializes object with zero referenced objects
    virtual void DecRef(){
        --m_RefCounter;
    }
private:
    ///initialize accumulators with given value
    ///@param [in] GenericValue init 
    void initializeWithValue(const llvm::GenericValue init){
        m_Predicate=init;
        m_BroadcastedValue=init;
        m_ReduceValue=init;
        m_PrefixSumValue=init;
    }

    ///accumulative variable stored within work group
    ///used by work_group_any(int) and work_group_all(int)
    llvm::GenericValue m_Predicate;
    ///accumulative variable stored within work group
    ///used by broadcast work group built-ins
    llvm::GenericValue m_BroadcastedValue;
    ///accumulative variable stored within work group
    ///used by reduce work group built-in
    llvm::GenericValue m_ReduceValue;
    ///accumulative variable stored within work group
    ///used by prefixsum work group built-ins
    llvm::GenericValue m_PrefixSumValue;
    ///reference counter
    ///how many work-items is executing work-group builtin in the same time
    int32_t m_RefCounter;
    ///name of built-in executing by work-items
    std::string m_LockedBy;
};

} // namespace Validation
#endif // WORK_GROUP_STORAGE_H
