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

#include "Optimizer.h"
#include "CPUDetect.h"
#include "CompilationUtils.h"
#include "MetadataAPI.h"
#include "OclTune.h"
#include "VecConfig.h"
#include "debuggingservicetype.h"

#include "PrintIRPass.h"
#include "mic_dev_limits.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Pass.h"
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

// TODO: vromanov to fix:
// #include "llvm/Transforms/Intel_OpenCLTransforms/Passes.h"
llvm::FunctionPass* createFMASplitterPass();

// INTEL VPO BEGIN
#include "llvm/Transforms/Intel_MapIntrinToIml/MapIntrinToIml.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/Intel_VPO/VPOPasses.h"
#include "llvm/Transforms/Vectorize.h"
#include "llvm/Transforms/Utils/Intel_VecClone.h"

static cl::opt<bool> DisableVPlanVec("disable-vplan-vectorizer",
                                     cl::init(false), cl::Hidden,
                                     cl::desc("Disable VPlan Vectorizer"));
// INTEL VPO END

extern "C"{

void *createInstToFuncCallPass(bool);

llvm::Pass *createVectorizerPass(SmallVector<Module *, 2> builtinModules,
                                 const intel::OptimizerConfig *pConfig);
llvm::Pass *createBarrierMainPass(intel::DebuggingServiceType debugType);

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
llvm::ModulePass *createBuiltInImportPass(const char *CPUName);
llvm::ImmutablePass *createImplicitArgsAnalysisPass(llvm::LLVMContext *C);
llvm::ModulePass *createChannelPipeTransformationPass();
llvm::ModulePass *createPipeIOTransformationPass();
llvm::ModulePass *createCleanupWrappedKernelsPass();
llvm::ModulePass *createPipeOrderingPass();
llvm::ModulePass *createPipeSupportPass();
llvm::ModulePass *createLocalBuffersPass(bool isNativeDebug);
llvm::ModulePass *createAddImplicitArgsPass();
llvm::ModulePass *createOclFunctionAttrsPass();
llvm::ModulePass *createOclSyncFunctionAttrsPass();
llvm::ModulePass *createInternalizeNonKernelFuncPass();
llvm::ModulePass *createInternalizeGlobalVariablesPass();
llvm::ModulePass *createGenericAddressStaticResolutionPass();
llvm::ModulePass *createGenericAddressDynamicResolutionPass();
llvm::ModulePass *createPrepareKernelArgsPass();
llvm::Pass *
createBuiltinLibInfoPass(SmallVector<Module *, 2> pRtlModuleList,
                         std::string type);
llvm::ModulePass *createUndifinedExternalFunctionsPass(
    std::vector<std::string> &undefinedExternalFunctions);
llvm::ModulePass *createKernelInfoWrapperPass();
llvm::ModulePass *createDuplicateCalledKernelsPass();
llvm::ModulePass *createPatchCallbackArgsPass();
llvm::ModulePass *createDeduceMaxWGDimPass();

llvm::ModulePass *createSpirMaterializer();
llvm::FunctionPass *createPrefetchPassLevel(int level);
llvm::ModulePass *createRemovePrefetchPass();
llvm::ModulePass *createPrintIRPass(int option, int optionLocation,
                                    std::string dumpDir);
llvm::ModulePass *createDebugInfoPass();
llvm::ModulePass *createProfilingInfoPass();
llvm::Pass *createSmartGVNPass(bool);

llvm::ModulePass *createSinCosFoldPass();
llvm::ModulePass *createResolveWICallPass();
llvm::ModulePass *createDetectRecursionPass();
llvm::ModulePass *createPreLegalizeBoolsPass();
llvm::ImmutablePass *createOCLAliasAnalysisPass();
llvm::ModulePass *createPrintfArgumentsPromotionPass();
}

using namespace intel;
// INTEL VPO BEGIN
using namespace vpo;
// INTEL VPO END
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
                                            bool isDBG, unsigned RunVPOParopt) {
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

// INTEL VPO BEGIN

  if (RunVPOParopt) {
    PM->add(llvm::createLoopRotatePass(-1));
    PM->add(llvm::createVPOCFGRestructuringPass());
    PM->add(llvm::createVPOParoptPreparePass(RunVPOParopt));
    PM->add(llvm::createVPOCFGRestructuringPass());
    PM->add(llvm::createVPOParoptPass(RunVPOParopt));
  }
// INTEL VPO END

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

// INTEL VPO BEGIN
  // VPO Driver
  if (!DisableVPlanVec && (RunVPOParopt & VPOParoptMode::OmpVec)) {
    PM->add(createVecClonePass());
    PM->add(llvm::createVPOCFGRestructuringPass());
    PM->add(llvm::createVPlanDriverPass());
  }
// INTEL VPO END
  if (!isDBG) {
    // If a function appeared in a loop is a candidate to be inlined,
    // LoopUnroll pass refuses to unroll the loop, so we should inline the function
    // first to help unroller to decide if it's worthy to unroll the loop.
    PM->add(llvm::createFunctionInliningPass(4096)); // Inline (not only small)
                                                     // functions
  }
  if (UnrollLoops) {
    PM->add(llvm::createLoopUnrollPass(OptLevel, 512, 0, 0)); // Unroll small loops
    // unroll loops with non-constant trip count
    const int thresholdBase = 16;
    if (rtLoopUnrollFactor > 1) {
      const int threshold = thresholdBase * rtLoopUnrollFactor;
      PM->add(llvm::createLoopUnrollPass(OptLevel,
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
// INTEL VPO BEGIN
  if (!DisableVPlanVec && (RunVPOParopt & VPOParoptMode::OmpVec)) {
    PM->add(createMapIntrinToImlPass());
  }
// INTEL VPO END
}

static void populatePassesPreFailCheck(llvm::legacy::PassManagerBase &PM,
                                       llvm::Module *M,
                                       SmallVector<Module*, 2> & pRtlModuleList,
                                       unsigned OptLevel,
                                       const intel::OptimizerConfig *pConfig,
                                       bool isOcl20,
                                       bool isFpgaEmulator,
                                       bool UnrollLoops) {
  DebuggingServiceType debugType =
      getDebuggingServiceType(pConfig->GetDebugInfoFlag());

  PrintIRPass::DumpIRConfig dumpIRAfterConfig(pConfig->GetIRDumpOptionsAfter());
  PrintIRPass::DumpIRConfig dumpIRBeforeConfig(
      pConfig->GetIRDumpOptionsBefore());

  bool HasGatherScatterPrefetch =
    pConfig->GetCpuId().HasGatherScatterPrefetch();

  // Here we are internalizing non-kernal functions to allow inliner to remove
  // functions' bodies without call sites
  if (debugType == intel::None)
    PM.add(createInternalizeNonKernelFuncPass());

  PM.add(createFMASplitterPass());
  PM.add(createOclSyncFunctionAttrsPass());
  PM.add(createPrintfArgumentsPromotionPass());

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

  if (isOcl20)
    PM.add(createSubGroupAdaptationPass());

  if (isOcl20) {
    // Flatten get_{local, global}_linear_id()
    PM.add(createLinearIdResolverPass());
  }

  PM.add(createBuiltinLibInfoPass(pRtlModuleList, ""));

  if (isFpgaEmulator) {
      PM.add(createChannelPipeTransformationPass());
      PM.add(createPipeIOTransformationPass());
      PM.add(createPipeOrderingPass());
      PM.add(createAutorunReplicatorPass());
  }

  // Adding module passes.

  if (dumpIRBeforeConfig.ShouldPrintPass(DUMP_IR_TARGERT_DATA)) {
    PM.add(createPrintIRPass(DUMP_IR_TARGERT_DATA, OPTION_IR_DUMPTYPE_BEFORE,
                             pConfig->GetDumpIRDir()));
  }

  // OCL2.0 add Generic Address Static Resolution pass
  if (isOcl20) {
    // Static resolution of generic address space pointers
    if (OptLevel > 0) {
      PM.add(llvm::createPromoteMemoryToRegisterPass());
    }
    PM.add(createGenericAddressStaticResolutionPass());
  }

  PM.add(llvm::createBasicAAWrapperPass());
  PM.add(createOCLAliasAnalysisPass());
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

  unsigned RunVPOParopt = 0;

  if (isFpgaEmulator) {
// INTEL VPO BEGIN
    // TODO: This approach has the following issues:
    //     1. We shouldn't be processing Clang's flags in LLVM.
    //     2. If -fintel-openmp-region is passed by other means than the VOLCANO
    //        env var (OpenCL source code, for example), this is not going to
    //        work.
    //     3. Default value of optionsClang has to be aligned with
    //        the clang_driver.
    //
    // The right implementation should process intel flags in Clang and then,
    // Clang should pass the right LLVM flags to LLVM.
    std::string optionsClang;

    // FIXME: OpenMP is not enabled by default, because it requires
    // -fintel-compatibility flag, which affects on clang behavior beyond
    // OpenMP. Should be enabled back when this issue gets resolved.
    //
    // std::string optionsClang = "-fopenmp -fintel-openmp-region -fopenmp-tbb
    //                             -fintel-compatibility";

    if (const char* opts = getenv("VOLCANO_CLANG_OPTIONS")) {
#ifdef NDEBUG
      // Append user options to default options.
      optionsClang += opts;
#else
      // Allow default to be overridden for debug purposes.

      // FIXME: This is primarily needed to disable VPO, but overriding options
      // in debug build is not obvious. There are better ways to do it:
      //
      //    1. Pass -fno-openmp and -fno-intel-openmp-region flags through
      //    VOLCANO_CLANG_OPTIONS.
      //
      //    2. Pass -disable-vplan-vectorizer through
      //    VOLCANO_LLVM_OPTIONS. Tricky part here is whether the IR produced by
      //    the clang with -fopenmp and -fintel-openmp-region is compatible with
      //    our backend without VPO.
      optionsClang = opts;
#endif
    }
    if (!optionsClang.empty()) {
      std::stringstream optionsSS(optionsClang);
      std::string buf;
      while (getline(optionsSS, buf,' ')) {
        if (buf.compare("-fintel-openmp-region") == 0) {
          RunVPOParopt |= VPOParoptMode::ParPrepare;
          RunVPOParopt |= VPOParoptMode::ParTrans;
          RunVPOParopt |= VPOParoptMode::OmpPar;
          RunVPOParopt |= VPOParoptMode::OmpVec;
          RunVPOParopt |= VPOParoptMode::OmpTpv;
        }
        else if (buf.compare("-fopenmp-simd") == 0) {
          RunVPOParopt |= VPOParoptMode::ParPrepare;
          RunVPOParopt |= VPOParoptMode::ParTrans;
          RunVPOParopt |= VPOParoptMode::OmpVec;
        }
        else if (buf.compare("-fopenmp-tbb") == 0) {
          RunVPOParopt |= VPOParoptMode::OmpTbb;
        }
      }
    }
// INTEL VPO END
  }

  bool allowAllocaModificationOpt = !HasGatherScatterPrefetch;

  createStandardLLVMPasses(
      &PM, OptLevel,
      UnitAtATime, UnrollLoops, rtLoopUnrollFactor, allowAllocaModificationOpt,
      debugType != intel::None,
      RunVPOParopt);  // INTEL VPO

  // check there is no recursion, if there is fail compilation
  PM.add(createDetectRecursionPass());

  // PipeSupport can fail if dynamic pipe access is discovered after LLVM
  // optimizations
  if (isFpgaEmulator) {
    PM.add(createPipeSupportPass());
  }
}

static void
populatePassesPostFailCheck(llvm::legacy::PassManagerBase &PM, llvm::Module *M,
                            SmallVector<Module *, 2> &pRtlModuleList,
                            unsigned OptLevel,
                            const intel::OptimizerConfig *pConfig,
                            std::vector<std::string> &UndefinedExternals,
                            bool isOcl20, bool isFpgaEmulator,
                            bool UnrollLoops) {
  bool isProfiling = pConfig->GetProfilingFlag();
  bool HasGatherScatter = pConfig->GetCpuId().HasGatherScatter();
  bool HasGatherScatterPrefetch = pConfig->GetCpuId().HasGatherScatterPrefetch();
  // Tune the maximum size of the basic block for memory dependency analysis
  // utilized by GVN.
  DebuggingServiceType debugType =
      getDebuggingServiceType(pConfig->GetDebugInfoFlag());

  PrintIRPass::DumpIRConfig dumpIRAfterConfig(pConfig->GetIRDumpOptionsAfter());
  PrintIRPass::DumpIRConfig dumpIRBeforeConfig(
      pConfig->GetIRDumpOptionsBefore());

  PM.add(createBuiltinLibInfoPass(pRtlModuleList, ""));
  PM.add(createImplicitArgsAnalysisPass(&M->getContext()));

  if (isOcl20) {
    // Repeat static resolution of generic address space pointers after
    // LLVM IR was optimized
    PM.add(createGenericAddressStaticResolutionPass());
    // No need to run function inlining pass here, because if there are still
    // non-inlined functions left - then we don't have to inline new ones.
  }
  // Run the OclFunctionAttrs pass after GenericAddressStaticResolution
  PM.add(createOclFunctionAttrsPass());

  PM.add(llvm::createUnifyFunctionExitNodesPass());


  PM.add(llvm::createBasicAAWrapperPass());
  PM.add(createOCLAliasAnalysisPass());

  // Should be called before vectorizer!
  PM.add((llvm::Pass*)createInstToFuncCallPass(HasGatherScatter));

  PM.add(createDuplicateCalledKernelsPass());

  if (debugType == intel::None) {
    PM.add(createKernelAnalysisPass());
    PM.add(createCLWGLoopBoundariesPass());
    PM.add(llvm::createDeadCodeEliminationPass());
    PM.add(llvm::createCFGSimplificationPass());
  }

  // In Apple build TRANSPOSE_SIZE_1 is not declared
  if (pConfig->GetTransposeSize() != 1 /*TRANSPOSE_SIZE_1*/
      && debugType == intel::None && OptLevel != 0) {

    // In profiling mode remove llvm.dbg.value calls before vectorizer.
    if (isProfiling) {
      PM.add(createProfilingInfoPass());
    }

    if (dumpIRBeforeConfig.ShouldPrintPass(DUMP_IR_VECTORIZER)) {
      PM.add(createPrintIRPass(DUMP_IR_VECTORIZER, OPTION_IR_DUMPTYPE_BEFORE,
                               pConfig->GetDumpIRDir()));
    }
    PM.add(createSinCosFoldPass());

    if (!pRtlModuleList.empty()) {
      PM.add(createVectorizerPass(pRtlModuleList, pConfig));
    }

    if (dumpIRAfterConfig.ShouldPrintPass(DUMP_IR_VECTORIZER)) {
      PM.add(createPrintIRPass(DUMP_IR_VECTORIZER, OPTION_IR_DUMPTYPE_AFTER,
                               pConfig->GetDumpIRDir()));
    }
    if (!HasGatherScatter) {
      // no point to run for older CPU archs
      if (pConfig->GetCpuId().HasSSE41()) {
        // Workaround boolean vectors legalization issue.
        PM.add(createPreLegalizeBoolsPass());
      }
    }

  }
#ifdef _DEBUG
  PM.add(llvm::createVerifierPass());
#endif

  // Unroll small loops with unknown trip count.
  PM.add(llvm::createLoopUnrollPass(OptLevel, 16, 0, 0, 1));
  // The ShiftZeroUpperBits pass should be added after the vectorizer because
  // the vectorizer may transform scalar shifts into vector shifts, and we want
  // this pass to fix all vector shift in this module.
  PM.add(createShiftZeroUpperBitsPass());
  PM.add(createOptimizeIDivPass());
  PM.add(createPreventDivisionCrashesPass());
  // We need InstructionCombining and GVN passes after ShiftZeroUpperBits,
  // PreventDivisionCrashes passes to optimize redundancy introduced by those
  // passes
  if (debugType == intel::None) {
    PM.add(llvm::createInstructionCombiningPass());
    PM.add(createSmartGVNPass(false));
  }

  // The debugType enum and isProfiling flag are mutually exclusive, with
  // precedence given to debugType.
  if (debugType == Simulator) {
    // DebugInfo pass must run before Barrier pass when debugging with simulator
    PM.add(createDebugInfoPass());
  } else if (isProfiling) {
    PM.add(createProfilingInfoPass());
  }

  if (isOcl20) {
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
  if (debugType == intel::None) {
    PM.add(createDeduceMaxWGDimPass());
    PM.add(createCLWGLoopCreatorPass());
  }

  if (isFpgaEmulator) {
    PM.add(createInfiniteLoopCreatorPass());
  }

  // Barrier pass can't work with a token type, so here we remove region
  // directives
  PM.add(llvm::createRemoveRegionDirectivesLegacyPass());

  PM.add(createBarrierMainPass(debugType));

  // After adding loops run loop optimizations.
  if (debugType == intel::None) {
    PM.add(createCLBuiltinLICMPass());
    PM.add(llvm::createLICMPass());
    PM.add(createLoopStridedCodeMotionPass());
    PM.add(createCLStreamSamplerPass());
  }

  if (pConfig->GetRelaxedMath()) {
    PM.add(createRelaxedPass());
  }

  // The following three passes (AddImplicitArgs/ResolveWICall/LocalBuffer)
  // must run before createBuiltInImportPass!
  PM.add(createAddImplicitArgsPass());
  PM.add(createResolveWICallPass());
  PM.add(createLocalBuffersPass(debugType == Native));
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

  if (!pRtlModuleList.empty()) {
    // Inline BI function
    PM.add(createBuiltInImportPass(pConfig->GetCpuId().GetCPUPrefix()));
    if (debugType == intel::None) {
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

  if (debugType == intel::None) {
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
  }
  // Some built-in functions contain calls to external functions which take
  // arguments that are retrieved from the function's implicit arguments.
  // Currently only applies to OpenCL 2.x
  if (isOcl20)
    PM.add(createPatchCallbackArgsPass());

  if (debugType == intel::None) {
    PM.add(llvm::createArgumentPromotionPass()); // Scalarize uninlined fn args
    PM.add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
    PM.add(llvm::createDeadStoreEliminationPass()); // Delete dead stores
    PM.add(llvm::createAggressiveDCEPass());        // Delete dead instructions
    PM.add(llvm::createCFGSimplificationPass());    // Merge & remove BBs
    PM.add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
    PM.add(llvm::createPromoteMemoryToRegisterPass());
  }

  // PrepareKernelArgsPass must run in debugging mode as well
  PM.add(createPrepareKernelArgsPass());

  if ( debugType == intel::None ) {
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

  // Add prefetches if useful for micro-architecture, if not in debug mode,
  // and don't change libraries
  if (debugType == intel::None && HasGatherScatterPrefetch) {
    int APFLevel = pConfig->GetAPFLevel();
    // do APF and following cleaning passes only if APF is not disabled
    if (APFLevel != APFLEVEL_0_DISAPF) {
      if (pConfig->GetCpuId().RequirePrefetch())
        PM.add(createPrefetchPassLevel(pConfig->GetAPFLevel()));
      PM.add(llvm::createDeadCodeEliminationPass()); // Delete dead instructions
      PM.add(llvm::createInstructionCombiningPass()); // Instruction combining
      PM.add(createSmartGVNPass(false));
#ifdef _DEBUG
      PM.add(llvm::createVerifierPass());
#endif
    }
  }
  if (UnrollLoops && debugType == intel::None) {
    PM.add(llvm::createLoopUnrollPass(OptLevel, 4, 0, 0)); // Unroll small loops
  }
}

Optimizer::~Optimizer() { }

Optimizer::Optimizer(llvm::Module *pModule,
                     llvm::SmallVector<llvm::Module *, 2> pRtlModuleList,
                     const intel::OptimizerConfig *pConfig)
    : m_pModule(pModule), m_pRtlModuleList(pRtlModuleList) {

  DebuggingServiceType debugType =
      getDebuggingServiceType(pConfig->GetDebugInfoFlag());

  TargetMachine* targetMachine = pConfig->GetTargetMachine();
  assert(targetMachine && "Uninitialized TargetMachine!");

  unsigned int OptLevel = 3;
  if (pConfig->GetDisableOpt() || debugType != intel::None)
    OptLevel = 0;
  const bool isFpgaEmulator = pConfig->isFpgaEmulator();

  // Detect OCL2.0 compilation mode
  const bool isOcl20 = CompilationUtils::fetchCLVersionFromMetadata(*pModule) >=
                       OclVersion::CL_VER_2_0;
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

  // Add passes which will run unconditionally
  populatePassesPreFailCheck(m_PreFailCheckPM, pModule, m_pRtlModuleList,
                             OptLevel, pConfig, isOcl20,
                             isFpgaEmulator, UnrollLoops);

  // Add passes which will be run only if hasFunctionPtrCalls() and
  // hasRecursion() will return false
  populatePassesPostFailCheck(
      m_PostFailCheckPM, pModule, m_pRtlModuleList, OptLevel,
      pConfig, m_undefinedExternalFunctions, isOcl20, isFpgaEmulator,
      UnrollLoops);
}

void Optimizer::Optimize() {
  legacy::PassManager materializerPM;
  materializerPM.add(createBuiltinLibInfoPass(m_pRtlModuleList, ""));
  materializerPM.add(createSpirMaterializer());
  materializerPM.run(*m_pModule);
  m_PreFailCheckPM.run(*m_pModule);

  // if there are still recursive calls  after standard
  // LLVM optimizations applied  Compilation will report failure
  if (hasRecursion()) {
    return;
  }

  // if not all pipe access were resolved statically.
  if (hasFpgaPipeDynamicAccess()) {
    return;
  }

  m_PostFailCheckPM.run(*m_pModule);
}

bool Optimizer::hasUndefinedExternals() const {
  return !m_undefinedExternalFunctions.empty();
}

const std::vector<std::string> &Optimizer::GetUndefinedExternals() const {
  return m_undefinedExternalFunctions;
}

bool Optimizer::hasRecursion() {
  return !GetInvalidFunctions(InvalidFunctionType::RECURSION).empty();
}

bool Optimizer::hasFpgaPipeDynamicAccess() {
  return !GetInvalidFunctions(
      InvalidFunctionType::FPGA_PIPE_DYNAMIC_ACCESS).empty();
}

bool Optimizer::hasFPGAChannelsWithDepthIgnored() {
  return !GetInvalidGlobals(InvalidGVType::FPGA_DEPTH_IS_IGNORED).empty();
}

std::vector<std::string> Optimizer::GetInvalidGlobals(InvalidGVType Ty) {
  assert(m_pModule && "Module is nullptr");
  std::vector<std::string> Res;

  for (auto &GV : m_pModule->globals()) {
    auto GVM = MetadataAPI::GlobalVariableMetadataAPI(&GV);

    switch (Ty) {
      case FPGA_DEPTH_IS_IGNORED:
        if (GVM.DepthIsIgnored.hasValue() && GVM.DepthIsIgnored.get()) {
          assert(GV.getName().endswith(".pipe") &&
              "Only global pipes are expected");
          Res.push_back(GV.getName().drop_back(5));
        }
    }
  }

  return Res;
}

std::vector<std::string>
Optimizer::GetInvalidFunctions(InvalidFunctionType Ty) {
  assert(m_pModule && "Module is NULL");
  std::vector<std::string> Res;

  for (auto &F : *m_pModule) {
    auto KMD = MetadataAPI::FunctionMetadataAPI(&F);

    bool Invalid = false;

    switch (Ty) {
    case RECURSION:
      Invalid = KMD.RecursiveCall.hasValue() && KMD.RecursiveCall.get();
      break;
    case FPGA_PIPE_DYNAMIC_ACCESS:
      Invalid = KMD.FpgaPipeDynamicAccess.hasValue() &&
        KMD.FpgaPipeDynamicAccess.get();
    }

    if (Invalid) {
      Res.push_back(F.getName());
    }
  }

  return Res;
}

}}}
