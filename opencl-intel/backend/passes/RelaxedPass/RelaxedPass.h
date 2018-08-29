// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __RELAXEDPASS_H__
#define __RELAXEDPASS_H__

#include <llvm/Pass.h>

#include <map>

namespace intel{
    using namespace llvm;

    /// @brief This pass is used for relaxed functions substitution
    class RelaxedPass : public ModulePass
    {
    public:
        typedef std::map<const StringRef, const StringRef> FRMMap;

        /// @brief Constructor, add mapping for relaxed functions
        RelaxedPass();

        /// @brief doPassInitialization - For this pass, it removes global symbol table
        ///        entries for primitive types.  These are never used for linking in GCC and
        ///        they make the output uglier to look at, so we nuke them.
        ///        Also, initialize instance variables.
        bool runOnModule(Module &M);

        /// @brief Pass identification, replacement for typeid
        static char ID;

    private:
        /// @brief Map for relaxed functions substitution
        FRMMap m_relaxedFunctions;

        /// @brief Map for OCL 2.0 relaxed functions desribed in table 7.2
        FRMMap m_relaxedFunctions_2_0;

        /// @brief This functions selects which mapping is to be used. The decision
        ///        is based on the OpenCL C version taken from the module
        FRMMap const& selectFRMMap(llvm::Module const& M) const;
    };

}// namespace intel
#endif // __RELAXEDPASS_H__
