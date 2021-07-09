//==== AOSToSOAOP.cpp - AOS-to-SOA with support for opaque pointers ====//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//
// This file implements the DTrans Array of Structures to Structure of Arrays
// data layout optimization pass with support for IR using either opaque or
// non-opaque pointers.
//
// The AOS-to-SOA transformation will convert an allocation of a structure
// into a form where pointers to the structures of the type transformed are
// no longer used, but instead accesses are made using an integer index into
// an array.
//
// The following example shows the use of non-opaque pointers, with opaque
// pointers things are similar except all pointer types will simply be 'ptr'.
//
// For example:
//   %struct.test = type { i64, %struct.test* }
//   %array = call i8* @malloc(i64 160)
//
// This would create an array of 10 instances of the structure. These elements
// can be accessed from the array. However, because a pointer to the type of the
// structure is stored, the address of an element of the array can be stored to
// memory allowing access an arbitrary element of the allocated array. To handle
// this, when the transformation takes place, pointers to the type transformed
// will be converted into integer indices of the allocated array. There are many
// conditions imposed to support this, such as, only allowing a single array of
// structures of the type being allocated in the program. This results in the
// transformation producing a new structure which will hold an array for each of
// the original field members of the structure type being transformed and a
// global variable of this new structure type, which will be initialized to hold
// the address where each of these arrays begins at the point when memory for
// the original array of structures was allocated.
//
// New structure:
//   %SOA_struct.test = type { i64*, i64* }
//   @SOA_VAR = internal global %SOA_struct.test zeroinitializer
//
// The original i64 type has been converted to a pointer to i64 elements.
// The %struct.test* has been converted into a pointer of i64 elements as well,
// because all load & stores of pointers to the type will be transformed to hold
// the integer index of the array during this transformation. Indexing will
// begin with 1 to allow the use of 0 to represent a nullptr element. When the
// allocation occurs, the global variable will be initialized with an address
// where each arrays begin.
//
// Note 1: This requires the allocation to allocate space for one more structure
// than the original allocation.
// Note 2: the size of the integer for the index in some cases will be 32-bits
// instead of 64-bits, when it is determined that the maximum number of elements
// will fit within a 32-bit value.
//
//===---------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/AOSToSOAOP.h"

#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnnotator.h"
#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOPOptBase.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/InitializePasses.h"

using namespace llvm;
using namespace dtransOP;
using dtrans::DTransAnnotator;
using dtrans::StructInfo;

#define DEBUG_TYPE "dtrans-aostosoaop"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// For testing purposes, this flag allows the types to be transformed to be
// specified in a comma separated list of structure names. Use this with
// -dtrans-aostosoaop-qual-override to also avoid checking for the conditions
// required for the allocation call of the type.
static cl::opt<std::string> AOSToSOAOPTypelist(
    "dtrans-aostosoaop-typelist",
    cl::desc("Specify structures for transforming via AOS-to-SOA"),
    cl::ReallyHidden);

// This flag is for being able to test the transformation on any IR without the
// structure needing to satisfy the conditions that are imposed on the type and
// allocation of the type. Generally, this will be combined with the above flag
// that allows selection of types to be transformed when implementing LIT tests.
static cl::opt<bool>
    DTransAOSToSOAOPQualOverride("dtrans-aostosoaop-qual-override",
                                 cl::init(false), cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// When set, this option causes the index size to be 32-bits, instead of the
// number of bits required to hold a pointer type, if possible. The safety
// checks of dependent structures may prevent the index from being 32-bits
// because it may not be possible to change the size of the depenedent type.
static cl::opt<bool> DTransAOSToSOAOPIndex32("dtrans-aostosoaop-index32",
                                             cl::init(true), cl::ReallyHidden);

namespace {

// This structure describes information about the index variable that will be
// used for accesses into the array. The index type is used to replace pointers
// to the type being transformed. However, it can be 32-bits or 64-bits wide,
// even when a pointer type is 64-bits wide.
struct SOAIndexInfoTy {
  uint64_t Width = 0;
  llvm::Type *LLVMType = nullptr;
  DTransType *DTType = nullptr;

  // When this is 'true', the index width will be 32-bits, even though the
  // pointer type it is replacing was 64-bits.
  bool PointerShrinkingEnabled = false;

  // When the pointer to the structure is transformed, there are several
  // attributes that may have been used on function parameters or return
  // types that are not valid on the index type being used. This variable
  // holds the list of incompatible attributes that need to be removed
  // from function signatures and call sites for the cloned routines.
  AttrBuilder IncompatibleTypeAttrs;
};

// This structure holds the information about structure type that is going to be
// transformed.
struct SOATypeInfoTy {
  // The original structure type that will be transformed.
  llvm::StructType *OrigStructType = nullptr;

  // The representation of the original as a DTransStructType which keeps track
  // of pointer element types.
  DTransStructType *OrigDTransType = nullptr;

  // The structure of arrays type that will replace the structure.
  llvm::StructType *SOAStructType = nullptr;
  DTransStructType *DTransSOAStructType = nullptr;

  // The global variable of the SOAStructType that will hold the base addresses
  // of the arrays.
  GlobalVariable *SOAVar = nullptr;

  // For transformations downstream of AOS-to-SOA, ptr.annotation intrinsics are
  // inserted to represent where the Index value and the SOA variable allocation
  // occur. These Value* objects are constant string GEPs that are used as
  // arguments when those intrinsic calls are inserted.
  Value *IndexAnnotationGEP = nullptr;
  Value *AllocAnnotationGEP = nullptr;

  // A vector of types that the field members point to.
  //
  // For example:
  // Transforming: %struct.t = type { i64, %struct.t*, %struct.a* }
  // Produces an SOA structure type of: { i64*, i64*, %struct.a**}
  //   (note: %struct.t* becomes an index value)
  // This vector will store: [ i64, i64, %struct.a* ]
  // With opaque pointers, it will be: [ i64, i64, ptr ]
  //
  // This is used because we will not be able to do getPointerElementType()
  // calls on the SOAStructType field members.
  SmallVector<llvm::Type *, 16> LLVMStructRemappedFieldsTypes;

  // An address space to use on pointer references to the original type to
  // recognize locations where the pointer needs to be converted into the
  // integer index type.
  unsigned AddrSpaceForType = 0;
};

// This class is responsible for all the transformation work for the AOS to SOA
// with Indexing conversion.
class AOSToSOAOPTransformImpl : public DTransOPOptBase {
public:
  AOSToSOAOPTransformImpl(LLVMContext &Ctx, DTransSafetyInfo *DTInfo,
                          StringRef DepTypePrefix, const DataLayout &DL,
                          AOSToSOAOPPass::GetTLIFuncType GetTLI,
                          SmallVectorImpl<dtrans::StructInfo *> &Types)
      : DTransOPOptBase(Ctx, DTInfo, DepTypePrefix), DL(DL), GetTLI(GetTLI) {
    std::copy(Types.begin(), Types.end(), std::back_inserter(TypesToTransform));
  }
  bool prepareTypes(Module &M) override;
  void populateTypes(Module &M) override;

  void prepareModule(Module &M) override;
  void processFunction(Function &F) override;
  void postprocessFunction(Function &OrigFunc, bool isCloned) override;

  struct SOATypeInfoTy &getSOATypeInfo(llvm::StructType *Ty);

private: // methods
  void qualifyDependentTypes(unsigned PointerSizeInBits);
  void initializeIndexType(LLVMContext &Ctx, unsigned BitWidth);
  llvm::Type *getIndexLLVMType() const;
  DTransType *getIndexDTransType() const;

private: // data
  const DataLayout &DL;
  AOSToSOAOPPass::GetTLIFuncType GetTLI;

  // The list of types to be transformed.
  SmallVector<dtrans::StructInfo *, 4> TypesToTransform;

  // Information about the structures being transformed.
  SmallVector<struct SOATypeInfoTy, 2> SOATypes;

  // A mapping from the original structure type to the new structure type for
  // structure types that need to change as a result of having a pointer to a
  // type being transformed. When pointer shrinking is enabled, the size of
  // these structures and byte offsets of fields will be modified.
  SmallPtrSet<llvm::StructType *, 4> DepTypesToTransform;

  // Value to use for the 3rd string argument of the ptr.annotation intrinsic.
  // This argument is used to represent a filename, and will be ignored.
  Value *AnnotationFilenameGEP = nullptr;

  // Information about the index type that will replace the pointer to the
  // structure being transformed.
  struct SOAIndexInfoTy IndexInfo;
};

class DTransAOSToSOAOPWrapper : public ModulePass {
private:
  AOSToSOAOPPass Impl;

public:
  static char ID;

  DTransAOSToSOAOPWrapper() : ModulePass(ID) {
    initializeDTransAOSToSOAOPWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    auto &DTAnalysisWrapper = getAnalysis<DTransSafetyAnalyzerWrapper>();
    DTransSafetyInfo &DTInfo = DTAnalysisWrapper.getDTransSafetyInfo(M);
    auto &WPInfo = getAnalysis<WholeProgramWrapperPass>().getResult();
    AOSToSOAOPPass::GetTLIFuncType GetTLI =
        [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    // This lambda function is to allow getting the DominatorTree analysis for a
    // specific function to allow analysis of loops when checking the dynamic
    // allocation of the structure type candidates of this transformation.
    AOSToSOAOPPass::DominatorTreeFuncType GetDT =
        [this](Function &F) -> DominatorTree & {
      return this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };

    bool Changed = Impl.runImpl(M, &DTInfo, WPInfo, GetTLI, GetDT);
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransSafetyAnalyzerWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};
} // end anonymous namespace

bool AOSToSOAOPTransformImpl::prepareTypes(Module &M) {
  unsigned PointerSizeInBits = DL.getPointerSizeInBits();
  qualifyDependentTypes(PointerSizeInBits);
  if (TypesToTransform.empty())
    return false;

  initializeIndexType(M.getContext(), IndexInfo.PointerShrinkingEnabled
                                          ? 32
                                          : PointerSizeInBits);

  for (auto Elem : enumerate(TypesToTransform)) {
    dtrans::StructInfo *StInfo = Elem.value();
    LLVM_DEBUG(dbgs() << "AOS-to-SOA: Original structure: "
                      << *StInfo->getLLVMType() << "\n");

    // Create the new types in the llvm and DTransOP type representations.
    StructType *OrigLLVMTy = cast<StructType>(StInfo->getLLVMType());
    StructType *NewLLVMTy = StructType::create(
        M.getContext(), (Twine("__SOA_" + OrigLLVMTy->getName()).str()));
    DTransStructType *DTransStructTy = TM.getStructType(OrigLLVMTy->getName());
    assert(DTransStructTy &&
           "Expected DTransStructType for original structure");
    DTransStructType *NewDTransStructTy = TM.getOrCreateStructType(NewLLVMTy);

    TypeRemapper.addTypeMapping(OrigLLVMTy, NewLLVMTy, DTransStructTy,
                                NewDTransStructTy);

    // When opaque pointers are not in use, we need to map pointers of structure
    // types being transformed into integer types.
    //   %struct.node* -> i32
    //
    // When opaque pointers are in use, the type remapper will not be able
    // distinguish %struct.node* from %struct.arc* because they will both be
    // represented by 'ptr'. Therefore we do not want to add a mapping of 'ptr'
    // to 'i32' to the map when using opaque pointers. Instead we will add a
    // mapping from a pointer in a different address space that will be used to
    // simulate %struct.node*, and during the routine that prepares IR to be
    // remapped we put the pointers that represent pointers to the type to be
    // transformed into that address space. (DTrans does not support code using
    // address spaces, so there will not be a conflict with any existing pointer
    // type.
    uint32_t AddrSpaceForType = UsingOpaquePtrs ? Elem.index() + 1 : 0;
    if (!UsingOpaquePtrs)
      TypeRemapper.addTypeMapping(
          OrigLLVMTy->getPointerTo(), getIndexLLVMType(),
          TM.getOrCreatePointerType(DTransStructTy), getIndexDTransType());
    else
      TypeRemapper.addTypeMapping(
          PointerType::get(OrigLLVMTy, AddrSpaceForType), getIndexLLVMType(),
          TM.getOrCreatePointerType(DTransStructTy), getIndexDTransType());

    SOATypeInfoTy Info;
    Info.AddrSpaceForType = AddrSpaceForType;
    Info.OrigStructType = OrigLLVMTy;
    Info.OrigDTransType = DTransStructTy;
    Info.SOAStructType = NewLLVMTy;
    Info.DTransSOAStructType = NewDTransStructTy;
    SOATypes.emplace_back(Info);
  }
  return true;
}

void AOSToSOAOPTransformImpl::populateTypes(Module &M) {
  for (auto *StInfo : TypesToTransform) {
    auto *OrigLLVMTy = cast<StructType>(StInfo->getLLVMType());
    struct SOATypeInfoTy &Info = getSOATypeInfo(OrigLLVMTy);
    llvm::StructType *NewLLVMTy = Info.SOAStructType;

    assert(NewLLVMTy && "Type mappings not initialized properly");

    DTransStructType *OrigDTransTy = Info.OrigDTransType;
    DTransStructType *NewDTransTy = Info.DTransSOAStructType;
    assert(OrigDTransTy && NewDTransTy &&
           "Type mappings not initialized properly");

    SmallVector<Type *, 16> LLVMDataTypes;
    SmallVector<DTransType *, 16> DTransDataTypes;
    size_t NumFields = StInfo->getNumFields();
    for (size_t Idx = 0; Idx < NumFields; ++Idx) {
      DTransType *DTransFieldTy = OrigDTransTy->getFieldType(Idx);
      assert(DTransFieldTy && "DTransStructType was not initialized properly");

      DTransType *DTransRemapType = TypeRemapper.remapType(DTransFieldTy);
      llvm::Type *LLVMRemapType = DTransRemapType->getLLVMType();
      Info.LLVMStructRemappedFieldsTypes.push_back(LLVMRemapType);
      LLVMDataTypes.push_back(LLVMRemapType->getPointerTo());
      DTransDataTypes.push_back(TM.getOrCreatePointerType(DTransRemapType));
    }

    NewLLVMTy->setBody(LLVMDataTypes, OrigLLVMTy->isPacked());
    NewDTransTy->setBody(DTransDataTypes);
    LLVM_DEBUG(dbgs() << "AOS-to-SOA: New structure body: " << *NewLLVMTy
                      << "\nDTrans structure body:" << *NewDTransTy << "\n");
  }
}

// Create a global variable for each of the transformed types, and variables
// needed for the ptr.annotation calls that will be inserted during the
// transformation.
void AOSToSOAOPTransformImpl::prepareModule(Module &M) {
  unsigned Count = 0;
  for (auto &SOAType : SOATypes) {
    StructType *StType = SOAType.OrigStructType;
    StructType *SOAVarTy = SOAType.SOAStructType;

    SOAType.SOAVar = new GlobalVariable(
        M, SOAVarTy, false, GlobalValue::InternalLinkage,
        /*init=*/ConstantAggregateZero::get(SOAVarTy),
        "__soa_" + StType->getName(),
        /*insertbefore=*/nullptr, GlobalValue::NotThreadLocal,
        /*AddressSpace=*/0, /*isExternallyInitialized=*/false);
    LLVM_DEBUG(dbgs() << "AOS-to-SOA: SOAVar: " << *SOAType.SOAVar
                      << "\n");

    std::string AllocationStr("{dtrans} AOS-to-SOA allocation");
    std::string IndexStr("{dtrans} AOS-to-SOA index");
    std::string ExtensionName = "";
    std::string CountStr(std::to_string(Count));

    // We normally only expect a single structure to be transformed,
    // so don't append a unique extension on the first set of variable
    // names.
    if (Count != 0)
      ExtensionName = CountStr;
    Count++;

    // Create the variables and a constant Value object that will be used in
    // calls to llvm.ptr.annotation intrinsics to mark the memory block
    // allocation and load/store of the index values that replace the pointers.
    // Separate strings will be created for each structure transformed, where
    // the 'id' element within the string can be used to pair the allocation
    // annotation with the index annotations.
    //
    // This will create a Value object of the form:
    // i8 *getelementptr inbounds([38 x i8],
    //       [38 x i8]* @__intel_dtrans_aostosoa_alloc, i32 0, i32 0)
    SOAType.AllocAnnotationGEP = DTransAnnotator::createConstantStringGEP(
        DTransAnnotator::getAnnotationVariable(
            M, DTransAnnotator::DPA_AOSToSOAAllocation,
            AllocationStr + " {id:" + CountStr + "}", ExtensionName),
        0);

    // Create the object for the index annotations:
    // i8* getelementptr inbounds ([33 x i8],
    //       [33 x i8]* @__intel_dtrans_aostosoa_index
    SOAType.IndexAnnotationGEP = DTransAnnotator::createConstantStringGEP(
        DTransAnnotator::getAnnotationVariable(
            M, DTransAnnotator::DPA_AOSToSOAIndex,
            IndexStr + " {id:" + CountStr + "}", ExtensionName),
        0);
  }

  // Create an empty string for the filename operand of the llvm.ptr.annotation
  // call. This is a generic string, and not specific to the type being
  // transformed.
  AnnotationFilenameGEP = DTransAnnotator::createConstantStringGEP(
      DTransAnnotator::createGlobalVariableString(
          M, "__intel_dtrans_aostosoa_filename", ""),
      0);
}

void AOSToSOAOPTransformImpl::processFunction(Function &F) {
  // TODO: Transform the statements the need to be done prior to type remapping.
}

void AOSToSOAOPTransformImpl::postprocessFunction(Function &OrigFunc,
                                                  bool isCloned) {
  // TODO: Cleanup statements inserted during processFunction that are no longer
  // necessary after type remapping.
}

struct SOATypeInfoTy &
AOSToSOAOPTransformImpl::getSOATypeInfo(llvm::StructType *Ty) {
  for (auto &SOAType : SOATypes)
    if (SOAType.OrigStructType == Ty)
      return SOAType;

  llvm_unreachable("Request for type not being transformed");
}

void AOSToSOAOPTransformImpl::qualifyDependentTypes(
    unsigned PointerSizeInBits) {
  // TODO: Check whether pointer shrinking is possible based on the safety of
  // the dependent types, and set 'IndexInfo.PointerShrinkingEnabled' to 'true',
  // if so. Also, populate the list of dependent structure types into
  // 'DepTypesToTransform' for the types that will need to have their size
  // changed.
}

// Initialize class fields that are dependent on the size of the SOA index type.
void AOSToSOAOPTransformImpl::initializeIndexType(LLVMContext &Ctx,
                                                  unsigned BitWidth) {
  assert(BitWidth == 32 || BitWidth == 64 && "Invalid index size");

  IndexInfo.Width = BitWidth;
  IndexInfo.LLVMType = Type::getIntNTy(Ctx, BitWidth);
  IndexInfo.DTType = TM.getOrCreateAtomicType(IndexInfo.LLVMType);
  IndexInfo.IncompatibleTypeAttrs =
      AttributeFuncs::typeIncompatible(IndexInfo.LLVMType);
}

// Return an integer type that will be used as a replacement type for pointers
// to the types being transformed.
llvm::Type *AOSToSOAOPTransformImpl::getIndexLLVMType() const {
  assert(IndexInfo.LLVMType && IndexInfo.Width && "Index type/width not set");
  return IndexInfo.LLVMType;
}

// Return an integer type that will be used as a replacement type for pointers
// to the types being transformed in the DTransType system.
DTransType *AOSToSOAOPTransformImpl::getIndexDTransType() const {
  assert(IndexInfo.DTType && IndexInfo.Width && "Index type/width not set");
  return IndexInfo.DTType;
}

char DTransAOSToSOAOPWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransAOSToSOAOPWrapper, "dtrans-aostosoaop",
                      "DTrans array of structures to structure of arrays with "
                      "opaque pointer support",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(DTransSafetyAnalyzerWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransAOSToSOAOPWrapper, "dtrans-aostosoaop",
                    "DTrans array of structures to structure of arrays with "
                    "opaque pointer support",
                    false, false)

ModulePass *llvm::createDTransAOSToSOAOPWrapperPass() {
  return new DTransAOSToSOAOPWrapper();
}

namespace llvm {
namespace dtransOP {
bool AOSToSOAOPPass::runImpl(Module &M, DTransSafetyInfo *DTInfo,
                             WholeProgramInfo &WPInfo,
                             AOSToSOAOPPass::GetTLIFuncType &GetTLI,
                             AOSToSOAOPPass::DominatorTreeFuncType &GetDT) {
  LLVM_DEBUG(dbgs() << "Running AOS-to-SOA for opaque pointers pass\n");
  if (!WPInfo.isWholeProgramSafe()) {
    LLVM_DEBUG(dbgs() << "  Not whole program safe\n");
    return false;
  }

  if (!DTInfo->useDTransSafetyAnalysis()) {
    LLVM_DEBUG(dbgs() << "  DTransSafetyAnalyzer results not available\n");
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

  AOSToSOAOPTransformImpl Transformer(M.getContext(), DTInfo, "__SOADT_",
                                      M.getDataLayout(), GetTLI,
                                      CandidateTypes);
  return Transformer.run(M);
}

// Populate the 'CandidateTypes' vector with all the structure types
// that should be considered for transformation. These are structure
// that passed the DTrans safety bit flags, but need to be checked for
// additional criteria with the qualifyCandidates function.
void AOSToSOAOPPass::gatherCandidateTypes(DTransSafetyInfo *DTInfo,
                                          StructInfoVecImpl &CandidateTypes) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Build the list of structures to convert based on a command line option
  // containing a list of structure names separated by commas.
  DTransTypeManager &TM = DTInfo->getTypeManager();
  if (!AOSToSOAOPTypelist.empty()) {
    SmallVector<StringRef, 16> SubStrings;
    SplitString(AOSToSOAOPTypelist, SubStrings, ",");
    for (auto &TypeName : SubStrings) {
      DTransStructType *DTransTy = TM.getStructType(TypeName);
      if (!DTransTy) {
        LLVM_DEBUG(dbgs() << "No structure found for: " << TypeName << "\n");
        continue;
      }
      dtrans::TypeInfo *TI = DTInfo->getTypeInfo(DTransTy);
      if (!TI) {
        LLVM_DEBUG(dbgs() << "No type info found for: " << TypeName << "\n");
        continue;
      }
      dtrans::StructInfo *SI = cast<dtrans::StructInfo>(TI);
      CandidateTypes.push_back(SI);
    }
    return;
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // TODO: Do candidate selection based on safety checks.
}

// This function filters the 'CandidateTypes' list to remove types that are
// not supported for transformation based on the contents of the structure or
// the use of the structure.
void AOSToSOAOPPass::qualifyCandidates(StructInfoVecImpl &CandidateTypes,
                                       Module &M, DTransSafetyInfo *DTInfo,
                                       DominatorTreeFuncType &GetDT) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Bypass the checks, and to allow testing on IR that does not satisfy all the
  // preconditions needed for the allocation of an array of structures.
  if (DTransAOSToSOAOPQualOverride)
    return;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // TODO: Verify the allocation of the candidate structures is supported.
}

PreservedAnalyses AOSToSOAOPPass::run(Module &M, ModuleAnalysisManager &AM) {
  DTransSafetyInfo *DTInfo = &AM.getResult<DTransSafetyAnalyzer>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  GetTLIFuncType GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  DominatorTreeFuncType GetDT = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };

  bool Changed = runImpl(M, DTInfo, WPInfo, GetTLI, GetDT);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.abandon<DTransSafetyAnalyzer>();
  return PA;
}

} // end namespace dtransOP
} // end namespace llvm
