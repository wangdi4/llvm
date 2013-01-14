/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __RELAXEDPASS_H__
#define __RELAXEDPASS_H__

#include <llvm/Pass.h>

#include <map>

namespace Intel { namespace OpenCL { namespace DeviceBackend {
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

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {
#endif // __RELAXEDPASS_H__
