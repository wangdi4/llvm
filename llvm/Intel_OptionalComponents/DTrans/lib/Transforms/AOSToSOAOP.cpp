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
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOPOptBase.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/InitializePasses.h"

using namespace llvm;
using namespace dtransOP;
using dtrans::DTransAnnotator;
using dtrans::StructInfo;

#define DEBUG_TYPE "dtrans-aostosoaop"
#define AOSTOSOA_VERBOSE "dtrans-aostosoaop-verbose"

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
// Forward references
class AOSToSOAOPTransformImpl;

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

  llvm::Type *getTransformedFieldType(uint32_t FieldIdx) {
    return LLVMStructRemappedFieldsTypes[FieldIdx];
  }
};

// This structure holds information about the instructions that will need to be
// processed during the transformation. A visitor class will identify
// instructions, and then the processFunction and postProcessFunction methods of
// the transformation will use this information to update the IR.
struct PerFunctionInfo {
  using ByteGEPInfo =
      std::tuple<GetElementPtrInst *, DTransStructType *, size_t>;
  // Identifiers for extracting elements from the ByteGEPInfo tuple.
  static const int ByteGEPInfoGEPMember = 0;
  static const int ByteGEPInfoTypeMember = 1;
  static const int ByteGEPInfoFieldMember = 2;

  // GetElementPtr instructions that need to be converted because of a structure
  // type being transformed.
  SmallVector<GetElementPtrInst *, 16> GEPsToConvert;
  SmallVector<ByteGEPInfo, 16> ByteGEPsToConvert;

  // GetElementPtr instructions that access fields that are of the type being
  // transformed from a dependent type.
  SmallVector<GetElementPtrInst *, 16> DepGEPsToConvert;

  // GetElementPtr instructions using the byte flattened form to access a
  // dependent type.
  SmallVector<ByteGEPInfo, 16> DepByteGEPsToConvert;

  // A list of pointer conversion instructions inserted as part of the
  // processFunction routine (pointer to int, int to pointer, or pointer of
  // original type to a pointer of the remapped type) that after the type
  // remapping process should have the same source and destination types, and
  // must be removed during postProcessFunction routine.
  SmallVector<CastInst *, 16> PtrConverts;

  // Instructions from the input IR that need to be removed at the end of
  // processFunction().
  SmallVector<Instruction *, 16> InstructionsToDelete;

  // Instructions that need to have their type changed prior to the type
  // remapping of the IR. In order to recognize the pointers that are going to
  // be converted to integer index values by the ValueMapper class, we are going
  // to put those pointers into a specific address space. When the ValueMapper
  // passes a llvm::Type object to our TypeMapper object, the pointer in the
  // specific address space will be recognized as needing to be converted.
  SmallVector<std::pair<Instruction *, Type *>, 32> InstructionsToMutate;

  // Set of constant null value pointers that need to have their type changed.
  // In this case, we need to store the instruction and the operand number
  // because with opaque pointers all 'ptr null' objects in the function will
  // refer to the same Value object.
  SmallVector<std::tuple<Instruction *, uint32_t, llvm::PointerType *>, 4>
      ConstantsToReplace;

  // List of instructions to be annotated as being pointers to the index value
  // for subsequent DTrans passes. The structure type field stores the original
  // type of structure, not the type being created by this transformation.
  SmallVector<std::pair<Instruction *, llvm::StructType *>, 16>
      InstructionsToAnnotate;
};

// This visitor will collect the instructions that need to be converted.
class AOSCollector : public InstVisitor<AOSCollector> {
public:
  AOSCollector(AOSToSOAOPTransformImpl &Transform, DTransSafetyInfo &DTInfo,
               PerFunctionInfo &FuncInfo)
      : Transform(Transform), DTInfo(DTInfo), PTA(DTInfo.getPtrTypeAnalyzer()),
        FuncInfo(FuncInfo) {}

  void visitGetElementPtrInst(GetElementPtrInst &GEP);
  void visitLoadInst(LoadInst &I);
  void visitStoreInst(StoreInst &I);
  // TODO: visit other instruction types

private:
  AOSToSOAOPTransformImpl &Transform;
  DTransSafetyInfo &DTInfo;
  PtrTypeAnalyzer &PTA;
  PerFunctionInfo &FuncInfo;
};

// This class is responsible for all the transformation work for the AOS to SOA
// with Indexing conversion.
class AOSToSOAOPTransformImpl : public DTransOPOptBase {
public:
  AOSToSOAOPTransformImpl(LLVMContext &Ctx, DTransSafetyInfo *DTInfo,
                          bool UsingOpaquePtrs, StringRef DepTypePrefix,
                          const DataLayout &DL,
                          AOSToSOAOPPass::GetTLIFuncType GetTLI,
                          SmallVectorImpl<dtrans::StructInfo *> &Types)
      : DTransOPOptBase(Ctx, DTInfo, UsingOpaquePtrs, DepTypePrefix), DL(DL),
        GetTLI(GetTLI), Materializer(*getTypeRemapper()) {
    std::copy(Types.begin(), Types.end(), std::back_inserter(TypesToTransform));
    PtrSizeIntLLVMType = Type::getIntNTy(Ctx, DL.getPointerSizeInBits());
  }

  ValueMaterializer *getMaterializer() override { return &Materializer; }

  bool prepareTypes(Module &M) override;
  void populateTypes(Module &M) override;

  void prepareModule(Module &M) override;
  void processFunction(Function &F) override;
  void postprocessFunction(Function &OrigFunc, bool isCloned) override;

  bool isTypeToTransform(llvm::Type *Ty) const;
  bool isDependentType(llvm::Type *Ty) const;
  bool isDependentTypeSizeChanged(llvm::Type *Ty) const;
  StructType *getDependentTypeReplacement(llvm::StructType *OrigTy);
  DTransStructType *getDependentDTransType(llvm::StructType *OrigTy);

  PointerType *getAddrSpacePtrForType(llvm::StructType *OrigStructTy);
  PointerType *getAddrSpacePtrForType(DTransStructType *OrigStructTy);

  struct SOATypeInfoTy &getSOATypeInfo(llvm::StructType *Ty);

private: // methods
  llvm::Type *getPtrSizedIntLLVMType() const { return PtrSizeIntLLVMType; }
  void qualifyDependentTypes(unsigned PointerSizeInBits);
  void initializeIndexType(LLVMContext &Ctx, unsigned BitWidth);
  llvm::Type *getIndexLLVMType() const;
  DTransType *getIndexDTransType() const;

  uint32_t getAddrSpaceForType(llvm::StructType *OrigStructTy);
  uint32_t getAddrSpaceForType(DTransStructType *OrigStructTy);

  void convertGEP(GetElementPtrInst *GEP);
  void convertByteGEP(GetElementPtrInst *GEP, DTransStructType *OrigStructTy,
                      size_t FieldNum);
  void convertDepGEP(GetElementPtrInst *GEP);
  void convertDepByteGEP(GetElementPtrInst *GEP, DTransStructType *OrigStructTy,
                         size_t FieldNum);

  CastInst *createCastToIndexType(Value *V,
                                  Instruction *InsertBefore = nullptr);

  Value *promoteOrTruncValueToWidth(Value *V, uint64_t DstWidth,
                                    Instruction *InsertBefore);

  GetElementPtrInst *
  createGEPFieldAddressReplacement(SOATypeInfoTy &SOAType, Value *IndexAsIntTy,
                                   Value *GEPBaseIdx, Value *GEPFieldNum,
                                   Instruction *InsertBefore);

  LoadInst *createSOAFieldLoad(SOATypeInfoTy &SOAType, Value *FieldNumVal,
                               Instruction *InsertBefore);

private: // data
  // Class to handle changing the type of a 'null' Value object during this
  // transformation. This is needed for creating the integer 0 that will be used
  // as the index on the transformed type to represent 'null' pointer.
  //
  // The materialize() method of this class is invoked by the ValueMapper to
  // check whether a value needs to be converted.
  class AOSToSOAMaterializer : public ValueMaterializer {
  public:
    AOSToSOAMaterializer(ValueMapTypeRemapper &TypeRemapper)
        : TypeRemapper(TypeRemapper) {}
    virtual ~AOSToSOAMaterializer() {}

    virtual Value *materialize(Value *V) override {
      // Check if a null value of a different type needs to be generated.
      // TODO: For generality, this should also be extended to handle 'undef'
      // values.
      auto *C = dyn_cast<Constant>(V);
      if (!C)
        return nullptr;

      if (!C->isNullValue())
        return nullptr;

      // Use the TypeRemapper to check whether the pointer type is being
      // changed. The change could be something like %struct.t* ->
      // %__SOADT_struct.t* when using typed-pointers, or it could be for a
      // pointer to the type being transformed that is going to be converted
      // into an integer index.
      Type *Ty = V->getType();
      Type *ReTy = TypeRemapper.remapType(Ty);
      if (Ty == ReTy)
        return nullptr;

      return Constant::getNullValue(ReTy);
    }

  private:
    ValueMapTypeRemapper &TypeRemapper;
  };

  const DataLayout &DL;
  AOSToSOAOPPass::GetTLIFuncType GetTLI;
  AOSToSOAMaterializer Materializer;

  // The list of types to be transformed.
  SmallVector<dtrans::StructInfo *, 4> TypesToTransform;

  // Information about the structures being transformed.
  SmallVector<struct SOATypeInfoTy, 2> SOATypes;

  // A vector of original structure type/corresponding DTrans structure type
  // pairs for types that need to change as a result of having a pointer to a
  // type being transformed. When pointer shrinking is enabled, the size of
  // these structures and byte offsets of fields will be modified.
  SmallVector<std::pair<llvm::StructType *, DTransStructType *>, 4>
      DepTypesToTransform;

  llvm::Type *PtrSizeIntLLVMType = nullptr;

  // Value to use for the 3rd string argument of the ptr.annotation intrinsic.
  // This argument is used to represent a filename, and will be ignored.
  Value *AnnotationFilenameGEP = nullptr;

  // Information about the index type that will replace the pointer to the
  // structure being transformed.
  struct SOAIndexInfoTy IndexInfo;

  // This object will be used to share information between the processFunction
  // and postProcessFunction about instructions that need to be processed.
  std::unique_ptr<PerFunctionInfo> FuncInfo;
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

void AOSCollector::visitGetElementPtrInst(GetElementPtrInst &GEP) {
  // The only cases that need to be converted are GEPs with 1 or 2 indices
  // because the type being transformed cannot contain any aggregate types, and
  // it cannot be nested within another type.
  unsigned NumIndices = GEP.getNumIndices();
  if (NumIndices > 2)
    return;

  Type *ElementTy = GEP.getSourceElementType();
  if (Transform.isTypeToTransform(ElementTy)) {
    FuncInfo.GEPsToConvert.push_back(&GEP);
    return;
  }

  // Check for byte-flattened GEP
  auto InfoPair = DTInfo.getByteFlattenedGEPElement(&GEP);
  if (InfoPair.first) {
    llvm::Type *Ty = InfoPair.first->getLLVMType();
    if (Transform.isTypeToTransform(Ty))
      FuncInfo.ByteGEPsToConvert.push_back(std::make_tuple(
          &GEP, cast<DTransStructType>(InfoPair.first), InfoPair.second));
    else if (Transform.isDependentTypeSizeChanged(Ty))
      FuncInfo.DepByteGEPsToConvert.push_back(std::make_tuple(
          &GEP, cast<DTransStructType>(InfoPair.first), InfoPair.second));
    return;
  }

  // Check for a GEP on a dependent type that is to a field which is the type
  // being transformed. In this case, the GEP ResultElementType member will
  // need to be modified for the case where it is an opaque pointer type.
  if (NumIndices == 2 && Transform.isDependentType(ElementTy)) {
    Value *FieldNum = GEP.getOperand(2);
    auto *FieldNumConst = dyn_cast<ConstantInt>(FieldNum);
    if (!FieldNumConst)
      return;

    uint32_t FieldIdx = FieldNumConst->getLimitedValue();
    auto *OrigStructTy = cast<llvm::StructType>(ElementTy);
    llvm::Type *OrigFieldType = OrigStructTy->getElementType(FieldIdx);
    if (!OrigFieldType->isPointerTy())
      return;

    if (!OrigFieldType->isOpaquePointerTy())
      return;

    llvm::StructType *NewStructTy =
        Transform.getDependentTypeReplacement(OrigStructTy);
    llvm::Type *NewFieldType = NewStructTy->getElementType(FieldIdx);
    if (NewFieldType->isIntegerTy())
      FuncInfo.DepGEPsToConvert.push_back(&GEP);
  }
}

void AOSCollector::visitLoadInst(LoadInst &I) {
  // TODO: Extend this to allow integers that represent pointers to be handled:
  //   %y = bitcast %struct.node* %x to i64*
  //   load i64, i64* %y
  // For now, just look at loading a pointer type.
  if (!I.getType()->isPointerTy())
    return;

  auto *Info = PTA.getValueTypeInfo(&I);
  assert(Info && "Expected PointerTypeAnalyzer to collect type");
  DTransType *Ty = PTA.getDominantAggregateUsageType(*Info);
  if (!Ty || !Ty->isPointerTy())
    return;

  auto *StructTy = dyn_cast<DTransStructType>(Ty->getPointerElementType());
  if (!StructTy)
    return;

  auto *LLVMStructType = cast<llvm::StructType>(StructTy->getLLVMType());
  if (!Transform.isTypeToTransform(LLVMStructType))
    return;

  FuncInfo.InstructionsToAnnotate.push_back({&I, LLVMStructType});
  if (!I.getType()->isOpaquePointerTy())
    return;

  // If the type loaded is a pointer to the transformed type, we want to
  // change it to appear in an address space so that the type remapping will
  // convert it to the index type.
  PointerType *AddrSpacePtr = Transform.getAddrSpacePtrForType(StructTy);
  assert(AddrSpacePtr &&
         "Opaque pointer being transformed must have addr space");
  FuncInfo.InstructionsToMutate.push_back({&I, AddrSpacePtr});
}

void AOSCollector::visitStoreInst(StoreInst &I) {
  // TODO: Extend this to allow integers that represent pointers to be handled:
  //   %x = ptrtoint %struct.node* to i64
  //   store i64 %x, i64* %y
  // For now, just look at storing a pointer type.
  Value *Val = I.getValueOperand();
  if (!Val->getType()->isPointerTy())
    return;

  // Get the type info for the value operand
  auto *Info = PTA.getValueTypeInfo(&I, 0);
  assert(Info && "Expected PointerTypeAnalyzer to collect type");
  DTransType *Ty = PTA.getDominantAggregateUsageType(*Info);
  if (!Ty || !Ty->isPointerTy())
    return;

  auto *StructTy = dyn_cast<DTransStructType>(Ty->getPointerElementType());
  if (!StructTy)
    return;

  auto *LLVMStructType = cast<llvm::StructType>(StructTy->getLLVMType());
  if (!Transform.isTypeToTransform(LLVMStructType))
    return;

  FuncInfo.InstructionsToAnnotate.push_back({&I, LLVMStructType});
  if (!Val->getType()->isOpaquePointerTy())
    return;

  if (!isa<ConstantData>((Val)))
    return;

  // When a 'null' value is being stored for the type being transformed, it
  // will need to be converted into storing the integer 0.
  //   store ptr null, ptr %struct.t.ptr
  // Modify this to be:
  //   store ptr addrspace(1) 0, ptr %struct.t.ptr
  //
  // We only need to change the 'null' object of the store, because it is a
  // compiler constant. Non-constant values will be handled when the
  // definition of the value being stored is processed. Also, note we need to
  // change the value stored, not just change it's type, because all 'null'
  // values within the routine will use the same Value object for 'ptr null'
  // After type remapping completes, the value 0 will be stored in the memory
  // location of the index that %struct.t.ptr will correspond to because the
  // materialize class will transform it.
  if (isa<ConstantPointerNull>(Val)) {
    PointerType *TypeInAddrSpace = Transform.getAddrSpacePtrForType(StructTy);
    assert(TypeInAddrSpace && "Expected type when opaque pointers are in use");
    FuncInfo.ConstantsToReplace.push_back({&I, 0, TypeInAddrSpace});
  } else {
    llvm_unreachable("Unhandled constant type");
  }
}

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
    LLVM_DEBUG(dbgs() << "AOS-to-SOA: SOAVar: " << *SOAType.SOAVar << "\n");

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
  // Lambda to clear the FuncInfo state that is no longer used after this
  // function completes.
  auto FuncInfoProcessFunctionComplete = [this]() {
    FuncInfo->GEPsToConvert.clear();
    FuncInfo->ByteGEPsToConvert.clear();
    FuncInfo->DepGEPsToConvert.clear();
    FuncInfo->DepByteGEPsToConvert.clear();
    FuncInfo->InstructionsToDelete.clear();
    FuncInfo->InstructionsToMutate.clear();
    FuncInfo->ConstantsToReplace.clear();
  };

  LLVM_DEBUG(dbgs() << "AOS-to-SOA: ProcessFunction: " << F.getName() << "\n");

  // The PerFunctionInfo data will be needed by this function and the
  // postProcessFunction.
  FuncInfo = std::make_unique<PerFunctionInfo>();
  AOSCollector Collector(*this, *DTInfo, *FuncInfo);
  Collector.visit(F);

  for (auto *GEP : FuncInfo->GEPsToConvert)
    convertGEP(GEP);
  for (auto &GEPInfo : FuncInfo->ByteGEPsToConvert)
    convertByteGEP(std::get<PerFunctionInfo::ByteGEPInfoGEPMember>(GEPInfo),
                   std::get<PerFunctionInfo::ByteGEPInfoTypeMember>(GEPInfo),
                   std::get<PerFunctionInfo::ByteGEPInfoFieldMember>(GEPInfo));

  // Process the instructions using dependent structure types.
  for (auto *GEP : FuncInfo->DepGEPsToConvert)
    convertDepGEP(GEP);

  for (auto &GEPInfo : FuncInfo->DepByteGEPsToConvert)
    convertDepByteGEP(
        std::get<PerFunctionInfo::ByteGEPInfoGEPMember>(GEPInfo),
        std::get<PerFunctionInfo::ByteGEPInfoTypeMember>(GEPInfo),
        std::get<PerFunctionInfo::ByteGEPInfoFieldMember>(GEPInfo));

  // Convert 'ptr' objects to be 'ptr addrspace(n)' objects so the type
  // remapping will recognize them.
  for (auto KV : FuncInfo->InstructionsToMutate)
    if (auto *Call = dyn_cast<CallInst>(KV.first))
      Call->mutateFunctionType(cast<FunctionType>(KV.second));
    else
      KV.first->mutateType(KV.second);

  for (auto &IOT : FuncInfo->ConstantsToReplace) {
    Instruction *I = std::get<0>(IOT);
    uint32_t OpNum = std::get<1>(IOT);
    llvm::PointerType *Ty = std::get<2>(IOT);
    I->setOperand(OpNum, ConstantPointerNull::get(Ty));
  }

  for (auto *I : FuncInfo->InstructionsToDelete)
    I->eraseFromParent();

  FuncInfoProcessFunctionComplete();
  DEBUG_WITH_TYPE(AOSTOSOA_VERBOSE,
                  dbgs() << "\nIR Before type-remapping:\n"
                         << F
                         << "---------------------------------------------\n");
}

void AOSToSOAOPTransformImpl::postprocessFunction(Function &OrigFunc,
                                                  bool IsCloned) {
  LLVM_DEBUG(dbgs() << "AOS-to-SOA: postprocessFunction: " << OrigFunc.getName()
                    << "\n");

  Function *Func = &OrigFunc;
  if (IsCloned)
    Func = cast<Function>(VMap[&OrigFunc]);

  DEBUG_WITH_TYPE(AOSTOSOA_VERBOSE,
                  dbgs() << "\nIR after type-remapping:\n"
                         << *Func
                         << "---------------------------------------------\n");

  // We need to get rid of all the cast instructions that were inserted to help
  // the type remapping because they are not valid after the type remapping
  // changed a pointer type to be an integer type.
  SmallVector<Instruction *, 16> InstructionsToDelete;
  for (auto *Conv : FuncInfo->PtrConverts) {
    if (IsCloned)
      Conv = cast<CastInst>(VMap[Conv]);
    if (Conv->user_empty()) {
      InstructionsToDelete.push_back(Conv);
      continue;
    }

    LLVM_DEBUG(dbgs() << "Post process deleting: " << *Conv << "\n");
    assert(Conv->getType() == Conv->getOperand(0)->getType() &&
           "Expected self-type in cast after remap");
    Conv->replaceAllUsesWith(Conv->getOperand(0));
    Conv->eraseFromParent();
  }

  // Insert the annotations that help the dynamic cloning recognize the new
  // "array index" variables.
  for (auto &InstTyPair : FuncInfo->InstructionsToAnnotate) {
    auto *I = InstTyPair.first;
    auto *Ty = InstTyPair.second;
    if (IsCloned)
      I = cast<Instruction>(VMap[I]);

    Value *Ptr = nullptr;
    if (auto *SI = dyn_cast<StoreInst>(I))
      Ptr = SI->getPointerOperand();
    else if (auto *LI = dyn_cast<LoadInst>(I))
      Ptr = LI->getPointerOperand();
    else
      llvm_unreachable("Instruction expected to be load/store");

    Module *M = I->getModule();
    SOATypeInfoTy &SOAType = getSOATypeInfo(Ty);
    Value *Annot = DTransAnnotator::createPtrAnnotation(
        *M, *Ptr, *SOAType.IndexAnnotationGEP, *AnnotationFilenameGEP, 0,
        "alloc_idx", I);
    (void)Annot;
    LLVM_DEBUG(dbgs() << "Adding annotation for pointer: " << *Ptr
                      << "\n  in: " << *I << "\n  " << *Annot << "\n");
  }

  for (auto *I : InstructionsToDelete)
    I->eraseFromParent();

  FuncInfo.reset();
  DEBUG_WITH_TYPE(AOSTOSOA_VERBOSE,
                  dbgs() << "\nAOS-to-SOA: Final IR:\n"
                         << *Func
                         << "---------------------------------------------\n");
}

bool AOSToSOAOPTransformImpl::isTypeToTransform(llvm::Type *Ty) const {
  if (!Ty->isStructTy())
    return false;

  for (auto &SOAType : SOATypes)
    if (SOAType.OrigStructType == Ty)
      return true;

  return false;
}

bool AOSToSOAOPTransformImpl::isDependentType(llvm::Type *Ty) const {
  if (!Ty->isStructTy())
    return false;

  for (auto &P : DepTypesToTransform)
    if (P.first == Ty)
      return true;

  return false;
}

bool AOSToSOAOPTransformImpl::isDependentTypeSizeChanged(llvm::Type *Ty) const {
  // Dependent types only need size adjustments when the pointer is 64-bits wide
  // and the index type is 32-bits wide.
  if (!IndexInfo.PointerShrinkingEnabled)
    return false;

  return isDependentType(Ty);
}

// Get the replacement type of a dependent structure type
StructType *
AOSToSOAOPTransformImpl::getDependentTypeReplacement(llvm::StructType *OrigTy) {
  return cast<llvm::StructType>(TypeRemapper.remapType(OrigTy));
}

// Get the DTransType that corresponds to the llvm::Type that was identified as
// a dependent type.
DTransStructType *
AOSToSOAOPTransformImpl::getDependentDTransType(llvm::StructType *OrigTy) {
  for (auto &P : DepTypesToTransform)
    if (P.first == OrigTy)
      return P.second;
  return nullptr;
}

// Get an opaque pointer in the address space that will be used for
// transformations on 'OrigStructTy'. If 'OrigStructTy' is not one of the
// structure types being transformed into an index by this transformation,
// returns nullptr.
PointerType *AOSToSOAOPTransformImpl::getAddrSpacePtrForType(
    llvm::StructType *OrigStructTy) {
  uint32_t AddrSpace = getAddrSpaceForType(OrigStructTy);
  if (!AddrSpace)
    return nullptr;
  return PointerType::get(OrigStructTy->getContext(), AddrSpace);
}

// Overload of getAddrSpacePtrForType that uses the DTransStructType as the
// lookup key.
PointerType *AOSToSOAOPTransformImpl::getAddrSpacePtrForType(
    DTransStructType *OrigStructTy) {
  uint32_t AddrSpace = getAddrSpaceForType(OrigStructTy);
  if (!AddrSpace)
    return nullptr;
  return PointerType::get(OrigStructTy->getContext(), AddrSpace);
}

struct SOATypeInfoTy &
AOSToSOAOPTransformImpl::getSOATypeInfo(llvm::StructType *Ty) {
  for (auto &SOAType : SOATypes)
    if (SOAType.OrigStructType == Ty)
      return SOAType;

  llvm_unreachable("Request for type not being transformed");
}

// Collect and qualify the safety of dependent types that are affected by the
// type being transformed. This will update the 'PointerShrinkingEnabled' member
// based on whether the dependent types qualify for allowing shrinking the index
// to use a 32-bit value. This requires all dependent types to be safe,
// otherwise the pointer shrinking will be inhibited.
void AOSToSOAOPTransformImpl::qualifyDependentTypes(
    unsigned PointerSizeInBits) {
  // Return 'true' if there are no safety issues with a dependent type 'Ty'
  // that prevent transforming a type.
  auto *TheDTInfo = DTInfo;
  auto IsDependentTypeSafe = [&TheDTInfo](DTransStructType *Ty) {
    auto *TI = TheDTInfo->getTypeInfo(Ty);
    assert(TI && "Expected DTrans to analyze container of dependent type");
    if (TheDTInfo->testSafetyData(TI, dtrans::DT_AOSToSOADependent))
      return false;

    // We know there is no direct address taken for any fields for the type
    // being transformed because that structure was not marked as "Address
    // Taken". However, if the analysis is assuming the address of one field
    // can be used to access another field, then any time the field address
    // taken is set for the dependent type, it will be unknown whether it
    // affects a field member that is a pointer to a type being transformed,
    // so check whether the code is allowing memory outside the boundaries of
    // a specific field to be accessed when taking the address.
    if (TheDTInfo->getDTransOutOfBoundsOK() &&
        TI->testSafetyData(dtrans::AnyFieldAddressTaken))
      return false;

    // No issues were found on this dependent type that prevent the AOS to SOA
    // transformation.
    return true;
  };

  auto IsDependentTypeSafeForShrinking = [&TheDTInfo](DTransStructType *Ty) {
    auto *TI = TheDTInfo->getTypeInfo(Ty);
    assert(TI && "Expected DTrans to analyze container of dependent type");
    if (TheDTInfo->testSafetyData(TI, dtrans::DT_AOSToSOADependentIndex32))
      return false;

    // TODO: The transformation does not support rewriting byte flattened GEPs
    // that are performed against global variables using a GEPOperator. To
    // support this the base class would need to inform the derived classes when
    // global variables of dependent types are replaced, and we should check
    // whether there are any byte-flattened GEPs of global variables of the
    // dependent type.

    return true;
  };

  bool DoPointerShrinking = DTransAOSToSOAOPIndex32 && PointerSizeInBits == 64;

  // Check whether the dependent types are safe, and whether they will support
  // pointer shrinking.
  SmallVector<dtrans::StructInfo *, 4> Qualified;
  for (auto *StInfo : TypesToTransform) {
    bool DepQualified = true;
    SmallVector<std::pair<llvm::StructType *, DTransStructType *>, 4>
        LocalDepTypesToTransform;
    auto *OrigTy = cast<DTransStructType>(StInfo->getDTransType());
    for (auto &DepTy : TypeToPtrDependentTypes[OrigTy]) {
      auto *DepStructTy = dyn_cast<DTransStructType>(DepTy);
      if (!DepStructTy)
        continue;

      // Don't check dependent types that are directly being transformed by
      // AOS-to-SOA.
      if (std::find(TypesToTransform.begin(), TypesToTransform.end(),
                    DTInfo->getTypeInfo(DepTy)) != TypesToTransform.end())
        continue;

      // Verify whether it is going to be safe to change a field member that is
      // a pointer to type being transformed into an integer type.
      if (!IsDependentTypeSafe(DepStructTy)) {
        LLVM_DEBUG(dbgs() << "AOS-to-SOA disqualifying type: " << *OrigTy
                          << " based on safety conditions of dependent type: "
                          << *DepTy << "\n");
        DepQualified = false;
        break;
      }

      // Verify whether it is going to be safe to use a 32-bit integer type
      // within the dependent type. Changing a 64-bit pointer into a 32-bit
      // integer means that the size of dependent structure will change,
      // requiring either that this transformation can update IR that is
      // dependent on the structure size (allocations, memory intrinsic calls,
      // pointer arithmetic, and byte flattened GEP uses) or that the dependent
      // type does not have IR for those usages.
      if (DoPointerShrinking && !IsDependentTypeSafeForShrinking(DepStructTy)) {
        LLVM_DEBUG(dbgs() << "AOS-to-SOA index shrinking "
                             "inhibited due to safety checks on: "
                          << *DepTy << "\n");
        DoPointerShrinking = false;
        continue;
      }

      LocalDepTypesToTransform.push_back(
          {cast<llvm::StructType>(DepStructTy->getLLVMType()), DepStructTy});

      LLVM_DEBUG(
          dbgs() << "AOS-to-SOA transforming type    : " << *OrigTy << "\n"
                 << "will also affect type           : " << *DepTy << "\n");
    }

    if (DepQualified) {
      Qualified.push_back(StInfo);
      for (auto &KV : LocalDepTypesToTransform)
        DepTypesToTransform.push_back(KV);
    }
  }

  IndexInfo.PointerShrinkingEnabled = DoPointerShrinking;
  TypesToTransform = std::move(Qualified);
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

// Get the address space to use for pointers to a structure of the specified
// StructType when preparing for type remapping. OrigStructTy needs to be one of
// the original llvm types, not one being produced by this transformation.
uint32_t
AOSToSOAOPTransformImpl::getAddrSpaceForType(llvm::StructType *OrigStructTy) {
  for (auto &SOAType : SOATypes)
    if (SOAType.OrigStructType == OrigStructTy)
      return SOAType.AddrSpaceForType;

  return 0;
}

// Overload of getAddrSpaceForType that uses the DTransStructType as the lookup
// key.
uint32_t
AOSToSOAOPTransformImpl::getAddrSpaceForType(DTransStructType *OrigStructTy) {
  for (auto &SOAType : SOATypes)
    if (SOAType.OrigDTransType == OrigStructTy)
      return SOAType.AddrSpaceForType;

  return 0;
}

void AOSToSOAOPTransformImpl::convertGEP(GetElementPtrInst *GEP) {
  LLVM_DEBUG(dbgs() << "Replacing GEP: " << *GEP << "\n");

  // There are 2 cases that need to be converted by this routine.
  // 1: Getting the address of a structure relative to a pointer to the
  // structure type.
  //    %addr1 = getelementptr %struct.t, ptr %base, i64 %n
  //
  // 2: Getting the address of a structure field.
  //    %addr2 = getelementptr %struct.t, ptr %base, i64 %n, i32 1
  //
  // These need to be processed prior to the type remapping process because
  // the type remapping will change field members within the dependent structure
  // to be an integer type, so we need to make sure that when value mapping and
  // function cloning are performed the types will match up.

  // There will not be more than 2 indices because nested structures are
  // disqualified during the safety checks.
  unsigned NumIndices = GEP->getNumIndices();
  assert(NumIndices <= 2 && "Unexpected index count for GEP");
  if (NumIndices == 1) {
    // This will convert the GEP of case 1 from:
    //    %addr1 = getelementptr %struct.t, ptr %base, i64 %base_idx
    //
    // When the index is not being shrunk, the index will be the same
    // bit width as a pointer:
    //    %soa_idx = ptrtoint ptr addrspace(1) %base to i64
    //    %add = add i64 %soa_idx, %base_idx
    //    %addr1 = inttoptr i64 %add to ptr addrspace(1)
    //
    // When the index is being shrunk to 32-bits, create:
    //    %soa_idx = ptrtoint ptr %base addrspace(1) to i32
    //    %base_idx.typed = trunc i64 %base_idx to i32
    //    %add = add i32 %soa_idx, %base_idx.typed
    //    %addr1 = inttoptr i32 %add to ptr addrspace(1)
    //
    // The ptrtoint and inttoptr instructions and the address space uses are
    // temporary. They are just used in order to support the type remapping
    // process. During the type remapping process, the pointer type within the
    // address space in those instructions will be converted into an integer
    // type, resulting in the ptrtoint/inttoptr instructions having the same
    // source and destination types. These will be removed during the function
    // post processing. The use of the conversion instruction here allows for
    // values into and out of affected instructions to be replaced without
    // violating the type matching.
    //
    // After the post processing function, the IR left will be:
    //    %addr1 = add i64 %soa_idx, %base_idx
    // or:
    //    %base_idx.typed = trunc i64 %base_idx to i32
    //    %addr1 = add i32 %soa_idx, %base_idx.typed

    // We should never have a GEP index member that is wider than the bit width
    // of a pointer. If we do, then the indexing calculations will not work.
    Value *BaseIdx = GEP->getOperand(1);
    assert(DL.getTypeSizeInBits(BaseIdx->getType()) <=
               DL.getTypeSizeInBits(getPtrSizedIntLLVMType()) &&
           "Unsupported GEP index type");

    // Match the GEP index value to the bit width of the index type being used.
    Value *Src = GEP->getPointerOperand();
    Value *IdxAsInt = createCastToIndexType(Src, GEP);
    Value *IdxIn = promoteOrTruncValueToWidth(BaseIdx, IndexInfo.Width, GEP);
    Value *Add = BinaryOperator::CreateAdd(IdxAsInt, IdxIn, "", GEP);

    // We will steal the name of the GEP and put it on this instruction
    // because even though uses of the GEP are going to be replaced with a
    // cast instruction, the cast is going to eventually be eliminated during
    // post processing of the function.
    Add->takeName(GEP);

    // Cast the computed index back to the original pointer type, and
    // substitute this into the users. When the type remapping occurs, the
    // users will be converted to an integer type, and the cast instruction
    // will be removed during post-processing.
    llvm::Type *GEPSrcTy = GEP->getSourceElementType();
    assert(GEPSrcTy->isStructTy());
    auto *OrigStructTy = cast<llvm::StructType>(GEPSrcTy);
    CastInst *ArrayIdx =
        CastInst::CreateBitOrPointerCast(Add, OrigStructTy->getPointerTo());
    ArrayIdx->insertBefore(GEP);
    FuncInfo->PtrConverts.push_back(ArrayIdx);
    GEP->replaceAllUsesWith(ArrayIdx);

    // When opaque pointers are in use, we need the pointer to be in an
    // address space to recognize it during type remapping.
    llvm::Type *TypeInAddrSpace = getAddrSpacePtrForType(OrigStructTy);
    if (TypeInAddrSpace)
      FuncInfo->InstructionsToMutate.push_back({ArrayIdx, TypeInAddrSpace});
    FuncInfo->InstructionsToDelete.push_back(GEP);
    return;
  }

  // This will convert the GEP of case 2 from:
  //    %elem_addr = getelementptr %struct.t, ptr %base, i64 %base_idx, i32 1
  //
  // When the index is not being shrunk, create:
  //    %soa_field_addr = getelementptr % __soa_struct.t, ptr @__soa_struct.t,
  //                        i64 0, i32 1
  //    %soa_addr = load ptr, ptr %soa_field_addr
  //    %soa_idx = ptrtoint ptr addrspace(1) %base to i64
  //    %array_idx = add i64 %soa_idx, %base_idx
  //    %elem_addr = getelementptr i32, ptr %soa_addr, i64 %array_idx
  //
  // When the index is being shrunk to 32-bits, create:
  //    %soa_field_addr = getelementptr % __soa_struct.t, ptr @__soa_struct.t,
  //                        i64 0, i32 1
  //    %soa_addr = load ptr, ptr %soa_field_addr
  //    %soa_idx = ptrtoint ptr  addrspace(1) %base to i32
  //    %base_idx.typed = trunc i64 %base_idx to i32
  //    %array_idx = add i32 %soa_idx, %base_idx.typed
  //    %array_idx.typed = zext i32 %adjusted_idx to i64
  //    %elem_addr = getelementptr i32, ptr %soa_addr, i64 %array_idx.typed
  //
  Type *ElementTy = GEP->getSourceElementType();
  assert(ElementTy->isStructTy() &&
         "Collector should only get structure type GEP");

  // Create and insert the instructions that get the address of the field in
  // the transformed structure.
  SOATypeInfoTy &SOAType = getSOATypeInfo(cast<llvm::StructType>(ElementTy));
  CastInst *SOAIdx = createCastToIndexType(GEP->getPointerOperand(), GEP);
  Value *FieldNum = GEP->getOperand(2);
  GetElementPtrInst *FieldGEP = createGEPFieldAddressReplacement(
      SOAType, SOAIdx, GEP->getOperand(1), FieldNum, GEP);

  // We will steal the name of the GEP and put it on this instruction
  // because even if the replacement value used prior to type remapping is done
  // via a cast statement, the cast is going to be eliminated during function
  // post processing.
  FieldGEP->takeName(GEP);

  // If the field type is pointer to a structure type that is being changed,
  // generate a temporary cast to the original type so that the users will have
  // expected types. These casts will become casts from/to the same type
  // after the remapping happens, and will be removed during post-processing.
  // After type remapping, the cast will be eliminated. For example: %struct.a*
  // is now %__AOSDT_struct.a*.
  //
  // When only opaque pointers are supported, this should no longer be
  // necessary.
  //
  unsigned FieldIdx = cast<ConstantInt>(FieldNum)->getLimitedValue();
  llvm::Type *SOAFieldTy = SOAType.SOAStructType->getElementType(FieldIdx);
  Type *OrigFieldTy = GEP->getType();
  Value *ReplVal = FieldGEP;
  if (SOAFieldTy != OrigFieldTy) {
    CastInst *CastToPtr =
        CastInst::CreateBitOrPointerCast(FieldGEP, OrigFieldTy);
    CastToPtr->insertBefore(GEP);
    FuncInfo->PtrConverts.push_back(CastToPtr);
    ReplVal = CastToPtr;
  }

  GEP->replaceAllUsesWith(ReplVal);
  FuncInfo->InstructionsToDelete.push_back(GEP);
}

void AOSToSOAOPTransformImpl::convertByteGEP(GetElementPtrInst *GEP,
                                             DTransStructType *OrigStructTy,
                                             size_t FieldNum) {
  // TODO: Handle byte-flattened GEPs on the type being transformed.
  llvm_unreachable("ByteGEP conversion not implemented yet");
}

// Update the GEP result element type on the GEPs because now the result of a
// GEP will be an integer index instead of a pointer type.
void AOSToSOAOPTransformImpl::convertDepGEP(GetElementPtrInst *GEP) {
  auto *LLVMStructTy = cast<llvm::StructType>(GEP->getSourceElementType());
  Value *FieldNum = GEP->getOperand(2);
  uint32_t FieldIdx = dyn_cast<ConstantInt>(FieldNum)->getLimitedValue();
  DTransStructType *DTransStructTy = getDependentDTransType(LLVMStructTy);
  assert(DTransStructTy && "DTransStructType missing for dependent type");
  DTransType *FieldTy = DTransStructTy->getFieldType(FieldIdx);
  assert(FieldTy && FieldTy->isPointerTy() &&
         FieldTy->getPointerElementType()->isStructTy() &&
         "Expect field to be pointer to structure");
  auto *FieldPointeeType =
      cast<DTransStructType>(FieldTy->getPointerElementType());
  Type *AddrSpaceForType = getAddrSpacePtrForType(FieldPointeeType);
  if (AddrSpaceForType)
    GEP->setResultElementType(AddrSpaceForType);
}

void AOSToSOAOPTransformImpl::convertDepByteGEP(GetElementPtrInst *GEP,
                                                DTransStructType *OrigStructTy,
                                                size_t FieldNum) {
  // TODO: Handle byte-flattened GEPs on a dependent type.
  llvm_unreachable("Dependent type ByteGEP conversion not implemented yet");
}

// Create an intermediate cast instruction to cast from a pointer to a
// structure type into the index type. If 'InsertBefore' is non-null the
// instruction will be inserted into the IR before it. This cast is added to the
// list of casts that are to be removed during post processing.
CastInst *
AOSToSOAOPTransformImpl::createCastToIndexType(Value *V,
                                               Instruction *InsertBefore) {
  CastInst *ToInt =
      CastInst::CreateBitOrPointerCast(V, IndexInfo.LLVMType, "", InsertBefore);
  FuncInfo->PtrConverts.push_back(ToInt);
  return ToInt;
}

// Create a load of the array address for a specific field of the SOA
// structure.
//
// SOAType is the SOA structure type descriptor.
// FieldNumVal is a constant integer for the field number.
// Instructions are inserted before 'InsertBefore'
//
// For example:
// If the SOA structure type represents: { i64*, i32*, struct.foo** }
// and the array for field 1 is desired. This generates the following
// to load the base address for that array:
//    %soa_field_addr = getelementptr % __soa_struct.t, ptr @__soa_struct.t,
//                   i64 0, i32 1
//    %soa_addr = load ptr, ptr %soa_field_addr
//
LoadInst *AOSToSOAOPTransformImpl::createSOAFieldLoad(
    SOATypeInfoTy &SOAType, Value *FieldNumVal, Instruction *InsertBefore) {
  assert(FieldNumVal && isa<ConstantInt>(FieldNumVal) &&
         "Need FieldNumVal must be constant integer value");
  assert(InsertBefore && "InsertBefore is not optional");

  uint32_t FieldIdx = cast<ConstantInt>(FieldNumVal)->getLimitedValue();
  GetElementPtrInst *SOAFieldAddr = GetElementPtrInst::Create(
      SOAType.SOAStructType, SOAType.SOAVar,
      {ConstantInt::get(getPtrSizedIntLLVMType(), 0), FieldNumVal}, "",
      InsertBefore);
  llvm::Type *FieldType = SOAType.SOAStructType->getElementType(FieldIdx);
  LoadInst *SOAAddr =
      new LoadInst(FieldType, SOAFieldAddr, "", false /*volatile*/,
                   DL.getABITypeAlign(FieldType));

  // Mark the load of the structure of arrays field member as being invariant.
  // When the memory allocation for the object was done, the address of each
  // array was stored within the members of the structure of arrays, and will
  // never be changed. Marking these are invariant allows other
  // optimizations to hoist these accesses to prevent repetitive accesses.
  SOAAddr->setMetadata(LLVMContext::MD_invariant_load,
                       MDNode::get(InsertBefore->getContext(), {}));
  SOAAddr->insertBefore(InsertBefore);
  return SOAAddr;
}

// If the bit width of the type for 'V' is different than 'DstWidth', create a
// sign-extend or truncate instruction that will allow 'V' be able to be used
// for arithmetic expressions of the requested bit width. Otherwise, just return
// 'V'.
Value *
AOSToSOAOPTransformImpl::promoteOrTruncValueToWidth(Value *V, uint64_t DstWidth,
                                                    Instruction *InsertBefore) {
  assert(V->getType()->isIntegerTy() && "Must be integer type");
  uint64_t SrcWidth = DL.getTypeSizeInBits(V->getType());
  if (SrcWidth == DstWidth)
    return V;

  llvm::Type *DstTy = llvm::Type::getIntNTy(V->getContext(), DstWidth);
  if (SrcWidth < DstWidth)
    return CastInst::Create(CastInst::SExt, V, DstTy, "", InsertBefore);

  return CastInst::Create(CastInst::Trunc, V, DstTy, "", InsertBefore);
}

// Create and insert the instruction sequence that gets the address of a
// specific element in the peeled structure.
//
// SOAType is the description of the type being transformed.
// FieldNumVal is a constant integer for the field number.
// GEPBaseIdx and GEPFieldNum are the two GEP indices.
//
// The set of instructions will be inserted immediately before
// 'InsertBefore'.
// Returns the getelementptr instruction that represents the address.
//
// When the index is not being shrunk, creates:
//    %soa_field_addr = getelementptr % __soa_struct.t, ptr @__soa_struct.t,
//                        i64 0, i32 1
//    %soa_addr = load ptr, ptr %soa_field_addr
//    %soa_idx = ptrtoint ptr addrspace(1) %base to i64
//    %array_idx = add i64 %soa_idx, %base_idx
//    %elem_addr = getelementptr i32, ptr %soa_addr, i64 %array_idx
//
// When the index is being shrunk to 32-bits, creates:
//    %soa_field_addr = getelementptr % __soa_struct.t, ptr @__soa_struct.t,
//                        i64 0, i32 1
//    %soa_addr = load ptr, ptr %soa_field_addr
//    %soa_idx = ptrtoint ptr  addrspace(1) %base to i32
//    %base_idx.typed = trunc i64 %base_idx to i32
//    %array_idx = add i32 %soa_idx, %base_idx.typed
//    %array_idx.typed = zext i32 %adjusted_idx to i64
//    %elem_addr = getelementptr i32, ptr %soa_addr, i64 %array_idx.typed
GetElementPtrInst *AOSToSOAOPTransformImpl::createGEPFieldAddressReplacement(
    SOATypeInfoTy &SOAType, Value *PeelIdxAsInt, Value *GEPBaseIdx,
    Value *GEPFieldNum, Instruction *InsertBefore) {
  LoadInst *SOAAddr = createSOAFieldLoad(SOAType, GEPFieldNum, InsertBefore);

  // If first index is not constant 0, then we need to offset the index by that
  // amount.
  Value *AdjustedPeelIdxAsInt = PeelIdxAsInt;
  if (!dtrans::isValueEqualToSize(GEPBaseIdx, 0)) {
    uint64_t PeelIdxWidth = IndexInfo.Width;
    GEPBaseIdx =
        promoteOrTruncValueToWidth(GEPBaseIdx, PeelIdxWidth, InsertBefore);
    BinaryOperator *Add =
        BinaryOperator::CreateAdd(PeelIdxAsInt, GEPBaseIdx, "", InsertBefore);
    AdjustedPeelIdxAsInt = Add;
  }

  // Identify the type in the new structure for the field being accessed.
  uint32_t FieldIdx = cast<ConstantInt>(GEPFieldNum)->getLimitedValue();
  Type *FieldElementTy = SOAType.getTransformedFieldType(FieldIdx);

  // Extend the index back to a 64-bit value for use in the GEP instruction
  // because the pointer-type size used as the base is a 64-bit type.
  if (IndexInfo.PointerShrinkingEnabled)
    AdjustedPeelIdxAsInt =
        CastInst::Create(CastInst::ZExt, AdjustedPeelIdxAsInt,
                         getPtrSizedIntLLVMType(), "", InsertBefore);

  GetElementPtrInst *FieldGEP = GetElementPtrInst::Create(
      FieldElementTy, SOAAddr, AdjustedPeelIdxAsInt, "", InsertBefore);

  return FieldGEP;
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

  AOSToSOAOPTransformImpl Transformer(
      M.getContext(), DTInfo, DTInfo->getPtrTypeAnalyzer().sawOpaquePointer(),
      "__SOADT_", M.getDataLayout(), GetTLI, CandidateTypes);
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
