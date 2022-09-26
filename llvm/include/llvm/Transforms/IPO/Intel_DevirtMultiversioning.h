#if INTEL_FEATURE_SW_DTRANS
//=-- Intel_DevirtMultiversioning.h - Intel Devirtualization Multiversion -*-=//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This is the header file for a helper class used by the Whole Program
// Devirtualization pass to apply multiversioning.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_DEVIRTMULTIVERSIONING_H
#define LLVM_TRANSFORMS_IPO_INTEL_DEVIRTMULTIVERSIONING_H

#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/Intel_XmainOptLevelPass.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

// Intel specialized print for debugging
#define INTEL_DEVIRT_DEBUG "intel-wholeprogramdevirt"

namespace llvm {

// Helper class used to generate the multiversioning
class IntelDevirtMultiversion {
public:
  IntelDevirtMultiversion(
      Module &M, WholeProgramInfo &WPInfo,
      std::function<const TargetLibraryInfo &(const Function &F)> GetTLI);

  // Try to generate multiple targets with if and else instructions
  // rather than a branch funnel
  bool tryMultiVersionDevirt();

  // Return true if the input virtual call site needs to include the
  // default target during devirtualization since the input target function
  // or the input caller function are libfuncs or external functions.
  bool tryAddingDefaultTargetIntoVCallSite(CallBase *VCallSite,
                                           Function *TargetFunc,
                                           Function *CallerFunc);

  // Find the possible downcasting to prevent devirtualization
  void filterDowncasting(Function *AssumeFunc);

  // Verify that the transformation was done correctly
  void runDevirtVerifier(Module &M);

  // Delete the bitcast for the vtable if there is no use of it.
  void deleteVTableCast(Value *VTablePtr);

  // Return the MDNode related to Intel multiversioning
  MDNode *getDevirtCallMDNode() { return DevirtCallMDNode; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Simplified print of the data collected by the devirtualization process,
  // useful for debugging
  void PrintVTableInfoAndTargets();
#endif // NDEBUG || LLVM_ENABLE_DUMP

  // Return true if multiversion is enabled, else return false
  bool isMultiversionEnabled() { return EnableDevirtMultiversion; }

  // Add a new target function
  void addTarget(Function *Fn);

  // Add a new virtual call
  void addVirtualCallSite(CallBase *VCallSite);

  // Clear all the data related to virtual calls
  void resetData();

private:
  // Structure that contains the information related to a target
  // of a virtual call
  struct TargetData {
    Value *TargetFunc;            // Value of the target function
    BasicBlock *TargetBasicBlock; // Basic block of the target's call site
    Instruction *CallInstruction; // Target's call instruction
    std::string TargetName;       // Basic Block's name
  };

  Module &M;
  WholeProgramInfo &WPInfo;
  std::function<const TargetLibraryInfo &(const Function &F)> GetTLI;
  bool EnableDevirtMultiversion;

  // Metadata node that will be used to mark a function call as being
  // created by the devirtualizer to help DTrans analyze bitcast function calls.
  MDNode *DevirtCallMDNode = nullptr;

  // Helper function to generate the branches for multiversioning
  void multiversionVCallSite(Module &M, CallBase *VCallSite, bool LibFuncFound,
                             const SetVector<Function *> &TargetFunctions);

  // Compute the basic block where all targets will jump after executing
  // the call instruction
  BasicBlock *getMergePoint(Module &M, CallBase *VCallSite);

  // Create the call sites and basic blocks for each target. Return false if
  // all the target functions have the same llvm::FunctionType as VCallSite,
  // else return true.
  bool createCallSiteBasicBlocks(Module &M,
                                 std::vector<TargetData *> &TargetVector,
                                 CallBase *VCallSite,
                                 const SetVector<Function *> &TargetFunctions,
                                 MDNode *Node);

  // Build the basic block for the default case
  TargetData *buildDefaultCase(Module &M, CallBase *VCallSite);

  // Fix the PHINodes in the unwind destinations
  void fixUnwindPhiNodes(CallBase *VCallSite, BasicBlock *MergePointBB,
                         std::vector<TargetData *> &TargetsVector,
                         TargetData *DefaultTarget, bool LibFuncFound);

  // Connect all targets with the if/else instructions
  void generateBranching(Module &M, BasicBlock *MainBB,
                         BasicBlock *MergePointBB, bool IsCallInst,
                         std::vector<TargetData *> &TargetsVector,
                         TargetData *DefaultTarget, bool LibFuncFound);

  // Replace the Users of the virtual call with a PHINode
  void generatePhiNodes(Module &M, BasicBlock *MergePointBB,
                        std::vector<TargetData *> TargetsVector,
                        TargetData *DefaultTarget, bool LibFuncFound);

  // Return true if the input function is a libfunc
  bool functionIsLibFuncOrExternal(Function *F);

  // Return true if the target function's type is the same as the virtual call
  // function's type.
  bool basedDerivedFunctionTypeMatches(FunctionType *VCallType,
                                       FunctionType *TargetFuncType);

  // Structure to store the basic information needed by the multiversioning
  struct VirtualCallsDataForMV {
    SetVector<Function *> TargetFunctions;    // Vector of Function* with
                                              //   the targets
    std::vector<CallBase *> VirtualCallSites; // vector of CallBase* with
                                              //   the virtual callsites
    bool HasLibFuncAsTarget;                  // True if at least one target
                                              //   is a libfunc or an
                                              //   external function , else
                                              //   false
  };

  void collectAssumeCallSitesNonOpaque(Function *AssumeFunc,
                                       std::vector<CallBase *> &AssumesVector);

  void collectAssumeCallSitesOpaque(Function *AssumeFunc,
                                    std::vector<CallBase *> &AssumesVector,
                                    dtransOP::PtrTypeAnalyzer &Analyzer,
                                    dtransOP::TypeMetadataReader &MDReader);

  VirtualCallsDataForMV VCallsData;

  SetVector<CallBase *> VCallsWithDefaultCase;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_DEVIRTMULTIVERSIONING_H

#endif // INTEL_FEATURE_SW_DTRANS
