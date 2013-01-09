/////////////////////////////////////////////////////////////////////////
// RelaxedPass.h:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2009 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel’s prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////

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
        /// @brief Constructor, add mapping for relaxed functions
        RelaxedPass();

        /// @brief doPassInitialization - For this pass, it removes global symbol table
        ///        entries for primitive types.  These are never used for linking in GCC and
        ///        they make the output uglier to look at, so we nuke them.
        ///        Also, initialize instance variables.
        bool runOnModule(Module &M);

        /// @brief Pass identification, replacement for typeid
        static char ID;

    protected:
        /// @brief Map for relaxed functions substitution
        std::map<std::string, std::string> m_relaxedFunctions;
    };

}// namespace intel
#endif // __RELAXEDPASS_H__
