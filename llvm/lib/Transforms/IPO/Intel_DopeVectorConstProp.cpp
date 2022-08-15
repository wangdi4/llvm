//===------- Intel_DopeVectorConstProp.cpp --------------------------------===//
//
// Copyright (C) 2019-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_DopeVectorConstProp.h"
#include "llvm/Analysis/Intel_DopeVectorAnalysis.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_OPAnalysisUtils.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Type.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

using namespace dvanalysis;

#define DEBUG_TYPE "dopevectorconstprop"
#define DEBUG_GLOBAL_CONSTPROP "dope-vector-global-const-prop"

STATISTIC(NumFormalsDVConstProp, "Number of DV formals const propagated");
STATISTIC(NumGlobalDVConstProp, "Number of Global DV const propagated");
STATISTIC(NumNestedDVConstProp, "Number of Nested DV const propagated");
STATISTIC(NumLocalDVConstProp, "Number of local DV const propagated");

// Enable the global dope vector constant propagation
static cl::opt<bool> DVGlobalConstProp("dope-vector-global-const-prop",
                                 cl::init(true), cl::ReallyHidden);

// Enable the local dope vector constant propagation
static cl::opt<bool> DVLocalConstProp("dope-vector-local-const-prop",
                                      cl::init(true), cl::ReallyHidden);

#if INTEL_FEATURE_SW_ADVANCED
// Lowest number of call instructions with the attribute
// "prefer-inline-tile-choice" in a function.
static cl::opt<unsigned> MinNumInlineTileMVCalls("dvcp-tile-mv-min-calls",
                                                 cl::init(7),
                                                 cl::ReallyHidden);
#endif // INTEL_FEATURE_SW_ADVANCED

//
// Return 'true' if the formal argument 'Arg' of Function 'F' is a pointer
// to a dope vector of rank 'ArrayRank' and 'ElementType', and has at least
// one provably constant value for its lower bound 'LB', stride 'ST', or
// extent 'EX'. The values in 'LB', 'ST', and 'EX' are stored in small
// vectors of size 'ArrayRank', and are set on return from this function,
// if the function returns 'true'. Optional values are used because a
// provably constant value may not be found for all values of LB, ST, and
// EX.
//
static bool hasDopeVectorConstants(const Function &F, const Argument &Arg,
                                   const uint32_t ArrayRank,
                                   const Type *ElementType,
                                   SmallVectorImpl<Optional<uint64_t>> &LB,
                                   SmallVectorImpl<Optional<uint64_t>> &ST,
                                   SmallVectorImpl<Optional<uint64_t>> &EX) {

  // Map 'V' to its potential integer constant value.
  auto OValue = [](Value *V) -> Optional<uint64_t> {
    auto CI = dyn_cast_or_null<ConstantInt>(V);
    return CI ? CI->getZExtValue() : Optional<uint64_t>();
  };

  // Merge two potentially constant values. The result is a constant value
  // only if 'X' and 'Y' are constants with the same value.
  auto Meet = [](Optional<uint64_t> X, Optional<uint64_t> Y) ->
      Optional<uint64_t> {
    return !X.hasValue() || !Y.hasValue() || X.getValue() != Y.getValue() ?
      Optional<uint64_t>() : X;
  };

  // Return 'true' if not all elements of the small vector 'V' are defined.
  auto IsBottom = [](SmallVectorImpl<Optional<uint64_t>> &V) -> bool {
    for (unsigned I = 0; I < V.size(); I++)
      if (V[I].hasValue())
        return false;
    return true;
  };

  bool FirstTime = true;
  const DataLayout &DL = F.getParent()->getDataLayout();

  // We have already checked that all of the users of 'F' are CallBase.
  // Merge the values of the dope vector constants of the actual arguments
  // corresponding to 'Arg'.
  for (const User *U : F.users()) {
    uint32_t ArRank;
    Type *ElemType = nullptr;
    auto CB = cast<CallBase>(U);
    Value *V = CB->getArgOperand(Arg.getArgNo());
    // Each actual must be a pointer to a dope vector of the same expected
    // rank and element type.
    Type *VTy = V->getType();
    if (!VTy->isPointerTy())
      return false;
    if (VTy->getContext().supportsTypedPointers()) {
      VTy = VTy->getNonOpaquePointerElementType();
      if (!isDopeVectorType(VTy, DL, &ArRank, &ElemType))
        return false;
      assert(ElemType &&
             "Pointer address not collected when checking for constants");
    }
    else {
      VTy = inferPtrElementType(*V);
      if (!VTy || !isDopeVectorType(VTy, DL, &ArRank, &ElemType))
        return false;
    }
    if (ArRank != ArrayRank || ElemType != ElementType)
      return false;
    // Use the dope analyzer to get the value of the dope vector constants.
    DopeVectorAnalyzer DVAActual(V);
    DVAActual.analyze(true);
    bool IsValid = DVAActual.getIsValid();
    if (!IsValid)
      return false;
    // Merge the values of the dope vector constants.
    if (FirstTime) {
      for (unsigned I = 0; I < ArRank; I++) {
        LB.push_back(OValue(DVAActual.getLowerBound(I)));
        ST.push_back(OValue(DVAActual.getStride(I)));
        EX.push_back(OValue(DVAActual.getExtent(I)));
      }
      FirstTime = false;
    } else {
      for (unsigned I = 0; I < ArRank; I++) {
        LB[I] = Meet(LB[I], OValue(DVAActual.getLowerBound(I)));
        ST[I] = Meet(ST[I], OValue(DVAActual.getStride(I)));
        EX[I] = Meet(EX[I], OValue(DVAActual.getExtent(I)));
      }
    }
    // If we are at bottom for all dope vector constants, there is no point to
    // continue.
    if (IsBottom(LB) && IsBottom(ST) && IsBottom(EX))
      return false;
  }
  return true;
}

//
// Use the information derived about the dope vector formal 'Arg' of rank
// 'ArrayRank' contained in 'DVAFormal' and the small vectors 'LB', 'ST', and
// 'EX' (representing the potentially constant values of 'Arg's lower bounds,
// strides, and extents), to replace references in the IR with those constant
// values.  Return 'true' if at least one replacement was performed.
//
static bool replaceDopeVectorConstants(Argument &Arg,
                                       DopeVectorAnalyzer &DVAFormal,
                                       const uint32_t ArrayRank,
                                       SmallVectorImpl<Optional<uint64_t>> &LB,
                                       SmallVectorImpl<Optional<uint64_t>> &ST,
                                       SmallVectorImpl<Optional<uint64_t>> &EX)
{
  // Use the 'Values' to replace the 'RFT' (either lower bound, stride, or
  // extent) for the 'GEP' representing an access to the array of lower
  // bound, extent, and stride information.
  auto ReplaceField = [&](SmallVectorImpl<Optional<uint64_t>> &Values,
                          DopeVectorAnalyzer::DopeVectorRankFields RFT,
                          GetElementPtrInst &GEP) -> bool {
    bool Change = false;
    // Get the base of the lower bound, stride, or extent array.
    auto FR = DVAFormal.findPerDimensionArrayFieldGEP(GEP, RFT);
    GetElementPtrInst *GEP2 = FR.first;
    if (!GEP2 || FR.second == DopeVectorAnalyzer::FindResult::FR_Invalid)
       return Change;
    for (unsigned I = 0; I < ArrayRank; I++) {
      if (!Values[I].hasValue())
        continue;
      // Get the Value representing an access to the lower bound, stride,
      // or extent of the specific dimension.
      Value *V = DVAFormal.findPerDimensionArrayFieldPtr(*GEP2, I);
      if (!V)
        continue;
      unsigned LoadCount = 0;
      // Replace all loads of the lower bound, stride, or extent from the
      // specific dimension with constants, if we have determined them to
      // be constant.
      for (User *W : V->users()) {
        // At this point, we have proved that only loads are the users of V
        auto LI = cast<LoadInst>(W);
        Type *I64Ty = IntegerType::getInt64Ty(GEP.getContext());
        auto CI = ConstantInt::get(I64Ty, Values[I].getValue(), false);
        LI->replaceAllUsesWith(CI);
        LoadCount++;
        Change = true;
      }
      LLVM_DEBUG({
        if (LoadCount > 0)
          dbgs() << "REPLACING " << LoadCount << " LOAD"
                 << (LoadCount > 1 ? "S " : " ") << "WITH "
                 << Values[I].getValue() << "\n";
      });
    }
    return Change;
  };

  // If 'GEP' points to the base of the per dimension array of a dope
  // vector, use 'DVAFormal' and the lower bound 'LB', stride 'ST', and
  // extent 'EX' to replace loads of these fields with constants.  Return
  // 'true' if at least one field is replaced.
  auto ReplaceFieldsForGEP = [&ReplaceField]
                             (GetElementPtrInst *GEP,
                              SmallVectorImpl<Optional<uint64_t>> &LB,
                              SmallVectorImpl<Optional<uint64_t>> &ST,
                              SmallVectorImpl<Optional<uint64_t>> &EX,
                              DopeVectorAnalyzer &DVAFormal) -> bool {
    bool Change = false;
    // Find the GEP accessing the array of lower bounds, strides, and extents
    // in the dope vector.
    auto DVFT = DVAFormal.identifyDopeVectorField(*(cast<GEPOperator>(GEP)));
    if (DVFT != DopeVectorFieldType::DV_PerDimensionArray) {
      LLVM_DEBUG(dbgs() << "COULD NOT FIND PER DIMENSION ARRAY\n");
      return Change;
    }
    // Replace constant lower bounds, strides, and extents with constants
    // in the IR.
    Change |= ReplaceField(EX, DopeVectorAnalyzer::DVR_Extent, *GEP);
    Change |= ReplaceField(ST, DopeVectorAnalyzer::DVR_Stride, *GEP);
    Change |= ReplaceField(LB, DopeVectorAnalyzer::DVR_LowerBound, *GEP);
    return Change;
  };

  // Replace dope vector fields with constants in the function where 'Arg'
  // is a dummy argument that is a pointer to a dope vector.
  bool Change = false;
  for (User *U : Arg.users()) {
    auto GEP = dyn_cast<GetElementPtrInst>(U);
    if (!GEP)
      continue;
    // If GEP points at the dope vector per dimension array, replace
    // the lower bound, strides, and extents of the dope vector with
    // constants when possible.
    Change |= ReplaceFieldsForGEP(GEP, LB, ST, EX, DVAFormal);
  }
  SmallPtrSet<Function *, 16> ContainedFunctionSet;
  // Replace dope vector fields with constants in that function's contained
  // functions.
  UplevelDVField UDVF = DVAFormal.getUplevelVar();
  Value *UpVar = UDVF.first;
  if (!UpVar)
    return Change;
  // The contained functions are a subset of the Users of UpVar.
  for (User *U : UpVar->users()) {
    auto CB = dyn_cast<CallBase>(U);
    if (!CB)
      continue;
    auto CF = CB->getCalledFunction();
    if (!CF)
      continue;
    // No need to repeat this, if we have already handled the contained
    // function.
    if (!ContainedFunctionSet.insert(CF).second)
      continue;
    // 'CF' is a contained function. Its 0th argument will be a pointer
    // to a structure, each field of which points to an uplevel variable.
    assert(CF->arg_size() != 0 && "Expecting at least one arg");
    if (CF->getContext().supportsTypedPointers())
      assert(CF->getArg(0)->getType()->isPointerTy() &&
          isUplevelVarType(
          CF->getArg(0)->getType()->getNonOpaquePointerElementType()) &&
          "Expecting pointer to uplevel type");
    // Identify GEPs that refer to the dope vector uplevel variable.
    Argument *Arg = CF->getArg(0);
    for (User *V : Arg->users()) {
      auto GEP = dyn_cast<GetElementPtrInst>(V);
      if (!GEP || GEP->getPointerOperand() != Arg)
        continue;
      auto CI1 = dyn_cast<ConstantInt>(GEP->getOperand(1));
      if (!CI1 || CI1->getZExtValue() != 0)
        continue;
      auto CI2 = dyn_cast<ConstantInt>(GEP->getOperand(2));
      if (!CI2 || CI2->getZExtValue() != UDVF.second)
        continue;
      // The GEP selects out the address of the dope vector variable.
      LLVM_DEBUG(dbgs() << "TESTING UPLEVEL #" << UDVF.second << " FOR "
                        << CF->getName() << "\n");
      for (User *W : GEP->users()) {
        auto LI = dyn_cast<LoadInst>(W);
        if (!LI || LI->getPointerOperand() != GEP)
          continue;
        for (User *X : LI->users()) {
          auto GEPDV = dyn_cast<GetElementPtrInst>(X);
          if (!GEPDV)
            continue;
          // If GEPDV points at the dope vector per dimension array, replace
          // the lower bound, strides, and extents of the uplevel variable
          // dope vector with constants when possible.
          Change |= ReplaceFieldsForGEP(GEPDV, LB, ST, EX, DVAFormal);
        }
      }
    }
  }
  return Change;
}

#if INTEL_FEATURE_SW_ADVANCED
// Return true if there is any condition in module M that should disable
// aggressive DVCP in the case of global dope vectors. The reason is that
// DVCP simplifies the instructions and makes the analysis difficult in
// loopopt.
static bool disableAggressiveDVCP(Module &M) {
  // Conditions to disable aggressive DVCP:
  //   1) The number of calls set as "prefer-inline-tile-choice" is equal
  //      or higher than MinNumInlineTileMVCalls
  //
  // TODO: List will be expanded as other conditions are identified.

  bool DisableAggressiveDVCP = false;
  for (auto &F : M.functions()) {
    unsigned NumCallsForInlineTile = 0;
    for (Instruction &I : instructions(F)) {
      auto *Call = dyn_cast<CallBase>(&I);
      if (!Call)
        continue;

      if (Call->hasFnAttr("prefer-inline-tile-choice"))
        NumCallsForInlineTile++;

      if (MinNumInlineTileMVCalls <= NumCallsForInlineTile) {
        DisableAggressiveDVCP = true;
        break;
      }
    }

    if (DisableAggressiveDVCP)
      break;
  }

  DEBUG_WITH_TYPE(DEBUG_GLOBAL_CONSTPROP, {
    if (DisableAggressiveDVCP)
      dbgs() << "Aggressive DVCP for global dope vectors is disabled\n";
  });

  return DisableAggressiveDVCP;
}
#endif // INTEL_FEATURE_SW_ADVANCED

// Actual function that propagates the constants for the input dope
// vector field
static bool propagateFieldConstant(DopeVectorFieldUse *DVField) {
  if (DVField->getIsBottom())
    return false;
  ConstantInt *CI = DVField->getConstantValue();
  if (!CI)
    return false;

  bool Change = false;
  unsigned LoadCount = 0;
  for(auto *LI : DVField->loads()) {
    if (DVField->isNotForDVCPLoad(LI)) {
      LLVM_DEBUG({
        dbgs() << "NOT REPLACING LOAD IN FXN "
               << LI->getFunction()->getName() << " ";
        LI->dump();
      });
      continue;
    }
    LoadCount++;
    LI->replaceAllUsesWith(CI);
    Change = true;
  }

  LLVM_DEBUG({
    if (Change)
      dbgs() << "REPLACING " << LoadCount << " LOAD"
             << (LoadCount > 1 ? "S " : " ") << "WITH "
             << CI->getZExtValue() << "\n";
  });

  return Change;
}

// Return true if the constants collected for the input GlobDV were propagated
static bool propagateGlobalDopeVectorConstants(GlobalDopeVector &GlobDV) {
  // Propagate the constants in the extent, stride and lower bound for the
  // input dope vector info
  auto PropagateDVConstant =[](DopeVectorInfo *DVInfo) -> bool {

    // Analysis must pass
    if (DVInfo->getAnalysisResult() != DopeVectorInfo::AnalysisResult::AR_Pass)
      return false;

    auto PtrAddr = DVInfo->getDopeVectorField(DV_ArrayPtr);
    assert(PtrAddr && "Accessing pointer address without collecting it");

    // If the array is not read, the extent, stride and lower bound won't be
    // read, don't propagate any data
    if (!PtrAddr->getIsRead())
      return false;

    unsigned long Rank = DVInfo->getRank();
    bool Change = false;
    for (unsigned long I = 0; I < Rank; I++) {
      auto *ExtentField = DVInfo->getDopeVectorField(DV_ExtentBase, I);
      auto *StrideField = DVInfo->getDopeVectorField(DV_StrideBase, I);
      auto *LBField = DVInfo->getDopeVectorField(DV_LowerBoundBase, I);

      assert((ExtentField && StrideField && LBField) &&
             "Trying to propagate dope vector constant information without "
             "collecting the proper information");

      Change |= propagateFieldConstant(ExtentField);
      Change |= propagateFieldConstant(StrideField);
      Change |= propagateFieldConstant(LBField);
    }

    if (Change)
      DVInfo->setConstantsPropagated();

    return Change;
  };

  // If the analysis didn't pass we can't propagate any constant
  if (GlobDV.getAnalysisResult() != GlobalDopeVector::AnalysisResult::AR_Pass)
    return false;

  // Propagate the constants for the global dope vector
  bool Change = false;
  Change = PropagateDVConstant(GlobDV.getGlobalDopeVectorInfo());
  if (Change)
    NumGlobalDVConstProp++;

  // Propagate the constants
  for (auto *NestedDV : GlobDV.getAllNestedDopeVectors())
    if (PropagateDVConstant(NestedDV)) {
      Change = true;
      NumNestedDVConstProp++;
    }
  return Change;
}

// Traverse through the global variables, and collect the information for
// those globals that are dope vectors. If the constant information was
// collected for the fields then propagate it.
static bool collectAndTransformDopeVectorGlobals(Module &M,
    const DataLayout &DL,
    std::function<const TargetLibraryInfo &(Function &F)> &GetTLI) {

  if (!DVGlobalConstProp)
    return false;

  bool Change = false;
#if INTEL_FEATURE_SW_ADVANCED
  // Check if we need to disable aggressive DVCP
  bool DisableAggressiveDVCP = disableAggressiveDVCP(M);
#endif // INTEL_FEATURE_SW_ADVANCED
  for (auto &Glob : M.globals()) {
    Type *GlobType = Glob.getValueType();

    if (!isDopeVectorType(GlobType, DL))
      continue;

    GlobalDopeVector GlobDV(&Glob, GlobType, GetTLI);
#if INTEL_FEATURE_SW_ADVANCED
    if (DisableAggressiveDVCP)
      GlobDV.disableAggressiveDVCP();
#endif // INTEL_FEATURE_SW_ADVANCED
    GlobDV.collectAndValidate(DL, /*ForDVCP=*/true);

    // Propagate the constants
    Change |= propagateGlobalDopeVectorConstants(GlobDV);

    DEBUG_WITH_TYPE(DEBUG_GLOBAL_CONSTPROP, {
      GlobDV.print();
      dbgs() << "\n";
    });
  }

  return Change;
}

static bool DopeVectorConstPropImpl(Module &M, WholeProgramInfo &WPInfo,
    std::function<const TargetLibraryInfo &(Function &F)> GetTLI) {

  // Return 'true' if not all of 'F's uses are CallBase.
  auto HasNonCallBaseUser = [](Function &F) -> bool {
    for (User *U : F.users()) {
      auto CB = dyn_cast<CallBase>(U);
      if (!CB)
        return true;
    }
    return false;
  };

  // Return 'true' if 'M' has at least one Fortran Function.
  auto ModuleHasFortranFunction = [](Module &M) -> bool {
    for (auto &F : M.functions())
      if (F.isFortran())
        return true;
    return false;
  };

  // Check if AVX2 is supported.
  LLVM_DEBUG(dbgs() << "DOPE VECTOR CONSTANT PROPAGATION: BEGIN\n");
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!WPInfo.isAdvancedOptEnabled(TTIAVX2)) {
    LLVM_DEBUG(dbgs() << "NOT AVX2\n");
    LLVM_DEBUG(dbgs() << "DOPE VECTOR CONSTANT PROPAGATION: END\n");
    return false;
  }

  // There must be at least one Fortran function.
  if (!ModuleHasFortranFunction(M)) {
    LLVM_DEBUG(dbgs() << "NO FORTRAN FUNCTION\n");
    LLVM_DEBUG(dbgs() << "DOPE VECTOR CONSTANT PROPAGATION: END\n");
    return false;
  }

  bool Change = false;
  const DataLayout &DL = M.getDataLayout();
  // Initialize OTPMapper only for opaque pointers
  std::unique_ptr<OpaquePointerTypeMapper> OPTMapper =
      (!M.getContext().supportsTypedPointers())
          ? std::make_unique<OpaquePointerTypeMapper>(M)
          : nullptr;

  for (auto &F : M.functions()) {
    // Cases we will give up on, at least for now.
    if (F.hasAddressTaken()) {
      LLVM_DEBUG(dbgs() << "FUNCTION " << F.getName()
                        << " IS ADDRESS TAKEN\n");
      continue;
    }
    if (HasNonCallBaseUser(F)) {
      LLVM_DEBUG(dbgs() << "FUNCTION " << F.getName()
                        << " HAS NON-CALLBASE USER\n");
      continue;
    }

    if (F.hasLocalLinkage()) {
      // Look for formal args which are pointers to dope vectors
      for (Argument &Arg : F.args()) {
        // Find if Arg is a pointer to a dope vector.
        uint32_t ArRank;
        Type *ElemType;
        const Type *Ty = Arg.getType();
        if (!Ty->isPointerTy())
          continue;
        if (Ty->getContext().supportsTypedPointers()) {
          Ty = Ty->getNonOpaquePointerElementType();
          if (!isDopeVectorType(Ty, DL, &ArRank, &ElemType))
             continue;
        } else {
          Ty = OPTMapper->getPointerElementType(&Arg);
          if (!Ty || !isDopeVectorType(Ty, DL, &ArRank, &ElemType))
            continue;
        }

        LLVM_DEBUG({
          // NOTE: This needs to be updated when opaque pointers support is
          // enabled.
          if (Ty->getContext().supportsTypedPointers())
            assert(ElemType &&
                   "Pointer address not collected for debugging");

          dbgs() << "DV FOUND: ARG #" << Arg.getArgNo() << " "
                 << F.getName() << " " << ArRank << " x ";
          if (ElemType)
            ElemType->dump();
          else
            dbgs() << "<UNKNOWN ELEMENT TYPE>\n";
        });
        DopeVectorAnalyzer DVAFormal(&Arg, Ty);
        DVAFormal.analyze(false);
        bool IsValid = DVAFormal.getIsValid();
        LLVM_DEBUG(dbgs() << (IsValid ? "VALID" : "NOT VALID") << "\n");
        if (!IsValid)
          continue;
        if (!DVAFormal.analyzeDopeVectorUseInFunction(F)) {
          LLVM_DEBUG(dbgs() << "UNSAFE USE OF DOPE VECTOR\n");
          continue;
        }
        // Collect the constant dope vector lower bounds, strides, and extents
        // into the small vectors.
        SmallVector<Optional<uint64_t>, 3> LowerBound;
        SmallVector<Optional<uint64_t>, 3> Stride;
        SmallVector<Optional<uint64_t>, 3> Extent;
        if (!hasDopeVectorConstants(F, Arg, ArRank, ElemType,
            LowerBound, Stride, Extent)) {
          LLVM_DEBUG(dbgs() << "NO CONSTANT DOPE VECTOR FIELDS\n");
          continue;
        }
        // Alter the IR to reflect the determined dope vector constants.
        NumFormalsDVConstProp++;
        LLVM_DEBUG({
          for (unsigned I = 0; I < ArRank; I++) {
            if (LowerBound[I].hasValue())
              dbgs() << "LB[" << I << "] = " << LowerBound[I].getValue() << "\n";
            if (Stride[I].hasValue())
              dbgs() << "ST[" << I << "] = " << Stride[I].getValue() << "\n";
            if (Extent[I].hasValue())
              dbgs() << "EX[" << I << "] = " << Extent[I].getValue() << "\n";
          }
        });
        Change |= replaceDopeVectorConstants(Arg, DVAFormal, ArRank,
            LowerBound, Stride, Extent);
      }
    } else {
      LLVM_DEBUG(dbgs() << "FUNCTION " << F.getName()
                        << " DOES NOT HAVE LOCAL LINKAGE\n");
      if (!DVLocalConstProp)
        continue;
    }

    if (F.isDeclaration())
      continue;

    // Try to find the dope vectors that are local to the function and won't be
    // passed to other functions. This case is simpler than arguments that are
    // dope vectors. It checks for allocation sites (AllocaInst), then runs
    // the DopeVectorAnalyzer and propagates the data. The DopeVectorAnalyzer
    // will check that the information doesn't escape out of the function.
    for (Instruction &I : instructions(F)) {
      auto *AllocI = dyn_cast<AllocaInst>(&I);
      if (!AllocI)
        continue;

      uint32_t ArRank;
      Type *ElemType;
      Type *Ty = AllocI->getAllocatedType();
      if (!isDopeVectorType(Ty, DL, &ArRank, &ElemType))
        continue;
      LLVM_DEBUG({
        // NOTE: This needs to be updated when opaque pointers support is
        // enabled.
        if (Ty->getContext().supportsTypedPointers())
          assert(ElemType &&
                 "Pointer address not collected for debugging AllocaInst");

        dbgs() << "  LOCAL DV FOUND: " << *AllocI
               << "\n    RANK: " << ArRank << "\n    TYPE: ";
        if (ElemType)
          ElemType->dump();
        else
          dbgs() << "<UNKNOWN ELEMENT TYPE>\n";
      });

      DopeVectorAnalyzer DVALocal(AllocI, Ty, &GetTLI);
      DVALocal.analyze(/*ForCreation*/ true, /*IsLocal*/ true);
      bool IsValid = DVALocal.getIsValid();
      LLVM_DEBUG(dbgs() << "    ANALYSIS RESULT: "
                        << (IsValid ? "VALID" : "NOT VALID") << "\n");
      if (!IsValid)
        continue;

      bool DVCPLocalRun = false;
      for (unsigned I = 0; I < ArRank; I++) {
        DopeVectorFieldUse *ExtentField =
            const_cast<DopeVectorFieldUse *>(&DVALocal.getExtentField(I));
        DopeVectorFieldUse *StrideField =
            const_cast<DopeVectorFieldUse *>(&DVALocal.getStrideField(I));
        DopeVectorFieldUse *LowerBoundField =
            const_cast<DopeVectorFieldUse*>(&DVALocal.getLowerBoundField(I));

        DVCPLocalRun = propagateFieldConstant(ExtentField);
        DVCPLocalRun |= propagateFieldConstant(StrideField);
        DVCPLocalRun |= propagateFieldConstant(LowerBoundField);
      }

      if (DVCPLocalRun) {
        NumLocalDVConstProp++;
        Change |= DVCPLocalRun;
      }

      LLVM_DEBUG(dbgs() << "\n");
    }
  }

  // Collect the information related to the global dope vectors
  Change |= collectAndTransformDopeVectorGlobals(M, DL, GetTLI);

  LLVM_DEBUG(dbgs() << "DOPE VECTOR CONSTANT PROPAGATION: END\n");
  return Change;
}

namespace {

struct DopeVectorConstPropLegacyPass : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  DopeVectorConstPropLegacyPass(void)
      : ModulePass(ID) {
    initializeDopeVectorConstPropLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
    AU.addPreserved<AndersensAAWrapperPass>();
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    auto WPInfo = getAnalysis<WholeProgramWrapperPass>().getResult();
    auto GetTLI = [this](Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };
    return DopeVectorConstPropImpl(M, WPInfo, GetTLI);
  }
};

}

char DopeVectorConstPropLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(DopeVectorConstPropLegacyPass, "dopevectorconstprop",
    "DopeVectorConstProp", false, false)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(DopeVectorConstPropLegacyPass, "dopevectorconstprop",
    "DopeVectorConstProp", false, false)

ModulePass *llvm::createDopeVectorConstPropLegacyPass(void) {
  return new DopeVectorConstPropLegacyPass();
}

DopeVectorConstPropPass::DopeVectorConstPropPass(void) {}

PreservedAnalyses DopeVectorConstPropPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(F);
  };
  if (!DopeVectorConstPropImpl(M, WPInfo, GetTLI))
    return PreservedAnalyses::all();
  auto PA = PreservedAnalyses();
  PA.preserve<AndersensAA>();
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}
