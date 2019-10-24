//===---------------- AOSToSOA.cpp - DTransAOStoSOAPass -------------------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Array of Structures to Structure of Arrays
// data layout optimization pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/AOSToSOA.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Analysis/DTransAnnotator.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOptBase.h"
#include "Intel_DTrans/Transforms/DTransOptUtils.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
using namespace llvm;
using namespace dtrans;

#define DEBUG_TYPE "dtrans-aostosoa"

// Debug type to show IR at various stages of the transformation. For example,
// the initial instruction conversion, prior to post-processing the function,
// and the final result.
#define AOSTOSOA_IR "dtrans-aostosoa-ir"

// Debug type to show function attributes conversion for pointer parameters
// that are changed to integers.
#define AOSTOSOA_ATTRIBUTES "dtrans-aostosoa-attributes"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// This option is used during testing to allow qualifying specific structure
// types be converted via the AOS-to-SOA transform without running the
// profitability heuristics. (The type must pass all other qualification tests,
// just the profitability test is skipped).
//
// This is a comma separated list of structure type names that will not be
// disqualified by the profitability heuristic.
static cl::opt<std::string>
    DTransAOSToSOAHeurOverride("dtrans-aostosoa-heur-override",
                               cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Minimum frequency relative to hottest structure frequency required to
// enable transformation of a qualified structure with the AOS-to-SOA transform.
// This value is a percentage that the structure's frequency must meet relative
// to the maximum structure frequency.
//   i.e. 100 * struct_freq / max_struct_freq >= threshold
static cl::opt<unsigned>
    DTransAOSToSOAFrequencyThreshold("dtrans-aostosoa-frequency-threshold",
                                     cl::init(20), cl::ReallyHidden);

// When set, this option causes the peeling index size to be 32-bits, instead of
// the number of bits required to hold a pointer type, if possible. The safety
// checks of dependent structures may prevent the index from being 32-bits, if
// it is determined that those structures cannot have their sizes changed.
static cl::opt<bool> DTransAOSToSOAIndex32("dtrans-aostosoa-index32",
                                           cl::init(true), cl::ReallyHidden);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// This class is a helper that can be passed to an output stream operator for
// printing structure types in the debug traces. For named structures, the print
// method outputs the structure's name. For unnamed structures, the print method
// outputs the structure's type.
class StructNamePrintHelper {
public:
  StructNamePrintHelper(llvm::Type *Ty) : Ty(Ty) {}

  void print(raw_ostream &OS) const {
    auto *StructTy = dyn_cast<llvm::StructType>(Ty);
    assert(StructTy && "Expected structure type");
    if (StructTy->hasName()) {
      OS << StructTy->getStructName();
      return;
    }

    OS << "unnamed " << *StructTy;
  }

private:
  llvm::Type *Ty;
};

raw_ostream &operator<<(raw_ostream &OS, const StructNamePrintHelper &Helper) {
  Helper.print(OS);
  return OS;
}
#endif

namespace {
// This class is used during the type remapping process to perform the
// translation of a null value pointer to an integer index.
//
// The AOS to SOA conversion modifies pointers to the structure to be
// integer indices. During the type remapping, pointers to the constant
// nullptr need to be converted to be represented with an integer 0 index.
class AOSToSOAMaterializer : public ValueMaterializer {
public:
  AOSToSOAMaterializer(ValueMapTypeRemapper &TypeRemapper)
      : TypeRemapper(TypeRemapper) {}

  virtual ~AOSToSOAMaterializer() {}

  virtual Value *materialize(Value *V) override {
    // Check if a null value of a different type needs to be generated.
    auto *C = dyn_cast<Constant>(V);
    if (!C)
      return nullptr;

    if (!C->isNullValue())
      return nullptr;

    Type *Ty = V->getType();
    Type *ReTy = TypeRemapper.remapType(Ty);
    if (Ty == ReTy)
      return nullptr;

    return Constant::getNullValue(ReTy);
  }

private:
  ValueMapTypeRemapper &TypeRemapper;
};

// This class is responsible for all the transformation work for the AOS to SOA
// with Indexing conversion.
class AOSToSOATransformImpl : public DTransOptBase {
private:
  // This enumeration is used to indicate the type of conversion an
  // instruction needs when checking whether the transformation needs to
  // process the instruction.
  //   AOS_NoConv  - Instruction does not need to be converted.
  //   AOS_SOAConv - Instruction requires the AOS-to-SOA with peeling
  //                 conversion.
  //   AOS_DepConv - Instruction requires changes due to data structure size
  //                 change made to a dependent type.
  typedef enum { AOS_NoConv, AOS_SOAConv, AOS_DepConv } AOSConvType;

public:
  // Constructor that takes parameters needed for the base class, plus a list of
  // types that have been qualified for the transformation.
  AOSToSOATransformImpl(
      DTransAnalysisInfo &DTInfo, LLVMContext &Context, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
      StringRef DepTypePrefix, DTransTypeRemapper *TypeRemapper,
      AOSToSOAMaterializer *Materializer,
      SmallVectorImpl<dtrans::StructInfo *> &Types)
      : DTransOptBase(&DTInfo, Context, DL, GetTLI, DepTypePrefix, TypeRemapper,
                      Materializer),
        PeelIndexWidth(64), PeelIndexType(nullptr),
        PointerShrinkingEnabled(false), AnnotationFilenameGEP(nullptr) {
    std::copy(Types.begin(), Types.end(), std::back_inserter(TypesToTransform));

    PtrSizedIntType = Type::getIntNTy(Context, DL.getPointerSizeInBits());
    Int8PtrType = llvm::Type::getInt8PtrTy(Context);
  }

  ~AOSToSOATransformImpl() {}

  // Create new data types for each of the types being converted.
  virtual bool prepareTypes(Module &M) override {
    // Collect the list of structure types that have dependencies on the types
    // being converted. When shrinking the index to 32-bits, instructions that
    // are dependent on the size of dependent data structures will need to be
    // updated.

    SmallVector<dtrans::StructInfo *, 4> Qualified;
    unsigned PointerSizeInBits = DL.getPointerSizeInBits();
    PointerShrinkingEnabled = DTransAOSToSOAIndex32 && PointerSizeInBits == 64;

    for (auto *StInfo : TypesToTransform) {
      StructType *OrigTy = cast<StructType>(StInfo->getLLVMType());
      auto It = TypeToDependentTypes.find(OrigTy);
      if (It == TypeToDependentTypes.end()) {
        Qualified.push_back(StInfo);
        continue;
      }

      bool DepQualified = true;
      for (auto *DepTy : It->second) {
        // TODO: This can be improved to ignore cases that do not contain a
        // pointer to the type being transformed. Such as structures containing
        // a pointer-to-pointer of the type being converted, or a containing a
        // function pointer that refers to a type being converted.

        // We only need to consider safety information for dependent types that
        // are structures, because arrays of types being transformed have
        // already prevented transforming a type within an array. However, we
        // may encounter an array here because we are not currently filtering
        // the dependent types that contains arrays of ptr-to-ptr for the type,
        // but those do not affect our ability to transform a structure.
        if (!isa<llvm::StructType>(DepTy))
          continue;

        // Don't check dependent types that are directly being transformed by
        // AOS-to-SOA.
        if (std::find(TypesToTransform.begin(), TypesToTransform.end(),
                      DTInfo->getTypeInfo(DepTy)) != TypesToTransform.end())
          continue;

        // Verify whether it is going to be safe to change a pointer type within
        // a type that refers to a type to be converted into an integer type.
        if (!checkDependentTypeSafety(DepTy)) {
          LLVM_DEBUG(dbgs() << "DTRANS-AOSTOSOA: Disqualifying type: "
                            << StructNamePrintHelper(OrigTy)
                            << " based on safety conditions of dependent type: "
                            << StructNamePrintHelper(DepTy) << "\n");
          DepQualified = false;
          break;
        }

        // Verify whether it is going to be safe to use a 32-bit integer type
        // within a dependent type.
        if (PointerShrinkingEnabled &&
            !checkDependentTypeSafeForShrinking(M, DepTy)) {
          LLVM_DEBUG(dbgs() << "DTRANS-AOSTOSOA: Peeling index shrinking "
                               "inhibited due to safety checks on: "
                            << StructNamePrintHelper(DepTy) << "\n");
          PointerShrinkingEnabled = false;
          continue;
        }

        DepTypesToTransform.insert(DepTy);

        LLVM_DEBUG(dbgs() << "DTRANS-AOSTOSOA: Transforming type    : "
                          << StructNamePrintHelper(OrigTy) << "\n"
                          << "                 will also affect type: "
                          << StructNamePrintHelper(DepTy) << "\n");
      }

      if (DepQualified)
        Qualified.push_back(StInfo);
    }

    std::swap(Qualified, TypesToTransform);

    // If indexes are not being changed from the size of a pointer then we
    // don't need to transform instructions dealing with dependent types. Clear
    // the list to avoid checks for them within processFunction.
    if (!PointerShrinkingEnabled)
      DepTypesToTransform.clear();

    initializePeeledIndexType(PointerShrinkingEnabled ? 32 : PointerSizeInBits);

    // Sort the structures by name so that the elements in vector of
    // original types to new types will be in a consistent order regardless
    // of the order the candidate structures were populated. This is only
    // necessary for being sure the id values used for the pointer annotations
    // that get inserted will always be deterministic.
    std::sort(TypesToTransform.begin(), TypesToTransform.end(),
              [](dtrans::StructInfo *a, dtrans::StructInfo *b) {
                return cast<StructType>(a->getLLVMType())->getName() <
                       cast<StructType>(b->getLLVMType())->getName();
              });
    for (auto *StInfo : TypesToTransform) {
      StructType *OrigTy = cast<StructType>(StInfo->getLLVMType());
      StructType *NewTy = StructType::create(
          Context, (Twine("__SOA_" + OrigTy->getName()).str()));
      TypeRemapper->addTypeMapping(OrigTy, NewTy);
      TypeRemapper->addTypeMapping(OrigTy->getPointerTo(),
                                   getPeeledIndexType());
      OrigToNewTypeMapping.push_back({OrigTy, NewTy});
    }

    return !OrigToNewTypeMapping.empty();
  }

  // Set the structure body of all the types this transformation created.
  virtual void populateTypes(Module &M) override {
    for (auto &ONPair : OrigToNewTypeMapping) {
      Type *OrigTy = ONPair.first;
      Type *NewTy = ONPair.second;

      SmallVector<Type *, 8> DataTypes;
      StructType *OrigStructTy = cast<StructType>(OrigTy);
      for (auto *MemberTy : OrigStructTy->elements()) {
        DataTypes.push_back(TypeRemapper->remapType(MemberTy)->getPointerTo());
      }

      StructType *NewStructTy = cast<StructType>(NewTy);
      NewStructTy->setBody(DataTypes);
      LLVM_DEBUG(dbgs() << "\nDTRANS-AOSTOSOA: Type replacement:\n  Old: "
                        << *OrigTy << "\n  New: " << *NewStructTy << "\n\n");
    }

    unsigned Count = 0;
    for (auto &ONPair : OrigToNewTypeMapping) {
      std::string AllocationStr("{dtrans} AOS-to-SOA allocation");
      std::string PeelIndexStr("{dtrans} AOS-to-SOA peeling index");
      std::string ExtensionName = "";
      std::string CountStr(std::to_string(Count));

      // We normally only expect a single structure to be transformed,
      // so don't append a unique extension on the first set of variable
      // names.
      if (Count != 0)
        ExtensionName = CountStr;
      Count++;

      // Create the variables and a constant Value object that will
      // be used in calls to llvm.ptr.annoation intrinsics to mark
      // the memory block allocation and peeling index addresses.
      // Separate strings will be created for each structure transformed, where
      // the 'id' element within the string can be used to pair the allocation
      // annotation with the peeling index annotations.
      //
      // This will create a Value object of the form:
      // i8 *getelementptr inbounds([38 x i8],
      //       [38 x i8] * @__intel_dtrans_aostosoa_alloc, i32 0, i32 0)
      AllocationAnnotationGEP[ONPair.first] =
          DTransAnnotator::createConstantStringGEP(
              DTransAnnotator::getAnnotationVariable(
                  M, DTransAnnotator::DPA_AOSToSOAAllocation,
                  AllocationStr + " {id:" + CountStr + "}", ExtensionName),
              0);

      // Create the object for the peeling index annotations:
      // i8* getelementptr inbounds ([41 x i8],
      //       [41 x i8]* @__intel_dtrans_aostosoa_index
      PeelIndexAnnotationGEP[ONPair.first] =
          DTransAnnotator::createConstantStringGEP(
              DTransAnnotator::getAnnotationVariable(
                  M, DTransAnnotator::DPA_AOSToSOAIndex,
                  PeelIndexStr + " {id:" + CountStr + "}", ExtensionName),
              0);
    }

    // Create an empty string for the filename operand of the ptr annotation
    // call.
    AnnotationFilenameGEP = DTransAnnotator::createConstantStringGEP(
        DTransAnnotator::createGlobalVariableString(M, "", ""), 0);
  }

  // Create a new global variable for each type peeled that will serve as the
  // base pointer to the peeled variable.
  virtual void prepareModule(Module &M) override {
    for (auto &ONPair : OrigToNewTypeMapping) {
      StructType *StType = cast<StructType>(ONPair.first);
      StructType *PeelTy = cast<StructType>(ONPair.second);

      auto *PeelVar = new GlobalVariable(
          M, PeelTy, false, GlobalValue::InternalLinkage,
          /*init=*/ConstantAggregateZero::get(PeelTy),
          "__soa_" + StType->getName(),
          /*insertbefore=*/nullptr, GlobalValue::NotThreadLocal,
          /*AddressSpace=*/0, /*isExternallyInitialized=*/false);
      PeeledTypeToVariable.insert(std::make_pair(PeelTy, PeelVar));
      LLVM_DEBUG(dbgs() << "DTRANS-AOSTOSOA: PeelVar: " << *PeelVar << "\n");
    }
  }

  virtual void processFunction(Function &F) override {
    SmallVector<GetElementPtrInst *, 16> GEPsToConvert;
    SmallVector<GetElementPtrInst *, 16> ByteGEPsToConvert;
    SmallVector<BitCastInst *, 16> BCsToConvert;
    SmallVector<BinaryOperator *, 16> BinOpsToConvert;
    SmallVector<PtrToIntInst *, 16> PTIsToConvert;
    SmallVector<LoadInst *, 16> LoadsToConvert;
    SmallVector<StoreInst *, 16> StoresToConvert;

    SmallVector<std::pair<AllocCallInfo *, StructInfo *>, 4> AllocsToConvert;
    SmallVector<std::pair<FreeCallInfo *, StructInfo *>, 4> FreesToConvert;
    SmallVector<std::pair<MemfuncCallInfo *, StructInfo *>, 4>
        MemfuncsToConvert;

    // These lists are for instructions that need to be updated because they
    // operate on types that are dependent on the type being transformed.
    SmallVector<GetElementPtrInst *, 16> DepByteGEPsToConvert;
    SmallVector<BinaryOperator *, 16> DepBinOptsToConvert;
    SmallVector<std::pair<AllocCallInfo *, StructInfo *>, 4> DepAllocsToResize;
    SmallVector<std::pair<MemfuncCallInfo *, StructInfo *>, 4>
        DepMemfuncsToResize;

    LLVM_DEBUG(dbgs() << "\nDTRANS-AOSTOSOA: Processing function: "
                      << F.getName() << "\n");

    for (auto It = inst_begin(&F), E = inst_end(&F); It != E; ++It) {
      AOSConvType ConvType = AOS_NoConv;
      Instruction *I = &*It;
      if (auto *Call = dyn_cast<CallBase>(I)) {
        updateCallAttributes(Call);

        // Check if the call needs to be transformed based on the CallInfo.
        if (auto *CInfo = DTInfo->getCallInfo(I)) {
          std::pair<llvm::Type *, AOSConvType> ElemConvPair =
              getCallInfoTypeToTransform(CInfo);
          if (ElemConvPair.second == AOS_NoConv)
            continue;

          auto *CInfoElemTy = ElemConvPair.first;
          ConvType = ElemConvPair.second;

          auto *TI = DTInfo->getTypeInfo(CInfoElemTy);
          assert(TI && "Expected TypeInfo for structure type");

          switch (CInfo->getCallInfoKind()) {
          case dtrans::CallInfo::CIK_Alloc: {
            auto *AInfo = cast<AllocCallInfo>(CInfo);
            // The candidate qualification should have only allowed calloc or
            // malloc for the types being directly transformed. Dependent type
            // only simply resized, so other calls are allowed.
            if (ConvType == AOS_SOAConv)
              assert((ConvType == AOS_DepConv ||
                      AInfo->getAllocKind() == AK_Calloc ||
                      AInfo->getAllocKind() == AK_Malloc) &&
                     "Only calloc or malloc expected for AOS-to-SOA types");

            if (ConvType == AOS_SOAConv)
              AllocsToConvert.push_back(
                  std::make_pair(AInfo, cast<StructInfo>(TI)));
            else
              DepAllocsToResize.push_back(
                  std::make_pair(AInfo, cast<StructInfo>(TI)));
            break;
          }
          case dtrans::CallInfo::CIK_Free:
            // We only need to update the 'free' if it's the AOS-to-SOA type
            // being converted. There is no impact on calls to free for
            // dependent types.
            if (ConvType == AOS_SOAConv)
              FreesToConvert.push_back(std::make_pair(cast<FreeCallInfo>(CInfo),
                                                      cast<StructInfo>(TI)));
            break;
          case dtrans::CallInfo::CIK_Memfunc:
            if (ConvType == AOS_SOAConv)
              MemfuncsToConvert.push_back(std::make_pair(
                  cast<MemfuncCallInfo>(CInfo), cast<StructInfo>(TI)));
            else
              DepMemfuncsToResize.push_back(std::make_pair(
                  cast<MemfuncCallInfo>(CInfo), cast<StructInfo>(TI)));
            break;
          }
        }
      } else if (auto *GEP = dyn_cast<GetElementPtrInst>(I)) {
        if ((ConvType = checkByteGEPConversionNeeded(GEP)) != AOS_NoConv) {
          if (ConvType == AOS_SOAConv)
            ByteGEPsToConvert.push_back(GEP);
          else
            DepByteGEPsToConvert.push_back(GEP);
        } else if (checkConversionNeeded(GEP))
          GEPsToConvert.push_back(GEP);
      } else if (auto *PTI = dyn_cast<PtrToIntInst>(I)) {
        // A pointer may be cast to an integer type, and after the
        // type remapping of the pointer to the structure this would be
        // a PtrToInt instruction with an integer type as the source type.
        //
        // The DTransAnalysis guarantees that the size of the target integer
        // will be the same size as the pointer type.
        // The DTransAnalysis guarantees that IntToPtr instructions do not
        // need to be considered currently.
        if (checkConversionNeeded(PTI))
          PTIsToConvert.push_back(PTI);
      } else if (auto *BC = dyn_cast<BitCastInst>(I)) {
        if (checkConversionNeeded(BC))
          BCsToConvert.push_back(BC);
      } else if (auto *BinOp = dyn_cast<BinaryOperator>(I)) {
        if ((ConvType = checkConversionNeeded(BinOp)) != AOS_NoConv) {
          if (ConvType == AOS_SOAConv)
            BinOpsToConvert.push_back(BinOp);
          else
            DepBinOptsToConvert.push_back(BinOp);
        }
      } else if (auto *LI = dyn_cast<LoadInst>(I)) {
        if (checkConversionNeeded(LI))
          LoadsToConvert.push_back(LI);
      } else if (auto *SI = dyn_cast<StoreInst>(I)) {
        if (checkConversionNeeded(SI))
          StoresToConvert.push_back(SI);
      }
    }

    for (auto &Alloc : AllocsToConvert)
      processAllocCall(Alloc.first, Alloc.second);

    for (auto &Free : FreesToConvert)
      processFreeCall(Free.first, Free.second);

    for (auto &MemCall : MemfuncsToConvert)
      processMemfuncCall(MemCall.first, MemCall.second);

    for (auto *GEP : ByteGEPsToConvert)
      processByteFlattendGEP(GEP);

    for (auto *GEP : GEPsToConvert)
      processGEP(GEP);

    for (auto *BinOp : BinOpsToConvert)
      processBinOp(BinOp);

    for (auto *PTI : PTIsToConvert)
      processPtrToInt(PTI);

    for (auto *SI : StoresToConvert)
      processStore(SI);

    for (auto *LI : LoadsToConvert)
      processLoad(LI);

    // The bitcasts should be processed after the 'free' calls and
    // byte-flattened GEPs, because those conversions are expecting
    // instructions with the bitcasts as an input.
    for (auto *BC : BCsToConvert)
      processBitcast(BC);

    // Process the instructions using dependent structure types.
    for (auto &Alloc : DepAllocsToResize)
      ProcessDepAllocCall(Alloc.first, Alloc.second);

    for (auto &MemCall : DepMemfuncsToResize)
      processDepMemfuncCall(MemCall.first, MemCall.second);

    for (auto *GEP : DepByteGEPsToConvert)
      processDepByteFlattendGEP(GEP);

    for (auto *BinOp : DepBinOptsToConvert)
      processDepBinOp(BinOp);

    for (auto *I : InstructionsToDelete)
      I->eraseFromParent();

    PeelIndexCache.clear();
    InstructionsToDelete.clear();

    DEBUG_WITH_TYPE(AOSTOSOA_IR,
                    dbgs() << "\nDTRANS-AOSTOSOA: After processFunction:\n"
                           << F << "\n");
  }

  bool checkConversionNeeded(GetElementPtrInst *GEP) {
    // The only cases that need to be converted are GEPs
    // with 1 or 2 indices because nested structures are not supported.
    if (GEP->getNumIndices() > 2)
      return false;

    Type *ElementTy = GEP->getSourceElementType();
    return isTypeToTransform(ElementTy);
  }

  // Return whether the GEP instruction type is of a type being transformed, or
  // is dependent on a type being transformed, or does not change.
  AOSConvType checkByteGEPConversionNeeded(GetElementPtrInst *GEP) {
    if (GEP->getNumIndices() != 1)
      return AOS_NoConv;

    auto InfoPair = DTInfo->getByteFlattenedGEPElement(GEP);
    if (!InfoPair.first)
      return AOS_NoConv;

    return isTypeOrDepTypeToTransform(InfoPair.first);
  }

  bool checkConversionNeeded(PtrToIntInst *PTI) {
    Type *Ty = PTI->getOperand(0)->getType()->getPointerElementType();
    return isTypeToTransform(Ty);
  }

  // Conversions to or from pointers to the struct type being converted need
  // to be processed, because after rewriting the types, these would become
  // a bitcast from or to an i64 type.
  bool checkConversionNeeded(BitCastInst *BC) {
    if (!BC->getType()->isPointerTy())
      return false;

    if (isTypeToTransform(BC->getDestTy()->getPointerElementType())) {
      // We only expect bitcasts to be with i8* due to the safety checks, assert
      // this to be sure.
      assert(BC->getSrcTy() == getInt8PtrType() &&
             "Only cast from i8* expected for transformed type");
      return true;
    }

    Type *Ty = BC->getSrcTy()->getPointerElementType();
    bool ShouldTransform = isTypeToTransform(Ty);
    if (!ShouldTransform)
      return false;

    // We only expect bitcasts to be to i8* due to the safety checks, assert
    // this to be sure.
    assert(BC->getDestTy() == getInt8PtrType() &&
           "Only cast to i8* expected for transformed type");
    return true;
  }

  // Return whether the subtract instruction type is of a type being
  // transformed, or is dependent on a type being transformed, or does not
  // change.
  AOSConvType checkConversionNeeded(BinaryOperator *BinOp) {
    if (BinOp->getOpcode() != Instruction::Sub)
      return AOS_NoConv;

    llvm::Type *PtrSubTy = DTInfo->getResolvedPtrSubType(BinOp);
    if (!PtrSubTy)
      return AOS_NoConv;

    return isTypeOrDepTypeToTransform(PtrSubTy);
  }

  // Check whether a load instruction needs to be modified directly by the
  // transformation rather than via the type remapper. If the load was converted
  // to use a pointer sized int or other generic equivalent, rather than the
  // pointer type itself, it will need to be transformed when pointer shrinking
  // is taking place, otherwise the load would be the size of a pointer rather
  // than the peeling index type. Also, return 'true' for the cases where the
  // level of indirection of the pointer should be updated. For instance,
  // %struct.test** will be i32* after the transformation takes place.
  bool checkConversionNeeded(LoadInst *LI) {
    auto *Ty = LI->getType();
    if (auto *PTy = dyn_cast<llvm::PointerType>(Ty))
      if (isTypeToTransform(PTy->getPointerElementType()))
        InstructionsToAnnotate.push_back({LI, PTy->getPointerElementType()});

    if (!isPointerShrinkingEnabled())
      return false;

    auto *ActualTy = DTInfo->getGenericLoadType(LI);
    if (!ActualTy)
      return false;

    if (!ActualTy->isPointerTy())
      return false;

    while (ActualTy->isPointerTy())
      ActualTy = ActualTy->getPointerElementType();

    return isTypeToTransform(ActualTy);
  }

  // Check whether a store instruction needs to be modified directly by the
  // transformation rather than via the type remapper. If the stored value was
  // converted to use a pointer sized int or other generic equivalent, rather
  // than the pointer type itself, it will need to be transformed when pointer
  // shrinking is taking place, otherwise the store would be the size of a
  // pointer rather than the peeling index type. Also, return 'true' for the
  // cases where the level of indirection of the pointer should be updated. For
  // instance, %struct.test** will be i32* after the transformation takes place.
  bool checkConversionNeeded(StoreInst *SI) {
    auto *Ty = SI->getValueOperand()->getType();
    if (auto *PTy = dyn_cast<llvm::PointerType>(Ty))
      if (isTypeToTransform(PTy->getPointerElementType()))
        InstructionsToAnnotate.push_back({SI, PTy->getPointerElementType()});

    if (!isPointerShrinkingEnabled())
      return false;

    auto *ActualTy = DTInfo->getGenericStoreType(SI);
    if (!ActualTy)
      return false;

    if (!ActualTy->isPointerTy())
      return false;

    while (ActualTy->isPointerTy())
      ActualTy = ActualTy->getPointerElementType();

    return isTypeToTransform(ActualTy);
  }

  void processGEP(GetElementPtrInst *GEP) {
    LLVM_DEBUG(dbgs() << "Replacing GEP: " << *GEP << "\n");

    // There are 2 cases that need to be converted.
    // 1: Getting the address of a structure within the array of structures.
    //    %addr1 = getelementptr %struct.t, %struct.t* %base, i64 %n
    //
    // 2: Getting the address of a structure field within the array of
    // structures.
    //    %addr2 = getelementptr %struct.t, %struct.t* %base, i64 %n, i32 1
    //
    // These need to be processed prior to the type remapping process because
    // the type remapping will change %struct.t* to be an integer type, which
    // would prevent identification of the instructions that need to be
    // rewritten.
    //
    // There will not be higher numbers of indices because nested structures are
    // disqualified during the safety checks.
    unsigned NumIndices = GEP->getNumIndices();
    assert(NumIndices <= 2 && "Unexpected index count for GEP");

    if (NumIndices == 1) {
      // This will convert the GEP of case 1 from:
      //    %arrayidx = getelementptr %struct.t, %struct.t* %base, i64 %idx_in
      //
      // When the peeling index is not being shrunk, creates:
      //    %peelIdxAsInt = ptrtoint %struct.t* %base to i64
      //    %add = add i64 %peelIdxAsInt, %idx_in
      //    %arrayidx = inttoptr i64 %add to %struct.t*
      //
      // When the peeling index is being shrunk to 32-bits, creates:
      //    %peelIdxAsInt = ptrtoint %struct.t* %base to i32
      //    %trunc_idx = trunc i64 %idx_in to i32
      //    %add = add i32 %peelIdxAsInt, %trunc_idx
      //    %arrayidx = inttoptr i32 %add to %struct.t*
      //
      // Note: We do not use a named variable in the IR for this, the names are
      // just shown here to make the following code transformation clearer.
      //
      // The ptrtoint and inttoptr instructions are temporary in order
      // to support the type remapping process. During the type remapping
      // process, the pointer type in those instructions will be converted
      // into an integer type, resulting in an i64 to i64 cast, which must
      // be removed during the function post processing. The use of the
      // conversion instruction here allows for values into and out of affected
      // instructions to be replaced without violating the type matching.
      Value *Src = GEP->getPointerOperand();
      Value *PeelIdxAsInt = createCastToPeelIndexType(Src, GEP);

      // Indexing into an array allows the GEP index value to be an integer of
      // any width, sign-extend the smaller of the GEP index and our peeling
      // index to make the types compatible for addition.
      Value *GEPBaseIdx = GEP->getOperand(1);
      llvm::Type *PeelIdxTy = getPeeledIndexType();
      uint64_t PeelIdxWidth = getPeeledIndexWidth();

      // We should never have a GEP index member that is larger than the size of
      // a pointer. If we do, then the indexing calculations will not work.
      assert(DL.getTypeSizeInBits(GEPBaseIdx->getType()) <=
                 DL.getTypeSizeInBits(getPtrSizedIntType()) &&
             "Unsupported GEP index type");

      // Match the base value to the size of peeling index. In the case where
      // the peeling index is being shrunk, this will result in truncating the
      // value, however that should be ok because by using the option, the user
      // is asserting the value must fit within 32-bits.
      GEPBaseIdx =
          promoteOrTruncValueToWidth(GEPBaseIdx, PeelIdxWidth, PeelIdxTy, GEP);

      Value *Add = BinaryOperator::CreateAdd(PeelIdxAsInt, GEPBaseIdx, "", GEP);

      // We will steal the name of the GEP and put it on this instruction
      // because even if the replacement is going to be done via a cast
      // statement, the cast is going to eventually be eliminated anyway.
      Add->takeName(GEP);

      // Cast the computed peeled index back to the original pointer type, and
      // substitute this into the users. When the type remapping occurs, the
      // users will be converted to an integer type, and the cast instruction
      // will be removed during post-processing.
      //
      // Note: Currently this requires the peeling index type to be the same
      // bit width as the pointer type, when updates are made for using a
      // smaller size, we will likely need to add some truncation here.
      CastInst *ArrayIdx =
          CastInst::CreateBitOrPointerCast(Add, Src->getType());
      ArrayIdx->insertBefore(GEP);
      PtrConverts.push_back(ArrayIdx);

      GEP->replaceAllUsesWith(ArrayIdx);
      InstructionsToDelete.insert(GEP);
    } else {
      // This will convert the GEP of case 2 from:
      //    %elem_addr =
      //         getelementptr %struct.t, %struct.t* %base, i64 %idx_n, i32 1
      //
      // When the peeling index is not being shrunk, creates:
      //    %peelIdxAsInt = ptrtoint %struct.test01* %base to i64
      //    %peel_base = getelementptr % __soa_struct.t,
      //                        %__soa_struct.t* @__soa_struct.t, i64 0, i32 1
      //    %soa_addr = load i32*, i32** %peel_base
      //    %adjusted_idx = add i64 %peelIdxAsInt, %idx_n
      //    %elem_addr = getelementptr i32, i32* %soa_addr, i64 %adjusted_idx
      //
      // When the peeling index is being shrunk to 32-bits, creates:
      //    %peelIdxAsInt = ptrtoint %struct.test01* %base to i32
      //    %peel_base = getelementptr % __soa_struct.t,
      //                        %__soa_struct.t* @__soa_struct.t, i64 0, i32 1
      //    %soa_addr = load i32*, i32** %peel_base
      //    %trunc_idx = trunc i64 %idx_n to i32
      //    %adjusted_idx = add i32 %peelIdxAsInt, %trunc_idx
      //    %zadjusted_idx = zext i32 %adjusted_idx to i64
      //    %elem_addr = getelementptr i32, i32* %soa_addr, i64 %zadjusted_idx

      // In this case, the 2nd GEP index will always be the field number which
      // is a constant integer.

      Type *ElementTy = GEP->getSourceElementType();
      StructType *PeelType = getTransformedType(ElementTy);
      Value *PeelVar = PeeledTypeToVariable[PeelType];
      assert(PeelVar && "Peeling variable should have already been created");

      CastInst *PeelIdxAsInt =
          createCastToPeelIndexType(GEP->getPointerOperand(), GEP);
      Value *FieldNum = GEP->getOperand(2);

      // Create and insert the instructions that get the address of the field in
      // the transformed structure.
      Instruction *FieldGEP = createGEPFieldAddressReplacement(
          PeelType, PeelVar, PeelIdxAsInt, GEP->getOperand(1), FieldNum, GEP);
      // We will steal the name of the GEP and put it on this instruction
      // because even if the replacement is going to be done via a cast
      // statement, the cast is going to eventually be eliminated anyway.
      FieldGEP->takeName(GEP);

      // If the field type is pointer to a structure that is being transformed,
      // generate a temporary cast to the original type so that the user
      // instruction will have expected types. These casts will become casts
      // from and to the same type after the remapping happens, and will be
      // removed during post-processing.
      Type *OrigFieldTy = GEP->getType();
      Value *ReplVal = FieldGEP;

      // Identify the type in the new structure for the field being accessed.
      unsigned FieldIdx = cast<ConstantInt>(FieldNum)->getLimitedValue();
      llvm::Type *PeelFieldTy = PeelType->getElementType(FieldIdx);

      if (PeelFieldTy != OrigFieldTy) {
        CastInst *CastToPtr =
            CastInst::CreateBitOrPointerCast(FieldGEP, OrigFieldTy);
        CastToPtr->insertBefore(GEP);
        PtrConverts.push_back(CastToPtr);
        ReplVal = CastToPtr;
      }

      GEP->replaceAllUsesWith(ReplVal);
      InstructionsToDelete.insert(GEP);
    }
  }

  // Create an intermediate cast instruction to cast a pointer to a
  // structure type being converted into the peeling index type. If
  // \p InsertBefore is non-null the instruction will be inserted into the IR
  // before it. This cast is added to the list of casts that are to be removed
  // during post processing because it will end up being i64 to i64 after the
  // type remapping.
  CastInst *createCastToPeelIndexType(Value *V,
                                      Instruction *InsertBefore = nullptr) {
    CastInst *ToInt = CastInst::CreateBitOrPointerCast(V, getPeeledIndexType(),
                                                       "", InsertBefore);
    PtrConverts.push_back(ToInt);

    return ToInt;
  }

  Value *promoteOrTruncValueToWidth(Value *V, uint64_t DstWidth,
                                    llvm::Type *DstTy,
                                    Instruction *InsertBefore) {
    assert(DL.getTypeSizeInBits(DstTy) == DstWidth &&
           "Target type size must match DstWdith");

    uint64_t SrcWidth = DL.getTypeSizeInBits(V->getType());
    if (SrcWidth < DstWidth)
      return CastInst::Create(CastInst::SExt, V, DstTy, "", InsertBefore);
    else if (DstWidth < SrcWidth)
      return CastInst::Create(CastInst::Trunc, V, DstTy, "", InsertBefore);

    return V;
  }

  // Create and insert the instruction sequence that gets the address of a
  // specific element in the peeled structure.
  // \p PeelType is the peeled structure type.
  // \p PeelVar is the global variable for the peeled structure.
  // \p FieldNumVal is a constant integer for the field number.
  // \p GEPBaseIdx and \p GEPFieldNum are the two GEP indices.
  // The set of instructions will be inserted immediately before \p
  // InsertBefore.
  // Returns the getelementptr instruction that represents the address.
  //
  // When the peeling index is not being shrunk, creates:
  //    %peel_base = getelementptr % __soa_struct.t,
  //                        %__soa_struct.t* @__soa_struct.t, i64 0, i32 1
  //    %soa_addr = load i32*, i32** %peel_base
  //    %adjusted_idx = add i64 %peelIdxAsInt, %idx_n
  //    %elem_addr = getelementptr i32, i32* %soa_addr, i64 %adjusted_idx
  //
  // When the peeling index is being shrunk to 32-bits, creates:
  //    %peel_base = getelementptr % __soa_struct.t,
  //                        %__soa_struct.t* @__soa_struct.t, i64 0, i32 1
  //    %soa_addr = load i32*, i32** %peel_base
  //    %trunc_idx = trunc i64 %idx_n to i32
  //    %adjusted_idx = add i32 %peelIdxAsInt, %trunc_idx
  //    %zadjusted_idx = zext i32 %adjusted_idx to i64
  //    %elem_addr = getelementptr i32, i32* %soa_addr, i64 %zadjusted_idx
  Instruction *createGEPFieldAddressReplacement(
      llvm::StructType *PeelType, Value *PeelVar, Value *PeelIdxAsInt,
      Value *GEPBaseIdx, Value *GEPFieldNum, Instruction *InsertBefore) {

    Instruction *SOAAddr =
        createPeelFieldLoad(PeelType, PeelVar, GEPFieldNum, InsertBefore);
    uint64_t PeelIdxWidth = getPeeledIndexWidth();

    Value *AdjustedPeelIdxAsInt = PeelIdxAsInt;
    // If first index is not constant 0, then we need to index by that amount
    if (!dtrans::isValueEqualToSize(GEPBaseIdx, 0)) {
      GEPBaseIdx = promoteOrTruncValueToWidth(
          GEPBaseIdx, PeelIdxWidth, PeelIdxAsInt->getType(), InsertBefore);
      BinaryOperator *Add =
          BinaryOperator::CreateAdd(PeelIdxAsInt, GEPBaseIdx, "", InsertBefore);
      AdjustedPeelIdxAsInt = Add;
    }

    // Identify the type in the new structure for the field being accessed.
    unsigned FieldIdx = cast<ConstantInt>(GEPFieldNum)->getLimitedValue();
    llvm::Type *PeelFieldTy = PeelType->getElementType(FieldIdx);

    // We know all elements of the peeled structure are pointer types.
    // Get the type that it points to.
    Type *FieldElementTy = cast<PointerType>(PeelFieldTy)->getElementType();

    // Extend the index back to a 64-bit value for use in the GEP instruction
    // because the pointer-type size used as the base is a 64-bit type.
    if (isPointerShrinkingEnabled())
      AdjustedPeelIdxAsInt =
          CastInst::Create(CastInst::ZExt, AdjustedPeelIdxAsInt,
                           getPtrSizedIntType(), "", InsertBefore);

    GetElementPtrInst *FieldGEP = GetElementPtrInst::Create(
        FieldElementTy, SOAAddr, AdjustedPeelIdxAsInt, "", InsertBefore);

    return FieldGEP;
  }

  // Create a load of the array address for a specific field of the peeled
  // structure.
  // \p PeelType is the peeled structure type
  // \p PeelVar is the global variable for the peeled structure
  // \p FieldNumVal is a constant integer for the field number.
  // Instructions are inserted before \p InsertBefore
  //
  // Generates:
  //    %peel_base = getelementptr % __soa_struct.t, %__soa_struct.t*
  //                     @__soa_struct.t, i64 0, i32 1
  //    %soa_addr = load i32*, i32** %peel_base
  Instruction *createPeelFieldLoad(llvm::Type *PeelType, Value *PeelVar,
                                   Value *FieldNumVal,
                                   Instruction *InsertBefore) {
    GetElementPtrInst *PeelBase = GetElementPtrInst::Create(
        PeelType, PeelVar,
        {Constant::getNullValue(getPtrSizedIntType()), FieldNumVal}, "",
        InsertBefore);
    LoadInst *SOAAddr = new LoadInst(PeelBase);

    // Mark the load of the structure of arrays field member as being invariant.
    // When the memory allocation for the object was done, the address of each
    // array was stored within the members of the structure of arrays, and will
    // never be changed. Marking these are invariant allows other
    // optimizations to hoist these accesses to prevent repetitive accesses. We
    // could also add struct-path TBAA annotations to these loads, however
    // results of experiments for adding TBAA annotations showed a small
    // reduction in the number of times the field was loaded in the binary, but
    // it did not show any performance gain.
    SOAAddr->setMetadata(LLVMContext::MD_invariant_load,
                         MDNode::get(InsertBefore->getContext(), {}));
    SOAAddr->insertBefore(InsertBefore);
    return SOAAddr;
  }

  // The analysis phase has resolved that the use of this subtract instruction
  // is used to divide by the structure size or some multiple of it.
  // Because the pointer to the structure has been converted to be an
  // integer index, the result of the subtract is the distance between
  // the pointers that the divide was going to compute when the divisor
  // equaled the structure size. Update the divide instruction to replace the
  // divisor.
  void processBinOp(BinaryOperator *BinOp) {
    llvm::Type *PtrSubTy = DTInfo->getResolvedPtrSubType(BinOp);
    assert(PtrSubTy && "Expected type for pointer Sub instruction");
    uint64_t OrigSize = DL.getTypeAllocSize(PtrSubTy);
    updatePtrSubDivUserSizeOperand(BinOp, OrigSize, 1);
  }

  void processPtrToInt(PtrToIntInst *PTI) {
    LLVM_DEBUG(dbgs() << "ptrtoint to convert: " << *PTI << "\n");

    // An instruction of the form: ptrtoint %struct.test01* %x to i64 will
    // not be valid after type remapping occurs.
    //
    // When index shrinking is not enabled, it will remap to:
    //    ptrtoint i64 %x to i64
    // In this case, the instruction becomes a meaningless cast, and should
    // be removed during post processing.
    if (!isPointerShrinkingEnabled()) {
      PtrConverts.push_back(PTI);
      LLVM_DEBUG(dbgs() << "ptrtoint will be deleted in post-processing\n");
      return;
    }

    // When index shrinking is enabled, we need to create a replacement
    // sequence to prepare for the type remapping, while maintaining the
    // uses of the instruction as i64 types by converting it into:
    //    %1 = ptrtoint %struct.test01* %x to i32
    //    %2 = zext i32 %1 to i64
    //
    // After type remapping, the ptrtoint replacement will be ptrtoint i32 to
    // i32 and removed during post processing.
    CastInst *NewPTI = CastInst::CreateBitOrPointerCast(
        PTI->getOperand(0), getPeeledIndexType(), "", PTI);
    Instruction *ZExt =
        CastInst::Create(CastInst::ZExt, NewPTI, PTI->getType(), "", PTI);
    PTI->replaceAllUsesWith(ZExt);
    ZExt->takeName(PTI);

    LLVM_DEBUG(dbgs() << "After convert:\n  " << *NewPTI << "\n  " << *ZExt
                      << "\n");

    InstructionsToDelete.insert(PTI);
    PtrConverts.push_back(NewPTI);
    return;
  }

  // Replace a load instruction that loads a pointer sized integer or generic
  // pointer type for the type being transformed to instead load a peeling index
  // type (or pointer to peeling index type).
  void processLoad(LoadInst *LI) {

    assert(isPointerShrinkingEnabled() &&
           "LoadInst transformation is only needed when shrinking the peeling "
           "index");

    LLVM_DEBUG(dbgs() << "Load to convert: " << *LI << "\n");

    // Replace a load of the form:
    //    %val_i64 = load i64, i64* %p_i64
    //
    // To be:
    //    %1 = bitcast i64* %p_i64 to i32*
    //    %2 = load i32, i32* %1
    //    %val_i64 = zext i32 %2 to i64
    //
    // The loaded value is extended to the original size so that it can
    // be used as a replacement in the existing instructions.
    //
    //
    // Similarly, a load using an i8* for the type will be handled as:
    //    %val = load i8*, i8** %ptr
    //
    // To be:
    //    %1 = bitcast i8** %ptr to i32*
    //    %2 = load i32, i32* %1
    //    %val = inttoptr i32 %2 to i8*
    //
    //
    // For higher levels of indirection of the underlying element type being
    // accessed:
    //    %val = load i8**, i8*** %ptr
    // where %ptr represents a struct.test**
    //
    // The load should be changed as a pointer to the peeling type, as in:
    //    %1 = bitcast i8*** %ptr to i32**
    //    %2 = load i32*, i32** %1
    //    %val = bitcast i32* %2 to i8**
    //
    Value *PtrOp = LI->getPointerOperand();
    auto *ActualTy = DTInfo->getGenericLoadType(LI);
    assert(ActualTy && "Unexpected load being converted");

    llvm::Type *RemapTy = TypeRemapper->remapType(ActualTy);
    Value *NewPtrOp = nullptr;
    if (auto *C = dyn_cast<Constant>(PtrOp)) {
      NewPtrOp = ConstantExpr::getBitCast(C, RemapTy->getPointerTo());
    } else {
      NewPtrOp = CastInst::CreateBitOrPointerCast(
          PtrOp, RemapTy->getPointerTo(), "", LI);
      IntermediateConverts.push_back(cast<CastInst>(NewPtrOp));
    }

    // Create a new load with the same attributes as the original instruction,
    // except for the alignment field. Because pointers to the structure are
    // being changed to an integer the original alignment may no longer be
    // valid, so set it to the ABI default for the type that the load will be
    // once remapping occurs.
    unsigned int Alignment = DL.getABITypeAlignment(RemapTy);
    Instruction *NewLI =
        new LoadInst(NewPtrOp, "", LI->isVolatile(), MaybeAlign(Alignment),
                     LI->getOrdering(), LI->getSyncScopeID(), LI);

    // Determine the type of conversion needed to match the type of the users
    // for the original load instruction.
    //
    //   Original load type    New load type     CastType
    //   ------------------    --------------    --------
    //   ptr sized int (i64)   i32               zext
    //   ptr (i8*)             i32               inttoptr
    //   ptr to ptr (i8**)     i32*              bitcast
    //
    auto *OrigLoadTy = LI->getType();
    auto CastType = CastInst::BitCast;
    if (NewLI->getType()->isIntegerTy()) {
      if (OrigLoadTy->isIntegerTy())
        CastType = CastInst::ZExt;
      else
        CastType = CastInst::IntToPtr;
    }

    Value *Repl = CastInst::Create(CastType, NewLI, LI->getType(), "", LI);
    LI->replaceAllUsesWith(Repl);
    Repl->takeName(LI);
    IntermediateConverts.push_back(cast<CastInst>(Repl));
    InstructionsToDelete.insert(LI);
    auto ActualPtrTy = cast<PointerType>(ActualTy);

    // Only annotate it if the load is for the peeling index, not
    // if it is a ptr-to-ptr for the peeling index.
    if (NewLI->getType()->isIntegerTy())
      InstructionsToAnnotate.push_back(
          {NewLI, ActualPtrTy->getPointerElementType()});

    LLVM_DEBUG(dbgs() << "After convert:\n  " << *NewPtrOp << "\n  " << *NewLI
                      << "\n  " << *Repl << "\n");
  }

  // Replace a store instruction that stores a pointer sized integer or generic
  // pointer type for the type being transformed to instead store a peeling
  // index type (or pointer to peeling index type).
  void processStore(StoreInst *SI) {
    assert(isPointerShrinkingEnabled() &&
           "StoreInst transformation is only needed when shrinking the peeling "
           "index");

    LLVM_DEBUG(dbgs() << "Store to convert: " << *SI << "\n");

    // Replace a store of the form:
    //    store i64 %val_i64, i64* %p_i64
    //
    // To be:
    //    %1 = trunc i64 %val_i64 to i32
    //    %2 = bitcast i32* %p_i64 to i32*
    //    store i32 %1, i32* %2
    //
    //
    // Similarly, a store using an i8* for the value type will be handled as:
    //    store i8* %val, i8** %ptr
    //
    // To be:
    //    %1 = ptrtoint i8* %val to i32
    //    %2 = bitcast i8** %ptr to i32*
    //    store i32 %1, i32* %2
    //
    //
    // For higher levels of indirection of the underlying element type being
    // accessed:
    //    store i8** %val, i8*** %ptr
    //
    // To be:
    //    %1 = bitcast i8** %val to i32*
    //    %2 = bitcast i8*** %ptr to i32**
    //    store i32* %1, i32** %2
    //
    auto *ActualTy = DTInfo->getGenericStoreType(SI);
    assert(ActualTy && "Unexpected store being converted");

    llvm::Type *RemapTy = TypeRemapper->remapType(ActualTy);
    Value *ValOp = SI->getValueOperand();
    llvm::Type *OrigValTy = ValOp->getType();

    // Determine the type of conversion needed to match the type of the value
    // being stored.
    //
    //   Original store type    New store type     CastType
    //   ------------------    --------------      --------
    //   ptr sized int (i64)   i32                 trunc
    //   ptr (i8*)             i32                 ptrtoint
    //   ptr to ptr (i8**)     i32*                bitcast
    //
    auto CastType = CastInst::BitCast;
    if (RemapTy->isIntegerTy()) {
      if (OrigValTy->isIntegerTy())
        CastType = CastInst::Trunc;
      else
        CastType = CastInst::PtrToInt;
    }

    Value *NewValOp = CastInst::Create(CastType, ValOp, RemapTy, "", SI);
    IntermediateConverts.push_back(cast<CastInst>(NewValOp));
    Value *PtrOp = SI->getPointerOperand();
    Value *NewPtrOp = nullptr;
    if (auto *C = dyn_cast<Constant>(PtrOp)) {
      NewPtrOp = ConstantExpr::getBitCast(C, RemapTy->getPointerTo());
    } else {
      NewPtrOp = CastInst::CreateBitOrPointerCast(
          PtrOp, RemapTy->getPointerTo(), "", SI);
      IntermediateConverts.push_back(cast<CastInst>(NewPtrOp));
    }
    // Create a new store with the same attributes as the original instruction,
    // except for the alignment field. Because the field type in the structure
    // is changing, the original alignment may no longer be valid, so set it
    // to the ABI default for the type that the load will be once remapping
    // occurs.
    unsigned int Alignment = DL.getABITypeAlignment(RemapTy);
    Instruction *NewSI =
        new StoreInst(NewValOp, NewPtrOp, SI->isVolatile(), Alignment,
                      SI->getOrdering(), SI->getSyncScopeID(), SI);
    InstructionsToDelete.insert(SI);
    auto *ActualPtrTy = cast<PointerType>(ActualTy);

    // Only annotate it if the store is for the peeling index, not
    // if it is a ptr-to-ptr for the peeling index.
    if (RemapTy->isIntegerTy())
      InstructionsToAnnotate.push_back(
        {NewSI, ActualPtrTy->getPointerElementType()});

    LLVM_DEBUG(dbgs() << "After convert:\n  " << *NewValOp << "\n  "
                      << *NewPtrOp << "\n  " << *NewSI << "\n");
  }

  // The byte-flattened GEP needs to be transformed from getting the address as:
  //    %p8_B = getelementptr i8, i8* %p, i64 8
  // (In this case field 1 is offset 8 bytes from the start of the structure)
  //
  // To:
  //    %peelIdxAsInt = ptrtoint %struct.test01* %p to i64
  //    %peel_base = getelementptr % __soa_struct.t,
  //                        %__soa_struct.t* @__soa_struct.t, i64 0, i32 1
  //    %soa_addr = load i32*, i32** %peel_base
  //    %field_gep = getelementptr i32, i32* %soa_addr, i64 %peelIdxAsInt
  //    %p8_B = bitcast i32* %elem_addr to i8*
  void processByteFlattendGEP(GetElementPtrInst *GEP) {
    auto InfoPair = DTInfo->getByteFlattenedGEPElement(GEP);
    auto *OrigStructTy = cast<llvm::StructType>(InfoPair.first);

    LLVM_DEBUG(dbgs() << "Replacing byte flattened GEP for field "
                      << InfoPair.second << ":\n  " << *GEP << "\n");

    // Trace the pointer operand to find the base value that is needed
    // for indexing into the field's array.
    Value *IndexAsInt =
        getPeelIndexFromValue(GEP->getPointerOperand(), OrigStructTy,
                              /*ForMemfunc=*/false);

    StructType *PeelType = getTransformedType(OrigStructTy);
    Value *PeelVar = PeeledTypeToVariable[PeelType];
    Value *FieldNum =
        ConstantInt::get(Type::getInt32Ty(GEP->getContext()), InfoPair.second);
    Instruction *FieldGEP = createGEPFieldAddressReplacement(
        PeelType, PeelVar, IndexAsInt,
        ConstantInt::get(getPtrSizedIntType(), 0), FieldNum, GEP);

    llvm::Type *PeelFieldTy = PeelType->getElementType(InfoPair.second);
    if (PeelFieldTy != GEP->getType())
      FieldGEP =
          CastInst::CreateBitOrPointerCast(FieldGEP, GEP->getType(), "", GEP);

    FieldGEP->takeName(GEP);
    GEP->replaceAllUsesWith(FieldGEP);
    InstructionsToDelete.insert(GEP);
  }

  // This is a helper function for identifying the peeling index to use
  // for byte-flattened GEPs and for memfunc transformations. For memfunc
  // transformations there is an additional pattern allowed which looks at
  // GEPs that get the address of a field within the original structure when
  // \p ForMemfunc is true.
  Value *getPeelIndexFromValue(Value *Op, StructType *OrigStructTy,
                               bool ForMemfunc) {
    auto &V = PeelIndexCache[Op];

    if (!V)
      V = createPeelIndexFromValue(Op, OrigStructTy, ForMemfunc);

    return V;
  }

  // This walks the definitions through bitcasts, selects and PHI nodes for
  // \p Op to find the index value to use for peeling index.
  Value *createPeelIndexFromValue(Value *Op, StructType *OrigStructTy,
                                  bool ForMemfunc) {
    // If \p Op is a bitcast from a pointer to the type being transformed to an
    // i8*, then we can directly use the source operand of bitcast, since we
    // know this is going to be turned into the peeling index type.
    if (auto *BC = dyn_cast<BitCastInst>(Op)) {
      if (BC->getOperand(0)->getType() == OrigStructTy->getPointerTo())
        return createCastToPeelIndexType(BC->getOperand(0), BC);
      else
        return getPeelIndexFromValue(BC->getOperand(0), OrigStructTy,
                                     ForMemfunc);
    }

    // For select and PHI nodes, create new instructions that will act on the
    // peeling index type, instead of the pointer type.
    if (auto *Sel = dyn_cast<SelectInst>(Op)) {
      Value *NewTrue =
          getPeelIndexFromValue(Sel->getTrueValue(), OrigStructTy, ForMemfunc);
      Value *NewFalse =
          getPeelIndexFromValue(Sel->getFalseValue(), OrigStructTy, ForMemfunc);
      Instruction *NewSel =
          SelectInst::Create(Sel->getCondition(), NewTrue, NewFalse, "", Sel);
      return NewSel;
    }

    if (auto *PHI = dyn_cast<PHINode>(Op)) {
      PHINode *NewPhi = PHINode::Create(getPeeledIndexType(), 0, "", PHI);

      // Save the new PHI so that if another reference to is found while walking
      // the definitions this one will be used.
      PeelIndexCache[PHI] = NewPhi;

      SmallVector<Value *, 4> NewPhiVals;
      for (Value *Val : PHI->incoming_values())
        NewPhiVals.push_back(
            getPeelIndexFromValue(Val, OrigStructTy, ForMemfunc));

      unsigned NumIncoming = PHI->getNumIncomingValues();
      for (unsigned Num = 0; Num < NumIncoming; ++Num)
        NewPhi->addIncoming(NewPhiVals[Num], PHI->getIncomingBlock(Num));

      return NewPhi;
    }

    // When back tracing for the index used in a memfunc, we need to also
    // consider the case where the address of the field is used to form
    // the i8* pointer passed to the call.
    // For example:
    //    %p = getelementptr %struct.ty, %struct.ty* %S, i64 %n, i32 0
    //
    // If %p were passed to a memfunc, we cannot simply convert the pointer to
    // an integer to get the peeling index. %S will be the peeling index, and we
    // need to offset this by the value of %n to get &S.field0[n]. In this case,
    // we return the expression (ptrtoint %S) + %n so that the conversion of the
    // memfunc call will start with that index value in all the fields being
    // touched. If %n equals 0, we can just return (ptrtoint %S)
    if (ForMemfunc)
      if (auto *GEP = dyn_cast<GetElementPtrInst>(Op))
        if (GEP->getSourceElementType() == OrigStructTy &&
            GEP->getNumIndices() == 2) {
          Instruction *PeelIdxAsInt =
              createCastToPeelIndexType(GEP->getPointerOperand(), GEP);

          Value *GEPBaseIdx = GEP->getOperand(1);
          if (!dtrans::isValueEqualToSize(GEPBaseIdx, 0)) {
            uint64_t PeelIdxWidth = getPeeledIndexWidth();
            uint64_t BaseIdxWidth = DL.getTypeSizeInBits(GEPBaseIdx->getType());
            if (BaseIdxWidth < PeelIdxWidth)
              GEPBaseIdx = CastInst::Create(CastInst::SExt, GEPBaseIdx,
                                            PeelIdxAsInt->getType(), "", GEP);

            PeelIdxAsInt =
                BinaryOperator::CreateAdd(PeelIdxAsInt, GEPBaseIdx, "", GEP);
          }
          return PeelIdxAsInt;
        }

    // We should not get here, but assert if we do in case there are additional
    // cases that require a conversion of the pointer type to the peeling index.
    llvm_unreachable("Not handled");
  }

  // Return 'true' if Instruction \p I is a conversion instruction of the type
  // UseConv, and the operand being converted is from an instruction of type
  // DefConv, and the operand into the DefConv instruction is the same type that
  // is being created by the UseConv instruction.
  //
  // For example:
  //      %val = inttoptr i32 %4 to i8*
  //      %5 = ptrtoint i8* %val to i32
  //
  template <typename DefConv, typename UseConv>
  static bool isCancellingConvert(Instruction *I) {
    if (auto *Conv = dyn_cast<UseConv>(I))
      if (auto *SrcConv = dyn_cast<DefConv>(Conv->getOperand(0)))
        if (SrcConv->getSrcTy() == Conv->getType())
          return true;

    return false;
  }

  // Update the signature of the function's clone and any instructions that need
  // to be modified after the type remapping of data types has taken place.
  virtual void postprocessFunction(Function &OrigFunc, bool isCloned) override {
    Function *Func = &OrigFunc;
    if (isCloned)
      Func = cast<Function>(VMap[&OrigFunc]);

    LLVM_DEBUG(dbgs() << "\nPost-processing function: " << Func->getName()
                      << "\n");
    DEBUG_WITH_TYPE(AOSTOSOA_IR,
                    dbgs() << "\nDTRANS-AOSTOSOA: Into postProcessFunction:\n"
                           << *Func << "\n");

    if (isCloned) {
      DEBUG_WITH_TYPE(AOSTOSOA_ATTRIBUTES, {
        dbgs() << "DTRANS-AOSTOSOA: Updating function attributes for: "
               << Func->getName() << " Type: " << *Func->getType() << "\n";
        Func->getAttributes().dump();
      });

      // For cloned functions, update any attributes on the return type or
      // arguments that are not compatible with types that have been converted
      // from pointers to integers. We do not change attributes on types
      // that were pointers-to-pointers of the transformed types because those
      // are now pointers-to-integer types, but the attributes they contained
      // should still be valid.
      llvm::Type *CloneRetTy = Func->getReturnType();
      llvm::Type *OrigRetTy = OrigFunc.getReturnType();
      if (!CloneRetTy->isPointerTy() && OrigRetTy->isPointerTy()) {

        // The only time we expect processing a function to change a pointer
        // type to an integer type is when the original argument was pointer to
        // the type being transformed, and integer type is the peeled type.
        assert(isTypeToTransform(OrigRetTy->getPointerElementType()) &&
               "Expected original return type to be a type being transformed");
        assert(CloneRetTy == getPeeledIndexType() &&
               "Expected clone return type to be peeling index type");

        Func->removeAttributes(0, IncompatiblePeelTypeAttrs);
      }

      assert(OrigFunc.arg_size() == Func->arg_size() &&
             "Expected clone arg for each original arg");
      Function::arg_iterator OrigArgIt = OrigFunc.arg_begin();
      Function::arg_iterator OrigArgEnd = OrigFunc.arg_end();
      Function::arg_iterator CloneArgIt = Func->arg_begin();
      for (uint64_t Idx = 0; OrigArgIt != OrigArgEnd;
           ++OrigArgIt, ++CloneArgIt, ++Idx) {
        llvm::Type *CloneArgType = CloneArgIt->getType();
        llvm::Type *OrigArgType = OrigArgIt->getType();
        if (!CloneArgType->isPointerTy() && OrigArgType->isPointerTy()) {
          assert(
              isTypeToTransform(OrigArgType->getPointerElementType()) &&
              "Expected original argument type to be a type being transformed");
          assert(CloneArgType == getPeeledIndexType() &&
                 "Expected clone argument type to be peeling index type");

          Func->removeParamAttrs(Idx, IncompatiblePeelTypeAttrs);
        }
      }

      DEBUG_WITH_TYPE(AOSTOSOA_ATTRIBUTES, {
        dbgs() << "DTRANS-AOSTOSOA: After function attribute update\n";
        Func->getAttributes().dump();
      });
    }

    for (auto *Conv : PtrConverts) {
      if (isCloned)
        Conv = cast<CastInst>(VMap[Conv]);

      LLVM_DEBUG(dbgs() << "Post process deleting: " << *Conv << "\n");

      assert(Conv->getType() == Conv->getOperand(0)->getType() &&
             "Expected self-type in cast after remap");
      Conv->replaceAllUsesWith(Conv->getOperand(0));
      Conv->eraseFromParent();
    }

    PtrConverts.clear();

    // Check for cast instructions generated that cancel out another cast.
    // These instructions would have eventually been removed by the instcombine
    // pass, but we cannot run instcombine between transformations now because
    // it would produce other IR instruction patterns that are not currently
    // recognized by DTransAnalysis, such as shift instructions.
    SmallVector<Instruction *, 4> DeadInsn;
    for (auto *Conv : IntermediateConverts) {
      if (isCloned)
        Conv = cast<Instruction>(VMap[Conv]);

      LLVM_DEBUG(dbgs() << "Post process checking for canceling conversion: "
                        << *Conv << "\n");

      bool NotNeeded = isCancellingConvert<BitCastInst, BitCastInst>(Conv) ||
                       isCancellingConvert<IntToPtrInst, PtrToIntInst>(Conv) ||
                       isCancellingConvert<PtrToIntInst, IntToPtrInst>(Conv) ||
                       isCancellingConvert<ZExtInst, TruncInst>(Conv);

      if (NotNeeded) {
        Instruction *SrcOperand = cast<Instruction>(Conv->getOperand(0));
        Conv->replaceAllUsesWith(SrcOperand->getOperand(0));
        Conv->eraseFromParent();

        // Check if the source is now dead, but defer deletion
        // until processing all the elements of this loop, in case
        // the instruction is contained in the vector being iterated.
        if (SrcOperand->user_empty())
          DeadInsn.push_back(SrcOperand);
      }
    }

    for (auto *I : DeadInsn)
      I->eraseFromParent();

    IntermediateConverts.clear();

    if (!InstructionsToAnnotate.empty()) {
      Module *M = Func->getParent();
      for (auto &InstTyPair : InstructionsToAnnotate) {
        auto *I = InstTyPair.first;
        auto *Ty = InstTyPair.second;
        if (isCloned)
          I = cast<Instruction>(VMap[I]);

        Value *Ptr = nullptr;
        if (auto *SI = dyn_cast<StoreInst>(I))
          Ptr = SI->getPointerOperand();
        else if (auto *LI = dyn_cast<LoadInst>(I))
          Ptr = LI->getPointerOperand();
        else
          llvm_unreachable("Instruction expected to be load/store");

        Value *Annot = DTransAnnotator::createPtrAnnotation(
            *M, *Ptr, *PeelIndexAnnotationGEP[Ty], *AnnotationFilenameGEP, 0,
            "alloc_idx", I);
        (void)Annot;

        LLVM_DEBUG(dbgs() << "Adding annotation for pointer: " << *Ptr
                          << "\n  in: " << *I << "\n  " << *Annot << "\n");
      }
      InstructionsToAnnotate.clear();
    }

    DEBUG_WITH_TYPE(AOSTOSOA_IR,
                    dbgs() << "\nDTRANS-AOSTOSOA: After postProcessFunction:\n"
                           << *Func << "\n");
  }

private:
  // Return 'true' if there are no safety issues with a dependent type \p Ty
  // that prevent transforming a type.
  bool checkDependentTypeSafety(llvm::Type *Ty) {
    auto *TI = DTInfo->getTypeInfo(Ty);
    assert(TI && "Expected DTrans to analyze container of dependent type");
    if (DTInfo->testSafetyData(TI, dtrans::DT_AOSToSOADependent))
      return false;

    if (TI->testSafetyData(dtrans::FieldAddressTaken)) {
      // We know there is no direct address taken for any fields for the type
      // being transformed because that structure was not marked as "Address
      // Taken". However, if the analysis is assuming the address of one field
      // can be used to access another field, then any time the field address
      // taken is set for the dependent type, it will be unknown whether it
      // affects a field member that is a pointer to a type being transformed,
      // so check whether the code is allowing memory outside the boundaries of
      // a specific field to be accessed when taking the address.
      if (DTInfo->getDTransOutOfBoundsOK())
        return false;
    }

    // No issues were found on this dependent type that prevent the AOS to SOA
    // transformation.
    return true;
  }

  // Return 'true' if there are no safety issues on a dependent type \Ty that
  // prevent shrinking the peeling index to 32-bits.
  bool checkDependentTypeSafeForShrinking(Module &M, llvm::Type *Ty) {
    auto *TI = DTInfo->getTypeInfo(Ty);
    assert(TI && "Expected DTrans to analyze container of dependent type");
    if (DTInfo->testSafetyData(TI, dtrans::DT_AOSToSOADependentIndex32))
      return false;

    // If there is a global instance of the type, then we need to check for
    // constant operators that directly address a field via a byte offset,
    // rather than a field index because we are not converting that form of
    // addressing on the dependent types when changing the size of those
    // structures. If there are no global instances, we're done.
    if (!TI->testSafetyData(dtrans::GlobalInstance))
      return true;

    // Look for a load or store that uses a pointer address of the form:
    //    getelementptr (bitcast @global to i8*) i64 <const>)
    //

    // Check for a bitcast to an i8*
    auto IsBCOpToInt8Ptr = [this](Value *V) -> bool {
      if (auto *BC = dyn_cast<BitCastOperator>(V))
        if (BC->getType() == getInt8PtrType())
          return true;
      return false;
    };

    // Empty lambda to end the recursion.
    auto NopOnMatch = [](Value *) -> bool { return true; };

    // Check for Value \p V getting used in a load/store, possibly
    // with some intervening bitcast and GEP operators.
    std::function<bool(Value *)> UsedInLoadStore;
    UsedInLoadStore = [&UsedInLoadStore, &NopOnMatch](Value *V) -> bool {
      // Recurse past any additional Bitcast or GEPOperators
      if (hasUseOfType(V, isType<BitCastOperator>, UsedInLoadStore) ||
          hasUseOfType(V, isType<GEPOperator>, UsedInLoadStore))
        return true;

      // Check if this value is used for a load/store instruction
      bool isLS = hasUseOfType(V, isType<LoadInst>, NopOnMatch) ||
                  hasUseOfType(V, isType<StoreInst>, NopOnMatch);
      LLVM_DEBUG({
        if (isLS)
          dbgs() << "Load/Store of dependent in Byte-GEP form: " << *V << "\n";
      });
      return isLS;
    };

    // Check whether the bitcast value \p V is used for a GEPOperator that
    // leads to a load/store instruction.
    auto OnBCOp = [&UsedInLoadStore](Value *V) -> bool {
      assert(isa<BitCastOperator>(V) && "Expected bitcast operator");
      return hasUseOfType(V, isType<GEPOperator>, UsedInLoadStore);
    };

    // Check all uses of global variables of the dependent type.
    for (auto &GV : M.globals()) {
      if (GV.getType()->getPointerElementType() != Ty)
        continue;

      if (hasUseOfType(&GV, IsBCOpToInt8Ptr, OnBCOp))
        return false;
    }

    // No issue found on this dependent type to prevent converting the peeling
    // index to 32-bits.
    return true;
  }

  // Helper function for checking uses of \p V.
  // For each use of \p V, if the test function, \p Test, returns true, then the
  // \p OnMatch function will be executed for the Value.
  template <typename IsACallable, typename DoOnMatch>
  static bool hasUseOfType(Value *V, IsACallable Test, DoOnMatch OnMatch) {
    for (auto *U : V->users()) {
      if (Test(U)) {
        return OnMatch(U);
      }
    }

    return false;
  }

  // Helper function for use in lambda expression to check if Value of a
  // specific type
  template <typename ValueType> static bool isType(Value *V) {
    return isa<ValueType>(V);
  }

  // The allocation call for a type being transformed into a structure of arrays
  // needs to be converted to initialize the peeling variable to store the
  // addresses that start each array in the new data structure.
  //
  // For example: struct t1 { int a, b, c};
  // will have the new global structure: struct __AOS_t1 { int *a, *b, *c };
  // This routine initializes the pointers of the global variable for a, b, c to
  // point to the appropriate location of the allocated block of memory.
  //
  // The size of the allocation, and uses of the result of the allocation call
  // will also be updated within the function.
  //
  // This function only operates on the pre-cloned version of a function.
  void processAllocCall(dtrans::AllocCallInfo *AInfo, StructInfo *StInfo) {
    auto *AllocCallInst = cast<CallInst>(AInfo->getInstruction());
    LLVM_DEBUG(dbgs() << "Updating allocation call: " << *AllocCallInst
                      << "\n");

    StructType *StructTy = cast<StructType>(StInfo->getLLVMType());
    uint64_t StructSize = DL.getTypeAllocSize(StructTy);
    StructType *PeelTy = getTransformedType(StructTy);
    Value *PeelVar = PeeledTypeToVariable[PeelTy];

    // Set up an IR builder to insert instructions starting before the
    // allocation statement. The IR builder will perform constant
    // folding for us when the allocation count is a constant when
    // computing new allocation sizes or offsets into the allocated memory
    // block.
    IRBuilder<> IRB(AllocCallInst);

    // This will store the adjusted number of elements allocated by the call.
    Value *NewAllocCountVal = nullptr;

    AllocKind Kind = AInfo->getAllocKind();
    unsigned OrigAllocSizeInd = 0;
    unsigned OrigAllocCountInd = 0;
    const TargetLibraryInfo &TLI = GetTLI(*AllocCallInst->getFunction());
    getAllocSizeArgs(Kind, AllocCallInst, OrigAllocSizeInd, OrigAllocCountInd,
                     TLI);

    auto *OrigAllocSizeVal = AllocCallInst->getArgOperand(OrigAllocSizeInd);
    if (Kind == dtrans::AK_Malloc) {
      assert(OrigAllocSizeVal && "getAllocSizeArgs should return size value");

      // Compute the number of elements being allocated. There is no need to
      // special case this for an allocation size that exactly matches the size
      // of a structure or check for constant values because the IR builder will
      // constant fold the calculations for us, if possible.
      llvm::Type *SizeType = OrigAllocSizeVal->getType();
      Value *StructSizeVal = ConstantInt::get(SizeType, StructSize);
      NewAllocCountVal = IRB.CreateSDiv(OrigAllocSizeVal, StructSizeVal);

      // Update the size allocated to hold one additional structure element.
      // This is necessary because a peeling index value of 0 will represent
      // the nullptr, and the array accesses will be in the range of 1..N.
      // Note: Rather than just adding 12 to the parameter size, we maintain the
      // invariant that the allocation is a multiple of the structure size.
      NewAllocCountVal =
          IRB.CreateAdd(NewAllocCountVal, ConstantInt::get(SizeType, 1));
      Value *NewAllocationSize = IRB.CreateMul(NewAllocCountVal, StructSizeVal);
      AllocCallInst->setOperand(OrigAllocSizeInd, NewAllocationSize);
      LLVM_DEBUG(dbgs() << "Modified allocation:\n"
                        << "\nSize: " << *NewAllocationSize << "\n  "
                        << *AllocCallInst << "\n");

    } else {
      assert(Kind == dtrans::AK_Calloc && "Expected calloc");
      auto *OrigAllocCountVal = AllocCallInst->getArgOperand(OrigAllocCountInd);
      assert(OrigAllocSizeVal && OrigAllocCountVal &&
             "getAllocSizeArgs should return size and count value");

      // Determine the values to use for the number of allocated objects, and
      // size being allocated.
      Value *AllocCountVal = nullptr;
      Value *AllocSizeVal = nullptr;
      if (isValueEqualToSize(OrigAllocSizeVal, StructSize)) {
        AllocCountVal = OrigAllocCountVal;
        AllocSizeVal = OrigAllocSizeVal;
      } else if (isValueEqualToSize(OrigAllocCountVal, StructSize)) {
        // Reverse the size and count parameters.
        AllocCountVal = OrigAllocSizeVal;
        AllocSizeVal = OrigAllocCountVal;
      } else {
        // At this point, we know that either the number allocated or the
        // allocation size is a multiple of the structure size, but not
        // how many elements are being allocated.
        Value *TotalSize = IRB.CreateMul(OrigAllocCountVal, OrigAllocSizeVal);
        AllocSizeVal = ConstantInt::get(TotalSize->getType(), StructSize);
        AllocCountVal = IRB.CreateSDiv(TotalSize, AllocSizeVal);
      }

      // Compute new values for the allocation count and size parameters.
      // We want the count to be 1 larger than the original number that
      // were being allocated, and the size parameter to equal the structure
      // size for the calloc parameters.
      NewAllocCountVal = IRB.CreateAdd(
          AllocCountVal, ConstantInt::get(AllocCountVal->getType(), 1));
      AllocCallInst->setOperand(OrigAllocCountInd, NewAllocCountVal);
      AllocCallInst->setOperand(OrigAllocSizeInd, AllocSizeVal);
      LLVM_DEBUG(dbgs() << "Modified allocation:\n"
                        << "Count:" << *NewAllocCountVal << "\nSize: "
                        << *AllocSizeVal << "\n  " << *AllocCallInst << "\n");
    }

    assert(NewAllocCountVal &&
           "Expected alloc call processing to set NewAllocCountVal");

    // Pointers in the peeled structure that corresponded to the base allocation
    // address will be accessed via index 1 in the peeled structure.\Update the
    // users of the allocation to refer to index 1. This loop collects the
    // values to be updated to avoid changing the users while walking them.
    SmallVector<StoreInst *, 4> StoresToFix;
    SmallVector<BitCastInst *, 4> BitCastsToFix;

    for (auto *User : AllocCallInst->users()) {
      if (auto *U = dyn_cast<Instruction>(&*User)) {
        if (auto *CmpI = dyn_cast<CmpInst>(U)) {
          // There is nothing to be changed on a comparison of the allocated
          // pointer against a null value. Both are i8* types, and will not
          // be transformed.
          (void)CmpI;
          assert((CmpI->getOperand(0) ==
                      Constant::getNullValue(AllocCallInst->getType()) ||
                  CmpI->getOperand(1) ==
                      Constant::getNullValue(AllocCallInst->getType())) &&
                 "Allocation result used in non-null comparison");
        } else if (auto *SI = dyn_cast<StoreInst>(U)) {
          // We expect the value operand to have been the use, because writing
          // to the allocated memory pointer should have set the safety flags
          // that inhibit the transformation.
          assert(SI->getValueOperand() == AllocCallInst &&
                 "Expected allocation result to be stored value");
          StoresToFix.push_back(SI);
        } else if (auto *BC = dyn_cast<BitCastInst>(U)) {
          // We expect the cast to only be to the type being transformed based
          // on the safety flags.
          assert(BC->getType() == StructTy->getPointerTo() &&
                 "Expected allocation result cast to be to type being "
                 "transformed");
          BitCastsToFix.push_back(BC);
        } else {
          llvm_unreachable("Unexpected instruction using allocation result");
        }
      } else {
        llvm_unreachable("Unexpected use of allocation result");
      }
    }

    // Update the stored value to be an index value of 1.
    for (auto *SI : StoresToFix) {
      CastInst *IndexAsPtr = CastInst::CreateBitOrPointerCast(
          ConstantInt::get(getPeeledIndexType(), 1),
          SI->getOperand(0)->getType());
      IndexAsPtr->insertBefore(SI);
      SI->setOperand(0, IndexAsPtr);
    }

    // Replace the BitCast instructions with an IntToPtr instruction that casts
    // index 1 to the original pointer type. After the type remapping
    // completes, the destination type of the cast will be the peeled index
    // type, and post processing will remove the instruction.
    for (auto *BC : BitCastsToFix) {
      CastInst *IndexAsPtr = CastInst::CreateBitOrPointerCast(
          ConstantInt::get(getPeeledIndexType(), 1), BC->getType());
      IndexAsPtr->insertBefore(BC);
      BC->replaceAllUsesWith(IndexAsPtr);
      InstructionsToDelete.insert(BC);
      PtrConverts.push_back(IndexAsPtr);
    }

    // Initialize the pointer fields of the peeled structure to store an
    // address of the allocated memory block. Padding may be needed
    // to align the start of the array for some elements. When padding
    // is required, we can start the array at the next available aligned
    // address, or we can leave a gap to simulate peeling of the original array
    // padding to get to the aligned address of the field. As an example
    // consider the structure {i32, i64, i32 }, padding may be required between
    // the 1st and 2nd arrays depending on the number of elements being
    // allocated. If we are allocating arrays of length 5 (after the adjustment
    // for the null element), then the array of i64 elements can begin at offset
    // 24 (minimal padding) or at offset 40 (original padding of i32 element is
    // effectively peeled)
    //
    // This version minimizes the padding, but we may revisit this.
    //
    // The memory writes generated below are on the global variable, so will be
    // safe even if the allocation call returns NULL.

    // Use the same type as the allocation parameter for the offset
    // calculations.
    Type *ArithType = NewAllocCountVal->getType();
    Value *AddrOffset = ConstantInt::get(ArithType, 0);
    Type *PrevArrayElemType = nullptr;

    // Insert the initialization of the field members to be right after the
    // allocation call.
    IRB.SetInsertPoint(AllocCallInst->getNextNode());
    unsigned int NumElements = PeelTy->getNumElements();
    for (unsigned FieldNum = 0; FieldNum < NumElements; ++FieldNum) {
      Type *PeelFieldType = PeelTy->getElementType(FieldNum);
      Type *ArrayElemType = PeelFieldType->getPointerElementType();

      if (FieldNum != 0) {
        // Compute the offset for the next array based on the size
        // of the previous element's array.
        unsigned PrevElemSize = DL.getTypeAllocSize(PrevArrayElemType);
        Value *Mul = IRB.CreateMul(NewAllocCountVal,
                                   ConstantInt::get(ArithType, PrevElemSize));
        if (AddrOffset == ConstantInt::get(ArithType, 0))
          AddrOffset = Mul;
        else
          AddrOffset = IRB.CreateAdd(AddrOffset, Mul);

        // Update the offset value to account for any padding that may be
        // needed, if this element has a stricter alignment requirement than the
        // previous element.
        uint64_t PrevFieldAlign = DL.getABITypeAlignment(PrevArrayElemType);
        uint64_t FieldAlign = DL.getABITypeAlignment(ArrayElemType);
        if (FieldAlign > PrevFieldAlign) {
          Value *Numerator = IRB.CreateAdd(
              AddrOffset, ConstantInt::get(ArithType, FieldAlign - 1));
          Value *Div = IRB.CreateSDiv(Numerator,
                                      ConstantInt::get(ArithType, FieldAlign));
          AddrOffset =
              IRB.CreateMul(Div, ConstantInt::get(ArithType, FieldAlign));
        }
      }

      // Compute the address in the memory block where the array for this field
      // will begin:
      //   %BlockAddr = getelementptr i8, i8* %AllocCallInst, i64/i32 %Offset
      Value *BlockAddr = IRB.CreateGEP(AllocCallInst, AddrOffset);

      // Annotate the GEP into the allocated block to allow subsequent runs
      // of DTransAnalysis to resolve the type as a pointer to an array of
      // elements. This is necessary because the allocated memory block is
      // being partitioned into multiple arrays.
      DTransAnnotator::createDTransTypeAnnotation(*cast<Instruction>(BlockAddr),
                                                  PeelFieldType);

      // Cast to the pointer type that will be stored:
      //   %CastToMemberTy = bitcast i8* %BlockAddr to %PeelFieldType
      Value *CastToMemberTy = IRB.CreateBitCast(BlockAddr, PeelFieldType);

      // Get the address in the global structure for the field to be stored:
      //   %FieldAddr = getelementptr %PeelVarType, %PeelVarType* %PeelVar i32
      //                %FieldNum
      Value *Idx[2];
      Idx[0] = Constant::getNullValue(Type::getInt64Ty(Context));
      Idx[1] = ConstantInt::get(Type::getInt32Ty(Context), FieldNum);
      Value *FieldAddr = IRB.CreateGEP(PeelTy, PeelVar, Idx);

      // Save the pointer to the global structure field.
      //   store %CastToMemberTy, %FieldAddr
      IRB.CreateStore(CastToMemberTy, FieldAddr);

      PrevArrayElemType = ArrayElemType;
    }

    // Annotate the allocation call for other the DTrans dynamic cloning
    // optimization.
    Module *M = AllocCallInst->getParent()->getParent()->getParent();
    auto *Annot = DTransAnnotator::createPtrAnnotation(
        *M, *AllocCallInst, *AllocationAnnotationGEP[StructTy],
        *AnnotationFilenameGEP, 0, "annot_alloc", nullptr);
    Annot->insertAfter(AllocCallInst);

    LLVM_DEBUG(dbgs() << "Adding annotation for allocation: " << *AllocCallInst
                      << "\n  : " << *Annot << "\n");
  }

  // The transformation of the call to free needs to change the parameter
  // passed to 'free' to be the address of our peeled global variable.
  void processFreeCall(FreeCallInfo *CInfo, StructInfo *StInfo) {
    Instruction *FreeCall = CInfo->getInstruction();
    assert(FreeCall && isa<CallInst>(FreeCall) &&
           "Instruction should be function call");

    // To free the transformed data structure, we need to get the address
    // of the first field stored within the global variable, and pass that
    // to free.
    StructType *StructTy = cast<StructType>(StInfo->getLLVMType());
    StructType *PeelTy = getTransformedType(StructTy);
    Value *PeelVar = PeeledTypeToVariable[PeelTy];
    Instruction *SOAAddr = createPeelFieldLoad(
        PeelTy, PeelVar, ConstantInt::get(Type::getInt32Ty(Context), 0),
        FreeCall);

    // The peeled structure only contains pointer types, and the value of the
    // first field is always the address that the allocation call returned. This
    // will be safe even if the memory allocation failed, because the nullptr
    // returned by the allocation call will be the value stored. Cast the value
    // loaded to an i8*, and update the parameter to free.
    CastInst *SOAAddrAsI8Ptr =
        CastInst::CreateBitOrPointerCast(SOAAddr, getInt8PtrType());
    SOAAddrAsI8Ptr->insertBefore(FreeCall);

    unsigned PtrArgInd = -1U;
    const TargetLibraryInfo &TLI = GetTLI(*FreeCall->getFunction());
    getFreePtrArg(CInfo->getFreeKind(), cast<CallInst>(FreeCall), PtrArgInd,
                  TLI);

    LLVM_DEBUG(dbgs() << "Updating free call:\n  "
                      << *FreeCall->getOperand(PtrArgInd) << "\n  " << *FreeCall
                      << "\n");

    FreeCall->setOperand(PtrArgInd, SOAAddrAsI8Ptr);

    LLVM_DEBUG(dbgs() << "to be:\n  " << *FreeCall->getOperand(PtrArgInd)
                      << "\n  " << *FreeCall << "\n");
  }

  // The transformation of the memfunc call needs to replace the original
  // call with a set of calls, one for each field being touched. This routine
  // dispatches to the appropriate routine based on the type of call.
  void processMemfuncCall(MemfuncCallInfo *CInfo, StructInfo *StInfo) {
    switch (CInfo->getMemfuncCallInfoKind()) {
    case MemfuncCallInfo::MK_Memset:
      processMemset(CInfo, StInfo);
      break;
    case MemfuncCallInfo::MK_Memcpy:
    case MemfuncCallInfo::MK_Memmove:
      // Because the supported cases only allow an exact match of the source
      // and destination structure types for the pointer parameters, the only
      // case where an overlap could occur is when the source and destination
      // are the same address, which means we can use the same code for
      // handling both memcpyy and memmove calls.
      processMemCpyOrMemmove(CInfo, StInfo);
      break;
    }
  }

  // Conversion of the memset requires that each field of the structure be set
  // individually because they are no longer adjacent to one another. This
  // routine will replace the original call with a set of memset calls which
  // operate on the arrays of each field that is being set.
  void processMemset(MemfuncCallInfo *CInfo, StructInfo *StInfo) {
    // Compute the number of elements that are going to be set.
    // Partial memfuncs can only operate on a single structure element, so
    // we don't update the size multiple for those.
    IntrinsicInst *I = cast<IntrinsicInst>(CInfo->getInstruction());
    LLVM_DEBUG(dbgs() << "Replacing memset call: " << *I << "\n");

    StructType *StructTy = cast<StructType>(StInfo->getLLVMType());
    llvm::Type *SizeType = I->getArgOperand(2)->getType();
    Value *CountToSet = nullptr;
    if (!CInfo->getIsCompleteAggregate(0)) {
      CountToSet = ConstantInt::get(SizeType, 1);
    } else {
      // Replace the size computation as if the size were effectively 1 to
      // make the computation equal the number of elements being modified.
      // Each memset created will multiply the size of the field by this
      // new value to set the expected number of bytes in the array.
      uint64_t OrigSize = DL.getTypeAllocSize(StructTy);
      const TargetLibraryInfo &TLI = GetTLI(*I->getFunction());
      updateCallSizeOperand(I, CInfo, OrigSize, 1, TLI);
      CountToSet = I->getOperand(2);
    }

    // Find the peeling index value to use for indexing into the arrays.
    Value *DestPeelIndex = getPeelIndexFromValue(I->getArgOperand(0), StructTy,
                                                 /*ForMemfunc/*/ true);
    assert(DestPeelIndex->getType() == getPeeledIndexType() &&
           "DestPeelIndex expected to match peeling type");

    StructType *PeelTy = getTransformedType(StructTy);
    unsigned FromField = 0;
    unsigned ToField = PeelTy->getNumElements();
    if (!CInfo->getIsCompleteAggregate(0)) {
      FromField = CInfo->getFirstField(0);
      ToField = CInfo->getLastField(0) + 1;
    }

    Value *PeelVar = PeeledTypeToVariable[PeelTy];
    IRBuilder<> IRB(I);
    llvm::Type *Int32Ty = Type::getIntNTy(I->getContext(), 32);

    for (unsigned FieldNum = FromField; FieldNum < ToField; ++FieldNum) {
      // Generate the load of the array address for the field, and then index
      // into that by the peeling index.
      Value *FieldNumVal = ConstantInt::get(Int32Ty, FieldNum);
      Instruction *FieldGEP = createGEPFieldAddressReplacement(
          PeelTy, PeelVar, DestPeelIndex,
          ConstantInt::get(getPtrSizedIntType(), 0), FieldNumVal, I);

      // Create the parameter to use for the pointer parameter of the memset
      Value *PtrI8 = IRB.CreateBitCast(FieldGEP, getInt8PtrType());

      // Compute the number of bytes to be written: CountToSet * sizeof(field)
      llvm::Type *FieldType =
          cast<llvm::PointerType>(PeelTy->getElementType(FieldNum))
              ->getPointerElementType();
      Value *BytesToSet = IRB.CreateMul(
          CountToSet,
          ConstantInt::get(SizeType, DL.getTypeStoreSize(FieldType)));

      // Create a clone of the original call that will maintain all the
      // original attributes, but on which we will modify the pointer and size
      // parameters. We maintain the same value being written because the value
      // being written is a byte, so will be the same for each field even if
      // there is padding between fields. We do not currently preserve DTrans
      // analysis info between transformation passes, and we did not add the
      // peeling structure to the list of analysis data types, so we do not
      // create new a CallInfo object for it. We only need the CallInfo when
      // we start processing the function to identify which ones to update.
      Instruction *CloneCall = I->clone();
      CloneCall->setOperand(0, PtrI8);
      CloneCall->setOperand(2, BytesToSet);
      IRB.Insert(CloneCall);
    }

    // Inform the base class the call info for the original call is no longer
    // valid. This is required because for cloned functions, the base class will
    // update the CallInfo instruction pointer after the cloning is done.
    deleteCallInfo(CInfo);
    InstructionsToDelete.insert(I);
  }

  // Conversion of the memcpy/memmove requires that each field of the structure
  // be processed individually because they are no longer adjacent to one
  // another. This routine will replace the original call with memcpy/memmove
  // calls that operate on the array for each field that is being processed.
  void processMemCpyOrMemmove(MemfuncCallInfo *CInfo, StructInfo *StInfo) {
    assert(CInfo->getIsCompleteAggregate(0) ==
               CInfo->getIsCompleteAggregate(1) &&
           "Expected source and destination to both be complete or both be "
           "partial aggregates");
    assert((CInfo->getIsCompleteAggregate(0) ||
            (CInfo->getFirstField(0) == CInfo->getFirstField(0) &&
             CInfo->getLastField(0) == CInfo->getLastField(1))) &&
           "Expected source and destination to be same field range for partial "
           "aggregates");

    IntrinsicInst *I = cast<IntrinsicInst>(CInfo->getInstruction());
    LLVM_DEBUG(dbgs() << "Replacing memcpy/memmove call: " << *I << "\n");

    StructType *StructTy = cast<StructType>(StInfo->getLLVMType());

    // Compute the number of elements that are going to be set.
    // Partial memfuncs can only operate on a single structure element, so
    // we don't update the size multiple for those.
    llvm::Type *SizeType = I->getArgOperand(2)->getType();
    Value *CountToSet = nullptr;
    if (!CInfo->getIsCompleteAggregate(0)) {
      CountToSet = ConstantInt::get(SizeType, 1);
    } else {
      // Replace the size computation as if the size were effectively 1 to
      // make the computation equal the number of elements being modified.
      // Each memcpy/memmove created will multiply the size of the field by this
      // new value to set the expected number of bytes in the array.
      uint64_t OrigSize = DL.getTypeAllocSize(StructTy);
      const TargetLibraryInfo &TLI = GetTLI(*I->getFunction());
      updateCallSizeOperand(I, CInfo, OrigSize, 1, TLI);
      CountToSet = I->getOperand(2);
    }

    // Find the peeling index value to use for indexing into the arrays.
    Value *DestPeelIndex = getPeelIndexFromValue(I->getArgOperand(0), StructTy,
                                                 /*ForMemfunc/*/ true);
    assert(DestPeelIndex->getType() == getPeeledIndexType() &&
           "DestPeelIndex expected to match peeling type");

    Value *SrcPeelIndex = getPeelIndexFromValue(I->getArgOperand(1), StructTy,
                                                /*ForMemfunc/*/ true);
    assert(SrcPeelIndex->getType() == getPeeledIndexType() &&
           "SrcPeelIndex expected to match peeling type");

    StructType *PeelTy = getTransformedType(StructTy);
    unsigned FromField = 0;
    unsigned ToField = PeelTy->getNumElements();
    if (!CInfo->getIsCompleteAggregate(0)) {
      FromField = CInfo->getFirstField(0);
      ToField = CInfo->getLastField(0) + 1;
    }

    Value *PeelVar = PeeledTypeToVariable[PeelTy];
    IRBuilder<> IRB(I);
    llvm::Type *Int32Ty = Type::getIntNTy(I->getContext(), 32);

    for (unsigned FieldNum = FromField; FieldNum < ToField; ++FieldNum) {
      // Generate the load of the array address for the field, and then index
      // into that by the peeling index. This only needs to be done once,
      // since both the source and destination pointers will just be different
      // offsets from that array because the only support cases are when the
      // types are the same.

      Value *FieldNumVal = ConstantInt::get(Int32Ty, FieldNum);
      Instruction *FieldAddr =
          createPeelFieldLoad(PeelTy, PeelVar, FieldNumVal, I);

      // Offset the address from the start of the array to the index
      Value *DestPtrAddr = IRB.CreateGEP(FieldAddr, DestPeelIndex);
      Value *DestPtrI8 = IRB.CreateBitCast(DestPtrAddr, getInt8PtrType());

      // The source element is from the same array as the destination
      Value *SrcPtrAddr = IRB.CreateGEP(FieldAddr, SrcPeelIndex);
      Value *SrcPtrI8 = IRB.CreateBitCast(SrcPtrAddr, getInt8PtrType());

      llvm::Type *FieldType =
          cast<llvm::PointerType>(PeelTy->getElementType(FieldNum))
              ->getPointerElementType();
      Value *BytesToSet = IRB.CreateMul(
          CountToSet, ConstantInt::get(CountToSet->getType(),
                                       DL.getTypeStoreSize(FieldType)));

      // Create a clone of the original call that will maintain all the
      // original attributes, but on which we can modify the pointer and size
      // parameters. We do not currently preserve DTrans analysis info between
      // transformation passes, and the new call is not  a form that the
      // memfunc analysis would support, so we do not create new CallInfo
      // objects.
      Instruction *CloneCall = I->clone();
      CloneCall->setOperand(0, DestPtrI8);
      CloneCall->setOperand(1, SrcPtrI8);
      CloneCall->setOperand(2, BytesToSet);
      IRB.Insert(CloneCall);
    }

    // Inform the base class the call info for the original call is no longer
    // valid. This is required because for cloned f, the base class will
    // update the CallInfo after the cloning is done.
    deleteCallInfo(CInfo);
    InstructionsToDelete.insert(I);
  }

  // For bitcasts from pointers to the type being transformed, we
  // need to update them because the pointer is going to be converted
  // to an integer type during type remapping. We also need to do this
  // if the cast is to the type being converted.
  void processBitcast(BitCastInst *BC) {

    // The bitcast may no longer be needed after processing the byte flattened
    // GEPs.
    if (BC->user_empty()) {
      LLVM_DEBUG(dbgs() << "Deleting bitcast: " << *BC << "\n");
      InstructionsToDelete.insert(BC);
      return;
    }

    // Convert a bitcast from or to a pointer of the type being transformed:
    //
    // From: %y = bitcast %struct.ty* %x to i8*
    // To  : %cast1 = ptrtoint %struct.ty* %x to i64
    //        %y = inttoptr i64 %cast1 to i8*
    //
    // From: %y = bitcast i8* %x to %struct.ty*
    // To  : %cast1 = ptrtoint i8* %x to i64
    //       %y = inttoptr i64 %cast1 to struct.ty*
    //
    // This will makes the types consistent. During post-processing
    // the one of the int-ptr conversions will just be a conversion
    // from i64 to i64, and will be removed.
    CastInst *ToInt = CastInst::CreateBitOrPointerCast(
        BC->getOperand(0), getPeeledIndexType(), "", BC);
    CastInst *ToPtr =
        CastInst::CreateBitOrPointerCast(ToInt, BC->getType(), "", BC);

    // Mark one of the casts for remove, and steal the name for the other
    if (isTypeToTransform(BC->getType()->getPointerElementType())) {
      PtrConverts.push_back(ToPtr);
      ToInt->takeName(BC);
    } else {
      PtrConverts.push_back(ToInt);
      ToPtr->takeName(BC);
    }

    LLVM_DEBUG(dbgs() << "Replacing bitcast: " << *BC << "\nTo be:\n  "
                      << *ToInt << "\n  " << *ToPtr << "\n");

    BC->replaceAllUsesWith(ToPtr);
    InstructionsToDelete.insert(BC);
  }

  // Update the size used for pointer arithmetic involving dependent structure
  // types.
  void processDepBinOp(BinaryOperator *BinOp) {
    LLVM_DEBUG(dbgs() << "Updating divide operation for "
                         "dependent type involving result of: "
                      << *BinOp << "\n");

    llvm::Type *PtrSubTy = DTInfo->getResolvedPtrSubType(BinOp);
    assert(PtrSubTy && "Expected type for pointer Sub instruction");
    llvm::Type *ReplTy = TypeRemapper->remapType(PtrSubTy);
    updatePtrSubDivUserSizeOperand(BinOp, PtrSubTy, ReplTy, DL);
  }

  // Update the offsets of a byte flattened GEP for dependent structure types.
  void processDepByteFlattendGEP(GetElementPtrInst *GEP) {
    auto InfoPair = DTInfo->getByteFlattenedGEPElement(GEP);
    auto *OrigStructTy = cast<llvm::StructType>(InfoPair.first);

    llvm::Type *ReplTy = TypeRemapper->remapType(OrigStructTy);
    const StructLayout *SL = DL.getStructLayout(cast<StructType>(ReplTy));
    uint64_t NewOffset = SL->getElementOffset(InfoPair.second);
    LLVM_DEBUG(dbgs() << "Replacing byte flattened GEP for "
                         "dependent type: "
                      << *GEP << "\n");
    GEP->setOperand(1,
                    ConstantInt::get(GEP->getOperand(1)->getType(), NewOffset));
    LLVM_DEBUG(dbgs() << "    with\n" << *GEP << "\n");
  }

  // Update the allocation size for a dependent structure type.
  void ProcessDepAllocCall(dtrans::AllocCallInfo *AInfo, StructInfo *StInfo) {
    LLVM_DEBUG(dbgs() << "Updating allocation size on "
                         "dependent type for call: "
                      << *AInfo->getInstruction() << "\n");

    llvm::Type *OrigTy = StInfo->getLLVMType();
    llvm::Type *ReplTy = TypeRemapper->remapType(OrigTy);
    const TargetLibraryInfo &TLI =
        GetTLI(*AInfo->getInstruction()->getFunction());
    updateCallSizeOperand(AInfo->getInstruction(), AInfo, OrigTy, ReplTy, TLI);
  }

  // Update the memfunc size operand for a dependent structure type.
  void processDepMemfuncCall(dtrans::MemfuncCallInfo *CInfo,
                             StructInfo *StInfo) {
    LLVM_DEBUG(dbgs() << "Updating memfunc size on dependent "
                         "type for call: "
                      << *CInfo->getInstruction() << "\n");

    assert(CInfo->getIsCompleteAggregate(0) &&
           "Partial memfuncs currently not supported for dependent structure "
           "types");

    llvm::Type *OrigTy = StInfo->getLLVMType();
    llvm::Type *ReplTy = TypeRemapper->remapType(OrigTy);
    const TargetLibraryInfo &TLI =
        GetTLI(*CInfo->getInstruction()->getFunction());
    updateCallSizeOperand(CInfo->getInstruction(), CInfo, OrigTy, ReplTy, TLI);
  }

  // Return an integer type that will be used as a replacement type for pointers
  // to the types being peeled.
  llvm::Type *getPeeledIndexType() const {
    assert(PeelIndexType && PeelIndexWidth &&
           "Peeling index type/width not set");
    return PeelIndexType;
  }

  // Initialize class fields that are dependent on the size
  // of the peeling index type.
  void initializePeeledIndexType(unsigned BitWidth) {
    assert(BitWidth >= 32 && "Peeling index must be 32-bits or larger");

    PeelIndexWidth = BitWidth;
    PeelIndexType = Type::getIntNTy(Context, PeelIndexWidth);

    IncompatiblePeelTypeAttrs = AttributeFuncs::typeIncompatible(PeelIndexType);
  }

  uint64_t getPeeledIndexWidth() const { return PeelIndexWidth; }
  llvm::Type *getPtrSizedIntType() const { return PtrSizedIntType; }
  llvm::Type *getInt8PtrType() const { return Int8PtrType; }

  bool isPointerShrinkingEnabled() const { return PointerShrinkingEnabled; }

  bool isTypeToTransform(llvm::Type *Ty) {
    if (!Ty->isStructTy())
      return false;

    // Since we expect to have very few types to check, just loop over them.
    // This could be converted to OrigToNewTypeMapping.count(Ty) in the
    // future, if it is found that is faster.
    for (auto &ONPair : OrigToNewTypeMapping)
      if (ONPair.first == Ty)
        return true;

    return false;
  }

  // Return 'true' if \p Ty is a structure type that is modified as a result of
  // containing a reference to a peeling index.
  bool isDepTypeToTransform(llvm::Type *Ty) {
    if (!Ty->isStructTy() || DepTypesToTransform.empty())
      return false;

    return DepTypesToTransform.count(Ty) != 0;
  }

  // Return the type of conversion being done for llvm::Type \p Ty.
  // If the type is a structure, then it could be a type being converted
  // to a structure of arrays. When the peeling index is smaller than a
  // pointer, then structures that contain pointers to the type also have
  // their size affected.
  AOSConvType isTypeOrDepTypeToTransform(llvm::Type *Ty) {
    if (!Ty->isStructTy())
      return AOS_NoConv;

    if (isTypeToTransform(Ty))
      return AOS_SOAConv;

    if (isPointerShrinkingEnabled() && isDepTypeToTransform(Ty))
      return AOS_DepConv;

    return AOS_NoConv;
  }

  // Helper function to get the Type for \p CallInfo cases
  // that can be transformed. If the CallInfo is for a case that is not
  // being transformed, return nullptr.
  std::pair<llvm::Type *, AOSConvType>
  getCallInfoTypeToTransform(dtrans::CallInfo *CInfo) {
    auto &TypeList = CInfo->getPointerTypeInfoRef().getTypes();

    // Only cases with a single type will be allowed during the transformation.
    // If there's more than one, we don't need the type, because we won't be
    // transforming it.
    if (TypeList.size() != 1)
      return std::make_pair(nullptr, AOS_NoConv);

    llvm::Type *Ty = *TypeList.begin();
    if (!Ty->isPointerTy())
      return std::make_pair(nullptr, AOS_NoConv);

    llvm::Type *ElemTy = Ty->getPointerElementType();
    if (isTypeToTransform(ElemTy))
      return std::make_pair(ElemTy, AOS_SOAConv);

    if (isDepTypeToTransform(ElemTy))
      return std::make_pair(ElemTy, AOS_DepConv);

    return std::make_pair(nullptr, AOS_NoConv);
  }

  // Get the new type for a structure type being transformed. This function
  // can only be used for types that are known to be transformed to avoid
  // the need for nullptr checks on the return value, and to allow casting
  // the result from the map to be a StructType.
  llvm::StructType *getTransformedType(llvm::Type *OrigTy) {
    assert(isTypeToTransform(OrigTy) &&
           "Expected input type to be type being transformed");

    // Since we expect to have very few types to check, just loop over them.
    // This could be converted to OrigToNewTypeMapping.count(Ty) in the
    // future, if it is found that is faster.
    for (auto &ONPair : OrigToNewTypeMapping)
      if (ONPair.first == OrigTy)
        return cast<StructType>(ONPair.second);

    llvm_unreachable("AOS-to-SOA transformation type not found");
  }

  // When rewriting pointers to the transformed structure type to integer
  // types, attributes on the return type and function parameters may need to
  // be updated because some attributes are only allowed on pointer
  // parameters. This routine updates CallBase instructions in the original
  // function prior to the clone function creation.
  void updateCallAttributes(CallBase *Call) {
    bool Changed = false;
    AttributeList Attrs = Call->getAttributes();

    // Only calls to functions to be cloned (or indirect calls) need to be
    // checked because the parameter types will not be changing for any
    // other calls.
    Function *Callee = Call->getCalledFunction();
    if (Callee && !OrigFuncToCloneFuncMap.count(Callee))
      return;

    DEBUG_WITH_TYPE(
        AOSTOSOA_ATTRIBUTES,
        dbgs() << "DTRANS-AOSTOSOA: Updating call attributes for: "
               << *Call << "\n");

    Type *OrigRetTy = Call->getType();
    if (OrigRetTy->isPointerTy() &&
        isTypeToTransform(OrigRetTy->getPointerElementType())) {
      // Argument index 0 is used for return type attributes
      Attrs = Attrs.removeAttributes(Context, 0, IncompatiblePeelTypeAttrs);
      Changed = true;
    }

    // Argument index numbers start with 1 for calls to removeAttributes.
    unsigned Idx = 1;
    for (auto &Arg : Call->args()) {
      Type *ArgTy = Arg->getType();
      if (ArgTy->isPointerTy() &&
          isTypeToTransform(ArgTy->getPointerElementType())) {
        Attrs = Attrs.removeAttributes(Context, Idx, IncompatiblePeelTypeAttrs);
        Changed = true;
      }
      ++Idx;
    }

    if (Changed)
      Call->setAttributes(Attrs);

    DEBUG_WITH_TYPE(AOSTOSOA_ATTRIBUTES,
                    dbgs() << "DTRANS-AOSTOSOA: After call update: "
                           << *Call << "\n");
  }

  // The list of types to be transformed.
  SmallVector<dtrans::StructInfo *, 4> TypesToTransform;

  // The list of dependent types that are impacted due to the
  // peeling index.
  SetVector<llvm::Type *> DepTypesToTransform;

  // A mapping from the original structure type to the new structure type
  SmallVector<std::pair<llvm::Type *, llvm::Type *>, 4> OrigToNewTypeMapping;

  // A mapping from the peeled structure type to the global variable used to
  // access it.
  DenseMap<StructType *, GlobalVariable *> PeeledTypeToVariable;

  // Pointers to the peeled structure will be converted to an index
  // value for the array element of the structure of arrays. These
  // variables hold the integer width and type that will used for the index.
  uint64_t PeelIndexWidth;
  llvm::Type *PeelIndexType;
  bool PointerShrinkingEnabled;

  SmallDenseMap<Type *, Value *> AllocationAnnotationGEP;
  SmallDenseMap<Type *, Value *> PeelIndexAnnotationGEP;
  Value *AnnotationFilenameGEP;

  // Integer type that has the same size as a pointer type. This may be
  // different than the peel index type.
  llvm::Type *PtrSizedIntType;

  // i8* Type
  llvm::Type *Int8PtrType;

  // When the pointer to the peeled structure is converted, there are several
  // attributes that may have been used on function parameters or return
  // types that are not valid on the index type being used. This variable
  // holds the list of incompatible attributes that need to be removed
  // from function signatures and call sites for the cloned routines.
  AttrBuilder IncompatiblePeelTypeAttrs;

  // A list of pointer conversion instructions (contains instructions that are
  // pointer to int, int to pointer, or pointer of original type to pointer of
  // remapped type) that after the remapping process should have the same source
  // and destination types, and are to be removed during function post
  // processing. This list is reset for each function processed.
  SmallVector<CastInst *, 16> PtrConverts;

  // When Load/Store instructions are converted for shrinking the index to
  // 32-bits, cast instructions are generated during processFunction around
  // the replacement load/store instruction to convert the original type to an
  // element/pointer of the shrunken index size/type that will be loaded/stored.
  // During post-processing, it may be possible to remove these casts because
  // the type remapping has modified operand types to make the casts cancel out.
  // This is necessary to avoid casts forms that would appear to be bad casting
  // for subsequent runs of the DTransAnalysis. For example:
  //    %53 = bitcast i32* %52 to i64*
  //    %54 = bitcast i64* %53 to i32*
  //
  // This list needs be cleared after processing each function.
  SmallVector<Instruction *, 16> IntermediateConverts;

  // When processing byte-flattened GEPs and memfuncs some instructions may be
  // processed to get the peeling index. This cache is used to get the
  // processed values so they do not need to be recomputed for multiple
  // instructions being processed. This is also necessary to avoid cycles when
  // walking PHI nodes. This map needs be cleared after processing each
  // function.
  DenseMap<Value *, Value *> PeelIndexCache;

  // List of instructions that are to be removed at the end of
  // processFunction().
  SmallPtrSet<Instruction *, 16> InstructionsToDelete;

  // List of instructions to be annotated as being pointers to the peeling
  // index. The Type field stores the original type of structure that
  // now uses a peeling index.
  SmallVector<std::pair<Instruction *, Type *>, 16> InstructionsToAnnotate;
};

class DTransAOSToSOAWrapper : public ModulePass {
private:
  dtrans::AOSToSOAPass Impl;

public:
  static char ID;

  DTransAOSToSOAWrapper() : ModulePass(ID) {
    initializeDTransAOSToSOAWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    auto &DTAnalysisWrapper = getAnalysis<DTransAnalysisWrapper>();
    DTransAnalysisInfo &DTInfo = DTAnalysisWrapper.getDTransInfo(M);
    auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    auto &WPInfo = getAnalysis<WholeProgramWrapperPass>().getResult();
    // This lambda function is to allow getting the DominatorTree analysis for a
    // specific function to allow analysis of loops for the dynamic allocation
    // of the structure.
    dtrans::AOSToSOAPass::DominatorTreeFuncType GetDT =
        [this](Function &F) -> DominatorTree & {
      return this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };

    bool Changed = Impl.runImpl(M, DTInfo, GetTLI, WPInfo, GetDT);
    if (Changed)
      DTAnalysisWrapper.setInvalidated();
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // TODO: Mark the actual required and preserved analyses.
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<DTransAnalysisWrapper>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // end anonymous namespace

char DTransAOSToSOAWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransAOSToSOAWrapper, "dtrans-aostosoa",
                      "DTrans array of structs to struct of arrays", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransAOSToSOAWrapper, "dtrans-aostosoa",
                    "DTrans array of structs to struct of arrays", false, false)

ModulePass *llvm::createDTransAOSToSOAWrapperPass() {
  return new DTransAOSToSOAWrapper();
}

namespace llvm {
namespace dtrans {

bool AOSToSOAPass::runImpl(
    Module &M, DTransAnalysisInfo &DTInfo,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
    WholeProgramInfo &WPInfo, AOSToSOAPass::DominatorTreeFuncType &GetDT) {

  if (!WPInfo.isWholeProgramSafe()) {
    LLVM_DEBUG(
        dbgs() << "DTRANS-AOSTOSOA: inhibited -- not whole program safe");
    return false;
  }

  if (!DTInfo.useDTransAnalysis()) {
    LLVM_DEBUG(
        dbgs() << "DTRANS-AOSTOSOA: inhibited -- dtrans-analysis disabled");
    return false;
  }

  // Check whether there are any candidate structures that can be transformed.
  StructInfoVec CandidateTypes;
  gatherCandidateTypes(DTInfo, CandidateTypes);
  if (CandidateTypes.empty())
    return false;

  qualifyCandidates(CandidateTypes, M, DTInfo, GetDT);

  if (CandidateTypes.empty())
    return false;

  // Perform the actual transformation.
  DTransTypeRemapper TypeRemapper;
  AOSToSOAMaterializer Materializer(TypeRemapper);
  AOSToSOATransformImpl Transformer(DTInfo, M.getContext(), M.getDataLayout(),
                                    GetTLI, "__SOADT_", &TypeRemapper,
                                    &Materializer, CandidateTypes);
  return Transformer.run(M);
}

// Populate the \p CandidateTypes vector with all the structure types
// that meet the minimum safety conditions to be considered for transformation.
void AOSToSOAPass::gatherCandidateTypes(DTransAnalysisInfo &DTInfo,
                                        StructInfoVecImpl &CandidateTypes) {

  for (dtrans::TypeInfo *TI : DTInfo.type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo)
      continue;

    if (cast<StructType>(TI->getLLVMType())->isLiteral())
      continue;

    if (DTInfo.testSafetyData(TI, dtrans::DT_AOSToSOA)) {
      LLVM_DEBUG(
          dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Unsupported safety data: "
                 << StructNamePrintHelper(TI->getLLVMType()) << "\n");
      continue;
    }

    CandidateTypes.push_back(cast<StructInfo>(TI));
  }
}

// This routine examines all the candidates and performs additional safety
// checks on the type and usage to determine whether a type is supported for
// being transformed. The \p CandidateTypes list will be updated to only contain
// the elements that pass all the safety checks.
void AOSToSOAPass::qualifyCandidates(
    StructInfoVecImpl &CandidateTypes, Module &M, DTransAnalysisInfo &DTInfo,
    AOSToSOAPass::DominatorTreeFuncType &GetDT) {
  if (!qualifyCandidatesTypes(CandidateTypes, DTInfo))
    return;

  if (!qualifyAllocations(CandidateTypes, DTInfo, GetDT))
    return;

  if (!qualifyHeuristics(CandidateTypes, M, DTInfo))
    return;

  LLVM_DEBUG({
    for (auto *Candidate : CandidateTypes)
      dbgs() << "DTRANS-AOSTOSOA: Passed qualification tests: "
             << StructNamePrintHelper(Candidate->getLLVMType()) << "\n";
  });
}

// Check for any types that are not supported for the transformation.
// 1. Types that are used as arrays are not supported, for example
//     [4 x struct.test], because we would need to handle all the allocation
//     checks and transformation code for these arrays, as well.
// 2. Types that contain arrays are not supported. This restriction could be
//     relaxed in a future version.
// 3. Types that contain vectors are not supported.
//
// Return 'true' if candidates remain after this filtering.
bool AOSToSOAPass::qualifyCandidatesTypes(StructInfoVecImpl &CandidateTypes,
                                          DTransAnalysisInfo &DTInfo) {
  // Collect a set of structure types that are arrays composed of structure
  // types so that we can check if any of these match the candidate types.
  SmallPtrSet<dtrans::StructInfo *, 4> ArrayElemTypes;
  for (auto *TI : DTInfo.type_info_entries()) {
    if (!isa<dtrans::ArrayInfo>(TI))
      continue;

    Type *ElemTy = TI->getLLVMType()->getArrayElementType();
    while (isa<ArrayType>(ElemTy))
      ElemTy = ElemTy->getArrayElementType();

    if (!isa<StructType>(ElemTy))
      continue;

    auto *StInfo = cast<dtrans::StructInfo>(DTInfo.getTypeInfo(ElemTy));
    ArrayElemTypes.insert(StInfo);
  }

  StructInfoVec Qualified;
  for (auto *Candidate : CandidateTypes) {
    if (ArrayElemTypes.find(Candidate) != ArrayElemTypes.end()) {
      LLVM_DEBUG(dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Array of type seen: "
                        << StructNamePrintHelper(Candidate->getLLVMType())
                        << "\n");
      continue;
    }

    // No arrays of the structure type were found, now check the structure
    // field types to verify all members are supported for the transformation.
    // Reject any that contain arrays or vectors. We don't need to check for
    // structures because those were rejected by the safety checks.
    bool Supported = true;
    for (auto &FI : Candidate->getFields()) {
      Type *Ty = FI.getLLVMType();
      if (Ty->isArrayTy() || Ty->isVectorTy()) {
        Supported = false;
        break;
      }
    }

    if (Supported)
      Qualified.push_back(Candidate);
    else
      LLVM_DEBUG(
          dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Unsupported structure "
                    "element type: "
                 << StructNamePrintHelper(Candidate->getLLVMType()) << "\n");
  }

  std::swap(CandidateTypes, Qualified);
  return !CandidateTypes.empty();
}

// Check that the type is only allocated once by malloc or calloc.
// Return 'true' if candidates remain after this filtering.
bool AOSToSOAPass::qualifyAllocations(StructInfoVecImpl &CandidateTypes,
                                      DTransAnalysisInfo &DTInfo,
                                      DominatorTreeFuncType &GetDT) {
  // Build a mapping from each allocated type to a single allocating
  // instruction, if one exists. If there are multiple allocations or an
  // unsupported allocation, map the type to 'nullptr'.
  DenseMap<dtrans::StructInfo *, Instruction *> TypeToAllocInstr;
  for (auto *Call : DTInfo.call_info_entries()) {
    auto *ACI = dyn_cast<dtrans::AllocCallInfo>(Call);
    if (!ACI || !ACI->getAliasesToAggregatePointer())
      continue;

    // We do not support transforming any allocations that are not
    // calloc/malloc. Invalidate the information for all types.
    if (ACI->getAllocKind() != dtrans::AK_Calloc &&
        ACI->getAllocKind() != dtrans::AK_Malloc) {
      for (auto *AllocatedTy : ACI->getPointerTypeInfoRef().getTypes()) {
        auto *Ty = AllocatedTy->getPointerElementType();
        auto *TI = DTInfo.getTypeInfo(Ty);
        if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI)) {
          LLVM_DEBUG({
            if (std::find(CandidateTypes.begin(), CandidateTypes.end(),
                          StInfo) != CandidateTypes.end() &&
                (!TypeToAllocInstr.count(StInfo) ||
                 TypeToAllocInstr[StInfo] != nullptr))
              dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Unsupported "
                        "allocation function: "
                     << StructNamePrintHelper(Ty) << "\n"
                     << "  " << *ACI->getInstruction() << "\n";
          });

          TypeToAllocInstr[StInfo] = nullptr;
        }
      }

      continue;
    }

    // For supported allocations, update the association between the type and
    // allocating instruction.
    for (auto *AllocatedTy : ACI->getPointerTypeInfoRef().getTypes()) {
      auto *Ty = AllocatedTy->getPointerElementType();
      auto *TI = DTInfo.getTypeInfo(Ty);
      if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI)) {
        if (TypeToAllocInstr.count(StInfo)) {
          LLVM_DEBUG({
            if (std::find(CandidateTypes.begin(), CandidateTypes.end(),
                          StInfo) != CandidateTypes.end() &&
                TypeToAllocInstr[StInfo] != nullptr)
              dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Too many allocations: "
                     << StructNamePrintHelper(Ty) << "\n";
          });
          TypeToAllocInstr[StInfo] = nullptr;
          continue;
        }

        TypeToAllocInstr[StInfo] = ACI->getInstruction();
      }
    }
  }

  // Select the types that passed the single allocation location test. Also,
  // populate a set of instructions  by function that need to be checked to
  // verify the allocation is not within a loop. We group these by function so
  // that the LoopInfo for a function only needs to be calculated one time.
  // This will reject any type that does not have a dynamic allocation because
  // currently the only way for the pointers in the peeled structure to be
  // initialized is by modifying the allocation site.
  StructInfoVec Qualified;
  DenseMap<Function *, DenseSet<std::pair<Instruction *, dtrans::StructInfo *>>>
      AllocPathMap;
  SmallVector<std::pair<Function *, Instruction *>, 4> CallChain;
  for (auto *TyInfo : CandidateTypes) {
    if (TypeToAllocInstr.count(TyInfo)) {
      if (TypeToAllocInstr[TyInfo] == nullptr)
        continue;

      Instruction *I = TypeToAllocInstr[TyInfo];
      Value *Unsupported;
      if (!supportedAllocationUsers(I, TyInfo->getLLVMType(), &Unsupported)) {
        LLVM_DEBUG(
            dbgs()
            << "DTRANS-AOSTOSOA: Rejecting -- Unsupported allocation usage: "
            << StructNamePrintHelper(TyInfo->getLLVMType()) << "\n"
            << "  " << *Unsupported << "\n");
        continue;
      }

      // Verify the call chain to the instruction consists of a single path
      CallChain.clear();
      if (!collectCallChain(I, CallChain)) {
        LLVM_DEBUG(
            dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Multiple call paths: "
                   << StructNamePrintHelper(TyInfo->getLLVMType()) << "\n");
        continue;
      }

      // Save the instruction and all it's caller to the list of locations that
      // will need to be checked for being within loops.
      AllocPathMap[I->getParent()->getParent()].insert(
          std::make_pair(I, TyInfo));
      for (auto &FuncInstrPair : CallChain)
        AllocPathMap[FuncInstrPair.first].insert(
            std::make_pair(FuncInstrPair.second, TyInfo));

      Qualified.push_back(TyInfo);
    }
  }

  std::swap(CandidateTypes, Qualified);
  if (CandidateTypes.empty())
    return false;

  // check the function's loops to see if the allocation (or call to the
  // allocation) instruction is within a loop
  for (auto &FuncToAllocPath : AllocPathMap) {
    Function *F = FuncToAllocPath.first;
    DominatorTree &DT = (GetDT)(*F);
    LoopInfo LI(DT);

    if (LI.size())
      for (auto &InstTypePair : FuncToAllocPath.second)
        if (LI.getLoopFor(InstTypePair.first->getParent())) {
          StructInfo *StInfo = InstTypePair.second;
          LLVM_DEBUG(dbgs()
                     << "DTRANS-AOSTOSOA: Rejecting -- Allocation in loop: "
                     << StructNamePrintHelper(StInfo->getLLVMType())
                     << "\n  Function: " << F->getName() << "\n");
          auto *It =
              std::find(CandidateTypes.begin(), CandidateTypes.end(), StInfo);
          if (It != CandidateTypes.end())
            CandidateTypes.erase(It);
        }
  }

  return !CandidateTypes.empty();
}

// The result of the allocation call is an i8* type. Because we are going to be
// replacing the ultimate pointer assignment during the transformation with an
// integer type, and storing the result of the allocation into a compiler
// generated structure, we need to perform some additional checks to make sure
// the allocation can be replaced. Specifically, the following uses will be
// allowed, and all others rejected:
//   icmp eq i8* %call, null   [also allow inequality, and reversed operands]
//   bitcast i8* %call to %struct.type* [ only allow cast to candidate type ptr]
//   store i8* %call, bitcast (@global to i8**)
//
// In the future, this may be extended to allow memset, memcpy, or memmove
// using the i8* result of the allocation directly.
// Using the result in a PHI/Select instructions could be safe, but will require
// additional analysis, and handling when processing the allocation call, so is
// not supported at this time.
bool AOSToSOAPass::supportedAllocationUsers(Instruction *AllocCall,
                                            llvm::Type *StructTy,
                                            Value **Unsupported) {
  assert(isa<CallInst>(AllocCall) &&
         "Instruction expected to be allocation call");

  *Unsupported = nullptr;
  for (auto *User : AllocCall->users()) {
    auto *I = dyn_cast<Instruction>(&*User);
    if (!I) {
      *Unsupported = User;
      return false;
    }

    switch (I->getOpcode()) {
    default:
      *Unsupported = I;
      return false;
    case Instruction::ICmp:
      if ((cast<ICmpInst>(I))->isRelational()) {
        *Unsupported = I;
        return false;
      }
      if (!(I->getOperand(0) ==
                Constant::getNullValue(I->getOperand(0)->getType()) ||
            I->getOperand(1) ==
                Constant::getNullValue(I->getOperand(1)->getType()))) {
        *Unsupported = I;
        return false;
      }
      break;
    case Instruction::Store:
      if ((cast<StoreInst>(I))->getPointerOperand() == AllocCall) {
        *Unsupported = I;
        return false;
      }
      break;
    case Instruction::BitCast:
      if (I->getType() != StructTy->getPointerTo()) {
        *Unsupported = I;
        return false;
      }
      break;
    }
  }

  return true;
}

// If there is a single call chain that reaches the instruction, \p I,
// add the function to the \p CallChain, and return 'true'. Otherwise, return
// 'false'.
bool AOSToSOAPass::collectCallChain(
    Instruction *I,
    SmallVectorImpl<std::pair<Function *, Instruction *>> &CallChain) {
  Function *F = I->getParent()->getParent();
  Instruction *Callsite = nullptr;

  for (auto *U : F->users()) {
    if (auto *Call = dyn_cast<CallInst>(*&U)) {
      // Check that we only have a single call path to the routine.
      if (Callsite != nullptr)
        return false;

      Callsite = Call;
    } else {
      return false;
    }
  }

  // Verify the top of the callchain is the 'main' routine
  if (!Callsite)
    return isMainFunction(*F);

  CallChain.push_back(
      std::make_pair(Callsite->getParent()->getParent(), Callsite));
  return collectCallChain(Callsite, CallChain);
}

// Filter the \p CandidateTypes list based on whether the type meets
// the criteria of the profitability heuristics.
// Return 'true' if candidates remain after this filtering.
bool AOSToSOAPass::qualifyHeuristics(StructInfoVecImpl &CandidateTypes,
                                     Module &M, DTransAnalysisInfo &DTInfo) {
  StructInfoVec Qualified;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Check for any structures specified on the command line, and use these
  // instead of the profitability heuristics.
  SmallVector<StringRef, 4> SubStrings;
  if (!DTransAOSToSOAHeurOverride.empty()) {
    SplitString(DTransAOSToSOAHeurOverride, SubStrings, ",");
    for (auto &Name : SubStrings) {
      Type *Ty = M.getTypeByName(Name);
      if (auto *StructTy = dyn_cast_or_null<StructType>(Ty)) {
        LLVM_DEBUG(
            dbgs()
            << "DTRANS-AOSTOSOA: Skipped profitability heuristics for type: "
            << Name << "\n");
        dtrans::TypeInfo *Info = DTInfo.getTypeInfo(StructTy);
        assert(Info &&
               "DTransAnalysisInfo does not contain info for structure");

        // Only allow the heuristic override to enable cases that actually
        // met the required safety conditions.
        dtrans::StructInfo *StInfo = cast<dtrans::StructInfo>(Info);
        if (std::find(CandidateTypes.begin(), CandidateTypes.end(), StInfo) ==
            CandidateTypes.end()) {
          LLVM_DEBUG(dbgs()
                     << "DTRANS-AOSTOSOA: Cannot force transformation on type "
                        "that fails safety checks: "
                     << Name << "\n");
          continue;
        }
        Qualified.push_back(StInfo);
      }
    }

    std::swap(CandidateTypes, Qualified);
    return !CandidateTypes.empty();
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // If we don't have good frequency info, then process all available
  // structures, otherwise select the structures that reach some percentage of
  // the max frequency.
  uint64_t MaxFreq = DTInfo.getMaxTotalFrequency();
  if (MaxFreq == 0 || MaxFreq == std::numeric_limits<uint64_t>::max())
    return !CandidateTypes.empty();

  for (auto *Candidate : CandidateTypes) {
    uint64_t Freq = Candidate->getTotalFrequency();
    uint64_t RelHotness = (uint64_t)((((float)Freq) / MaxFreq) * 100.0);
    if (RelHotness < DTransAOSToSOAFrequencyThreshold) {
      LLVM_DEBUG(
          dbgs()
          << "DTRANS-AOSTOSOA: Rejecting -- Does not meet hotness threshold: "
          << StructNamePrintHelper(Candidate->getLLVMType()) << "\n  "
          << RelHotness << " < " << DTransAOSToSOAFrequencyThreshold << "\n");
      continue;
    }

    Qualified.push_back(Candidate);
  }

  std::swap(CandidateTypes, Qualified);
  return !CandidateTypes.empty();
}

PreservedAnalyses AOSToSOAPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function*>(&F)));
  };
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  DominatorTreeFuncType GetDT = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };

  bool Changed = runImpl(M, DTransInfo, GetTLI, WPInfo, GetDT);

  if (!Changed)
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // end namespace dtrans
} // end namespace llvm
