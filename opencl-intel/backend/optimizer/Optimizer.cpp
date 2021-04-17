// INTEL CONFIDENTIAL
//
// Copyright 2012-2020 Intel Corporation.
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

#include "Optimizer.h"
#include "ChannelPipeUtils.h"
#include "CompilationUtils.h"
#include "InitializeOCLPasses.hpp"
#include "MetadataAPI.h"
#include "OCLAliasAnalysis.h"
#include "OclTune.h"
#include "VecConfig.h"
#include "debuggingservicetype.h"
#include "PrintIRPass.h"
#include "mic_dev_limits.h"

#include "cl_cpu_detect.h"

#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/InferFunctionAttrs.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/InstSimplifyPass.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

// TODO:
// #include "llvm/Transforms/Intel_OpenCLTransforms/Passes.h"
llvm::FunctionPass* createFMASplitterPass();

#include "llvm/Transforms/Intel_MapIntrinToIml/MapIntrinToIml.h"
#include "llvm/Transforms/Utils/Intel_VecClone.h"
#include "llvm/Transforms/VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/VPO/VPOPasses.h"
#include "llvm/Transforms/Vectorize.h"
#include "llvm/Transforms/Vectorize/VectorCombine.h"

static cl::opt<bool>
    DisableVPlanCM("disable-ocl-vplan-cost-model",
                   cl::init(false), cl::Hidden,
                   cl::desc("Disable cost model for VPlan vectorizer"));

// This flag enables VPlan for OpenCL.
static cl::opt<bool>
    EnableVPlan("enable-vplan-kernel-vectorizer", cl::init(true),
                cl::Hidden,
                cl::desc("Enable VPlan Kernel Vectorizer"));

// Enables kernel vectorizer identification message.
static cl::opt<bool>
    EmitKernelVectorizerSignOn("emit-kernel-vectorizer-sign-on",
                              cl::init(false),
                              cl::Hidden,
                              cl::desc("Emit which vectorizer is used "
                                       "(Volcano or Vplan)"));

// TODO: The switch is required until subgroup implementation passes
// the conformance test fully (meaning that masked kernel is integrated).
static cl::opt<bool>
    EnableNativeOpenCLSubgroups("enable-native-opencl-subgroups", cl::init(false),
                                cl::Hidden,
                                cl::desc("Enable native subgroup functionality"));

using TStringToVFState = Intel::OpenCL::DeviceBackend::TStringToVFState;
using CPUDetect = Intel::OpenCL::Utils::CPUDetect;

extern "C"{

void *createInstToFuncCallPass(const CPUDetect *CpuId);
FunctionPass *createWeightedInstCounter(bool, const CPUDetect *);
FunctionPass *createScalarizerPass(const CPUDetect *CpuId,
                                   bool InVPlanPipeline);
llvm::Pass *createVectorizerPass(SmallVector<Module *, 2> builtinModules,
                                 const intel::OptimizerConfig *pConfig);
llvm::Pass *createOCLReqdSubGroupSizePass();
llvm::Pass *createOCLVecClonePass(const CPUDetect *, bool IsOCL);
llvm::Pass *createOCLVPOCheckVFPass(const intel::OptimizerConfig &Config,
                                    TStringToVFState &kernelVFStates);
llvm::Pass *createOCLPostVectPass();
llvm::Pass *createImplicitGIDPass(bool HandleBarrier);
llvm::Pass *createBarrierMainPass(unsigned OptLevel, intel::DebuggingServiceType debugType,
                                  bool useTLSGlobals);
llvm::ModulePass* createCreateSimdVariantPropagationPass();
llvm::ModulePass *createSGSizeCollectorPass(const CPUDetect *CPUId);
llvm::ModulePass *createSGSizeCollectorIndirectPass(const CPUDetect *CPUId);
llvm::ModulePass *createVectorVariantLoweringPass(const CPUDetect *CPUId);
llvm::ModulePass* createVectorVariantFillInPass();
llvm::ModulePass* createUpdateCallAttrsPass();
llvm::ModulePass* createIndirectCallLoweringPass();

llvm::ModulePass *createInfiniteLoopCreatorPass();
llvm::ModulePass *createAutorunReplicatorPass();
llvm::ModulePass *createCLWGLoopCreatorPass();
llvm::ModulePass *createCLWGLoopBoundariesPass();
llvm::Pass *createCLBuiltinLICMPass();
llvm::Pass *createLoopStridedCodeMotionPass();
llvm::Pass *createCLStreamSamplerPass();
llvm::Pass *createPreventDivisionCrashesPass();
llvm::Pass *createOptimizeIDivPass();
llvm::Pass *createShiftZeroUpperBitsPass();
llvm::Pass *createBuiltinCallToInstPass();
llvm::Pass *createRelaxedPass();
llvm::Pass *createLinearIdResolverPass();
llvm::ModulePass *createSubGroupAdaptationPass();
llvm::ModulePass *createKernelAnalysisPass();
llvm::ModulePass *createHandleVPlanMaskPass();
llvm::ModulePass *createBuiltInImportPass(const char *CPUName);
llvm::ImmutablePass *createImplicitArgsAnalysisPass(llvm::LLVMContext *C);
llvm::ModulePass *createChannelPipeTransformationPass();
llvm::ModulePass *createPipeIOTransformationPass();
llvm::ModulePass *createCleanupWrappedKernelsPass();
llvm::ModulePass *createPipeOrderingPass();
llvm::ModulePass *createPipeSupportPass();
llvm::ModulePass *createLocalBuffersPass(bool isNativeDebug,
                                         bool useTLSGlobals);
llvm::ModulePass *createAddImplicitArgsPass();
llvm::ModulePass *createOclFunctionAttrsPass();
llvm::ModulePass *createOclSyncFunctionAttrsPass();
llvm::ModulePass *createInternalizeNonKernelFuncPass();
llvm::ModulePass *createExternalizeGlobalVariablesPass();
llvm::ModulePass *createInternalizeGlobalVariablesPass();
llvm::ModulePass *createGenericAddressStaticResolutionPass();
llvm::ModulePass *createGenericAddressDynamicResolutionPass();
llvm::ModulePass *createPrepareKernelArgsPass(bool useTLSGlobals);
llvm::Pass *
createBuiltinLibInfoPass(SmallVector<Module *, 2> pRtlModuleList,
                         std::string type);
llvm::ModulePass *createUndifinedExternalFunctionsPass(
    std::vector<std::string> &undefinedExternalFunctions);
llvm::ModulePass *createKernelInfoWrapperPass();
llvm::ModulePass *createKernelSubGroupInfoPass();
llvm::ModulePass *createDuplicateCalledKernelsPass();
llvm::ModulePass *createPatchCallbackArgsPass(bool useTLSGlobals);
llvm::ModulePass *createDeduceMaxWGDimPass();

llvm::ModulePass *createLLVMEqualizerPass();
llvm::FunctionPass *createPrefetchPassLevel(int level);
llvm::ModulePass *createRemovePrefetchPass();
llvm::ModulePass *createPrintIRPass(int option, int optionLocation,
                                    std::string dumpDir);
llvm::ModulePass *createDebugInfoPass();
llvm::ModulePass *createProfilingInfoPass();
llvm::Pass *createSmartGVNPass(bool);

llvm::ModulePass *createSinCosFoldPass();
llvm::ModulePass *createResolveWICallPass(bool isUniformWGSize,
                                          bool useTLSGlobals);
llvm::Pass *createResolveSubGroupWICallPass(bool ResolveSGBarrier);
llvm::ModulePass *createDetectRecursionPass();
llvm::Pass *createResolveBlockToStaticCallPass();
llvm::ImmutablePass *createOCLAliasAnalysisPass();
llvm::ModulePass *createPrintfArgumentsPromotionPass();
llvm::ModulePass *createChannelsUsageAnalysisPass();
llvm::ModulePass *createSYCLPipesHackPass();
llvm::ModulePass *createAddTLSGlobalsPass();
llvm::ModulePass *createCoerceTypesPass();
llvm::ModulePass *createRemoveAtExitPass();
llvm::FunctionPass *createAddNTAttrPass();
llvm::ModulePass *createChooseVectorizationDimensionModulePass();
llvm::ModulePass *createCoerceWin64TypesPass();
llvm::FunctionPass *createAddFastMathPass();
llvm::Pass *createResolveVariableTIDCallPass();
llvm::ModulePass *createVectorKernelDiscardPass(const intel::OptimizerConfig *);
}

using namespace intel;
namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

// Several PMs are populated in the Optimizer flow:
// Materializer, PreFail and PostFail PMs.
// In order to dump IR for debug purposes one can schedule PrintModulePass
// to a Pass Manager of choice.

/// createStandardModulePasses - Add the standard module passes.  This is
/// expected to be run after the standard function passes.
static inline void createStandardLLVMPasses(llvm::legacy::PassManagerBase *PM,
                                            unsigned OptLevel, bool UnitAtATime,
                                            bool UnrollLoops,
                                            int rtLoopUnrollFactor,
                                            bool allowAllocaModificationOpt,
                                            bool /*UseVplan*/) {
  if (OptLevel == 0) {
    return;
  }

  if (UnitAtATime) {
    // If a function has internal linkage type this pass can eliminate one or
    // even more arguments in a function call. Due to VPO passes are split
    // in the optimizer that will lead to a mismatch between number of parameters in
    // the function callee and its vectorized form. Therefore, this pass should
    // be launched before VPO.
    PM->add(llvm::createDeadArgEliminationPass());
  }

  if (UnitAtATime) {
    PM->add(llvm::createGlobalOptimizerPass());    // Optimize out global vars
    PM->add(llvm::createIPSCCPPass());             // IP SCCP
  }

  PM->add(llvm::createInstSimplifyLegacyPass());
  PM->add(llvm::createInstructionCombiningPass()); // Clean up after IPCP & DAE
  PM->add(llvm::createCFGSimplificationPass());    // Clean up after IPCP & DAE

  // Set readonly/readnone attrs
  if (UnitAtATime)
    PM->add(llvm::createInferFunctionAttrsLegacyPass());
  if (OptLevel > 2)
    PM->add(llvm::createArgumentPromotionPass()); // Scalarize uninlined fn args

  // Break up aggregate allocas
  PM->add(llvm::createSROAPass());
  PM->add(llvm::createEarlyCSEPass());           // Catch trivial redundancies
  PM->add(llvm::createInstSimplifyLegacyPass());
  PM->add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
  PM->add(llvm::createJumpThreadingPass());        // Thread jumps.
  // Propagate conditionals
  PM->add(llvm::createCorrelatedValuePropagationPass());
  PM->add(llvm::createCFGSimplificationPass());      // Merge & remove BBs
  PM->add(llvm::createInstructionCombiningPass());   // Combine silly seq's

  PM->add(llvm::createTailCallEliminationPass()); // Eliminate tail calls
  PM->add(llvm::createCFGSimplificationPass());   // Merge & remove BBs
  PM->add(llvm::createReassociatePass());         // Reassociate expressions
  PM->add(llvm::createLoopRotatePass());          // Rotate Loop
  PM->add(llvm::createLICMPass());                // Hoist loop invariants
  PM->add(llvm::createLoopUnswitchPass(OptLevel < 3));
  PM->add(llvm::createInstructionCombiningPass());
  PM->add(llvm::createInstSimplifyLegacyPass());
  PM->add(llvm::createIndVarSimplifyPass()); // Canonicalize indvars
  PM->add(llvm::createLoopDeletionPass());   // Delete dead loops

  // If a function appeared in a loop is a candidate to be inlined,
  // LoopUnroll pass refuses to unroll the loop, so we should inline the function
  // first to help unroller to decide if it's worthy to unroll the loop.
  PM->add(llvm::createFunctionInliningPass(4096)); // Inline (not only small)
                                                   // functions
  if (UnrollLoops) {
    // Unroll small loops
    // Parameters for unrolling are as follows:
    // Optimization level, OnlyWhenForced (If false, use cost model to
    // determine loop unrolling profitability. If true, only loops that
    // explicitly request unrolling via metadata are considered),
    // ForgetAllSCEV (If false, when SCEV is invalidated, only forget
    // everything in the top-most loop), cost threshold, explicit unroll
    // count, allow partial unrolling, allow runtime unrolling.
    PM->add(llvm::createLoopUnrollPass(OptLevel, false, false, 512, 0, 0, 0));
    // unroll loops with non-constant trip count
    const int thresholdBase = 16;
    if (rtLoopUnrollFactor > 1) {
      const int threshold = thresholdBase * rtLoopUnrollFactor;
      PM->add(llvm::createLoopUnrollPass(OptLevel, false, false,
                                         threshold, rtLoopUnrollFactor, 0, 1));
    }
  }
  // Break up aggregate allocas
  PM->add(llvm::createSROAPass());
  // Clean up after the unroller
  PM->add(llvm::createInstructionCombiningPass());
  PM->add(llvm::createInstSimplifyLegacyPass());
  if (allowAllocaModificationOpt) {
    if (OptLevel > 1)
      PM->add(llvm::createGVNPass());     // Remove redundancies
    PM->add(llvm::createMemCpyOptPass()); // Remove memcpy / form memset
  }
  PM->add(llvm::createSCCPPass()); // Constant prop with SCCP

  // Run instcombine after redundancy elimination to exploit opportunities
  // opened up by them.
  PM->add(llvm::createInstructionCombiningPass());
  PM->add(llvm::createJumpThreadingPass()); // Thread jumps
  PM->add(llvm::createCorrelatedValuePropagationPass());
  // We pass "false" to DSE here (and in all other instances) to disable
  // the MemorySSA algorithm, to improve compile speed. We can re-enable it
  // later if the community improves it.
  PM->add(llvm::createDeadStoreEliminationPass()); // Delete dead stores
  PM->add(llvm::createAggressiveDCEPass());        // Delete dead instructions
  PM->add(llvm::createCFGSimplificationPass());    // Merge & remove BBs
  PM->add(llvm::createInstructionCombiningPass()); // Clean up after everything.

  if (UnitAtATime) {
    // Get rid of dead prototypes
    PM->add(llvm::createStripDeadPrototypesPass());

    // GlobalOpt already deletes dead functions and globals, at -O3 try a
    // late pass of GlobalDCE.  It is capable of deleting dead cycles.
    if (OptLevel > 2)
      PM->add(llvm::createGlobalDCEPass()); // Remove dead fns and globals.

    if (OptLevel > 1)
      PM->add(llvm::createConstantMergePass()); // Merge dup global constants
  }
  PM->add(llvm::createUnifyFunctionExitNodesPass());
}

static void populatePassesPreFailCheck(
    llvm::legacy::PassManagerBase &PM, llvm::Module * /*M*/,
    SmallVector<Module *, 2> &pRtlModuleList, unsigned OptLevel,
    const intel::OptimizerConfig *pConfig, bool isOcl20, bool isFpgaEmulator,
    bool UnrollLoops, bool EnableInferAS, bool isSPIRV, bool UseVplan) {
  PrintIRPass::DumpIRConfig dumpIRAfterConfig(pConfig->GetIRDumpOptionsAfter());
  PrintIRPass::DumpIRConfig dumpIRBeforeConfig(
      pConfig->GetIRDumpOptionsBefore());

  bool HasGatherScatterPrefetch =
      pConfig->GetCpuId()->HasGatherScatterPrefetch();

  if (isSPIRV && pConfig->GetRelaxedMath()) {
    PM.add(createAddFastMathPass());
  }

  // Here we are internalizing non-kernal functions to allow inliner to remove
  // functions' bodies without call sites
  if (OptLevel > 0)
    PM.add(createInternalizeNonKernelFuncPass());

  PM.add(createFMASplitterPass());
  PM.add(createOclSyncFunctionAttrsPass());
  PM.add(createPrintfArgumentsPromotionPass());
  if (isOcl20) {
    // OCL2.0 resolve block to static call
    PM.add(createResolveBlockToStaticCallPass());
  }

  if (OptLevel > 0) {
    PM.add(llvm::createCFGSimplificationPass());
    if (OptLevel == 1)
      PM.add(llvm::createPromoteMemoryToRegisterPass());
    else {
      PM.add(llvm::createSROAPass());
    }
    PM.add(llvm::createInstructionCombiningPass());
    PM.add(llvm::createInstSimplifyLegacyPass());
  }

  // No adaptation layer is required for native subgroups
  if (!EnableNativeOpenCLSubgroups)
    PM.add(createSubGroupAdaptationPass());

  if (isOcl20) {
    // Flatten get_{local, global}_linear_id()
    PM.add(createLinearIdResolverPass());
  }

  PM.add(createBuiltinLibInfoPass(pRtlModuleList, ""));

  if (isFpgaEmulator) {
      // ChannelPipeTransformation and SYCLPipesHack passes populate
      // channel/pipes error log.
      Intel::OpenCL::DeviceBackend::ChannelPipesErrorLog.clear();
      PM.add(createSYCLPipesHackPass());
      PM.add(createChannelPipeTransformationPass());
      PM.add(createPipeIOTransformationPass());
      PM.add(createPipeOrderingPass());
      PM.add(createAutorunReplicatorPass());
      PM.add(createChannelsUsageAnalysisPass());
  }

  // Adding module passes.

  if (dumpIRBeforeConfig.ShouldPrintPass(DUMP_IR_TARGERT_DATA)) {
    PM.add(createPrintIRPass(DUMP_IR_TARGERT_DATA, OPTION_IR_DUMPTYPE_BEFORE,
                             pConfig->GetDumpIRDir()));
  }

  // OCL2.0 add Generic Address Resolution
  // LLVM IR converted from any version of SPIRV may have Generic
  // adress space pointers.
  if (isOcl20 || isSPIRV) {
    // Static resolution of generic address space pointers
    if (OptLevel > 0) {
      PM.add(llvm::createPromoteMemoryToRegisterPass());
    }
    if (EnableInferAS) {
      PM.add(llvm::createInferAddressSpacesPass());
    } else {
      PM.add(createGenericAddressStaticResolutionPass());
    }
  }

  PM.add(llvm::createBasicAAWrapperPass());
  if (pConfig->EnableOCLAA()) {
    PM.add(createOCLAliasAnalysisPass());
    PM.add(createExternalAAWrapperPass(
      [](Pass &P, Function &, AAResults &AAR) {
        if (auto *OCLAA = P.getAnalysisIfAvailable<OCLAliasAnalysis>())
          AAR.addAAResult(OCLAA->getOCLAAResult());
      }));
  }
  if (dumpIRAfterConfig.ShouldPrintPass(DUMP_IR_TARGERT_DATA)) {
    PM.add(createPrintIRPass(DUMP_IR_TARGERT_DATA, OPTION_IR_DUMPTYPE_AFTER,
                             pConfig->GetDumpIRDir()));
  }
  if (getenv("DISMPF") != nullptr || intel::Statistic::isEnabled())
    PM.add(createRemovePrefetchPass());

  PM.add(createBuiltinCallToInstPass());

  // When running the standard optimization passes, do not change the
  // loop-unswitch
  // pass on modules which contain barriers. This pass is illegal for
  // barriers.
  bool UnitAtATime = true;

  int rtLoopUnrollFactor = pConfig->GetRTLoopUnrollFactor();

  bool allowAllocaModificationOpt = !HasGatherScatterPrefetch;

  createStandardLLVMPasses(&PM, OptLevel, UnitAtATime, UnrollLoops,
                           rtLoopUnrollFactor, allowAllocaModificationOpt,
                           UseVplan);

  // check there is no recursion, if there is fail compilation
  PM.add(createDetectRecursionPass());

  // PipeSupport can fail if dynamic pipe access is discovered after LLVM
  // optimizations
  if (isFpgaEmulator) {
    PM.add(createPipeSupportPass());
  }
}

static void populatePassesPostFailCheck(
    llvm::legacy::PassManagerBase &PM, llvm::Module *M,
    SmallVector<Module *, 2> &pRtlModuleList, unsigned OptLevel,
    const intel::OptimizerConfig *pConfig,
    std::vector<std::string> &UndefinedExternals, bool isOcl20,
    bool isFpgaEmulator, bool isEyeQEmulator, bool UnrollLoops,
    bool EnableInferAS, bool UseVplan, bool IsSYCL, bool IsOMP,
    TStringToVFState &kernelVFStates) {
  bool isProfiling = pConfig->GetProfilingFlag();
  bool HasGatherScatter = pConfig->GetCpuId()->HasGatherScatter();
  bool HasGatherScatterPrefetch =
      pConfig->GetCpuId()->HasGatherScatterPrefetch();
  // Tune the maximum size of the basic block for memory dependency analysis
  // utilized by GVN.
  DebuggingServiceType debugType =
      getDebuggingServiceType(pConfig->GetDebugInfoFlag(), M,
                              pConfig->GetUseNativeDebuggerFlag());
  bool UseTLSGlobals = (debugType == intel::Native) && !isEyeQEmulator;

  PrintIRPass::DumpIRConfig dumpIRAfterConfig(pConfig->GetIRDumpOptionsAfter());
  PrintIRPass::DumpIRConfig dumpIRBeforeConfig(
      pConfig->GetIRDumpOptionsBefore());

  if (pConfig->EnableOCLAA()) {
    PM.add(createOCLAliasAnalysisPass());
    PM.add(createExternalAAWrapperPass(
      [](Pass &P, Function &, AAResults &AAR) {
        if (auto *OCLAA = P.getAnalysisIfAvailable<OCLAliasAnalysis>())
          AAR.addAAResult(OCLAA->getOCLAAResult());
      }));
  }

  PM.add(createBuiltinLibInfoPass(pRtlModuleList, ""));
  PM.add(createImplicitArgsAnalysisPass(&M->getContext()));

  if (isOcl20) {
    // Repeat resolution of generic address space pointers after LLVM
    // IR was optimized
    if (EnableInferAS) {
      PM.add(llvm::createInferAddressSpacesPass());
      // Cleanup after InferAddressSpacesPass
      if (OptLevel > 0) {
        PM.add(llvm::createCFGSimplificationPass());
        PM.add(llvm::createSROAPass());
        PM.add(llvm::createEarlyCSEPass());
        PM.add(llvm::createPromoteMemoryToRegisterPass());
        PM.add(llvm::createInstructionCombiningPass());
      }
    } else {
      PM.add(createGenericAddressStaticResolutionPass());
    }
    // No need to run function inlining pass here, because if there are still
    // non-inlined functions left - then we don't have to inline new ones.
  }

  PM.add(createResolveVariableTIDCallPass());

  // Run few more passes after GenericAddressStaticResolution
  PM.add(createOclFunctionAttrsPass());
  PM.add(llvm::createUnifyFunctionExitNodesPass());


  PM.add(llvm::createBasicAAWrapperPass());

  // Should be called before vectorizer!
  PM.add((llvm::Pass *)createInstToFuncCallPass(pConfig->GetCpuId()));

  PM.add(createDuplicateCalledKernelsPass());

  if (debugType == Simulator) {
    // DebugInfo pass must run before KernelAnalysis and Barrier pass when
    // debugging with simulator. DebugInfo inserts get_global_id call to
    // un-inlined callee, and KernelAnalysis will set no_barrier_path to false
    // for caller kernel. The kernel will be handled in Barrier instead of
    // LoopCreator pass.
    PM.add(createDebugInfoPass());
  }

  if (OptLevel > 0) {
    PM.add(llvm::createCFGSimplificationPass());
    PM.add(createKernelAnalysisPass());
    PM.add(createCLWGLoopBoundariesPass());
    PM.add(llvm::createDeadCodeEliminationPass());
    PM.add(llvm::createCFGSimplificationPass());
  }

  // Mark the kernels using subgroups
  if (EnableNativeOpenCLSubgroups)
    PM.add(createKernelSubGroupInfoPass());

  // In Apple build TRANSPOSE_SIZE_1 is not declared
  if (pConfig->GetTransposeSize() != 1 /*TRANSPOSE_SIZE_1*/
      && OptLevel != 0) {

    // In profiling mode remove llvm.dbg.value calls before vectorizer.
    if (isProfiling) {
      PM.add(createProfilingInfoPass());
    }

    if (dumpIRBeforeConfig.ShouldPrintPass(DUMP_IR_VECTORIZER)) {
      PM.add(createPrintIRPass(DUMP_IR_VECTORIZER, OPTION_IR_DUMPTYPE_BEFORE,
                               pConfig->GetDumpIRDir()));
    }
    if (!isEyeQEmulator) {
      PM.add(createSinCosFoldPass());
    }

    if (!pRtlModuleList.empty()) {
      if (UseVplan) {
        if (EmitKernelVectorizerSignOn)
          dbgs() << "Kernel Vectorizer\n";

        // Replace 'div' and 'rem' instructions with calls to optimized library
        // functions
        PM.add(createMathLibraryFunctionsReplacementPass());

        // Merge returns : this pass ensures that the function has at most one
        // return instruction.
        PM.add(createUnifyFunctionExitNodesPass());
        PM.add(createCFGSimplificationPass(
            SimplifyCFGOptions()
            .bonusInstThreshold(1)
            .forwardSwitchCondToPhi(false)
            .convertSwitchToLookupTable(false)
            .needCanonicalLoops(true)
            .sinkCommonInsts(true)));
        PM.add(createInstructionCombiningPass());
        PM.add(createGVNHoistPass());
        PM.add(createDeadCodeEliminationPass());
        PM.add(createOCLReqdSubGroupSizePass());

        // Calculate VL.
        PM.add(createWeightedInstCounter(true, pConfig->GetCpuId()));

        // Do all vectorization factor checks here.
        PM.add(createOCLVPOCheckVFPass(*pConfig, kernelVFStates));

        PM.add(createVectorVariantLoweringPass(pConfig->GetCpuId()));
        PM.add(createCreateSimdVariantPropagationPass());
        PM.add(createSGSizeCollectorPass(pConfig->GetCpuId()));
        PM.add(createSGSizeCollectorIndirectPass(pConfig->GetCpuId()));

        // Prepare Function for VecClone and call VecClone
        // We won't automatically switch vectorization dimension for SYCL.
        if (!IsSYCL)
          PM.add(createChooseVectorizationDimensionModulePass());
        PM.add(createOCLVecClonePass(pConfig->GetCpuId(), !IsSYCL && !IsOMP));
        PM.add(createScalarizerPass(pConfig->GetCpuId(), true));

        PM.add(createVectorVariantFillInPass());
        PM.add(createUpdateCallAttrsPass());

        // Call VPlan
        PM.add(llvm::createPromoteMemoryToRegisterPass());
        PM.add(createLowerSwitchPass());
        PM.add(createLoopSimplifyPass());
        PM.add(createLCSSAPass());
        PM.add(createLICMPass());
        PM.add(createVPOCFGRestructuringPass());
        PM.add(createVPlanDriverPass([](Function *F) {
          F->addFnAttr(CompilationUtils::ATTR_VECTOR_VARIANT_FAILURE,
                       CompilationUtils::ATTR_VALUE_FAILED_TO_VECTORIZE);
        }));
        PM.add(createOCLPostVectPass());

        // Final cleaning up
        PM.add(createVPODirectiveCleanupPass());
        PM.add(createInstructionCombiningPass());
        PM.add(createCFGSimplificationPass());
        PM.add(createPromoteMemoryToRegisterPass());
        PM.add(createAggressiveDCEPass());

        // Add cost model to discard vectorized kernels if they have higher
        // cost. This is done only for native OpenCL program. In SYCL, unless
        // programmer explicitly asks not to vectorize (SG size of 1, OCL env
        // to disable vectorization, etc), compiler shall vectorize along the
        // fastest moving dimension (that maps to get_global_id(0) for LLVM IR
        // in our implementation). The vec/no-vec decision belongs to the
        // programmer.
        if (!IsSYCL && !DisableVPlanCM)
          PM.add(createVectorKernelDiscardPass(pConfig));
      } else {
        if (EmitKernelVectorizerSignOn)
          dbgs() << "OpenCL Kernel Vectorizer\n";
        PM.add(createVectorizerPass(pRtlModuleList, pConfig));
      }
    }

    if (UseVplan)
      PM.add(createHandleVPlanMaskPass());

    if (dumpIRAfterConfig.ShouldPrintPass(DUMP_IR_VECTORIZER)) {
      PM.add(createPrintIRPass(DUMP_IR_VECTORIZER, OPTION_IR_DUMPTYPE_AFTER,
                               pConfig->GetDumpIRDir()));
    }

  } else {
    // When forced VF equals 1 or in O0 case, check subgroup semantics AND
    // prepare subgroup_emu_size for sub-group emulation.
    if (UseVplan && EnableNativeOpenCLSubgroups) {
      PM.add(createOCLReqdSubGroupSizePass());
      PM.add(createOCLVPOCheckVFPass(*pConfig, kernelVFStates));
    }
  }
#ifdef _DEBUG
  PM.add(llvm::createVerifierPass());
#endif

  if (EnableNativeOpenCLSubgroups)
    PM.add(createResolveSubGroupWICallPass(/*ResolveSGBarrier*/ false));

  // Unroll small loops with unknown trip count.
  PM.add(llvm::createLoopUnrollPass(OptLevel, false, false, 16, 0, 0, 1));
  // The ShiftZeroUpperBits pass should be added after the vectorizer because
  // the vectorizer may transform scalar shifts into vector shifts, and we want
  // this pass to fix all vector shift in this module.
  PM.add(createShiftZeroUpperBitsPass());
  if (!isEyeQEmulator) {
    PM.add(createOptimizeIDivPass());
  }
  PM.add(createPreventDivisionCrashesPass());
  // We need InstructionCombining and GVN passes after ShiftZeroUpperBits,
  // PreventDivisionCrashes passes to optimize redundancy introduced by those
  // passes
  if (OptLevel > 0) {
    PM.add(llvm::createInstructionCombiningPass());
    PM.add(createSmartGVNPass(false));
    PM.add(createVectorCombinePass());
    // In specACCEL/124, InstCombine may generate a cross-barrier bool value
    // used as a condition of a 'br' instruction, which leads to performance
    // degradation. JumpThreading eliminates the cross-barrier value.
    PM.add(llvm::createJumpThreadingPass());
  }

  // The debugType enum and isProfiling flag are mutually exclusive, with
  // precedence given to debugType.
  if (isProfiling) {
    PM.add(createProfilingInfoPass());
  }

  if (isOcl20 && !EnableInferAS) {
    // Resolve (dynamically) generic address space pointers which are relevant
    // for correct execution
    PM.add(createGenericAddressDynamicResolutionPass());
    // No need to run function inlining pass here, because if there are still
    // non-inlined functions left - then we don't have to inline new ones.
  }

  // Get Some info about the kernel should be called before BarrierPass and
  // createPrepareKernelArgsPass
  if (!pRtlModuleList.empty()) {
    PM.add(createKernelInfoWrapperPass());
  }

  // Adding WG loops
  if (OptLevel > 0) {
    PM.add(createDeduceMaxWGDimPass());
    if (pConfig->GetStreamingAlways())
      PM.add(createAddNTAttrPass());
    if (debugType == Native)
      PM.add(createImplicitGIDPass(/*HandleBarrier*/ false));
    PM.add(createCLWGLoopCreatorPass());
  }
  PM.add(createIndirectCallLoweringPass());

  // Clean up scalar kernel after WGLoop for native subgroups.
  if (OptLevel > 0) {
    PM.add(llvm::createDeadCodeEliminationPass()); // Delete dead instructions
    PM.add(llvm::createCFGSimplificationPass());   // Simplify CFG
  }

  if (isFpgaEmulator) {
    PM.add(createInfiniteLoopCreatorPass());
  }

  // Barrier pass can't work with a token type, so here we remove region
  // directives
  PM.add(llvm::createRemoveRegionDirectivesLegacyPass());

  PM.add(createUnifyFunctionExitNodesPass());
  PM.add(createBarrierMainPass(OptLevel, debugType, UseTLSGlobals));

  // After adding loops run loop optimizations.
  if (OptLevel > 0) {
    // Add LoopSimplify pass before CLBuiltinLICM pass as CLBuiltinLICM pass
    // requires loops in Simplified Form.
    PM.add(createLoopSimplifyPass());
    PM.add(createCLBuiltinLICMPass());
    PM.add(llvm::createLICMPass());
    PM.add(createLoopStridedCodeMotionPass());
    PM.add(createCLStreamSamplerPass());
  }

  if (pConfig->GetRelaxedMath()) {
    PM.add(createRelaxedPass());
  }

  // The following three passes (AddImplicitArgs/AddTLSGlobals, ResolveWICall,
  // LocalBuffer) must run before createBuiltInImportPass!
  if (UseTLSGlobals)
    PM.add(createAddTLSGlobalsPass());
  else
    PM.add(createAddImplicitArgsPass());

  PM.add(createResolveWICallPass(pConfig->GetUniformWGSize(), UseTLSGlobals));
  PM.add(createLocalBuffersPass(debugType == Native, UseTLSGlobals));
  // clang converts OCL's local to global.
  // createLocalBuffersPass changes the local allocation from global to a
  // kernel argument.
  // The next pass createGlobalOptimizerPass cleans the unused global
  // allocation in order to make sure we will not allocate redundant space on
  // the jit
  if (debugType != Native)
    PM.add(llvm::createGlobalOptimizerPass());

#ifdef _DEBUG
  PM.add(llvm::createVerifierPass());
#endif

  // This pass checks if the module uses an undefined function or not
  // assumption: should run after WI function resolving
  PM.add(createUndifinedExternalFunctionsPass(UndefinedExternals));

  // Externalize globals if IR is generated from OpenMP offloading. Now we
  // cannot get address of globals with internal/private linkage from LLJIT
  // (by design), but it's necessary by OpenMP to pass address of declare
  // target variables to the underlying OpenCL Runtime via
  // clSetKernelExecInfo. So we have to externalize globals for IR generated
  // from OpenMP.
  if (IsOMP)
    PM.add(createExternalizeGlobalVariablesPass());

  if (!pRtlModuleList.empty()) {
    // Inline BI function
    const char *CPUPrefix = pConfig->GetCpuId()->GetCPUPrefix();
    assert(CPUPrefix && "CPU Prefix should not be null");
    PM.add(createBuiltInImportPass(CPUPrefix));
    if (OptLevel > 0) {
      // After the globals used in built-ins are imported - we can internalize
      // them with further wiping them out with GlobalDCE pass
      PM.add(createInternalizeGlobalVariablesPass());
      // Cleaning up internal globals
      PM.add(llvm::createGlobalDCEPass());
    }
    // Need to convert shuffle calls to shuffle IR before running inline pass
    // on built-ins
    PM.add(createBuiltinCallToInstPass());
  }

// funcPassMgr->add(new intel::SelectLower());

#ifdef _DEBUG
  PM.add(llvm::createVerifierPass());
#endif

  if (OptLevel > 0) {
    if (HasGatherScatter)
      // Original motivation for this customization was a limitation of the
      // vectorizer to vectorize only kernels without function calls.
      // Missing vectorization caused huge performance loss on KNC,
      // so inlining was done more agressively.
      // This was transferred to AVX-512 SKX/CNL through HasGatherScatter flag.
      // So far there's no data that would suggest dropping this customization.
      PM.add(llvm::createFunctionInliningPass(4096)); // Inline (not only small) functions.
    else
      PM.add(llvm::createFunctionInliningPass());     // Inline small functions
  } else if (isOcl20) {
    // Ensure that the built-in functions to be processed by PatchCallbackArgsPass
    // are inlined.
    PM.add(llvm::createAlwaysInlinerLegacyPass());
  }
  // Some built-in functions contain calls to external functions which take
  // arguments that are retrieved from the function's implicit arguments.
  // Currently only applies to OpenCL 2.x
  if (isOcl20)
    PM.add(createPatchCallbackArgsPass(UseTLSGlobals));

  if (OptLevel > 0) {
    PM.add(llvm::createArgumentPromotionPass()); // Scalarize uninlined fn args
    PM.add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
    PM.add(llvm::createDeadStoreEliminationPass()); // Delete dead stores
    PM.add(llvm::createAggressiveDCEPass());        // Delete dead instructions
    PM.add(llvm::createCFGSimplificationPass());    // Merge & remove BBs
    PM.add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
    PM.add(llvm::createPromoteMemoryToRegisterPass());
  }
  // Only support CPU Device
  if (OptLevel > 0 && !isFpgaEmulator && !isEyeQEmulator) {
    PM.add(llvm::createLICMPass());      // Hoist loop invariants
    PM.add(llvm::createLoopIdiomPass()); // Transform simple loops to non-loop form, e.g. memcpy
    PM.add(createLoopDeletionPass()); // Delete dead loops
  }

  // PrepareKernelArgsPass must run in debugging mode as well
  PM.add(createPrepareKernelArgsPass(UseTLSGlobals));

  if ( OptLevel > 0 ) {
    // These passes come after PrepareKernelArgs pass to eliminate the
    // redundancy reducced by it
    PM.add(llvm::createFunctionInliningPass());    // Inline
    PM.add(llvm::createDeadCodeEliminationPass()); // Delete dead instructions
    PM.add(llvm::createCFGSimplificationPass());   // Simplify CFG
    PM.add(llvm::createInstructionCombiningPass()); // Instruction combining
    PM.add(llvm::createDeadStoreEliminationPass()); // Eliminated dead stores
    PM.add(llvm::createEarlyCSEPass());
    PM.add(createSmartGVNPass(true)); // GVN with "no load" heuristic
#ifdef _DEBUG
    PM.add(llvm::createVerifierPass());
#endif
  } else {
    // Functions with the alwaysinline attribute need to be inlined for
    // functional purposes
    PM.add(llvm::createAlwaysInlinerLegacyPass());
  }

  // After kernels are inlined into their wrappers we can cleanup the bodies
  PM.add(createCleanupWrappedKernelsPass());

  // Add prefetches if useful for micro-architecture,
  // and don't change libraries
  if (OptLevel > 0 && HasGatherScatterPrefetch) {
    int APFLevel = pConfig->GetAPFLevel();
    // do APF and following cleaning passes only if APF is not disabled
    if (APFLevel != APFLEVEL_0_DISAPF) {
      if (pConfig->GetCpuId()->RequirePrefetch())
        PM.add(createPrefetchPassLevel(pConfig->GetAPFLevel()));
      PM.add(llvm::createDeadCodeEliminationPass()); // Delete dead instructions
      PM.add(llvm::createInstructionCombiningPass()); // Instruction combining
      PM.add(createSmartGVNPass(false));
#ifdef _DEBUG
      PM.add(llvm::createVerifierPass());
#endif
    }
  }
  if (UnrollLoops && OptLevel > 0) {
    // Unroll small loops
    PM.add(llvm::createLoopUnrollPass(OptLevel, false, false, 4, 0, 0));
  }
}

OptimizerOCL::~OptimizerOCL() {}

OptimizerOCL::OptimizerOCL(llvm::Module *pModule,
                           llvm::SmallVector<llvm::Module *, 2> &pRtlModuleList,
                           const intel::OptimizerConfig *pConfig)
    : Optimizer(pModule), m_pRtlModuleList(pRtlModuleList),
      m_IsFpgaEmulator(pConfig->isFpgaEmulator()),
      m_IsEyeQEmulator(pConfig->isEyeQEmulator()) {
  TargetMachine* targetMachine = pConfig->GetTargetMachine();
  assert(targetMachine && "Uninitialized TargetMachine!");

  unsigned int OptLevel = 3;
  if (pConfig->GetDisableOpt())
    OptLevel = 0;

  // Detect OCL2.0 compilation mode
  const bool isOcl20 = (CompilationUtils::fetchCLVersionFromMetadata(
                            *pModule) >= OclVersion::CL_VER_2_0);

  const bool isSPIRV = CompilationUtils::generatedFromSPIRV(*pModule);

  bool UnrollLoops = true;

  // Initialize TTI
  m_PreFailCheckPM.add(createTargetTransformInfoWrapperPass(
    targetMachine->getTargetIRAnalysis()));
  m_PostFailCheckPM.add(createTargetTransformInfoWrapperPass(
    targetMachine->getTargetIRAnalysis()));

  // Add an appropriate TargetLibraryInfo pass for the module's triple.
  TargetLibraryInfoImpl TLII(Triple(pModule->getTargetTriple()));
  m_PreFailCheckPM.add(new TargetLibraryInfoWrapperPass(TLII));
  m_PostFailCheckPM.add(new TargetLibraryInfoWrapperPass(TLII));

  bool EnableInferAS = !getenv("DISABLE_INFER_AS");

  bool IsOMP = CompilationUtils::generatedFromOMP(*pModule);
  // Add passes which will run unconditionally
  populatePassesPreFailCheck(m_PreFailCheckPM, pModule, m_pRtlModuleList,
                             OptLevel, pConfig, isOcl20, m_IsFpgaEmulator,
                             UnrollLoops, EnableInferAS, isSPIRV, EnableVPlan);

  // Add passes which will be run only if hasFunctionPtrCalls() and
  // hasRecursion() will return false
  populatePassesPostFailCheck(m_PostFailCheckPM, pModule, m_pRtlModuleList,
                              OptLevel, pConfig, m_undefinedExternalFunctions,
                              isOcl20, m_IsFpgaEmulator, m_IsEyeQEmulator,
                              UnrollLoops, EnableInferAS, EnableVPlan, m_IsSYCL,
                              IsOMP, m_kernelToVFState);
}

void OptimizerOCL::Optimize() {
  legacy::PassManager materializerPM;
  materializerPM.add(createBuiltinLibInfoPass(m_pRtlModuleList, ""));
  materializerPM.add(createLLVMEqualizerPass());
  Triple TargetTriple(m_M->getTargetTriple());
  if (!m_IsEyeQEmulator && TargetTriple.isArch64Bit()) {
    if (TargetTriple.isOSLinux())
      materializerPM.add(createCoerceTypesPass());
    else if (TargetTriple.isOSWindows())
      materializerPM.add(createCoerceWin64TypesPass());
  }
  if (m_IsFpgaEmulator) {
    materializerPM.add(createRemoveAtExitPass());
  }
  materializerPM.run(*m_M);
  m_PreFailCheckPM.run(*m_M);

  // if there are still unsupported recursive calls after standard LLVM
  // optimizations applied, compilation will report failure.
  if (hasUnsupportedRecursion()) {
    return;
  }

  // if not all pipe access were resolved statically.
  if (hasFpgaPipeDynamicAccess()) {
    return;
  }

  m_PostFailCheckPM.run(*m_M);

  // if not all must vec functions have been vectorized.
  // Serves as a safe guard to not execute the code below that
  // might be added in the future.
  if (hasVectorVariantFailure()) {
    return;
  }
}

Optimizer::Optimizer(llvm::Module *M)
    : m_M(M), m_IsSYCL(CompilationUtils::generatedFromOCLCPP(*M)) {}

const TStringToVFState& Optimizer::GetKernelVFStates() const {
  return m_kernelToVFState;
}

bool Optimizer::hasUndefinedExternals() const {
  return !m_undefinedExternalFunctions.empty();
}

const std::vector<std::string> &Optimizer::GetUndefinedExternals() const {
  return m_undefinedExternalFunctions;
}

bool Optimizer::hasUnsupportedRecursion() {
  return m_IsSYCL
             ? !GetInvalidFunctions(InvalidFunctionType::RECURSION_WITH_BARRIER)
                    .empty()
             : !GetInvalidFunctions(InvalidFunctionType::RECURSION).empty();
}

bool Optimizer::hasFpgaPipeDynamicAccess() const {
  return !GetInvalidFunctions(
      InvalidFunctionType::FPGA_PIPE_DYNAMIC_ACCESS).empty();
}

bool Optimizer::hasVectorVariantFailure() const {
  return !GetInvalidFunctions(InvalidFunctionType::VECTOR_VARIANT_FAILURE)
              .empty();
}

bool Optimizer::hasFPGAChannelsWithDepthIgnored() const {
  return !GetInvalidGlobals(InvalidGVType::FPGA_DEPTH_IS_IGNORED).empty();
}

std::vector<std::string> Optimizer::GetInvalidGlobals(InvalidGVType Ty) const {
  assert(m_M && "Module is nullptr");
  std::vector<std::string> Res;

  for (auto &GV : m_M->globals()) {
    auto GVM = MetadataAPI::GlobalVariableMetadataAPI(&GV);

    switch (Ty) {
      case FPGA_DEPTH_IS_IGNORED:
        if (GVM.DepthIsIgnored.hasValue() && GVM.DepthIsIgnored.get()) {
          assert(GV.getName().endswith(".pipe") &&
              "Only global pipes are expected");
          Res.push_back(std::string(GV.getName().drop_back(5)));
        }
    }
  }

  return Res;
}

std::vector<std::string>
Optimizer::GetInvalidFunctions(InvalidFunctionType Ty) const {
  assert(m_M && "Module is NULL");
  std::vector<std::string> Res;

  for (auto &F : *m_M) {
    auto KMD = MetadataAPI::FunctionMetadataAPI(&F);

    bool Invalid = false;

    switch (Ty) {
    case RECURSION:
      Invalid = KMD.RecursiveCall.hasValue() && KMD.RecursiveCall.get();
      break;
    case RECURSION_WITH_BARRIER:
      Invalid = F.hasFnAttribute(CompilationUtils::ATTR_RECURSION_WITH_BARRIER);
      break;
    case FPGA_PIPE_DYNAMIC_ACCESS:
      Invalid = KMD.FpgaPipeDynamicAccess.hasValue() &&
        KMD.FpgaPipeDynamicAccess.get();
      break;
    case VECTOR_VARIANT_FAILURE:
      Invalid = F.hasFnAttribute(CompilationUtils::ATTR_VECTOR_VARIANT_FAILURE);
      break;
    }

    if (Invalid) {
      std::string Message;
      llvm::raw_string_ostream MStr(Message);
      MStr << std::string(F.getName());
      if (auto SP = F.getSubprogram()) {
        MStr << " at ";
        MStr << "file: " << SP->getFilename () << ", line:" << SP->getLine();
      }
      MStr.str();
      Res.push_back(std::move(Message));
    }
  }

  return Res;
}

void OptimizerOCL::initializePasses() {
  // Initialize passes so that -print-after/-print-before work.
  PassRegistry &Registry = *PassRegistry::getPassRegistry();
  initializeCore(Registry);
  initializeCoroutines(Registry);
  initializeScalarOpts(Registry);
  initializeObjCARCOpts(Registry);
  initializeVectorization(Registry);
  initializeIPO(Registry);
  initializeAnalysis(Registry);
  initializeTransformUtils(Registry);
  initializeInstCombine(Registry);
  initializeInstrumentation(Registry);
  initializeTarget(Registry);
  // For codegen passes, only passes that do IR to IR transformation are
  // supported.
  initializeExpandMemCmpPassPass(Registry);
  initializeScalarizeMaskedMemIntrinLegacyPassPass(Registry);
  initializeCodeGenPreparePass(Registry);
  initializeAtomicExpandPass(Registry);
  initializeRewriteSymbolsLegacyPassPass(Registry);
  initializeWinEHPreparePass(Registry);
  initializeDwarfEHPrepareLegacyPassPass(Registry);
  initializeSafeStackLegacyPassPass(Registry);
  initializeSjLjEHPreparePass(Registry);
  initializePreISelIntrinsicLoweringLegacyPassPass(Registry);
  initializeGlobalMergePass(Registry);
  initializeInterleavedAccessPass(Registry);
  initializeEntryExitInstrumenterPass(Registry);
  initializePostInlineEntryExitInstrumenterPass(Registry);
  initializeUnreachableBlockElimLegacyPassPass(Registry);
  initializeExpandReductionsPass(Registry);
  initializeWriteBitcodePassPass(Registry);

  initializeIntel_LoopAnalysis(Registry);
  initializeIntel_LoopTransforms(Registry);
  initializeVecClonePass(Registry);
  initializeMapIntrinToImlPass(Registry);
  initializeIntel_OpenCLTransforms(Registry);
  initializeVPOAnalysis(Registry);
  initializeVPOTransforms(Registry);
  initializeOptimizeDynamicCastsWrapperPass(Registry);

  initializeOCLPasses(Registry);
}
}}}
