//===-------DTransFieldModRef.cpp - DTrans Field ModRef Analysis-----------===//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

// This file implements checks for determining whether fields of structures
// may be modified or referenced by specific functions. This is done based
// on DTrans analysis by first verifying the use of structure passes the DTrans
// safety checks. Pointer fields of the structure are then checked to verify
// they can be tracked to an allocation site, and that aliases of the
// allocated pointer do not escape.

#include "Intel_DTrans/Analysis/DTransFieldModRef.h"

#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Format.h"

// Debug type for field mod/ref analyzer.
#define DTRANS_FMR "dtrans-fmr"

// Print the candidates that field analysis will be performed on before the
// the analysis is done.
#define DTRANS_FMR_CANDIDATES_PRE "dtrans-fmr-candidates-pre"

// Print the candidates after the analysis is done.
#define DTRANS_FMR_CANDIDATES_POST "dtrans-fmr-candidates-post"

// Detailed trace of instructions analyzed by the field mod/ref analyzer.
#define DTRANS_FMR_VERBOSE "dtrans-fmr-verbose"

// Print information regarding the queries for mod/ref information when
// checking whether a function call will modify or reference the location.
#define DTRANS_FMR_QUERIES "dtrans-fmr-queries"

namespace llvm {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// This option is strictly for testing the behavior of querying the Mod/Ref
// status functionality. When this is set, after the results are computed, each
// load/store of the function will be evaluated against the calls made by the
// function, and the results printed.
static cl::opt<bool> DTransFieldModRefEval("dtrans-fieldmodref-eval",
                                           cl::init(false), cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

static const char *ModRefInfoToString(ModRefInfo &Info) {
  switch (Info) {
  case ModRefInfo::NoModRef:
    return "NoModRef";
  case ModRefInfo::Mod:
    return "Mod";
  case ModRefInfo::Ref:
    return "Ref";
  case ModRefInfo::ModRef:
    return "ModRef";
  case ModRefInfo::Must:
    return "Must";
  case ModRefInfo::MustMod:
    return "MustMod";
  case ModRefInfo::MustRef:
    return "MustRef";
  case ModRefInfo::MustModRef:
    return "MustModRef";
  }
  llvm_unreachable("Fall-through from fully covered switch");
}

void FieldModRefResult::addReader(llvm::StructType *Ty, size_t FieldNum,
                                  Function *F) {
  auto &Cand = Candidates[std::make_pair(Ty, FieldNum)];
  Cand.FieldReaders.insert(F);
}

void FieldModRefResult::addWriter(llvm::StructType *Ty, size_t FieldNum,
                                  Function *F) {
  auto &Cand = Candidates[std::make_pair(Ty, FieldNum)];
  Cand.FieldWriters.insert(F);
}

bool FieldModRefResult::isReader(llvm::StructType *Ty, size_t FieldNum,
                                 Function *F) {
  auto &Cand = Candidates[std::make_pair(Ty, FieldNum)];
  return Cand.FieldReaders.count(F);
}

bool FieldModRefResult::isWriter(llvm::StructType *Ty, size_t FieldNum,
                                 Function *F) {
  auto &Cand = Candidates[std::make_pair(Ty, FieldNum)];
  return Cand.FieldWriters.count(F);
}

void FieldModRefResult::addValueReader(llvm::StructType *Ty, size_t FieldNum,
                                       Function *F) {
  auto &Cand = Candidates[std::make_pair(Ty, FieldNum)];
  Cand.ValueReaders.insert(F);
}

void FieldModRefResult::addValueWriter(llvm::StructType *Ty, size_t FieldNum,
                                       Function *F) {
  auto &Cand = Candidates[std::make_pair(Ty, FieldNum)];
  Cand.ValueWriters.insert(F);
}

bool FieldModRefResult::isValueReader(llvm::StructType *Ty, size_t FieldNum,
                                      Function *F) {
  auto &Cand = Candidates[std::make_pair(Ty, FieldNum)];
  return Cand.ValueReaders.count(F);
}

bool FieldModRefResult::isValueWriter(llvm::StructType *Ty, size_t FieldNum,
                                      Function *F) {
  auto &Cand = Candidates[std::make_pair(Ty, FieldNum)];
  return Cand.ValueWriters.count(F);
}

bool FieldModRefResult::isCandidate(llvm::StructType *Ty, size_t FieldNum) {
  return Candidates.count(std::make_pair(Ty, FieldNum));
}

void FieldModRefResult::reset() { Candidates.clear(); }

ModRefInfo FieldModRefResult::getModRefInfo(const CallBase *Call,
                                            const MemoryLocation &Loc) {
  // A helper routine to retrieve a {structure type, field index} pair from the
  // GetElementPtrInst.
  //
  // This is similar to the function dtrans::getStructField, but because this
  // is operating without the results of DTransAnalysisInfo being available
  // byte-flattened GEPs will not be supported, resulting in conservative
  // results being reported for these.
  auto GetStructField = [](const GetElementPtrInst *GEP)
      -> std::pair<llvm::StructType *, uint64_t> {
    if (!GEP || !GEP->hasAllConstantIndices())
      return std::make_pair(nullptr, 0);

    // Need to give up for anything that is a byte-flattened GEP form because we
    // don't have DTrans analysis info preserved.
    if (GEP->getNumIndices() == 1)
      return std::make_pair(nullptr, 0);

    auto StructTy = dyn_cast<StructType>(GEP->getSourceElementType());
    if (!StructTy)
      return std::make_pair(nullptr, 0);

    if (!cast<ConstantInt>(GEP->getOperand(1))->isZeroValue())
      return std::make_pair(nullptr, 0);

    uint64_t FieldIndex = 0;
    for (unsigned NI = 2; NI <= GEP->getNumIndices(); ++NI) {
      auto IndexConst = cast<ConstantInt>(GEP->getOperand(NI));
      FieldIndex = IndexConst->getLimitedValue();
      if (FieldIndex >= StructTy->getNumElements())
        return std::make_pair(nullptr, 0);
      if (NI == GEP->getNumIndices())
        break;
      auto *Ty = StructTy->getElementType(FieldIndex);
      auto *NewStructTy = dyn_cast<StructType>(Ty);
      if (!NewStructTy)
        return std::make_pair(nullptr, 0);
      StructTy = NewStructTy;
    }
    return std::make_pair(StructTy, FieldIndex);
  };

  DEBUG_WITH_TYPE(DTRANS_FMR_QUERIES, {
    dbgs() << " getModRefInfo_begin\n";
    dbgs() << "Call:  " << *Call << "\n";
    if (!isa<Function>(Loc.Ptr))
      dbgs() << "Loc: " << *(Loc.Ptr) << "\n";
    else
      dbgs() << "Loc: " << cast<Function>(Loc.Ptr)->getName() << "\n";
  });

  // Return a conservative answer for direct calls to memcpy, memset, etc. This
  // is necessary because information about what they modify within our data
  // structures is stored with the calling function. Alternatively, we could
  // return the Mod/Ref info about the calling function, but this case is
  // not necessary at the moment.
  if (isa<MemIntrinsic>(Call)) {
    DEBUG_WITH_TYPE(DTRANS_FMR_QUERIES,
                    dbgs() << "Result: ModRef [MemIntrinsic]\n");
    return ModRefInfo::ModRef;
  }

  auto *GEP = dyn_cast<GetElementPtrInst>(Loc.Ptr);
  if (!GEP) {
    DEBUG_WITH_TYPE(DTRANS_FMR_QUERIES, dbgs() << "Result: ModRef [Not GEP]\n");
    return ModRefInfo::ModRef;
  }

  // Try to identify the structure type and field being requested in the GEP,
  // and the see if any information is available for structure field.
  auto StTyAndField = GetStructField(GEP);
  if (!StTyAndField.first) {
    DEBUG_WITH_TYPE(DTRANS_FMR_QUERIES,
                    dbgs() << "Result: ModRef [No info for structure]\n");
    return ModRefInfo::ModRef;
  }

  llvm::StructType *StTy = StTyAndField.first;
  size_t FieldNum = StTyAndField.second;
  if (!isCandidate(StTy, FieldNum)) {
    DEBUG_WITH_TYPE(DTRANS_FMR_QUERIES,
                    dbgs() << "Result: ModRef [Field " << FieldNum << " of "
                           << *StTy << " is not a tracked candidate]\n");
    return ModRefInfo::ModRef;
  }

  // Any field read or written by an indirect call (or reachable from the
  // indirect call) has been eliminated as a candidate. Therefore, this indirect
  // call must not modify or reference the field.
  if (Call->isIndirectCall())
    return ModRefInfo::NoModRef;

  // Information is available for the structure field, check the function and
  // all the reachable functions from it to determine whether the field may be
  // modified or referenced by the call.
  ModRefInfo MRI = ModRefInfo::NoModRef;
  SmallPtrSet<Function *, 16> Visited;
  unionModRefInfo(MRI, Call->getCalledFunction(), StTy, FieldNum,
                  /*Indirect=*/true, Visited);
  DEBUG_WITH_TYPE(DTRANS_FMR_QUERIES,
                  dbgs() << "Result: " << ModRefInfoToString(MRI) << "\n");

  return MRI;
}

// Update the \p Status value to add Mod or Ref (or both) based on the
// behavior that function \p F may have on field number \p FNum of Structure
// \p StTy.
//
// When \p Indirect is set, also include the functions that read or write values
// within the indirect array pointed to by a pointer field.
void FieldModRefResult::unionModRefInfo(ModRefInfo &Status, Function *F,
                                        llvm::StructType *StTy, unsigned FNum,
                                        bool Indirect,
                                        SmallPtrSetImpl<Function *> &Visited,
                                        unsigned Indent) {
  // We skip over external functions because any unsupported cases would have
  // been disqualified during the candidate selection and analysis.
  if (F->isDeclaration())
    return;

  if (!Visited.insert(F).second)
    return;

  DEBUG_WITH_TYPE(DTRANS_FMR_QUERIES, {
    dbgs().indent(Indent);
    dbgs() << "  Checking: " << F->getName() << "\n";
  });

  if (isReader(StTy, FNum, F))
    Status = unionModRef(Status, ModRefInfo::Ref);
  if (isWriter(StTy, FNum, F))
    Status = unionModRef(Status, ModRefInfo::Mod);

  if (Indirect) {
    if (isValueReader(StTy, FNum, F))
      Status = unionModRef(Status, ModRefInfo::Ref);
    if (isValueWriter(StTy, FNum, F))
      Status = unionModRef(Status, ModRefInfo::Mod);
  }

  // No need to continue, once the worst case is encountered.
  if (isModAndRefSet(Status))
    return;

  // Check all the called functions. Currently, we compute this on demand,
  // without caching results because we don't expect many queries to be made
  // from LoopOpt. If this changes to be an AliasAnalysis result, this may need
  // to be changed.
  SmallPtrSet<Function *, 16> ToCheck;
  for (auto &I : instructions(F)) {
    if (auto *Call = dyn_cast<CallBase>(&I)) {
      // We only need to check direct calls here, because any field accessible
      // by an indirect function call has been disqualified as a candidate.
      if (Function *Callee = Call->getCalledFunction())
        ToCheck.insert(Callee);
    }
  }

  for (auto *Callee : ToCheck) {
    unionModRefInfo(Status, Callee, StTy, FNum, Indirect, Visited, Indent + 2);
    if (isModAndRefSet(Status))
      return;
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void FieldModRefResult::dump() const { print(dbgs()); }

void FieldModRefResult::print(raw_ostream &OS) const {
  auto PrintFunctionSet = [](raw_ostream &OS, const FunctionSet &FSet) {
    dtrans::printCollectionSorted(
        OS, FSet.begin(), FSet.end(), ", ",
        [](const Function *F) { return F->getName(); });
  };

  auto PrintCandidate =
      [&PrintFunctionSet](
          const std::pair<CandFieldTy, FieldModRefCandidateInfo> &P) {
        std::string OutputVal;
        raw_string_ostream OS(OutputVal);

        OS << "FieldModRefInfo: " << *P.first.first << "\n";
        // Format decimal is used here so that when sorting occurs on this
        // generated string, field 2, output as ' 2', will appear before field
        // 10, output as '10'.
        OS << "  FieldNum: " << format_decimal(P.first.second, 2) << "\n";
        OS << "    FieldReaders: ";
        PrintFunctionSet(OS, P.second.FieldReaders);
        OS << "\n    FieldWriters: ";
        PrintFunctionSet(OS, P.second.FieldWriters);
        OS << "\n    ValueReaders: ";
        PrintFunctionSet(OS, P.second.ValueReaders);
        OS << "\n    ValueWriters: ";
        PrintFunctionSet(OS, P.second.ValueWriters);
        OS << "\n";

        OS.flush();
        return OutputVal;
      };

  OS << "FieldModRefResult:\n";
  dtrans::printCollectionSorted(OS, Candidates.begin(), Candidates.end(), "",
                                PrintCandidate);
  OS << "\n";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

bool DTransModRefAnalyzer::runAnalysis(Module &M,
                                       DTransAnalysisInfo &DTransInfo,
                                       WholeProgramInfo &WPInfo,
                                       FieldModRefResult &FMRResult) {
  DTInfo = &DTransInfo;
  FMRResult.reset();

  if (!WPInfo.isWholeProgramSafe())
    return false;

  if (!DTInfo->useDTransAnalysis())
    return false;

  initialize(M);
  DEBUG_WITH_TYPE(DTRANS_FMR_CANDIDATES_PRE, {
    printCandidateInfo("ModRef candidate structures before analysis:");
  });

  if (Candidates.empty())
    return false;

  analyzeModule(M);

  // Set the results for candidates that passed the safety checks into the
  // preserved object.
  populateResults(FMRResult);

  DEBUG_WITH_TYPE(DTRANS_FMR_CANDIDATES_POST, {
    printCandidateInfo("ModRef candidate structures after analysis:");
  });

  DEBUG_WITH_TYPE(DTRANS_FMR, { FMRResult.print(dbgs()); });

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (DTransFieldModRefEval)
    printQueryResults(M, FMRResult);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  return true;
}

// Do an initial pruning of fields that may not be able to be analyzed for the
// sets of functions that modify or reference the fields based on the DTrans
// safety data of their container structures.
void DTransModRefAnalyzer::initialize(Module &M) {
  // Helper to collect all the functions that may be called from Function \p
  // F. Also, include all the functions those calls make.
  std::function<void(Function *, SmallPtrSetImpl<Function *> &)>
      CollectReachable;
  CollectReachable =
      [&CollectReachable](Function *F,
                          SmallPtrSetImpl<Function *> &Closure) -> void {
    if (!Closure.insert(F).second)
      return;

    for (auto &I : instructions(F))
      if (auto *Call = dyn_cast<CallBase>(&I))
        if (Function *Callee = Call->getCalledFunction())
          CollectReachable(Callee, Closure);
  };

  // List of safety conditions that definitely invalidate the ability to
  // perform some ModRef analysis of the field usage.
  const dtrans::SafetyData MandatoryMask =
      dtrans::BadCasting | dtrans::BadAllocSizeArg |
      dtrans::BadPtrManipulation | dtrans::AmbiguousGEP |
      dtrans::UnsafePointerStore | dtrans::GlobalPtr | dtrans::UnsafePtrMerge |
      dtrans::BadMemFuncSize | dtrans::BadMemFuncManipulation |
      dtrans::AmbiguousPointerTarget | dtrans::AddressTaken |
      dtrans::NoFieldsInStruct | dtrans::NestedStruct | dtrans::SystemObject |
      dtrans::MismatchedArgUse | dtrans::BadCastingConditional |
      dtrans::BadCastingForRelatedTypes |
      dtrans::BadPtrManipulationForRelatedTypes |
      dtrans::UnsafePointerStoreRelatedTypes |
      dtrans::MemFuncNestedStructsPartialWrite |
      dtrans::UnsafePointerStoreConditional;

  // List of additional safety conditions that are done to be conservative.
  // These could be relaxed in the future with additional analysis.
  const dtrans::SafetyData AdditionSafetyMask =
      dtrans::GlobalInstance | dtrans::GlobalArray | dtrans::LocalInstance |
      dtrans::WholeStructureReference | dtrans::NestedStruct |
      dtrans::HasVTable | dtrans::HasFnPtr | dtrans::HasZeroSizedArray;

  dtrans::SafetyData ModRefSafetyMask = MandatoryMask | AdditionSafetyMask;

  // When DTransOutOfBoundsOK is set, the address of any field is assumed to
  // be able to be used to access any other field.
  if (DTInfo->getDTransOutOfBoundsOK())
    ModRefSafetyMask |= dtrans::FieldAddressTaken;

  // Find all the address taken functions. Any field that is read/written by
  // an address taken function (or is reachable from an address taken
  // function) will be disqualified from the analysis.
  SmallPtrSet<Function *, 8> AddrTakenFuncs;
  for (auto &F : M)
    if (F.hasAddressTaken())
      AddrTakenFuncs.insert(&F);

  // Add all the functions that are reachable from the address taken calls
  SmallPtrSet<Function *, 16> AddrTakenClosure;
  for (auto *F : AddrTakenFuncs)
    CollectReachable(F, AddrTakenClosure);

  for (dtrans::TypeInfo *TI : DTInfo->type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo)
      continue;

    auto *StTy = cast<llvm::StructType>(StInfo->getLLVMType());
    if (StTy->isLiteral())
      continue;

    if (StInfo->testSafetyData(ModRefSafetyMask)) {
      setAllFieldsToBottom(StInfo);
      DEBUG_WITH_TYPE(DTRANS_FMR, dbgs() << "Disqualifying all fields of "
                                            "structure based on safety data: "
                                         << *StInfo->getLLVMType() << "\n");
      continue;
    }

    bool HasNonBottomFields = false;
    size_t FNum = 0;
    for (dtrans::FieldInfo &FI : StInfo->getFields()) {
      // The analysis does not support array accesses or pointers-to-pointers,
      // so disqualify those now.
      llvm::Type *FieldTy = FI.getLLVMType();
      if (FieldTy->isArrayTy()) {
        DEBUG_WITH_TYPE(DTRANS_FMR, dbgs() << "Disqualifying field #" << FNum
                                           << " of " << *StInfo->getLLVMType()
                                           << ": Array field\n");
        FI.setRWBottom();
      } else if (FieldTy->isPointerTy() &&
                 FieldTy->getPointerElementType()->isPointerTy()) {
        DEBUG_WITH_TYPE(DTRANS_FMR, dbgs() << "Disqualifying field #" << FNum
                                           << " of " << *StInfo->getLLVMType()
                                           << ": Ptr-to-Ptr field\n");
        FI.setRWBottom();
      } else if (FI.isAddressTaken()) {
        DEBUG_WITH_TYPE(DTRANS_FMR, dbgs() << "Disqualifying field #" << FNum
                                           << " of " << *StInfo->getLLVMType()
                                           << ": Address taken field\n");
        FI.setRWBottom();
      }
      else if (FI.isMismatchedElementAccess()) {
        DEBUG_WITH_TYPE(DTRANS_FMR, dbgs() << "Disqualifying field #" << FNum
          << " of " << *StInfo->getLLVMType()
          << ": Mismatched element access on field\n");
        FI.setRWBottom();
      }
      else if (std::any_of(FI.writers().begin(), FI.writers().end(),
                           [&AddrTakenClosure](Function *F) {
                             return AddrTakenClosure.count(F) == true;
                           })) {
        // Disqualify the field if a function that writes it is address taken,
        // or may be called from a function that is address taken.
        DEBUG_WITH_TYPE(DTRANS_FMR, dbgs()
                                        << "Disqualifying field #" << FNum
                                        << " of " << *StInfo->getLLVMType()
                                        << ": Writer function reachable from "
                                           "address taken function\n");
        FI.setRWBottom();
      } else if (std::any_of(FI.readers().begin(), FI.readers().end(),
                             [&AddrTakenClosure](Function *F) {
                               return AddrTakenClosure.count(F) == true;
                             })) {
        // Also exclude readers to simplify the implementation by allowing
        // all indirect calls to be ignored later.
        DEBUG_WITH_TYPE(DTRANS_FMR, dbgs() << "Disqualifying field #" << FNum
                                           << " of " << *StInfo->getLLVMType()
                                           << ":  Reader function reachable "
                                              "from address taken function\n");
        FI.setRWBottom();
      } else {
        HasNonBottomFields = true;
      }
      ++FNum;
    }

    if (HasNonBottomFields)
      Candidates.insert(StTy);
  }
}

// Set the ModRef analysis state for all fields of the structure (and any
// nested structures) to bottom.
void DTransModRefAnalyzer::setAllFieldsToBottom(dtrans::StructInfo *StInfo) {
  for (auto &FI : StInfo->getFields()) {
    FI.setRWBottom();
    dtrans::TypeInfo *FieldTypeInfo = DTInfo->getTypeInfo(FI.getLLVMType());
    if (auto *FieldStInfo = dyn_cast<dtrans::StructInfo>(FieldTypeInfo))
      setAllFieldsToBottom(FieldStInfo);
  }
}

void DTransModRefAnalyzer::analyzeModule(Module &M) {
  for (auto &F : M)
    analyzeFunction(F);
}

// Check all the GEPs of the function that could get the address of a
// structure field that we are interested in for the usage of the field.
void DTransModRefAnalyzer::analyzeFunction(Function &F) {
  DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                  dbgs() << "Analyzing function: " << F.getName() << "\n");

  if (F.isDeclaration())
    return;

  for (auto &I : instructions(F)) {
    if (auto *GEP = dyn_cast<GetElementPtrInst>(&I)) {
      // Check for a byte-flattened GEP getting the address of a structure
      // field.
      if (GEP->getNumIndices() == 1) {
        auto InfoPair = DTInfo->getByteFlattenedGEPElement(GEP);
        if (!InfoPair.first)
          continue;

        llvm::Type *SrcTy = InfoPair.first;
        if (auto *SrcStTy = dyn_cast<llvm::StructType>(SrcTy)) {
          if (Candidates.count(SrcStTy)) {
            dtrans::StructInfo *StInfo =
                cast<dtrans::StructInfo>(DTInfo->getTypeInfo(SrcStTy));
            size_t FieldNum = InfoPair.second;
            dtrans::FieldInfo &FI = StInfo->getField(FieldNum);
            if (FI.isRWBottom())
              continue;

            if (!analyzeFieldForEscapes(GEP, SrcStTy, FieldNum, FI)) {
              DEBUG_WITH_TYPE(DTRANS_FMR,
                              dbgs() << "Set field Mod/Ref to bottom: Field #"
                                     << InfoPair.second << " of "
                                     << SrcStTy->getName() << "\n");
              FI.setRWBottom();

              // When DTrans is allowing for out-of-bounds mode, an unsupported
              // access to one field needs to disqualify all fields of the
              // structure.
              if (DTInfo->getDTransOutOfBoundsOK())
                setAllFieldsToBottom(StInfo);
            }
          }
        }
        continue;
      }

      auto StTyAndField = DTInfo->getStructField(cast<GEPOperator>(GEP));
      llvm::StructType *StTy = StTyAndField.first;
      size_t FieldNum = StTyAndField.second;
      if (!StTy)
        continue;

      if (!Candidates.count(StTy))
        continue;

      dtrans::StructInfo *StInfo =
          cast<dtrans::StructInfo>(DTInfo->getTypeInfo(StTy));
      dtrans::FieldInfo &FI = StInfo->getField(FieldNum);
      if (FI.isRWBottom())
        continue;

      if (!analyzeFieldForEscapes(GEP, StTy, FieldNum, FI)) {
        DEBUG_WITH_TYPE(DTRANS_FMR,
                        dbgs() << "Set field Mod/Ref to bottom: Field #"
                               << StTyAndField.second << " of "
                               << StTyAndField.first->getName() << "\n");
        FI.setRWBottom();

        // When DTrans is allowing for out-of-bounds mode, an unsupported
        // access to one field needs to disqualify all fields of the
        // structure.
        if (DTInfo->getDTransOutOfBoundsOK())
          setAllFieldsToBottom(StInfo);
      }
    }
  }
}

// Check if the \p GEP, which holds the address of a structure field, is
// used in a way that will store the address into another memory location that
// can be accessed without going through the containing structure.
bool DTransModRefAnalyzer::analyzeFieldForEscapes(GetElementPtrInst *GEP,
                                                  llvm::StructType *StTy,
                                                  size_t FieldNum,
                                                  dtrans::FieldInfo &FI) {
  DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE, {
    dbgs() << "  Analyze field #" << FieldNum << " of " << StTy->getName()
           << "\n";
    dbgs() << "  " << *GEP << "\n";
  });

  llvm::Type *FieldTy = FI.getLLVMType();
  bool IsPointer = FieldTy->isPointerTy();

  // For non-pointers, we only need to verify the pointer produced by the GEP
  // does not get stored to another memory location, which has already been
  // done by the DTrans FieldAddressTaken safety check.
  if (!IsPointer)
    return true;

  // For pointers, we need to examine what is done to the pointer once it
  // is loaded to determine whether there may be another memory location that
  // can be used to access the pointer elements.
  SmallPtrSet<Value *, 8> AliasSet;
  gatherValueAliases(GEP, /*IncludeNonPointerTypes=*/false, AliasSet);

  // Keep track of uses that have been checked, so they are not repeated.
  SmallPtrSet<Value *, 8> Verified;
  for (auto *A : AliasSet) {
    DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                    dbgs() << "      Field alias:      " << *A << "\n");

    // Check whether all the uses of the value are supported.
    for (auto *U : A->users()) {
      DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                      dbgs() << "      Field alias user:   " << *U << "\n");
      if (!Verified.insert(U).second)
        continue;

      // If the use is just to create a new alias, skip it.
      if (AliasSet.count(U))
        continue;

      // Check that the pointer loaded from the field is not escaped to
      // another location.
      if (auto *LI = dyn_cast<LoadInst>(U)) {
        if (!checkAllValuesUsingIndirectAddress(StTy, FieldNum, LI))
          return false;
        continue;
      }

      // Check if the value stored into the field is not escaped to another
      // location.
      if (auto *SI = dyn_cast<StoreInst>(U)) {
        if (!checkStoredValueSafe(StTy, FieldNum, SI, SI->getValueOperand()))
          return false;

        continue;
      }

      DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                      dbgs() << "Unhandled use of field: " << *U << "\n");
      return false;
    }
  }

  return true;
}

// Find a set of Value objects that also could be used to access the Value \p
// V. Store these into \p Aliases.
//
// When \p IncludeNonPointerTypes is set, also add any integer objects produced
// from PtrToInt, etc, that can be converted back to be a pointer alias of the
// value object.
void DTransModRefAnalyzer::gatherValueAliases(
    Value *V, bool IncludeNonPointerTypes, SmallPtrSetImpl<Value *> &Aliases) {
  if (!Aliases.insert(V).second)
    return;

  // Look for transfers of V from one Value object to another. Also, include
  // GEPs that access an offset location from V.
  for (auto *U : V->users()) {
    if (auto *Sel = dyn_cast<SelectInst>(U))
      gatherValueAliases(Sel, IncludeNonPointerTypes, Aliases);
    else if (auto *Phi = dyn_cast<PHINode>(U))
      gatherValueAliases(Phi, IncludeNonPointerTypes, Aliases);
    else if (auto *BC = dyn_cast<BitCastInst>(U))
      gatherValueAliases(BC, IncludeNonPointerTypes, Aliases);
    else if (auto *GEP = dyn_cast<GetElementPtrInst>(U))
      if (GEP->getNumIndices() == 1)
        gatherValueAliases(GEP, IncludeNonPointerTypes, Aliases);

    if (IncludeNonPointerTypes) {
      // These are needed when analyzing the pattern used to perform a memory
      // alignment on the allocated pointer.
      if (auto *PTI = dyn_cast<PtrToIntInst>(U))
        gatherValueAliases(PTI, IncludeNonPointerTypes, Aliases);
      if (auto *ITP = dyn_cast<IntToPtrInst>(U))
        gatherValueAliases(ITP, IncludeNonPointerTypes, Aliases);
      if (auto *BinOp = dyn_cast<BinaryOperator>(V))
        if (BinOp->getOpcode() == Instruction::And ||
            BinOp->getOpcode() == Instruction::Add)
          if (isa<Constant>(BinOp->getOperand(1)))
            gatherValueAliases(BinOp, IncludeNonPointerTypes, Aliases);
    }
  }
}

// A field value, which is a pointer, has been loaded into \p V.
//   %v = load i32*, i32** %field_addr
//
// We need to check the users of V to determine that V does not get written to
// memory or escape the function. Also, check for reads/writes that reference
// the memory of the pointer array.
bool DTransModRefAnalyzer::checkAllValuesUsingIndirectAddress(
    llvm::StructType *StTy, size_t FieldNum, Value *V) {

  // First, collect all the value objects that directly or indirectly hold the
  // value V. Direct references result from Bitcasts, PhiNodes, Select
  // instructions. Indirect references result from a GetElementPtr that adjust
  // the value to a different offset of the array.
  SmallPtrSet<Value *, 8> AliasSet;
  gatherValueAliases(V, /*IncludeNonPointerTypes=*/false, AliasSet);

  // Next, check that the value does not escape via an alias.
  SmallPtrSet<Value *, 8> Verified;
  for (auto *A : AliasSet) {
    DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                    dbgs() << "        Value alias:      " << *A << "\n");

    for (auto *U : A->users()) {
      DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                      dbgs() << "        Value alias user:   " << *U << "\n");
      if (!Verified.insert(U).second)
        continue;

      // If the use is just to create a new alias, skip it.
      if (AliasSet.count(U))
        continue;

      // Loading the value is always safe.
      if (auto *LI = dyn_cast<LoadInst>(U)) {
        DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                        dbgs() << "Reader of indirect array: "
                               << LI->getFunction()->getName() << "\n");

        addIndirectReader(StTy, FieldNum, LI->getFunction());
        continue;
      }

      // Using an alias location to store a value is safe, unless the value
      // being stored is one of the aliasing locations.
      if (auto *SI = dyn_cast<StoreInst>(U)) {
        if (AliasSet.count(SI->getValueOperand()))
          return false;

        DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                        dbgs() << "Writer of indirect array: "
                               << SI->getFunction()->getName() << "\n");

        addIndirectWriter(StTy, FieldNum, SI->getFunction());
        continue;
      }

      // ICmp will not escape the value.
      if (isa<ICmpInst>(U))
        continue;

      // Passing the address to a memory intrinsic is safe because these have
      // been analyzed by the DTrans safety checks.
      if (auto *MI = dyn_cast<MemSetInst>(U)) {
        DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                        dbgs() << "Writer of indirect array: "
                               << MI->getFunction()->getName() << "\n");

        addIndirectWriter(StTy, FieldNum, MI->getFunction());
        continue;
      }

      bool isMemCpy = isa<MemCpyInst>(U);
      bool isMemMove = isa<MemMoveInst>(U);
      if (isMemCpy || isMemMove) {
        Function *F = cast<Instruction>(U)->getFunction();
        DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                        dbgs() << "Reader/Writer of indirect array: "
                               << F->getName() << "\n");

        addIndirectWriter(StTy, FieldNum, F);
        addIndirectReader(StTy, FieldNum, F);
        continue;
      }

      // Allow passing the field value to free.
      if (auto *CI = dyn_cast<CallBase>(V)) {
        dtrans::CallInfo *Info = DTInfo->getCallInfo(CI);
        if (Info && (Info->getCallInfoKind() == dtrans::CallInfo::CIK_Free))
          continue;
      }

      DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                      dbgs() << "Unhandled use of field value location: " << *U
                             << "\n");
      return false;
    }
  }

  return true;
}

void DTransModRefAnalyzer::addIndirectReader(llvm::StructType *Ty,
                                             size_t FieldNum, Function *F) {
  auto &Cand = IndirectFieldReaders[std::make_pair(Ty, FieldNum)];
  Cand.insert(F);
}

void DTransModRefAnalyzer::addIndirectWriter(llvm::StructType *Ty,
                                             size_t FieldNum, Function *F) {
  auto &Cand = IndirectFieldWriters[std::make_pair(Ty, FieldNum)];
  Cand.insert(F);
}

// Check that a value being stored into the a pointer field is safe.
//
// This is checking that the value stored is either a nullptr, or can
// be traced back to a malloc call. And that the result of the malloc
// does not get stored to a location that does not require access to the
// structure field.
bool DTransModRefAnalyzer::checkStoredValueSafe(llvm::StructType *StTy,
                                                size_t FieldNum, StoreInst *SI,
                                                Value *V) {
  if (V == Constant::getNullValue(V->getType()))
    return true;

  DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                  dbgs() << "        Stored value: " << *V << "\n");

  // Try to trace the stored pointer back to a memory allocation call.
  SmallVector<Value *, 8> AllocPath;
  Value *Allocation = traceToAllocation(V, AllocPath);
  if (!Allocation)
    return false;

  // Make sure none of the intermediate values were written to memory except
  // with the incoming StoreInst or within the allocated memory itself. First,
  // we need to know the set of Value objects that directly or indirectly
  // contain the address. In this case, also capture the integer values that
  // are used for adjusting the alignment of the pointer.
  SmallPtrSet<Value *, 16> AliasSet;
  for (auto AllocPathElem : AllocPath)
    gatherValueAliases(AllocPathElem, /*IncludeNonPointerTypes=*/true,
                       AliasSet);

  // Next walk all the Value objects the allocation went to, and check if they
  // are supported.
  SmallPtrSet<Value *, 8> Verified;
  for (auto *A : AliasSet) {
    DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                    dbgs() << "        Store alias:      " << *A << "\n");
    for (auto *U : A->users()) {
      DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                      dbgs() << "        Store alias user:   " << *U << "\n");
      if (!Verified.insert(U).second)
        continue;

      // If the use is just to create a new alias, skip it.
      if (AliasSet.count(U))
        continue;

      if (auto *AliasSI = dyn_cast<StoreInst>(U)) {
        // Disregard the store that started the search for the memory
        // allocation.
        if (AliasSI == SI)
          continue;

        // Storing one of the alias values itself is not safe, unless the
        // memory location is also part of the alias set. i.e. it will only be
        // reachable when referencing the allocated memory.
        if (AliasSet.count(AliasSI->getValueOperand()))
          if (!AliasSet.count(AliasSI->getPointerOperand())) {
            return false;
          } else {
            // assume %114 holds an the alignment adjusted allocation address.
            // %116 = inttoptr i64 %114 to i8**
            // %117 = getelementptr inbounds i8*, i8** %116, i64 -1
            // store i8* %106, i8** %117, align 8
            //
            // The memory address of %117 is part of the allocated memory so
            // is safe to be stored to. But, we are only expecting one use of
            // the value, so check that.
            if (!AliasSI->getPointerOperand()->hasOneUse())
              return false;
          }
        continue;
      }

      // Loading from a pointer of the allocated memory block is safe.
      if (isa<LoadInst>(U))
        continue;

      // ICmp will not escape the value.
      if (isa<ICmpInst>(U))
        continue;

      // Passing the address to memory intrinsics is safe since DTrans safety
      // checked them. Remember the function as writing the allocated memory of
      // the pointer field.
      if (auto *MI = dyn_cast<MemSetInst>(U)) {
        DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                        dbgs() << "Writer of indirect array: "
                               << MI->getFunction()->getName() << "\n");

        addIndirectWriter(StTy, FieldNum, MI->getFunction());
        continue;
      }

      bool isMemCpy = isa<MemCpyInst>(U);
      bool isMemMove = isa<MemMoveInst>(U);
      if (isMemCpy || isMemMove) {
        Function *F = cast<Instruction>(U)->getFunction();
        DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                        dbgs() << "Reader/Writer of indirect array: "
                               << F->getName() << "\n");

        addIndirectWriter(StTy, FieldNum, F);
        addIndirectReader(StTy, FieldNum, F);
        continue;
      }

      DEBUG_WITH_TYPE(DTRANS_FMR_VERBOSE,
                      dbgs() << "Unhandled use of field value location: " << *U
                             << "\n");
      return false;
    }
  }

  return true;
}

// Try to trace back the Value \p V to a memory allocation call.
// If successful, return the allocation call, otherwise nullptr.
//
// The stored value of the pointer field needs to look like:
//   %83 = tail call i8* @malloc(i64 %82)
//   %84 = icmp eq i8* %83, null (ignored for now)
//   %89 = ptrtoint i8* %83 to i64 (optional)
//   %90 = add i64 %89, 71 (optional)
//   %91 = and i64 %90, -64 (optional)
//   %92 = inttoptr i64 %91 to i8* (optional)
//   store i8* %92, i8** %97
Value *
DTransModRefAnalyzer::traceToAllocation(Value *V,
                                        SmallVectorImpl<Value *> &AllocPath) {
  if (auto *BC = dyn_cast<BitCastInst>(V)) {
    AllocPath.push_back(V);
    return traceToAllocation(BC->getOperand(0), AllocPath);
  }

  if (auto *PTI = dyn_cast<PtrToIntInst>(V)) {
    AllocPath.push_back(V);
    return traceToAllocation(PTI->getOperand(0), AllocPath);
  }

  if (auto *ITP = dyn_cast<IntToPtrInst>(V)) {
    AllocPath.push_back(V);
    return traceToAllocation(ITP->getOperand(0), AllocPath);
  }

  if (auto *BinOp = dyn_cast<BinaryOperator>(V)) {
    if (BinOp->getOpcode() == Instruction::And ||
        BinOp->getOpcode() == Instruction::Add) {
      if (isa<Constant>(BinOp->getOperand(1))) {
        AllocPath.push_back(V);
        return traceToAllocation(BinOp->getOperand(0), AllocPath);
      }
    }
  }

  if (auto *CI = dyn_cast<CallBase>(V)) {
    dtrans::CallInfo *Info = DTInfo->getCallInfo(CI);
    if (Info && Info->getCallInfoKind() == dtrans::CallInfo::CIK_Alloc) {
      AllocPath.push_back(V);
      return CI;
    }
  }

  return nullptr;
}

// Set the FieldModRefResult data structure for the fields that passed the
// safety checks. The DTransAnalysisInfo may be discarded because that pass is
// not preserved, but the FieldModRefResult should be preserved for use by
// other passes.
void DTransModRefAnalyzer::populateResults(FieldModRefResult &FMRResult) {
  for (auto *StTy : Candidates) {
    auto *StInfo = cast<dtrans::StructInfo>(DTInfo->getTypeInfo(StTy));
    size_t FieldNum = 0;
    for (auto &FI : StInfo->getFields()) {
      if (FI.isRWBottom()) {
        ++FieldNum;
        continue;
      }

      FI.setRWComputed();
      for (auto *F : FI.readers())
        FMRResult.addReader(StTy, FieldNum, F);

      for (auto *F : FI.writers())
        FMRResult.addWriter(StTy, FieldNum, F);

      ++FieldNum;
    }

    for (auto It : IndirectFieldReaders) {
      llvm::StructType *StTy = It.first.first;
      size_t FieldNum = It.first.second;
      if (FMRResult.isCandidate(StTy, FieldNum))
        for (auto *F : It.second)
          FMRResult.addValueReader(StTy, FieldNum, F);
    }

    for (auto It : IndirectFieldWriters) {
      llvm::StructType *StTy = It.first.first;
      size_t FieldNum = It.first.second;
      if (FMRResult.isCandidate(StTy, FieldNum))
        for (auto *F : It.second)
          FMRResult.addValueWriter(StTy, FieldNum, F);
    }
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DTransModRefAnalyzer::printCandidateInfo(StringRef Header) {
  DTransAnalysisInfo &Info = *DTInfo;
  auto PrintCandidate = [&Info](StructType *Ty) {
    std::string OutputVal;
    raw_string_ostream OS(OutputVal);
    OS << "LLVMType: ";
    Ty->print(OS);
    OS << "\n";
    auto *StInfo = cast<dtrans::StructInfo>(Info.getTypeInfo(Ty));
    size_t FNum = 0;
    for (dtrans::FieldInfo &Field : StInfo->getFields()) {
      OS << "  " << FNum << ")Field LLVM Type: " << *Field.getLLVMType()
         << "\n";
      OS << "    Readers: ";
      dtrans::printCollectionSorted(
          OS, Field.readers().begin(), Field.readers().end(), ", ",
          [](const Function *F) { return F->getName(); });
      OS << "\n";
      OS << "    Writers: ";
      dtrans::printCollectionSorted(
          OS, Field.writers().begin(), Field.writers().end(), ", ",
          [](const Function *F) { return F->getName(); });
      OS << "\n";
      OS << "    RWState: "
         << (Field.isRWBottom() ? "bottom"
                                : (Field.isRWComputed() ? "computed" : "top"))
         << "\n";
      ++FNum;
    }
    OS.flush();
    return OutputVal;
  };

  dbgs() << Header << "\n";
  dtrans::printCollectionSorted(dbgs(), Candidates.begin(), Candidates.end(),
                                "\n", PrintCandidate);
  dbgs() << "\n";
}

void DTransModRefAnalyzer::printQueryResults(Module &M,
                                             FieldModRefResult &Result) {
  for (auto &F : M) {
    SmallVector<Instruction *, 16> MemInst;
    SmallVector<CallBase *, 16> Calls;

    for (auto &I : instructions(F)) {
      if (auto *LI = dyn_cast<LoadInst>(&I))
        MemInst.push_back(LI);
      else if (auto *SI = dyn_cast<StoreInst>(&I))
        MemInst.push_back(SI);
      else if (auto *Call = dyn_cast<CallBase>(&I))
        Calls.push_back(Call);
    }

    for (auto *Call : Calls)
      for (auto *I : MemInst) {
        dbgs() << "FieldModRefQuery Begin:\n  Function: " << F.getName()
               << "\n";
        dbgs() << "  Instruction: " << *I << "\n";
        dbgs() << "  Call       : " << *Call << "\n";
        MemoryLocation Loc = MemoryLocation::get(I);
        ModRefInfo LocResult = Result.getModRefInfo(Call, Loc);

        dbgs() << "  Result     : " << ModRefInfoToString(LocResult) << "\n";
        dbgs() << "FieldModRefQuery End\n\n";
      }
  }
}

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
} // end namespace llvm

using namespace llvm;

DTransFieldModRefAnalysisWrapper::DTransFieldModRefAnalysisWrapper()
    : ModulePass(ID) {
  initializeDTransFieldModRefAnalysisWrapperPass(
      *PassRegistry::getPassRegistry());
}

bool DTransFieldModRefAnalysisWrapper::runOnModule(Module &M) {
  DTransAnalysisWrapper &DTAnalysisWrapper =
      getAnalysis<DTransAnalysisWrapper>();
  DTransAnalysisInfo &DTInfo = DTAnalysisWrapper.getDTransInfo(M);

  auto &FMRResult = getAnalysis<DTransFieldModRefResultWrapper>().getResult();
  WholeProgramInfo &WPInfo = getAnalysis<WholeProgramWrapperPass>().getResult();
  return Impl.runAnalysis(M, DTInfo, WPInfo, FMRResult);
}

void DTransFieldModRefAnalysisWrapper::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.addRequired<DTransAnalysisWrapper>();
  AU.addRequired<DTransFieldModRefResultWrapper>();
  AU.addRequired<WholeProgramWrapperPass>();
  AU.setPreservesAll();
}

char DTransFieldModRefAnalysisWrapper::ID = 0;

INITIALIZE_PASS_BEGIN(DTransFieldModRefAnalysisWrapper,
                      "dtrans-fieldmodref-analysis",
                      "DTrans field mod/ref analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(DTransFieldModRefResultWrapper)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransFieldModRefAnalysisWrapper,
                    "dtrans-fieldmodref-analysis",
                    "DTrans field mod/ref analysis", false, true)

ModulePass *llvm::createDTransFieldModRefAnalysisWrapperPass() {
  return new DTransFieldModRefAnalysisWrapper();
}

AnalysisKey DTransFieldModRefAnalysis::Key;

DTransFieldModRefAnalysis::Result
DTransFieldModRefAnalysis::run(Module &M, ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  DTransModRefAnalyzer Analyzer;
  FieldModRefResult &FMRResult = AM.getResult<DTransFieldModRefResult>(M);
  Analyzer.runAnalysis(M, DTransInfo, WPInfo, FMRResult);
  return FMRResult;
}

char DTransFieldModRefResultWrapper::ID = 0;
INITIALIZE_PASS(DTransFieldModRefResultWrapper, "dtrans-fieldmodref-result",
                "dtrans field mod/ref result", false, true)

DTransFieldModRefResultWrapper::DTransFieldModRefResultWrapper()
    : ImmutablePass(ID) {
  initializeDTransFieldModRefResultWrapperPass(
      *PassRegistry::getPassRegistry());
}

void DTransFieldModRefResultWrapper::getAnalysisUsage(
  AnalysisUsage &AU) const {
  AU.setPreservesAll();
}

ImmutablePass *llvm::createDTransFieldModRefResultWrapperPass() {
  return new DTransFieldModRefResultWrapper();
}

AnalysisKey DTransFieldModRefResult::Key;

FieldModRefResult DTransFieldModRefResult::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  return FieldModRefResult();
}

FieldModRefResult DTransFieldModRefResult::run(Function &F,
                                               FunctionAnalysisManager &AM) {
  return FieldModRefResult();
}
