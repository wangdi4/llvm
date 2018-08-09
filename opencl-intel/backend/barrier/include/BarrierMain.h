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

#ifndef __BARRIER_MAIN_H__
#define __BARRIER_MAIN_H__

#include "debuggingservicetype.h"
#include "BuiltinLibInfo.h"
#include "llvm/Pass.h"
#include <map>
#include <string>

using namespace llvm;

namespace intel {


  /// @brief BarrierMain pass is a module pass that handles the whole
  /// barriers passes. It simply calls all other passes for you.
  class BarrierMain : public ModulePass {

  public:
    static char ID;

    /// @brief C'tor
    /// @param debugType the type of debugging support enabled
    BarrierMain(DebuggingServiceType debugType);

    /// @brief D'tor
    ~BarrierMain() {}

    /// @brief Provides name of pass
    virtual llvm::StringRef getPassName() const {
      return "Intel OpenCL BarrierMain";
    }

    /// @brief execute pass on given module
    /// @param M module to optimize
    /// @returns True if module was modified
    virtual bool runOnModule(Module &M);

    /// @brief Inform about usage/mofication/dependency of this pass
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<BuiltinLibInfo>();
    }

    /// @brief return special buffer stride size map
    /// @param bufferStrideMap - the map to output all data into
    void getStrideMap(std::map<std::string, unsigned int>& bufferStrideMap) {
      bufferStrideMap.clear();
      bufferStrideMap.insert(m_bufferStrideMap.begin(), m_bufferStrideMap.end());
    }
  private:
    /// The type of debugging service enabled
    DebuggingServiceType m_debugType;

    /// This holds a map between kernel function name and buffer stride size
    std::map<std::string, unsigned int> m_bufferStrideMap;

  };

} // namespace intel

#endif // __BARRIER_MAIN_H__

