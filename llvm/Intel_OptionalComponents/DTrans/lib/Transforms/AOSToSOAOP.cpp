//==== AOSToSOAOP.cpp - AOS-to-SOA with support for opaque pointers ====//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
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
#include "Intel_DTrans/Analysis/DTransOPUtils.h"
#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/Analysis/DTransTypeMetadataBuilder.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOPOptBase.h"
#include "Intel_DTrans/Transforms/DTransOptUtils.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/InitializePasses.h"

using namespace llvm;
using namespace dtransOP;
using dtrans::AllocCallInfo;
using dtrans::CallInfo;
using dtrans::DTransAnnotator;
using dtrans::FreeCallInfo;
using dtrans::MemfuncCallInfo;
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
  AttributeMask IncompatibleTypeAttrs;
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

  // Alloca instructions that should be converted from allocating a pointer to
  // allocating an integer for the peeling index.
  SmallVector<AllocaInst *, 2> AllocasToConvert;

  // Load/Store cases where the pointer to structure being transformed is
  // loaded/stored as a pointer-sized integer.
  SmallVector<std::pair<LoadInst *, DTransStructType *>, 2>
      PtrSizedIntLoadsToConvert;
  SmallVector<std::pair<StoreInst *, DTransStructType *>, 2>
      PtrSizedIntStoresToConvert;

  SmallVector<std::pair<BitCastInst *, DTransStructType *>, 4> BCsToConvert;

  // Memory allocation and free calls for the type being transformed that need
  // to be transformed.
  SmallVector<std::pair<AllocCallInfo *, StructInfo *>, 1> AllocsToConvert;
  SmallVector<std::pair<FreeCallInfo *, StructInfo *>, 1> FreesToConvert;

  // PtrToInt instructions need to be processed to either eliminate the
  // conversion, or perform a size conversion based on whether pointer
  // shrinking is enabled or not.
  SmallVector<std::pair<PtrToIntInst *, DTransStructType *>, 4>
      PtrToIntToConvert;

  // Pointer subtracts followed by division of the structure size that need to
  // be transformed.
  SmallVector<std::pair<BinaryOperator *, DTransStructType *>, 4>
      BinOpsToConvert;

  // GetElementPtr instructions that access fields that are of the type being
  // transformed from a dependent type.
  SmallVector<GetElementPtrInst *, 16> DepGEPsToConvert;

  // GetElementPtr instructions using the byte flattened form to access a
  // dependent type.
  SmallVector<ByteGEPInfo, 16> DepByteGEPsToConvert;

  // Calls for dependent types that need to be resized.
  SmallVector<std::pair<AllocCallInfo *, StructInfo *>, 1> DepAllocsToResize;
  SmallVector<std::pair<MemfuncCallInfo *, StructInfo *>, 1>
      DepMemfuncsToResize;

  // Pointer subtracts followed by division on dependent types that have their
  // size changed.
  SmallVector<std::pair<BinaryOperator *, DTransStructType *>, 4>
      DepBinOpsToConvert;

  // A list of pointer conversion instructions inserted as part of the
  // processFunction routine (pointer to int, int to pointer, or pointer of
  // original type to a pointer of the remapped type) that after the type
  // remapping process should have the same source and destination types, and
  // must be removed during postProcessFunction routine. A set is used rather
  // than a vector because a cast instruction may be reused for multiple
  // byte-flattened GEP accesses.
  SmallPtrSet<CastInst *, 16> PtrConverts;

  // When some instructions are converted for shrinking the index to 32-bits, cast
  // instructions are generated during processFunction around the replacement
  // instructions to convert the original type to be compatible with the
  // value type before it is remapped. During post-processing, it may be
  // possible to remove these casts because the type remapping has modified
  // operand types to make the casts cancel out. This helps avoid instructions
  // that could cause safety flags to be set on subsequent runs of the
  // DTransSafetyAnalyzer. For example:
  //    %53 = ptrtoint i32* %52 to i64
  //    %54 = inttoptr i64 %53 to i32*
  SmallVector<Instruction *, 4> IntermediateConverts;

  // Set of call sites in the function that may need to have attributes on the
  // return or parameter types updated.
  SmallVector<CallBase *, 16> CallsToConvert;

  // Instructions from the input IR that need to be removed at the end of
  // processFunction().
  SetVector<Instruction *> InstructionsToDelete;

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

  // List of instructions that need to have the DTrans type metadata updated to
  // reflect that a type and pointer indirection level is being changed on the
  // type being transformed. The type remapper will update the types for a case
  // where %struct.arc changes to %_DT_struct.arc because the level of
  // indirection remains the same. However, for the type being transformed in
  // this transformation, the metadata needs to be directly updated because
  // pointers to %struct.node are changed to be an integer with one less level
  // of indirection. The DTransType component of the pair holds the type decoded
  // from the metadata prior to type remapping.
  SmallVector<std::pair<Instruction *, DTransType *>, 4> InstMDToUpdate;

  // When processing byte-flattened GEPs some instructions may be processed to
  // get a Value for the index variable. This cache is used to get the processed
  // values so they do not need to be recomputed when multiple instructions
  // being processed. This is also necessary to avoid cycles when walking PHI
  // nodes. This map needs to be cleared after processing each function.
  DenseMap<Value *, Value *> IndexCache;
};

// This visitor will collect the instructions that need to be converted.
class AOSCollector : public InstVisitor<AOSCollector> {
public:
  AOSCollector(AOSToSOAOPTransformImpl &Transform, DTransSafetyInfo &DTInfo,
               PerFunctionInfo &FuncInfo)
      : Transform(Transform), DTInfo(DTInfo), PTA(DTInfo.getPtrTypeAnalyzer()),
        MDReader(DTInfo.getTypeMetadataReader()), FuncInfo(FuncInfo) {}

  void visitGetElementPtrInst(GetElementPtrInst &GEP);
  void visitLoadInst(LoadInst &I);
  void visitStoreInst(StoreInst &I);
  void visitCallBase(CallBase &I);
  void visitReturnInst(ReturnInst &I);
  void visitPHINode(PHINode &I);
  void visitSelectInst(SelectInst &I);
  void visitICmpInst(ICmpInst &I);
  void visitBitCastInst(BitCastInst &I);
  void visitPtrToIntInst(PtrToIntInst &I);
  void visitBinaryOperator(BinaryOperator &I);
  void visitAllocaInst(AllocaInst &I);
  void visitInstruction(Instruction &I);

  void checkForConstantToConvert(Instruction *I, uint32_t OpNum);
  DTransStructType *getDTransStructTypeforValue(Value *V);

  void evaluateCallInfo(dtrans::CallInfo *CInfo);

private:
  AOSToSOAOPTransformImpl &Transform;
  DTransSafetyInfo &DTInfo;
  PtrTypeAnalyzer &PTA;
  TypeMetadataReader &MDReader;
  PerFunctionInfo &FuncInfo;
};

// This class is responsible for all the transformation work for the AOS to SOA
// with Indexing conversion.
class AOSToSOAOPTransformImpl : public DTransOPOptBase {
public:
  AOSToSOAOPTransformImpl(LLVMContext &Ctx, DTransSafetyInfo *DTInfo,
                          StringRef DepTypePrefix, const DataLayout &DL,
                          AOSToSOAOPPass::GetTLIFuncType GetTLI,
                          SmallVectorImpl<dtrans::StructInfo *> &Types)
      : DTransOPOptBase(Ctx, DTInfo, DepTypePrefix), DL(DL), GetTLI(GetTLI),
        Materializer(*getTypeRemapper()) {
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

  bool isFnClonedForIndex(Function *F) const;
  Function *getClonedFunction(Function *F) const;

  PointerType *getAddrSpacePtrForType(llvm::StructType *OrigStructTy);
  PointerType *getAddrSpacePtrForType(DTransStructType *OrigStructTy);

  struct SOATypeInfoTy &getSOATypeInfo(llvm::StructType *Ty);

  bool isPointerSizedIntType(llvm::Type *Ty) const {
    return Ty == PtrSizeIntLLVMType;
  }

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
  void convertAlloca(AllocaInst *AI);
  void convertPtrSizedIntLoad(LoadInst *LI, DTransStructType *DTransTy);
  void convertPtrSizedIntStore(StoreInst *SI, DTransStructType *DTransTy);
  void convertDepGEP(GetElementPtrInst *GEP);
  void convertDepByteGEP(GetElementPtrInst *GEP, DTransStructType *OrigStructTy,
                         size_t FieldNum);
  void convertBC(BitCastInst *BC, DTransStructType *StructTy);
  bool hasLiveUser(Instruction *I);
  void convertPtrToInt(PtrToIntInst *I, DTransStructType *StructTy);
  void convertBinaryOperator(BinaryOperator *I, DTransStructType *StructTy);

  void convertAllocCall(AllocCallInfo *AInfo, StructInfo *StInfo);
  void convertFreeCall(FreeCallInfo *CInfo, StructInfo *StInfo);
  void convertDepAllocCall(AllocCallInfo *AInfo, StructInfo *StInfo);
  void convertDepMemfuncCall(MemfuncCallInfo *CInfo, StructInfo *StInfo);
  void convertDepBinaryOperator(BinaryOperator *I, DTransStructType *StructTy);

  void updateDTransMetadata(Instruction *I, DTransType *Ty);
  void updateCallAttributes(CallBase *Call);
  void updateFunctionAttributes(Function &OrigFn, Function &CloneFn);
  bool updateAttributeList(llvm::FunctionType *OrigFnType,
                           llvm::FunctionType *NewFnType, AttributeList &Attrs);

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

  Value *getIndexForValue(Value *Op, StructType *OrigStructTy);
  Value *createIndexFromValue(Value *Op, StructType *OrigStructTy);

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

  // The set of functions that were cloned as a result of transforming a
  // parameter or return type from being a pointer to the structure to being an
  // integer index value.
  SmallPtrSet<Function *, 16> FnClonedForIndex;
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
  // Check for integer loads that represent pointers to a structure type being
  // transformed:
  //   %x = bitcast %struct.node* to i64
  //   load i64, i64* %x
  llvm::Type *Ty = I.getType();
  bool PtrSizedIntLoad = Transform.isPointerSizedIntType(Ty);
  if (!I.getType()->isPointerTy() && !PtrSizedIntLoad)
    return;

  auto *Info = PTA.getValueTypeInfo(&I);
  if (!Info) {
    if (PtrSizedIntLoad)
      return;
    else
      llvm_unreachable("Expected PointerTypeAnalyzer to collect type");
  }

  DTransType *DTransTy = PTA.getDominantAggregateUsageType(*Info);
  if (!DTransTy || !DTransTy->isPointerTy())
    return;

  auto *StructTy =
      dyn_cast<DTransStructType>(DTransTy->getPointerElementType());
  if (!StructTy)
    return;

  auto *LLVMStructType = cast<llvm::StructType>(StructTy->getLLVMType());
  if (!Transform.isTypeToTransform(LLVMStructType))
    return;

  if (PtrSizedIntLoad) {
    FuncInfo.PtrSizedIntLoadsToConvert.push_back({&I, StructTy});
    return;
  }

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
  // Check for integer stores that represent pointers to a structure type
  // being transformed:
  //   %x = ptrtoint %struct.node* to i64
  //   store i64 %x, i64* %y
  Value *Val = I.getValueOperand();
  llvm::Type *Ty = Val->getType();
  bool PtrSizedIntStore = Transform.isPointerSizedIntType(Ty);
  if (!Ty->isPointerTy() && !PtrSizedIntStore)
    return;

  // Get the type info for the value operand
  auto *Info = PTA.getValueTypeInfo(&I, 0);
  if (!Info) {
    if (PtrSizedIntStore)
      return;
    else
      llvm_unreachable("Expected PointerTypeAnalyzer to collect type");
  }

  DTransType *DTransTy = PTA.getDominantAggregateUsageType(*Info);
  if (!DTransTy || !DTransTy->isPointerTy())
    return;

  auto *StructTy =
      dyn_cast<DTransStructType>(DTransTy->getPointerElementType());
  if (!StructTy)
    return;

  auto *LLVMStructType = cast<llvm::StructType>(StructTy->getLLVMType());
  if (!Transform.isTypeToTransform(LLVMStructType))
    return;

  if (PtrSizedIntStore) {
    FuncInfo.PtrSizedIntStoresToConvert.push_back({&I, StructTy});
    return;
  }

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

void AOSCollector::visitCallBase(CallBase &I) {
  // Check if the call needs to be transformed based on the CallInfo.
  if (auto *CInfo = DTInfo.getCallInfo(&I)) {
    evaluateCallInfo(CInfo);
    // The remaining checks of this function are not relevant for calls to
    // function that been identified as CallInfo objects.
    return;
  }

  // The type qualification checks rejected any types passed to address taken
  // functions, so there is nothing to check for indirect calls.
  if (I.isIndirectCall())
    return;

  // Check for a bitcast function call. With opaque pointers, there will not be
  // bitcast calls, because the call type will just be 'ptr', but for
  // type-pointers a check is necessary to find the target function.
  Value *Callee = I.getCalledOperand();
  Function *F = dyn_cast<Function>(Callee->stripPointerCasts());
  assert(F && "Expected to identify called function");
  if (!Transform.isFnClonedForIndex(F))
    return;

  // The call may need to have attributes updated, add it to the list to be
  // processed.
  FuncInfo.CallsToConvert.push_back(&I);

  // When typed-pointers are being used, the value mapper class can handle all
  // the conversions necessary. When opaque pointers are in use, we need to
  // process the pointers to change the function call signature to pass integers
  // instead of pointer types.
  if (!F->getType()->isOpaquePointerTy())
    return;

  Function *CloneFn = Transform.getClonedFunction(F);
  assert(CloneFn &&
         "FnClonedForIndex set out of sync with OrigFuncToCloneFuncMap map");
  auto *OrigFnType = cast<llvm::FunctionType>(F->getValueType());
  auto *CloneFnType = cast<llvm::FunctionType>(CloneFn->getValueType());
  llvm::Type *RetTy = OrigFnType->getReturnType();

  // Check for a return value changing from a pointer type to an integer type.
  if (RetTy->isPointerTy() && !CloneFnType->getReturnType()->isPointerTy()) {
    auto *Info = PTA.getValueTypeInfo(&I);
    assert(Info && "Expected PointerTypeAnalyzer to collect type");
    DTransType *Ty = PTA.getDominantAggregateUsageType(*Info);
    if (Ty && Ty->isPointerTy() && Ty->getPointerElementType()->isStructTy()) {
      auto *StructTy = cast<DTransStructType>(Ty->getPointerElementType());
      PointerType *AddrSpacePtr = Transform.getAddrSpacePtrForType(StructTy);
      if (AddrSpacePtr)
        RetTy = AddrSpacePtr;
    }
  }

  // Check for a parameter changing from a pointer type to an integer type.
  SmallVector<Type *, 16> SigTypes;
  unsigned NumArgs = F->arg_size();
  for (unsigned ArgIdx = 0; ArgIdx < NumArgs; ++ArgIdx) {
    llvm::Type *OrigArgTy = OrigFnType->getParamType(ArgIdx);
    llvm::Type *CloneArgTy = CloneFnType->getParamType(ArgIdx);
    llvm::Type *ArgTy = OrigArgTy;
    if (OrigArgTy->isPointerTy() && !CloneArgTy->isPointerTy()) {
      auto *Info = PTA.getValueTypeInfo(&I, ArgIdx);
      assert(Info && "Expected PointerTypeAnalyzer to collect type");
      DTransType *Ty = PTA.getDominantAggregateUsageType(*Info);
      assert(Ty && Ty->isPointerTy() &&
             Ty->getPointerElementType()->isStructTy() &&
             "Changing argument should be pointer to struct type");
      auto *StructTy = cast<DTransStructType>(Ty->getPointerElementType());
      PointerType *AddrSpacePtr = Transform.getAddrSpacePtrForType(StructTy);
      if (AddrSpacePtr)
        ArgTy = AddrSpacePtr;
    }

    SigTypes.push_back(ArgTy);
  }

  auto *NewFnType = FunctionType::get(RetTy, SigTypes, F->isVarArg());
  FuncInfo.InstructionsToMutate.push_back({&I, NewFnType});

  // Check for 'null' pointer values being passed. These need to be updated to
  // allow them to be converted to an integer index value.
  unsigned NumOps = I.arg_size();
  for (unsigned ArgNum = 0; ArgNum < NumOps; ++ArgNum)
    checkForConstantToConvert(&I, ArgNum);
}

void AOSCollector::evaluateCallInfo(dtrans::CallInfo *CInfo) {
  // This enumeration is used to indicate the type of conversion an
  // call instruction needs when checking the CallInfo object for the type the
  // call operates on.
  //   AOS_NoConv  - Instruction does not need to be converted.
  //   AOS_SOAConv - Instruction requires conversion of the pointer to index.
  //   AOS_DepConv - Instruction requires changes due to data structure size
  //                 change made to a dependent type.
  typedef enum { AOS_NoConv, AOS_SOAConv, AOS_DepConv } AOSConvType;

  // Get the Type for CallInfo cases that can be transformed. If the CallInfo is
  // for a case that is not being transformed, return nullptr as the first
  // element of the pair.
  auto &TheTransform = Transform;
  auto GetCallInfoTypeToTransform = [&TheTransform](dtrans::CallInfo *CInfo)
      -> std::pair<DTransStructType *, AOSConvType> {
    auto &TypeList = CInfo->getElementTypesRef();

    // Only cases with a single type will be allowed during the transformation.
    // If there's more than one, we don't need the type, because we won't be
    // transforming it.
    if (TypeList.getNumTypes() != 1)
      return std::make_pair(nullptr, AOS_NoConv);

    llvm::Type *ElemTy = TypeList.getElemLLVMType(0);
    if (TheTransform.isTypeToTransform(ElemTy))
      return std::make_pair(
          cast<DTransStructType>(TypeList.getElemDTransType(0)), AOS_SOAConv);

    if (TheTransform.isDependentTypeSizeChanged(ElemTy))
      return std::make_pair(
          cast<DTransStructType>(TypeList.getElemDTransType(0)), AOS_DepConv);

    return std::make_pair(nullptr, AOS_NoConv);
  };

  DTransStructType *CInfoElemTy;
  AOSConvType ConvType = AOS_NoConv;
  std::tie(CInfoElemTy, ConvType) = GetCallInfoTypeToTransform(CInfo);
  if (ConvType == AOS_NoConv)
    return;

  auto *TI = DTInfo.getTypeInfo(CInfoElemTy);
  assert(TI && "Expected TypeInfo for structure type");
  auto *StructTI = cast<StructInfo>(TI);
  switch (CInfo->getCallInfoKind()) {
  case dtrans::CallInfo::CIK_Alloc: {
    auto *AInfo = cast<AllocCallInfo>(CInfo);
    // The candidate qualification should have only allowed calloc or
    // malloc for the types being directly transformed. Dependent types
    // are only resized, so other allocation types are allowed.
    if (ConvType == AOS_SOAConv)
      assert((AInfo->getAllocKind() == dtrans::AK_Calloc ||
              AInfo->getAllocKind() == dtrans::AK_Malloc) &&
             "Only calloc or malloc expected for AOS-to-SOA types");

    if (ConvType == AOS_SOAConv)
      FuncInfo.AllocsToConvert.push_back({AInfo, StructTI});
    else
      FuncInfo.DepAllocsToResize.push_back({AInfo, StructTI});
    break;
  }
  case dtrans::CallInfo::CIK_Free:
    // We only need to update the 'free' if it's the AOS-to-SOA type
    // being converted. There is no impact on calls to free for
    // dependent types.
    if (ConvType == AOS_SOAConv)
      FuncInfo.FreesToConvert.push_back({cast<FreeCallInfo>(CInfo), StructTI});
    break;
  case dtrans::CallInfo::CIK_Memfunc:
    if (ConvType == AOS_SOAConv)
      llvm_unreachable(
          "Memfuncs currently not permitted on the type being transformed");
    else
      FuncInfo.DepMemfuncsToResize.push_back(
          {cast<MemfuncCallInfo>(CInfo), StructTI});
    break;
  }
}

// Check for a return instruction that returns an opaque 'null' pointer value of
// the type being transformed.
void AOSCollector::visitReturnInst(ReturnInst &I) {
  Value *V = I.getReturnValue();
  if (!V)
    return;

  if (!isa<ConstantPointerNull>(V))
    return;

  if (!V->getType()->isOpaquePointerTy())
    return;

  Function *F = I.getFunction();
  if (!Transform.isFnClonedForIndex(F))
    return;

  // The PointerTypeAnalyzer does not currently collect a type for 'null'
  // pointer values of ReturnInst, check it here. The PointerTypeAnalyzer could
  // be expanded in the future to do this, if it is helpful.
  auto *FnTy =
      dyn_cast_or_null<DTransFunctionType>(MDReader.getDTransTypeFromMD(F));
  assert(FnTy && "Must have type if function is being transformed");

  DTransType *DTransRetTy = FnTy->getReturnType();
  if (!DTransRetTy || !DTransRetTy->isPointerTy())
    return;

  auto *StructTy =
      dyn_cast<DTransStructType>(DTransRetTy->getPointerElementType());
  if (!StructTy)
    return;

  auto *LLVMStructType = cast<llvm::StructType>(StructTy->getLLVMType());
  if (Transform.isTypeToTransform(LLVMStructType)) {

    PointerType *TypeInAddrSpace = Transform.getAddrSpacePtrForType(StructTy);
    if (TypeInAddrSpace)
      FuncInfo.ConstantsToReplace.push_back({&I, 0, TypeInAddrSpace});
  }
}

// For a PHINode instruction, when opaque pointers are in use, if the type
// represents a pointer to the type being transformed, we need to mark the
// instruction with the address space to recognize that the type will be changed
// to an integer index. Also, we need to consider the 'null' value pointer to
// see if an integer 0 needs to be substituted for the index.
void AOSCollector::visitPHINode(PHINode &I) {
  llvm::Type *Ty = I.getType();
  if (!Ty->isOpaquePointerTy())
    return;

  DTransStructType *StructTy = getDTransStructTypeforValue(&I);
  if (!StructTy)
    return;

  PointerType *TypeInAddrSpace = Transform.getAddrSpacePtrForType(StructTy);
  if (!TypeInAddrSpace)
    return;

  FuncInfo.InstructionsToMutate.push_back({&I, TypeInAddrSpace});
  for (auto Elem : enumerate(I.incoming_values()))
    if (isa<ConstantPointerNull>((Elem.value())))
      FuncInfo.ConstantsToReplace.push_back(
          {&I, Elem.index(), TypeInAddrSpace});
}

// For a select instruction, when opaque pointers are in use, if the type
// represents a pointer to the type being transformed, we need to mark the
// instruction with the address space to recognize that the type will be changed
// to an integer index. Also, we need to consider the 'null' value pointer to
// see if an integer 0 needs to be substituted for the index.
void AOSCollector::visitSelectInst(SelectInst &I) {
  llvm::Type *Ty = I.getType();
  if (!Ty->isOpaquePointerTy())
    return;

  DTransStructType *StructTy = getDTransStructTypeforValue(&I);
  if (!StructTy)
    return;

  PointerType *TypeInAddrSpace = Transform.getAddrSpacePtrForType(StructTy);
  if (!TypeInAddrSpace)
    return;

  FuncInfo.InstructionsToMutate.push_back({&I, TypeInAddrSpace});
  if (isa<ConstantPointerNull>(I.getTrueValue()))
    FuncInfo.ConstantsToReplace.push_back({&I, 1, TypeInAddrSpace});
  if (isa<ConstantPointerNull>(I.getFalseValue()))
    FuncInfo.ConstantsToReplace.push_back({&I, 2, TypeInAddrSpace});
}

// If one of the operands of the 'icmp' is a pointer to a type being
// transformed, and the other is a 'null' pointer, then we need to mark the
// 'null' pointer as needing to be converted to the integer index 0.
void AOSCollector::visitICmpInst(ICmpInst &I) {
  Value *Op0 = I.getOperand(0);
  Value *Op1 = I.getOperand(1);
  bool IsNull0 = isa<ConstantPointerNull>(Op0);
  bool IsNull1 = isa<ConstantPointerNull>(Op1);
  if ((!IsNull0 && !IsNull1) || (IsNull0 && IsNull1))
    return;

  Value *NullOp = IsNull0 ? Op0 : Op1;
  if (!NullOp->getType()->isOpaquePointerTy())
    return;

  Value *NonNullOp = IsNull0 ? Op1 : Op0;
  DTransStructType *StructTy = getDTransStructTypeforValue(NonNullOp);
  if (!StructTy)
    return;

  // If the structure type is being transformed, there will be an address space
  // to use for the opaque pointer.
  PointerType *TypeInAddrSpace = Transform.getAddrSpacePtrForType(StructTy);
  if (!TypeInAddrSpace)
    return;

  uint32_t NullOpIdx = IsNull0 ? 0 : 1;
  FuncInfo.ConstantsToReplace.push_back({&I, NullOpIdx, TypeInAddrSpace});
}

void AOSCollector::visitBitCastInst(BitCastInst &I) {
  if (!I.getType()->isPointerTy())
    return;

  auto *Info = PTA.getValueTypeInfo(&I);
  assert(Info && "Expected PointerTypeAnalyzer to collect type");
  DTransType *Ty = PTA.getDominantAggregateUsageType(*Info);
  if (Ty && Ty->isPointerTy() &&
      Ty->getPointerElementType()->isStructTy()) {
    auto *StructTy = cast<DTransStructType>(Ty->getPointerElementType());
    if (Transform.isTypeToTransform(StructTy->getLLVMType()))
      FuncInfo.BCsToConvert.push_back(std::make_pair(&I, StructTy));
  }
}

void AOSCollector::visitPtrToIntInst(PtrToIntInst &I) {
  Value *PtrOp = I.getPointerOperand();
  DTransStructType *StructTy = getDTransStructTypeforValue(PtrOp);
  if (!StructTy)
    return;

  if (Transform.isTypeToTransform(StructTy->getLLVMType()))
    FuncInfo.PtrToIntToConvert.push_back({&I, StructTy});
}

void AOSCollector::visitBinaryOperator(BinaryOperator &I) {
  if (I.getOpcode() != Instruction::Sub)
    return;

  DTransType *PtrSubTy = DTInfo.getResolvedPtrSubType(&I);
  if (!PtrSubTy || !isa<DTransStructType>(PtrSubTy))
    return;

  auto *StTy = cast<DTransStructType>(PtrSubTy);
  if (Transform.isTypeToTransform(StTy->getLLVMType()))
    FuncInfo.BinOpsToConvert.push_back(std::make_pair(&I, StTy));
  else if (Transform.isDependentTypeSizeChanged(StTy->getLLVMType()))
    FuncInfo.DepBinOpsToConvert.push_back({ &I, StTy });
}

void AOSCollector::visitAllocaInst(AllocaInst &I) {
  // Alloca instructions may have metadata attachments that describe the DTrans
  // type to handle the case where a pointer type is being allocated. When a
  // pointer to the structure type being transformed is allocated, then the
  // metadata needs to be updated to reflect that a pointer to an integer type
  // with one less level of indirection will be used.
  DTransType *DType = MDReader.getDTransTypeFromMD(&I);
  if (!DType)
    return;

  DTransType *BaseType = DType;
  while (BaseType->isArrayTy())
    BaseType = BaseType->getArrayElementType();

  unsigned PtrLevel = 0;
  while (BaseType->isPointerTy()) {
    BaseType = BaseType->getPointerElementType();
    ++PtrLevel;
  }

  if (!Transform.isTypeToTransform(BaseType->getLLVMType()))
    return;

  // Because the type being allocated will have one less level of indirection,
  // if the type was originally just a pointer to the structure then it needs to
  // be transformed to an integer allocation. In the typed pointer IR, this
  // happens automatically via the TypeRemapper. However, with opaque pointers
  // we need to handle this in the convertAlloca routine. We only need to handle
  // it for the case of one level of indirection because the allocation type
  // remains 'ptr' for all other cases, and just the metadata needs updating.
  if (I.getType()->isOpaquePointerTy() && PtrLevel == 1)
    FuncInfo.AllocasToConvert.push_back(&I);

  FuncInfo.InstMDToUpdate.push_back({&I, DType});
}

// Catch any other instruction. If the instruction produces the type being
// transformed, throw an assertion that it is not handled.
void AOSCollector::visitInstruction(Instruction &I) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  ValueTypeInfo *Info = PTA.getValueTypeInfo(&I);
  if (!Info)
    return;

  DTransType *Ty = PTA.getDominantAggregateUsageType(*Info);
  if (!Ty || !Ty->isPointerTy())
    return;

  auto *StructTy = dyn_cast<DTransStructType>(Ty->getPointerElementType());
  if (!StructTy)
    return;

  auto *LLVMStructType = cast<llvm::StructType>(StructTy->getLLVMType());
  assert(!Transform.isTypeToTransform(LLVMStructType) &&
         "Instruction not handled by AOS-to-SOA transformation");
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
}

// Check whether the PointerTypeAnalyzer identified operand number 'OpNum' of
// Instruction 'I' as a constant pointer of the type being transformed.
void AOSCollector::checkForConstantToConvert(Instruction *I, uint32_t OpNum) {
  auto *Val = I->getOperand(OpNum);

  // TODO: Currently, this is only handling 'ptr null' types, but should
  // probably be expanded to handle other constant values such as 'undef null'
  if (!isa<ConstantPointerNull>((Val)))
    return;

  ValueTypeInfo *Info = PTA.getValueTypeInfo(I, OpNum);
  if (!Info)
    return;

  DTransType *Ty = PTA.getDominantAggregateUsageType(*Info);
  if (!Ty || !Ty->isPointerTy())
    return;

  auto *StructTy = dyn_cast<DTransStructType>(Ty->getPointerElementType());
  if (!StructTy)
    return;

  auto *LLVMStructType = cast<llvm::StructType>(StructTy->getLLVMType());
  if (Transform.isTypeToTransform(LLVMStructType)) {
    PointerType *TypeInAddrSpace = Transform.getAddrSpacePtrForType(StructTy);
    if (TypeInAddrSpace)
      FuncInfo.ConstantsToReplace.push_back({I, OpNum, TypeInAddrSpace});
  }
}

// If the instruction was identified by the PointerTypeAnalyzer as being a
// pointer to a structure type, return the structure type.
DTransStructType *AOSCollector::getDTransStructTypeforValue(Value *V) {
  assert(V && "null input not permitted");
  if (!V->getType()->isPointerTy())
    return nullptr;

  auto *Info = PTA.getValueTypeInfo(V);
  assert(Info && "Expected PointerTypeAnalyzer to collect type");
  DTransType *Ty = PTA.getDominantAggregateUsageType(*Info);
  if (!Ty || !Ty->isPointerTy())
    return nullptr;

  // Return a struct type or null
  auto *StructTy = dyn_cast<DTransStructType>(Ty->getPointerElementType());
  return StructTy;
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

  // Find the functions that the base class cloned due to a pointer return or
  // parameter type being changed into an integer.
  for (auto &KV : OrigFuncToCloneFuncMap) {
    for (auto Arg : zip(KV.first->getValueType()->subtypes(),
                        KV.second->getValueType()->subtypes())) {
      if (std::get<0>(Arg)->isPointerTy() && !std::get<1>(Arg)->isPointerTy()) {
        FnClonedForIndex.insert(KV.first);
        break;
      }
    }
  }
}

void AOSToSOAOPTransformImpl::processFunction(Function &F) {
  // Lambda to clear the FuncInfo state that is no longer used after this
  // function completes.
  auto FuncInfoProcessFunctionComplete = [this]() {
    FuncInfo->GEPsToConvert.clear();
    FuncInfo->ByteGEPsToConvert.clear();
    FuncInfo->AllocasToConvert.clear();
    FuncInfo->PtrSizedIntLoadsToConvert.clear();
    FuncInfo->PtrSizedIntStoresToConvert.clear();
    FuncInfo->BCsToConvert.clear();
    FuncInfo->AllocsToConvert.clear();
    FuncInfo->FreesToConvert.clear();
    FuncInfo->PtrToIntToConvert.clear();
    FuncInfo->BinOpsToConvert.clear();
    FuncInfo->DepGEPsToConvert.clear();
    FuncInfo->DepByteGEPsToConvert.clear();
    FuncInfo->DepAllocsToResize.clear();
    FuncInfo->DepMemfuncsToResize.clear();
    FuncInfo->DepBinOpsToConvert.clear();
    FuncInfo->CallsToConvert.clear();
    FuncInfo->InstructionsToDelete.clear();
    FuncInfo->InstructionsToMutate.clear();
    FuncInfo->ConstantsToReplace.clear();
    FuncInfo->IndexCache.clear();
    FuncInfo->InstMDToUpdate.clear();
  };

  LLVM_DEBUG(dbgs() << "AOS-to-SOA: ProcessFunction: " << F.getName() << "\n");

  // The PerFunctionInfo data will be needed by this function and the
  // postProcessFunction.
  FuncInfo = std::make_unique<PerFunctionInfo>();
  AOSCollector Collector(*this, *DTInfo, *FuncInfo);
  Collector.visit(F);

  // Convert the allocation calls first because the result of the allocation can
  // be used in a GetElementPtr instruction, and the conversion of the
  // allocation will update those GEP instructions.
  for (auto &Alloc : FuncInfo->AllocsToConvert)
    convertAllocCall(Alloc.first, Alloc.second);
  for (auto &Free : FuncInfo->FreesToConvert)
    convertFreeCall(Free.first, Free.second);

  for (auto *AI : FuncInfo->AllocasToConvert)
    convertAlloca(AI);
  for (auto *GEP : FuncInfo->GEPsToConvert)
    convertGEP(GEP);
  for (auto &GEPInfo : FuncInfo->ByteGEPsToConvert)
    convertByteGEP(std::get<PerFunctionInfo::ByteGEPInfoGEPMember>(GEPInfo),
                   std::get<PerFunctionInfo::ByteGEPInfoTypeMember>(GEPInfo),
                   std::get<PerFunctionInfo::ByteGEPInfoFieldMember>(GEPInfo));
  for (auto &KV : FuncInfo->PtrSizedIntLoadsToConvert)
    convertPtrSizedIntLoad(KV.first, KV.second);
  for (auto &KV : FuncInfo->PtrSizedIntStoresToConvert)
    convertPtrSizedIntStore(KV.first, KV.second);

  for (auto &KV : FuncInfo->BCsToConvert)
    convertBC(KV.first, KV.second);

  for (auto &KV : FuncInfo->PtrToIntToConvert)
    convertPtrToInt(KV.first, KV.second);

  for (auto &KV : FuncInfo->BinOpsToConvert)
    convertBinaryOperator(KV.first, KV.second);

  for (auto &KV : FuncInfo->InstMDToUpdate)
    updateDTransMetadata(KV.first, KV.second);

  // Process the instructions using dependent structure types.
  for (auto *GEP : FuncInfo->DepGEPsToConvert)
    convertDepGEP(GEP);

  for (auto &GEPInfo : FuncInfo->DepByteGEPsToConvert)
    convertDepByteGEP(
        std::get<PerFunctionInfo::ByteGEPInfoGEPMember>(GEPInfo),
        std::get<PerFunctionInfo::ByteGEPInfoTypeMember>(GEPInfo),
        std::get<PerFunctionInfo::ByteGEPInfoFieldMember>(GEPInfo));

  for (auto &Alloc : FuncInfo->DepAllocsToResize)
    convertDepAllocCall(Alloc.first, Alloc.second);

  for (auto &MemCall : FuncInfo->DepMemfuncsToResize)
    convertDepMemfuncCall(MemCall.first, MemCall.second);

  for (auto &KV : FuncInfo->DepBinOpsToConvert)
    convertDepBinaryOperator(KV.first, KV.second);

  for (auto *Call : FuncInfo->CallsToConvert)
    updateCallAttributes(Call);

  // Convert 'ptr' objects to be 'ptr addrspace(n)' objects so the type
  // remapping will recognize them.
  for (auto KV : FuncInfo->InstructionsToMutate)
    if (auto *Call = dyn_cast<CallInst>(KV.first))
      Call->mutateFunctionType(cast<FunctionType>(KV.second));
    else
      KV.first->mutateType(KV.second);

  // Note, this needs to be done before instruction deletion, because there
  // could be an 'icmp' marked for deletion by the allocation conversion that is
  // on the list of instructions to be deleted.
  for (auto &IOT : FuncInfo->ConstantsToReplace) {
    Instruction *I = std::get<0>(IOT);
    uint32_t OpNum = std::get<1>(IOT);
    llvm::PointerType *Ty = std::get<2>(IOT);
    I->setOperand(OpNum, ConstantPointerNull::get(Ty));
  }

  // When opaque pointers are in use, we need to also mutate the types of any
  // incoming opaque pointer arguments that are going to have their uses
  // converted into integers.
  if (Function *CloneFn = getClonedFunction(&F)) {
    auto *OrigFnType = cast<llvm::FunctionType>(F.getValueType());
    auto *CloneFnType = cast<llvm::FunctionType>(CloneFn->getValueType());
    DTransFunctionType *DFnTy = dyn_cast<DTransFunctionType>(
        DTInfo->getTypeMetadataReader().getDTransTypeFromMD(&F));
    assert(DFnTy && "DTransType should exist for any function being cloned");
    unsigned NumArgs = F.arg_size();
    for (unsigned ArgIdx = 0; ArgIdx < NumArgs; ++ArgIdx) {
      llvm::Type *OrigArgTy = OrigFnType->getParamType(ArgIdx);
      llvm::Type *CloneArgTy = CloneFnType->getParamType(ArgIdx);
      if (OrigArgTy->isPointerTy() && !CloneArgTy->isPointerTy()) {
        DTransType *DTArgTy = DFnTy->getArgType(ArgIdx);
        assert(DTArgTy->isPointerTy() &&
               DTArgTy->getPointerElementType()->isStructTy() &&
               "Only pointers to structures should be changed to int types");
        if (OrigArgTy->isOpaquePointerTy()) {
          auto *StructTy =
              cast<DTransStructType>(DTArgTy->getPointerElementType());
          llvm::Type *TypeInAddrSpace = getAddrSpacePtrForType(StructTy);
          Argument *A = F.getArg(ArgIdx);
          A->mutateType(TypeInAddrSpace);
        }
      }
    }
  }

  for (auto *I : FuncInfo->InstructionsToDelete)
    I->eraseFromParent();

  FuncInfoProcessFunctionComplete();
  DEBUG_WITH_TYPE(AOSTOSOA_VERBOSE,
                  dbgs() << "\nIR Before type-remapping:\n"
                         << F
                         << "---------------------------------------------\n");
}

// Return 'true' if Instruction 'I' is a conversion instruction of the type
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

void AOSToSOAOPTransformImpl::postprocessFunction(Function &OrigFunc,
                                                  bool IsCloned) {
  LLVM_DEBUG(dbgs() << "AOS-to-SOA: postprocessFunction: " << OrigFunc.getName()
                    << "\n");

  Function *Func = &OrigFunc;
  if (IsCloned) {
    Func = cast<Function>(VMap[&OrigFunc]);
    updateFunctionAttributes(OrigFunc, *Func);
  }

  DTransType *DTy =
      DTInfo->getTypeMetadataReader().getDTransTypeFromMD(&OrigFunc);
  if (DTy) {
    // Reset the !intel.dtrans.func_type metadata and 'intel_dtrans_func_index'
    // attributes on the function signature to reflect the new function type.
    // (If the metadata was not there before, then there is no need to update it
    // because no new pointer types will be created on a signature that did not
    // have them before.) This is necessary for the AOS-to-SOA transformation
    // due the way pointers are converted to be integer indices, and
    // pointer-to-pointer types being converted to just be a pointer type. The
    // pointer types are not stored directly, but rather store as a type and
    // level of indirection causing special handling that cannot be handled by
    // the type remapper to update the level of indirection.
    //
    // For example:
    //   define "intel_dtrans_func_index"="1" ptr @test01(
    //     ptr "intel_dtrans_func_index"="2" %in1
    //     ptr "intel_dtrans_func_index"="3" %in2) !intel.dtrans.func_type !4
    //   !1 = { %struct.test01 zeroinitializer, i32 1 } ; %struct.test01*
    //   !2 = { %struct.test01 zeroinitializer, i32 2 } ; %struct.test01**
    //   !3 = { %struct.test01dep zeroinitializer, i32 1 } ; %struct.test01dep*
    //   !4 = distinct !{!1, !2, !3}               ; list of type encodings.
    //
    // If we are transforming %struct.test01, then this will become:
    //   define i64 @test01(
    //      ptr "intel_dtrans_func_index"="1" %in1,
    //      ptr "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type !3
    //   !4 = distinct !{!2, !3}               ; list of type encodings.
    //   !2 = { i64 0, i32 1 } ; i64*
    //   !3 = { %__SOADT_struct.test01dep zeroinitializer, i32 1 }
    //      ; %__SOADTstruct.test01dep*
    //
    auto *DReplTy = cast<DTransFunctionType>(TypeRemapper.remapType(DTy));
    DTransTypeMetadataBuilder::setDTransFuncMetadata(Func, DReplTy);
  }

  DEBUG_WITH_TYPE(AOSTOSOA_VERBOSE,
                  dbgs() << "\nIR after type-remapping:\n"
                         << *Func
                         << "---------------------------------------------\n");

  // We need to get rid of all the cast instructions that were inserted to help
  // the type remapping because they are not valid after the type remapping
  // changed a pointer type to be an integer type.
  SmallPtrSet<Instruction *, 16> InstructionsToDelete;
  for (auto *Conv : FuncInfo->PtrConverts) {
    if (IsCloned)
      Conv = cast<CastInst>(VMap[Conv]);
    if (Conv->user_empty()) {
      InstructionsToDelete.insert(Conv);
      continue;
    }

    LLVM_DEBUG(dbgs() << "Post process deleting: " << *Conv << "\n");
    assert(Conv->getType() == Conv->getOperand(0)->getType() &&
           "Expected self-type in cast after remap");
    Conv->replaceAllUsesWith(Conv->getOperand(0));
    InstructionsToDelete.insert(Conv);
  }

  // Check for cast instructions generated that cancel out another cast.
  // These instructions would have eventually been removed by the instcombine
  // pass, but we cannot run instcombine between transformations now because
  // it would produce other IR instruction patterns that are not currently
  // recognized by DTransAnalysis, such as shift instructions.
  for (auto *Conv : FuncInfo->IntermediateConverts) {
    if (IsCloned)
      Conv = cast<Instruction>(VMap[Conv]);

    bool NotNeeded = isCancellingConvert<IntToPtrInst, PtrToIntInst>(Conv) ||
                     isCancellingConvert<PtrToIntInst, IntToPtrInst>(Conv) ||
                     isCancellingConvert<ZExtInst, TruncInst>(Conv);
    if (NotNeeded) {
      Instruction *SrcOperand = cast<Instruction>(Conv->getOperand(0));
      Conv->replaceAllUsesWith(SrcOperand->getOperand(0));
      InstructionsToDelete.insert(Conv);

      // Check if the source is now dead, but defer deletion
      // until processing all the elements of this loop, in case
      // the instruction is contained in the vector being iterated.
      if (SrcOperand->user_empty())
        InstructionsToDelete.insert(SrcOperand);
    }
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

bool AOSToSOAOPTransformImpl::isFnClonedForIndex(Function *F) const {
  return FnClonedForIndex.count(F) != 0;
}

Function *AOSToSOAOPTransformImpl::getClonedFunction(Function *F) const {
  return OrigFuncToCloneFuncMap.lookup(F);
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
  IndexInfo.IncompatibleTypeAttrs.merge(
      AttributeFuncs::typeIncompatible(IndexInfo.LLVMType));
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
    FuncInfo->PtrConverts.insert(ArrayIdx);
    GEP->replaceAllUsesWith(ArrayIdx);

    // When opaque pointers are in use, we need the pointer to be in an
    // address space to recognize it during type remapping.
    llvm::Type *TypeInAddrSpace = getAddrSpacePtrForType(OrigStructTy);
    if (TypeInAddrSpace)
      FuncInfo->InstructionsToMutate.push_back({ArrayIdx, TypeInAddrSpace});
    FuncInfo->InstructionsToDelete.insert(GEP);
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
    FuncInfo->PtrConverts.insert(CastToPtr);
    ReplVal = CastToPtr;
  }

  GEP->replaceAllUsesWith(ReplVal);
  FuncInfo->InstructionsToDelete.insert(GEP);
}

// The byte-flattened GEP needs to be transformed from getting the address as:
//    %p8_B = getelementptr i8, i8* %p, i64 8
// (Assume field 1 is an i32 that is offset 8 bytes from the start of the
// structure)
//
// To:
//    %indexAsInt = ptrtoint %struct.test01* %p to i64
//    %soaField = getelementptr % __soa_struct.t,
//                              %__soa_struct.t* @__soa_struct.t, i64 0, i32 1
//    %soaAddr = load i32*, i32** %soaField
//    %elementAddr = getelementptr i32, i32* %soaAddr, i64 %indexAsInt
//    %p8_B = bitcast i32* %elementAddr to i8*
void AOSToSOAOPTransformImpl::convertByteGEP(GetElementPtrInst *GEP,
                                             DTransStructType *OrigStructTy,
                                             size_t FieldNum) {
  LLVM_DEBUG(dbgs() << "Replacing byte flattened GEP for field "
                    << *OrigStructTy << "@" << FieldNum << ":\n  " << *GEP
                    << "\n");

  auto *OrigLLVMStructTy = cast<llvm::StructType>(OrigStructTy->getLLVMType());
  struct SOATypeInfoTy &SOAInfo = getSOATypeInfo(OrigLLVMStructTy);

  // Trace the pointer operand to find the base value that is needed
  // for indexing into the field's array.
  Value *IndexAsInt =
      getIndexForValue(GEP->getPointerOperand(), OrigLLVMStructTy);

  StructType *SOAType = SOAInfo.SOAStructType;
  Value *FieldNumValue =
      ConstantInt::get(Type::getInt32Ty(GEP->getContext()), FieldNum);
  Instruction *FieldGEP = createGEPFieldAddressReplacement(
      SOAInfo, IndexAsInt, ConstantInt::get(PtrSizeIntLLVMType, 0),
      FieldNumValue, GEP);

  llvm::Type *FieldTy = SOAType->getElementType(FieldNum);
  if (FieldTy != GEP->getType())
    FieldGEP =
        CastInst::CreateBitOrPointerCast(FieldGEP, GEP->getType(), "", GEP);

  FieldGEP->takeName(GEP);
  GEP->replaceAllUsesWith(FieldGEP);
  FuncInfo->InstructionsToDelete.insert(GEP);
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
  llvm::StructType *ReplTy = getDependentTypeReplacement(
      cast<llvm::StructType>(OrigStructTy->getLLVMType()));
  const StructLayout *SL = DL.getStructLayout(cast<StructType>(ReplTy));
  uint64_t NewOffset = SL->getElementOffset(FieldNum);
  GEP->setOperand(1,
                  ConstantInt::get(GEP->getOperand(1)->getType(), NewOffset));
}

// Modify an allocation that was used to allocate a pointer to the type being
// transformed to be an allocation of the peeling index type, which is an
// integer type. For example, when 'ptr' represents a pointer to the type being
// transformed:
//   %local = alloca ptr
// becomes:
//   %local = alloca i32
//
// Note: This is only reachable when opaque pointers are used because the code
// relies on the TypeRemapper to handle this when typed pointers are in use.
void AOSToSOAOPTransformImpl::convertAlloca(AllocaInst *AI) {
  assert(AI->getType()->isOpaquePointerTy() &&
         "Only opaque pointer uses expected");
  assert(!AI->getAllocatedType()->isArrayTy() && "Unexpected array type");

  AI->setAllocatedType(IndexInfo.LLVMType);
}

// A load of a pointer to the structure done as a pointer sized integer load
// needs to handle the case where the index bit width does not match the
// size of the pointer bit width.
void AOSToSOAOPTransformImpl::convertPtrSizedIntLoad(
    LoadInst *LI, DTransStructType *DTransTy) {
  if (!IndexInfo.PointerShrinkingEnabled)
    return;

  // Replace a load of the form:
  //    %val_i64 = load i64, i64* %p_i64
  //
  // To be one of the following forms:
  // (non-opaque pointers)
  //    %1 = bitcast i64* %p_i64 to i32*
  //    %2 = load i32, i32* %1
  //    %val_i64 = zext i32 %2 to i64
  //
  // (opaque pointers)
  //    %2 = load i32, ptr %1
  //    %val_i64 = zext i32 %2 to i64
  //
  Value *PtrOp = LI->getPointerOperand();
  if (!PtrOp->getType()->isOpaquePointerTy()) {
    auto *NewPtrOp = CastInst::CreateBitOrPointerCast(
        PtrOp, IndexInfo.LLVMType->getPointerTo(), "", LI);
    FuncInfo->IntermediateConverts.push_back(NewPtrOp);
    PtrOp = NewPtrOp;
  }

  // Create a new load with the same attributes as the original instruction,
  // except for the alignment field. Because pointers to the structure are
  // being changed to an integer the original alignment may no longer be
  // valid, so set it to the ABI default for the type that the load will be
  // once type remapping occurs.
  Align Alignment = DL.getABITypeAlign(IndexInfo.LLVMType);
  Instruction *NewLI =
      new LoadInst(IndexInfo.LLVMType, PtrOp, "", LI->isVolatile(), Alignment,
                   LI->getOrdering(), LI->getSyncScopeID(), LI);
  Instruction *Repl = CastInst::Create(CastInst::ZExt, NewLI, LI->getType(), "", LI);
  LI->replaceAllUsesWith(Repl);
  Repl->takeName(LI);
  FuncInfo->IntermediateConverts.push_back(Repl);
  FuncInfo->InstructionsToDelete.insert(LI);

  FuncInfo->InstructionsToAnnotate.push_back(
      {NewLI, cast<llvm::StructType>(DTransTy->getLLVMType())});
}

// A store of a pointer to the structure done as a pointer sized integer store
// needs to handle the case where the index bit width does not match the
// size of the pointer bit width.
void AOSToSOAOPTransformImpl::convertPtrSizedIntStore(
    StoreInst *SI, DTransStructType *DTransTy) {
  if (!IndexInfo.PointerShrinkingEnabled)
    return;

  // Replace a store of the form:
  //    store i64 %val_i64, i64* %p_i64
  //
  // To be one of the following forms:
  // (non-opaque pointers)
  //    %1 = trunc i64 %val_i64 to i32
  //    %2 = bitcast i32* %p_i64 to i32*
  //    store i32 %1, i32* %2
  //
  // (opaque pointers)
  //    %1 = trunc i64 %val_i64 to i32
  //    store i32 %1,ptr %2
  Instruction *NewValOp = CastInst::Create(
      CastInst::Trunc, SI->getValueOperand(), IndexInfo.LLVMType, "", SI);
  FuncInfo->IntermediateConverts.push_back(NewValOp);

  Value *PtrOp = SI->getPointerOperand();
  Value *NewPtrOp = nullptr;
  if (auto *C = dyn_cast<Constant>(PtrOp))
    NewPtrOp = ConstantExpr::getBitCast(C, IndexInfo.LLVMType->getPointerTo());
  else {
    NewPtrOp = CastInst::CreateBitOrPointerCast(
      PtrOp, IndexInfo.LLVMType->getPointerTo(), "", SI);
    FuncInfo->IntermediateConverts.push_back(cast<Instruction>(NewPtrOp));
  }
  // Create a new store with the same attributes as the original instruction,
  // except for the alignment field. Because the field type in the structure
  // is changing, the original alignment may no longer be valid, so set it
  // to the ABI default for the type that the load will be once type remapping
  // occurs.
  Align Alignment = DL.getABITypeAlign(IndexInfo.LLVMType);
  Instruction *NewSI =
      new StoreInst(NewValOp, NewPtrOp, SI->isVolatile(), Alignment,
                    SI->getOrdering(), SI->getSyncScopeID(), SI);
  FuncInfo->InstructionsToDelete.insert(SI);

  FuncInfo->InstructionsToAnnotate.push_back(
      {NewSI, cast<llvm::StructType>(DTransTy->getLLVMType())});
}

void AOSToSOAOPTransformImpl::convertBC(BitCastInst *BC,
                                        DTransStructType *StructTy) {
  // The bitcast should no longer be needed after processing other instructions.
  if (!hasLiveUser(BC)) {
    LLVM_DEBUG(dbgs() << "Deleting bitcast: " << *BC << "\n");
    FuncInfo->InstructionsToDelete.insert(BC);
    return;
  }

  llvm_unreachable("Unexpected live Bitcast");
}

// Return 'true' if any instruction that uses the result of 'I' is not in the
// list of instructions to be deleted as part of processing the current
// function.
bool AOSToSOAOPTransformImpl::hasLiveUser(Instruction *I) {
  for (auto *U : I->users())
    if (auto *Inst = dyn_cast<Instruction>(U))
      if (!FuncInfo->InstructionsToDelete.contains(Inst))
        return true;

  return false;
}

void AOSToSOAOPTransformImpl::convertPtrToInt(PtrToIntInst *I,
                                              DTransStructType *StructTy) {
  // An instruction of the form:
  //   ptrtoint ptr addrspace(1) %x to i64
  // will be remapped into either:
  //   ptrtoint i64 %x to i64
  // or
  //   ptrtoint i32 %x to i64
  // based on the value of 'PointerShrinkingEnabled'
  if (!IndexInfo.PointerShrinkingEnabled) {
    // The form: ptrtoint i64 %x to i64
    // will be a meaningless cast, and should be removed during post processing.
    FuncInfo->PtrConverts.insert(I);
    return;
  }

  // When shrinking is enabled, we need to create a replacement
  // sequence to prepare for the type remapping, while maintaining the
  // uses of the instruction as i64 types by converting it into:
  //   %1 = ptrtoint ptr addrspace(1) %x to i32
  //   %2 = zext i32 %1 to i64
  //
  // After type remapping, the ptrtoint replacement will be:
  //   ptrtoint i32 to i32
  // which can be removed during post processing.
  CastInst *NewPTI = CastInst::CreateBitOrPointerCast(
      I->getPointerOperand(), IndexInfo.LLVMType, "", I);
  auto *ZExt = CastInst::Create(CastInst::ZExt, NewPTI, I->getType(), "", I);
  I->replaceAllUsesWith(ZExt);
  ZExt->takeName(I);

  LLVM_DEBUG(dbgs() << "After convert:\n  " << *NewPTI << "\n  " << *ZExt
                    << "\n");

  FuncInfo->InstructionsToDelete.insert(I);
  FuncInfo->PtrConverts.insert(NewPTI);
}

void AOSToSOAOPTransformImpl::convertBinaryOperator(
    BinaryOperator *I, DTransStructType *StructTy) {
  // The analysis phase has resolved that the use of this subtract instruction
  // is used to divide by the structure size or some multiple of it.
  // Because the pointer to the structure has been converted to be an
  // integer index, the result of the subtract is the distance between
  // the pointers that the divide was going to compute when the divisor
  // equaled the structure size. Update the divide instruction to replace the
  // divisor.
  uint64_t OrigSize = DL.getTypeAllocSize(StructTy->getLLVMType());
  dtrans::updatePtrSubDivUserSizeOperand(I, OrigSize, 1);
}

// The allocation call for a type being transformed into a structure of arrays
// needs to be converted to initialize the SOA variable that stores the
// addresses of the start of each array.
//
// For example: struct t1 { int a, b, c};
// will have the new global structure: struct __AOS_t1 { int *a, *b, *c };
// This routine initializes the pointers of the global variable for a, b, c to
// point to the appropriate location of the allocated block of memory.
//
// The size of the allocation, and uses of the result of the allocation call
// will also be updated within the function.
//
void AOSToSOAOPTransformImpl::convertAllocCall(AllocCallInfo *AInfo,
                                               StructInfo *StInfo) {
  auto *AllocCallInst = cast<CallInst>(AInfo->getInstruction());
  LLVM_DEBUG(dbgs() << "Updating allocation call: " << *AllocCallInst << "\n");

  StructType *OrigStructTy = cast<StructType>(StInfo->getLLVMType());
  uint64_t StructSize = DL.getTypeAllocSize(OrigStructTy);
  struct SOATypeInfoTy &SOAInfo = getSOATypeInfo(OrigStructTy);
  StructType *SOAStructType = SOAInfo.SOAStructType;
  GlobalVariable *SOAVar = SOAInfo.SOAVar;

  // Set up an IR builder to insert instructions starting before the
  // allocation statement. The IR builder will perform constant
  // folding when the allocation count is a constant when computing new
  // allocation sizes or offsets into the allocated memory block.
  IRBuilder<> IRB(AllocCallInst);

  // This will store the adjusted number of elements allocated by the call.
  Value *NewAllocCountVal = nullptr;

  dtrans::AllocKind Kind = AInfo->getAllocKind();
  unsigned OrigAllocSizeInd = 0;
  unsigned OrigAllocCountInd = 0;
  const TargetLibraryInfo &TLI = GetTLI(*AllocCallInst->getFunction());
  getAllocSizeArgs(Kind, AllocCallInst, OrigAllocSizeInd, OrigAllocCountInd,
                   TLI);

  auto *OrigAllocSizeVal = AllocCallInst->getArgOperand(OrigAllocSizeInd);
  if (Kind == dtrans::AK_Malloc) {
    assert(OrigAllocSizeVal && "getAllocSizeArgs should return size value");

    // Compute the number of elements being allocated.
    llvm::Type *SizeType = OrigAllocSizeVal->getType();
    Value *StructSizeVal = ConstantInt::get(SizeType, StructSize);
    NewAllocCountVal = IRB.CreateSDiv(OrigAllocSizeVal, StructSizeVal);

    // Update the size allocated to hold one additional structure element.
    // This is necessary because an index value of 0 will represent
    // the nullptr, and the array accesses will be in the range of 1..N.
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
    if (dtrans::isValueEqualToSize(OrigAllocSizeVal, StructSize)) {
      AllocCountVal = OrigAllocCountVal;
      AllocSizeVal = OrigAllocSizeVal;
    } else if (dtrans::isValueEqualToSize(OrigAllocCountVal, StructSize)) {
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

  // Pointers in the structure that corresponded to the base allocation
  // address will be accessed via index 1 in the new structure. Update the
  // users of the allocation to refer to index 1. This loop collects the
  // values to be updated to avoid changing the users while walking them.
  SmallVector<ICmpInst *, 2> ICmpsToFix;
  SmallVector<StoreInst *, 2> StoresToFix;
  SmallVector<BitCastInst *, 2> BitCastsToFix;
  SmallVector<GetElementPtrInst *, 16> GepsToFix;
  for (auto *User : AllocCallInst->users()) {
    if (auto *U = dyn_cast<Instruction>(&*User)) {
      if (auto *ICmp = dyn_cast<ICmpInst>(U)) {
        ICmpsToFix.push_back(ICmp);
      } else if (auto *SI = dyn_cast<StoreInst>(U)) {
        // We expect the value operand to have been the use, because writing
        // to the allocated memory pointer should have set the safety flags
        // that inhibit the transformation.
        assert(SI->getValueOperand() == AllocCallInst &&
               "Expected allocation result to be stored value");
        StoresToFix.push_back(SI);
      } else if (auto *BC = dyn_cast<BitCastInst>(U)) {
        BitCastsToFix.push_back(BC);
      } else if (auto *GEP = dyn_cast<GetElementPtrInst>(U)) {
        GepsToFix.push_back(GEP);
      } else {
        llvm_unreachable("Unexpected instruction using allocation result");
      }
    } else {
      llvm_unreachable("Unexpected use of allocation result");
    }
  }

  // To support the typed-pointer form, also check for a use of a bitcast of the
  // allocation against a nullptr, to keep that as a null-pointer check.
  //   %mem = call i8* @calloc(i64 10, i64 24)
  //   %st = bitcast i8* %mem to %struct.test01*
  //   %success2 = icmp eq %struct.test01* %st, null
  for (auto *BC : BitCastsToFix)
    for (auto *U : BC->users())
      if (auto *ICmp = dyn_cast<ICmpInst>(U))
        if (isa<ConstantPointerNull>(ICmp->getOperand(0)) ||
            isa<ConstantPointerNull>(ICmp->getOperand(1)))
          ICmpsToFix.push_back(ICmp);

  for (auto *ICmp : ICmpsToFix) {
    // For the ICmp, this was originally a direct use of the allocation function
    // call result, but the Value object is inferred as being a pointer to the
    // transformed type. We want to keep this as a pointer comparison to null,
    // and not rewrite it as an integer comparison.
    //   %56 = call ptr @calloc(i64 %num, i64 %size)
    //   %cmp = icmp eq ptr %56, null
    auto *NewICmp = ICmpInst::Create(
        Instruction::ICmp, ICmpInst::ICMP_EQ, AllocCallInst,
        ConstantPointerNull::get(cast<PointerType>(AllocCallInst->getType())),
        "", ICmp);
    NewICmp->takeName(ICmp);
    ICmp->replaceAllUsesWith(NewICmp);
    FuncInfo->InstructionsToDelete.insert(ICmp);
  }

  // Update the stored value to be an index value of 1.
  // We will use a special address space on the pointer when opaque pointers are
  // in use, so that the type remapping will recognize it and change it back
  // into the index type, so that the casts become no-ops.
  uint32_t AddrSpace = getAddrSpaceForType(OrigStructTy);
  llvm::Type *TypeInAddrSpace = PointerType::get(OrigStructTy, AddrSpace);
  for (auto *SI : StoresToFix) {
    CastInst *IndexAsPtr = CastInst::CreateBitOrPointerCast(
        ConstantInt::get(IndexInfo.LLVMType, 1), TypeInAddrSpace);
    IndexAsPtr->insertBefore(SI);
    SI->setOperand(0, IndexAsPtr);
    FuncInfo->PtrConverts.insert(IndexAsPtr);
  }

  // Replace the BitCast instructions with an IntToPtr instruction that casts
  // index 1 to the original pointer type. After the type remapping
  // completes, the destination type of the cast will be the index
  // type, and post processing will remove the instruction.
  for (auto *BC : BitCastsToFix) {
    CastInst *IndexAsPtr = CastInst::CreateBitOrPointerCast(
        ConstantInt::get(IndexInfo.LLVMType, 1), BC->getType());
    IndexAsPtr->insertBefore(BC);
    BC->replaceAllUsesWith(IndexAsPtr);
    FuncInfo->InstructionsToDelete.insert(BC);
    FuncInfo->PtrConverts.insert(IndexAsPtr);

    // Change the type IntToPtr instruction so that it will be recognized as
    // needing to be updated during type remapping.
    IndexAsPtr->mutateType(TypeInAddrSpace);
  }

  // When opaque pointers are in use, the result of the allocation will be used
  // directly in the GEP instructions without being bitcast. Because the
  // allocated elements will be accessed by index following the transformation,
  // change the GEP to use an index value. When the GEPs are converted, these
  // will be updated to access the element from the SOA variable.
  for (auto *GEP : GepsToFix) {
    CastInst *IndexAsPtr = CastInst::CreateBitOrPointerCast(
        ConstantInt::get(IndexInfo.LLVMType, 1),
        PointerType::get(GEP->getSourceElementType(), AddrSpace));
    IndexAsPtr->insertBefore(GEP);
    GEP->setOperand(0, IndexAsPtr);
    FuncInfo->PtrConverts.insert(IndexAsPtr);
  }

  // Initialize the pointer fields of the SOA structure to store an
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
  LLVMContext &Ctx = AllocCallInst->getContext();
  DTransStructType *DTransStructTy = SOAInfo.DTransSOAStructType;
  IRB.SetInsertPoint(AllocCallInst->getNextNode());
  unsigned int NumElements = SOAStructType->getNumElements();
  for (unsigned FieldNum = 0; FieldNum < NumElements; ++FieldNum) {
    Type *ArrayElemType = SOAInfo.LLVMStructRemappedFieldsTypes[FieldNum];
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
        Value *Div =
            IRB.CreateSDiv(Numerator, ConstantInt::get(ArithType, FieldAlign));
        AddrOffset =
            IRB.CreateMul(Div, ConstantInt::get(ArithType, FieldAlign));
      }
    }

    // Compute the address in the memory block where the array for this field
    // will begin:
    //   %BlockAddr = getelementptr i8, ptr %AllocCallInst, i64 %Offset
    Value *BlockAddr =
        IRB.CreateGEP(Type::getInt8Ty(Ctx), AllocCallInst, AddrOffset);

    // Annotate the GEP into the allocated block to allow subsequent runs
    // of DTrans passes to resolve the type as a pointer to an array of
    // elements. This is necessary because the allocated memory block is
    // being partitioned into multiple arrays.
    unsigned PtrLevel = 0;
    DTransType *DTransFieldType = DTransStructTy->getFieldType(FieldNum);
    assert(DTransFieldType && "Invalid DTrans structure type");
    DTransType *BaseDTransType = DTransFieldType;
    while (BaseDTransType->isPointerTy()) {
      ++PtrLevel;
      BaseDTransType = BaseDTransType->getPointerElementType();
    }
    DTransAnnotator::createDTransTypeAnnotation(
        *cast<Instruction>(BlockAddr), BaseDTransType->getLLVMType(), PtrLevel);

    // Cast to the pointer type that will be stored to support non-opaque
    // pointers:
    //   %CastToMemberTy = bitcast i8* %BlockAddr to %SOAFieldType
    Type *SOAFieldType = SOAStructType->getElementType(FieldNum);
    Value *CastToMemberTy = IRB.CreateBitCast(BlockAddr, SOAFieldType);

    // Get the address in the global structure for the field to be stored:
    //   %FieldAddr = getelementptr
    //                   %SOAVarType, ptr %SOAVar, i64 0, i32 %FieldNum
    LLVMContext &Ctx = SOAStructType->getContext();
    Value *Idx[2];
    Idx[0] = Constant::getNullValue(Type::getInt64Ty(Ctx));
    Idx[1] = ConstantInt::get(Type::getInt32Ty(Ctx), FieldNum);
    Value *FieldAddr = IRB.CreateGEP(SOAStructType, SOAVar, Idx);

    // Save the pointer to the global structure field.
    //   store %CastToMemberTy, %FieldAddr
    IRB.CreateStore(CastToMemberTy, FieldAddr);

    PrevArrayElemType = ArrayElemType;
  }

  // Annotate the allocation call for other the DTrans dynamic cloning
  // optimization.
  Module *M = AllocCallInst->getModule();
  auto *Annot = DTransAnnotator::createPtrAnnotation(
      *M, *AllocCallInst, *SOAInfo.AllocAnnotationGEP, *AnnotationFilenameGEP,
      0, "annot_alloc", nullptr);
  Annot->insertAfter(AllocCallInst);

  LLVM_DEBUG(dbgs() << "Adding annotation for allocation: " << *AllocCallInst
                    << "\n  : " << *Annot << "\n");
}

// The transformation of the call to free needs to change the parameter
// passed to 'free' to be the address stored within our SOA global variable.
void AOSToSOAOPTransformImpl::convertFreeCall(FreeCallInfo *CInfo,
                                              StructInfo *StInfo) {
  auto CollectNullChecks = [](Value *V,
                              SmallVectorImpl<ICmpInst *> &CheckInsts) {
    for (auto *User : V->users())
      if (auto *U = dyn_cast<Instruction>(&*User))
        if (auto *ICmp = dyn_cast<ICmpInst>(U))
          if (isa<ConstantPointerNull>(ICmp->getOperand(0)) ||
              isa<ConstantPointerNull>(ICmp->getOperand(1)))
            CheckInsts.push_back(ICmp);
  };

  Instruction *FreeCall = CInfo->getInstruction();
  assert(FreeCall && isa<CallInst>(FreeCall) &&
         !cast<CallInst>(FreeCall)->isIndirectCall() &&
         "Instruction should be direct function call");

  unsigned PtrArgInd = -1U;
  const TargetLibraryInfo &TLI = GetTLI(*FreeCall->getFunction());
  getFreePtrArg(CInfo->getFreeKind(), cast<CallInst>(FreeCall), PtrArgInd, TLI);
  Value *FreeArg = FreeCall->getOperand(PtrArgInd);
  auto *FreeArgInst = dyn_cast<Instruction>(FreeArg);
  // The type being transformed cannot have a global instance, so the argument
  // to free it must be defined by an Instruction, and not a global constant.
  assert(FreeArgInst && "FreeArg must be instruction");

  // If there is a null pointer test of the free call argument, we want to find
  // it, and substitute the address of the memory stored in the SOA variable
  // created by the allocation conversion.
  Instruction *InsertionPoint = FreeArgInst;
  SmallVector<ICmpInst *, 2> ICmpsToFix;
  CollectNullChecks(FreeArgInst, ICmpsToFix);
  if (auto *BC = dyn_cast<BitCastInst>(FreeArgInst)) {
    auto *BCOpInst = dyn_cast<Instruction>(BC->getOperand(0));
    assert(BCOpInst && "FreeArg bitcast must be instruction");
    CollectNullChecks(BCOpInst, ICmpsToFix);
    InsertionPoint = BCOpInst;
  }

  // To free the transformed data structure, we need to get the address
  // stored in the first field of the global variable, and pass that
  // to free.
  StructType *OrigStructTy = cast<StructType>(StInfo->getLLVMType());
  struct SOATypeInfoTy &SOAInfo = getSOATypeInfo(OrigStructTy);

  GlobalVariable *SOAVar = SOAInfo.SOAVar;
  Instruction *SOAAddr = createSOAFieldLoad(
      SOAInfo, ConstantInt::get(Type::getInt32Ty(SOAVar->getContext()), 0),
      InsertionPoint);
  Value *NewFreeArg = SOAAddr;
  if (!FreeArg->getType()->isOpaquePointerTy()) {
    CastInst *SOAAddrAsI8Ptr =
        CastInst::CreateBitOrPointerCast(SOAAddr, FreeArg->getType());
    SOAAddrAsI8Ptr->insertAfter(SOAAddr);
    NewFreeArg = SOAAddrAsI8Ptr;
  }

  for (auto *ICmp : ICmpsToFix) {
    // The original 'icmp' instruction may have been collected by the visitICmp
    // instruction to convert to an integer index test, so insert a new ICmp,
    // instead of changing the operands.
    auto *NewICmp = ICmpInst::Create(
        Instruction::ICmp, ICmpInst::ICMP_EQ, NewFreeArg,
        ConstantPointerNull::get(cast<PointerType>(NewFreeArg->getType())), "",
        ICmp);
    NewICmp->takeName(ICmp);
    ICmp->replaceAllUsesWith(NewICmp);
    FuncInfo->InstructionsToDelete.insert(ICmp);
  }

  LLVM_DEBUG(dbgs() << "AOS-to-SOA updating free call: " << *FreeCall << "\n");
  FreeCall->setOperand(PtrArgInd, NewFreeArg);
  LLVM_DEBUG(dbgs() << "                           to: " << *FreeCall << "\n");
}

void AOSToSOAOPTransformImpl::convertDepAllocCall(AllocCallInfo *AInfo,
                                                  StructInfo *StInfo) {
  llvm::StructType *OrigTy = cast<llvm::StructType>(StInfo->getLLVMType());
  llvm::StructType *ReplTy = getDependentTypeReplacement(OrigTy);
  const TargetLibraryInfo &TLI =
      GetTLI(*AInfo->getInstruction()->getFunction());
  dtrans::updateCallSizeOperand(AInfo->getInstruction(), AInfo, OrigTy, ReplTy,
                                TLI);
}

void AOSToSOAOPTransformImpl::convertDepMemfuncCall(MemfuncCallInfo *CInfo,
                                                    StructInfo *StInfo) {
  assert(CInfo->getIsCompleteAggregate(0) &&
         "Partial memfuncs currently not supported for dependent structure "
         "types");

  llvm::StructType *OrigTy = cast<llvm::StructType>(StInfo->getLLVMType());
  llvm::StructType *ReplTy = getDependentTypeReplacement(OrigTy);
  const TargetLibraryInfo &TLI =
      GetTLI(*CInfo->getInstruction()->getFunction());
  dtrans::updateCallSizeOperand(CInfo->getInstruction(), CInfo, OrigTy, ReplTy,
                                TLI);
}

void AOSToSOAOPTransformImpl::convertDepBinaryOperator(
    BinaryOperator *I, DTransStructType *StructTy) {
  auto *OrigLLVMTy = dyn_cast<llvm::StructType>(StructTy->getLLVMType());
  assert(OrigLLVMTy && "Failed to get LLVMType for DTransStructTy");
  llvm::Type *ReplTy = getDependentTypeReplacement(OrigLLVMTy);
  dtrans::updatePtrSubDivUserSizeOperand(I, OrigLLVMTy, ReplTy, DL);
}

void AOSToSOAOPTransformImpl::updateDTransMetadata(Instruction *I,
                                                   DTransType *Ty) {
  // Clear or update the new DTrans metadata type node based on whether the
  // allocation is still allocating a pointer after the transformation.
  DTransType *NewTy = TypeRemapper.remapType(Ty);
  MDNode *NewMD =
      hasPointerType(NewTy) ? NewTy->createMetadataReference() : nullptr;
  DTransTypeMetadataBuilder::addDTransMDNode(*I, NewMD);
}

void AOSToSOAOPTransformImpl::updateCallAttributes(CallBase *Call) {
  LLVM_DEBUG(dbgs() << "AOS-to-SOA: Checking call attributes for: " << *Call
                    << "\n");

  Value *CallOp = Call->getCalledOperand();
  Function *Callee = dyn_cast<Function>(CallOp->stripPointerCasts());
  assert(Callee && "Expect to identify called function");
  auto *OrigFnType = cast<llvm::FunctionType>(Callee->getValueType());
  Function *CalleeClone = getClonedFunction(Callee);
  assert(CalleeClone &&
         "updateCallAttributes only needed for functions being cloned");
  auto *CloneFnType = cast<llvm::FunctionType>(CalleeClone->getValueType());
  AttributeList Attrs = Call->getAttributes();
  if (updateAttributeList(OrigFnType, CloneFnType, Attrs))
    Call->setAttributes(Attrs);

  LLVM_DEBUG(dbgs() << "AOD-to-SOA: After call update: " << Call << "\n");
}

// Remove any attributes on the function return type or parameters that are not
// compatible with changes made to convert a pointer to a structure type into an
// integer index.
void AOSToSOAOPTransformImpl::updateFunctionAttributes(Function &OrigFn,
                                                       Function &CloneFn) {
  AttributeList Attrs = CloneFn.getAttributes();
  auto *OrigFnType = cast<llvm::FunctionType>(OrigFn.getValueType());
  auto *CloneFnType = cast<llvm::FunctionType>(CloneFn.getValueType());
  if (updateAttributeList(OrigFnType, CloneFnType, Attrs))
    CloneFn.setAttributes(Attrs);
}

// This handles updating attributes on a function definition or call site.
// 'Attrs' holds the attributes currently on the function/call site, and will be
// updated to remove attributes that are no longer compatible with the function
// signature. Returns 'true' if the 'Attrs' is changed.
bool AOSToSOAOPTransformImpl::updateAttributeList(
    llvm::FunctionType *OrigFnType, llvm::FunctionType *CloneFnType,
    AttributeList &Attrs) {
  LLVMContext &Ctx = CloneFnType->getContext();
  bool Changed = false;

  llvm::Type *OrigRetTy = OrigFnType->getReturnType();
  llvm::Type *CloneRetTy = CloneFnType->getReturnType();
  if (OrigRetTy->isPointerTy() && !CloneRetTy->isPointerTy()) {
    AttributeSet RetAttrs = Attrs.getAttributes(AttributeList::ReturnIndex);
    if (AttrBuilder(Ctx, RetAttrs).overlaps(IndexInfo.IncompatibleTypeAttrs)) {
      Attrs = Attrs.removeRetAttributes(Ctx, IndexInfo.IncompatibleTypeAttrs);
      Changed = true;
    }
  }

  unsigned NumArgs = OrigFnType->getNumParams();
  for (unsigned ArgIdx = 0; ArgIdx < NumArgs; ++ArgIdx) {
    llvm::Type *OrigArgTy = OrigFnType->getParamType(ArgIdx);
    llvm::Type *CloneArgTy = CloneFnType->getParamType(ArgIdx);
    if (OrigArgTy->isPointerTy() && !CloneArgTy->isPointerTy()) {
      AttributeSet ParamAttrs =
          Attrs.getAttributes(ArgIdx + AttributeList::FirstArgIndex);
      if (AttrBuilder(Ctx, ParamAttrs).overlaps(
            IndexInfo.IncompatibleTypeAttrs)) {
        Attrs =
            Attrs.removeAttributesAtIndex(Ctx,
                                        ArgIdx + AttributeList::FirstArgIndex,
                                        IndexInfo.IncompatibleTypeAttrs);
        Changed = true;
      }
    }
  }

  return Changed;
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
  FuncInfo->PtrConverts.insert(ToInt);
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

// This is a helper function for identifying the index to use for byte-flattened
// GEPs. This can be expanded to also support pointers passed to memintrinsic
// calls to support passing the address of a field being transformed to a
// memintrinsic call if support for those is added.
Value *AOSToSOAOPTransformImpl::getIndexForValue(
  Value *Op, StructType *OrigStructTy) {
  auto &V = FuncInfo->IndexCache[Op];
  if (!V)
    V = createIndexFromValue(Op, OrigStructTy);

  return V;
}

// This walks the definitions through bitcasts, selects and PHI nodes for
// 'Op' to find the index value to use for the index value.
Value *AOSToSOAOPTransformImpl::createIndexFromValue(
  Value *Op, StructType *OrigStructTy) {
  // If 'Op' is a bitcast from a pointer to the type being transformed to an
  // i8*, then we can directly use the source operand of bitcast, since we
  // know this the pointer that needs to be turned into the index type.
  if (auto *BC = dyn_cast<BitCastInst>(Op))
    return getIndexForValue(BC->getOperand(0), OrigStructTy);

  // When opaque pointers are in use, it's possible to directly use the result
  // of a load that is effectively a pointer to the structure type within a GEP
  // that is performing byte offset indexing.
  //  %sa = load ptr, ptr %source
  //  %fa = getelementptr i8, ptr %sa, i64 12
  //
  // In this case, we need to convert %sa to be the index to enable access as:
  //   @soaVar->field3[%sa]
  if (auto *LI = dyn_cast<LoadInst>(Op)) {
    CastInst *NewCast = createCastToIndexType(LI, nullptr);
    NewCast->insertAfter(LI);
    FuncInfo->PtrConverts.insert(NewCast);
    FuncInfo->IndexCache[LI] = NewCast;
    return NewCast;
  }

  // For select and PHI nodes, create new instructions that will act on the
  // index value type, instead of the pointer type.
  if (auto *Sel = dyn_cast<SelectInst>(Op)) {
    Value *NewTrue =
      getIndexForValue(Sel->getTrueValue(), OrigStructTy);
    Value *NewFalse =
      getIndexForValue(Sel->getFalseValue(), OrigStructTy);
    Instruction *NewSel =
      SelectInst::Create(Sel->getCondition(), NewTrue, NewFalse, "", Sel);
    return NewSel;
  }

  if (auto *PHI = dyn_cast<PHINode>(Op)) {
    PHINode *NewPhi = PHINode::Create(PtrSizeIntLLVMType, 0, "", PHI);

    // Save the new PHI so that if another reference to is found while walking
    // the definitions this one will be used.
    FuncInfo->IndexCache[PHI] = NewPhi;

    SmallVector<Value *, 4> NewPhiVals;
    for (Value *Val : PHI->incoming_values())
      NewPhiVals.push_back(
        getIndexForValue(Val, OrigStructTy));

    unsigned NumIncoming = PHI->getNumIncomingValues();
    for (unsigned Num = 0; Num < NumIncoming; ++Num)
      NewPhi->addIncoming(NewPhiVals[Num], PHI->getIncomingBlock(Num));

    return NewPhi;
  }

  llvm_unreachable("unexpected instruction");
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
  gatherCandidateTypes(*DTInfo, CandidateTypes);
  if (CandidateTypes.empty())
    return false;

  qualifyCandidates(CandidateTypes, M, *DTInfo, WPInfo, GetDT);
  if (CandidateTypes.empty()) {
    LLVM_DEBUG(dbgs() << "AOS-to-SOA: No types to transform\n");
    return false;
  }

  AOSToSOAOPTransformImpl Transformer(M.getContext(), DTInfo, "__SOADT_",
                                      M.getDataLayout(), GetTLI,
                                      CandidateTypes);
  return Transformer.run(M);
}

// Populate the 'CandidateTypes' vector with all the structure types
// that should be considered for transformation. These are structure
// that passed the DTrans safety bit flags, but need to be checked for
// additional criteria with the qualifyCandidates function.
void AOSToSOAOPPass::gatherCandidateTypes(DTransSafetyInfo &DTInfo,
                                          StructInfoVecImpl &CandidateTypes) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Build the list of structures to convert based on a command line option
  // containing a list of structure names separated by commas.
  DTransTypeManager &TM = DTInfo.getTypeManager();
  if (!AOSToSOAOPTypelist.empty()) {
    SmallVector<StringRef, 16> SubStrings;
    SplitString(AOSToSOAOPTypelist, SubStrings, ",");
    for (auto &TypeName : SubStrings) {
      DTransStructType *DTransTy = TM.getStructType(TypeName);
      if (!DTransTy) {
        LLVM_DEBUG(dbgs() << "No structure found for: " << TypeName << "\n");
        continue;
      }
      dtrans::TypeInfo *TI = DTInfo.getTypeInfo(DTransTy);
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

  for (dtrans::TypeInfo *TI : DTInfo.type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo)
      continue;

    if (cast<StructType>(TI->getLLVMType())->isLiteral())
      continue;

    if (DTInfo.testSafetyData(TI, dtrans::DT_AOSToSOA)) {
      LLVM_DEBUG(dbgs() << "AOS-to-SOA rejecting -- Unsupported safety data: "
                        << *TI->getDTransType() << "\n");
      continue;
    }

    LLVM_DEBUG(dbgs() << "AOS-to-SOA -- Passes safety data: "
                      << *TI->getDTransType() << "\n");
    CandidateTypes.push_back(StInfo);
  }
}

// This function filters the 'CandidateTypes' list to remove types that are
// not supported for transformation based on the contents of the structure or
// the use of the structure.
void AOSToSOAOPPass::qualifyCandidates(StructInfoVecImpl &CandidateTypes,
                                       Module &M, DTransSafetyInfo &DTInfo,
                                       WholeProgramInfo &WPInfo,
                                       DominatorTreeFuncType &GetDT) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Bypass the checks, and to allow testing on IR that does not satisfy all the
  // preconditions needed for the allocation of an array of structures.
  if (DTransAOSToSOAOPQualOverride)
    return;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  if (!qualifyCandidatesTypes(CandidateTypes, DTInfo))
    return;

  if (!qualifyCalls(M, WPInfo, CandidateTypes, DTInfo, GetDT))
    return;

  qualifyInstructions(M, CandidateTypes, DTInfo);
}

// Check for types in 'CandidateTypes' that are not supported by the
// transformation.
// 1. Types that are used as arrays are not supported. For example
//     [4 x %struct.test]. Because we would need to handle all the allocation
//     checks and transformation code for these arrays, as well.
// 2. Arrays of direct pointers to the type are not supported. For example
//     [4 x %struct.test*], which would be [4 x ptr] with opaque pointers.
//    The conversion of this when pointer shrinking is enabled would result
//    in the type [4 x i32], and could require converting the array type
//    specified for users, such as a getelementptr to be modified, which is not
//    supported.
// 3. Types that contain arrays are not supported. This restriction could be
//     relaxed in a future version.
// 4. Types that contain vectors are not supported.
//
// Return 'true' if candidates remain after this filtering.
bool AOSToSOAOPPass::qualifyCandidatesTypes(StructInfoVecImpl &CandidateTypes,
                                            DTransSafetyInfo &DTInfo) {
  // Collect a set of structure types that are contained within an array type.
  SmallPtrSet<dtrans::StructInfo *, 4> ArrayElemTypes;
  for (auto *TI : DTInfo.type_info_entries()) {
    if (!isa<dtrans::ArrayInfo>(TI))
      continue;

    DTransType *ElemTy = TI->getDTransType()->getArrayElementType();
    while (isa<DTransArrayType>(ElemTy))
      ElemTy = ElemTy->getArrayElementType();

    if (auto PtrTy = dyn_cast<DTransPointerType>(ElemTy))
      ElemTy = ElemTy->getPointerElementType();
    if (!isa<DTransStructType>(ElemTy))
      continue;

    // The type in the array is either a structure or direct pointer to a
    // structure. Add the type to list of disallowed types.
    auto *ElemTI = DTInfo.getTypeInfo(ElemTy);
    assert(ElemTI && "Expecting TypeInfo objeect for structure type");
    ArrayElemTypes.insert(cast<dtrans::StructInfo>(ElemTI));
  }

  // Find which of the candidates qualify for the transformation
  StructInfoVec Qualified;
  for (auto *Candidate : CandidateTypes) {
    if (ArrayElemTypes.contains(Candidate)) {
      LLVM_DEBUG(dbgs() << "AOS-to-SOA rejecting -- Array of type seen: "
                        << *Candidate->getDTransType() << "\n");
      continue;
    }

    // No arrays of the structure type were found. Check the structure
    // field types to verify all members are supported for the transformation.
    // Reject any structure that contains an array or vector type.
    //
    // Also, reject the structure if an element-zero access was seen
    // (hasNonGEPAccess) to simplify the transformation. Otherwise, we would
    // need to identify and transform load/store instructions such as the
    // following that use a pointer to structure to access a field within it,
    // without using a getelementptr instruction. This could be supported in the
    // future if necessary but would require additional tracking data from the
    // safety analyzer.
    //   %struct_ptr = load ptr, ptr %p0
    //   %field0_value = load i32, ptr %struct_ptr
    bool Supported = true;
    for (auto &FI : Candidate->getFields()) {
      Type *Ty = FI.getLLVMType();
      if (Ty->isArrayTy() || Ty->isVectorTy() || FI.hasNonGEPAccess()) {
        Supported = false;
        break;
      }
    }

    if (!Supported) {
      LLVM_DEBUG(dbgs() << "AOS-to-SOA rejecting -- Unsupported structure "
                           "element type: "
                        << *Candidate->getDTransType() << "\n");
      continue;
    }

    Qualified.push_back(Candidate);
  }

  CandidateTypes = std::move(Qualified);
  return !CandidateTypes.empty();
}

// Check that the type is only allocated once by malloc or calloc.
// Return 'true' if candidates remain after this filtering.
bool AOSToSOAOPPass::qualifyCalls(Module &M, WholeProgramInfo &WPInfo,
                                  StructInfoVecImpl &CandidateTypes,
                                  DTransSafetyInfo &DTInfo,
                                  DominatorTreeFuncType &GetDT) {

  auto &TheDTInfo = DTInfo;
  auto AddCallInfoStructTypes = [&TheDTInfo](
                                    CallInfo *CI,
                                    SmallPtrSetImpl<StructInfo *> &Elements) {
    assert(CI->getAliasesToAggregateType() && "Should have aggregate type");

    for (auto *ElemTy : CI->getElementTypesRef().element_dtrans_types()) {
      auto *TI = TheDTInfo.getTypeInfo(ElemTy);
      if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI))
        if (Elements.insert(StInfo).second)
          LLVM_DEBUG(dbgs()
                     << "AOS-to-SOA rejecting -- Unsupported call info use: "
                     << *ElemTy << "\n"
                     << *CI->getInstruction() << "\n");
    }
  };

  SmallPtrSet<StructInfo *, 8> Disqualified;
  DenseMap<dtrans::StructInfo *, Instruction *> AllocTypeInfoToInstr;
  DenseMap<dtrans::StructInfo *, Instruction *> FreeTypeInfoToInstr;
  for (auto *CInfo : DTInfo.call_info_entries()) {
    if (!CInfo->getAliasesToAggregateType())
      continue;

    if (CInfo->getElementTypesRef().getNumTypes() != 1) {
      AddCallInfoStructTypes(CInfo, Disqualified);
      continue;
    }

    // Do not support any memfuncs on the candidate type. These can be
    // supported, but are not currently needed.
    if (isa<MemfuncCallInfo>(CInfo)) {
      AddCallInfoStructTypes(CInfo, Disqualified);
      continue;
    }

    if (auto *ACI = dyn_cast<dtrans::AllocCallInfo>(CInfo)) {
      if (ACI->getAllocKind() != dtrans::AK_Calloc &&
          ACI->getAllocKind() != dtrans::AK_Malloc) {
        AddCallInfoStructTypes(ACI, Disqualified);
      } else {
        auto &TypeList = CInfo->getElementTypesRef();
        DTransType *AllocatedTy = TypeList.getElemDTransType(0);
        if (AllocatedTy->isStructTy()) {
          // Save the first allocation seen, otherwise disqualify the type.
          auto *TI = DTInfo.getTypeInfo(AllocatedTy);
          assert(TI && "TypeInfo not found. DTransInfo out of date?");
          auto *STI = cast<StructInfo>(TI);
          Instruction *Inst = CInfo->getInstruction();
          if (!AllocTypeInfoToInstr.insert(std::make_pair(STI, Inst)).second) {
            Disqualified.insert(STI);
            LLVM_DEBUG(dbgs()
                       << "AOS-to-SOA rejecting -- Too many allocations: "
                       << *AllocatedTy << "\n");
          }
        }
      }
    } else if (auto *FCI = dyn_cast<dtrans::FreeCallInfo>(CInfo)) {
      auto &TypeList = CInfo->getElementTypesRef();
      DTransType *FreeTy = TypeList.getElemDTransType(0);
      if (FreeTy->isStructTy()) {
        auto *TI = dyn_cast<StructInfo>(DTInfo.getTypeInfo(FreeTy));
        if (FCI->getFreeKind() != dtrans::FK_Free) {
          Disqualified.insert(TI);
          LLVM_DEBUG(dbgs() << "AOS-to-SOA rejecting -- Deallocation not using "
                               "call to 'free': "
                            << *FreeTy << "\n");
        }

        // Save the first free seen, otherwise disqualify the type. The limit
        // that 'free' is only called in one location is not strictly necessary
        // for the transformation. It is simply checked here as a simplification
        // for the checks for 'BitCast' instructions.
        Instruction *Inst = CInfo->getInstruction();
        if (!FreeTypeInfoToInstr.insert(std::make_pair(TI, Inst)).second) {
          if (Disqualified.insert(TI).second)
            LLVM_DEBUG(dbgs() << "AOS-to-SOA rejecting -- Too many free-sites: "
                              << *FreeTy << "\n");
        }
      }
    }
  }

  StructInfoVec Qualified;
  for (auto *Candidate : CandidateTypes)
    if (!Disqualified.contains(Candidate))
      Qualified.push_back(Candidate);

  CandidateTypes = std::move(Qualified);
  if (CandidateTypes.empty())
    return false;

  // Select the types that have a single allocation call. Also, populate a set
  // of instructions by function that need to be checked to verify the
  // allocation is not within a loop. We group these by function so that the
  // LoopInfo for a function only needs to be calculated one time. This will
  // reject any type that does not have a dynamic allocation because currently
  // the only way for the pointers in the peeled structure to be initialized is
  // by rewriting the code at the allocation call.
  DenseMap<Function *, DenseSet<std::pair<Instruction *, dtrans::StructInfo *>>>
      AllocPathMap;
  SmallVector<std::pair<Function *, Instruction *>, 4> CallChain;
  Qualified.clear();
  for (auto *TyInfo : CandidateTypes) {
    if (AllocTypeInfoToInstr[TyInfo] == nullptr) {
      LLVM_DEBUG(dbgs() << "AOS-to-SOA rejecting -- No allocation found: "
                        << *TyInfo->getDTransType() << "\n");
      continue;
    }

    Instruction *I = AllocTypeInfoToInstr[TyInfo];
    Value *Unsupported;
    if (!checkAllocationUsers(I, TyInfo->getLLVMType(), &Unsupported)) {
      LLVM_DEBUG(
          dbgs() << "AOS-to-SOA rejecting -- Unsupported allocation user: "
                 << *TyInfo->getDTransType() << "\n"
                 << "  " << *Unsupported << "\n");
      continue;
    }

    Instruction *Free = FreeTypeInfoToInstr[TyInfo];
    if (Free) {
      CallInst *FreeCall = cast<CallInst>(Free);
      Value *FreeArg = FreeCall->getOperand(0);
      if (auto *BC = dyn_cast<BitCastInst>(FreeArg))
        SafeBitCasts.insert(BC);
    }

    // Verify the call chain to the instruction consists of a single path from
    // 'main'
    CallChain.clear();
    if (!collectCallChain(WPInfo, I, CallChain)) {
      LLVM_DEBUG(dbgs() << "AOS-to-SOA rejecting -- Multiple call paths: "
                        << *TyInfo->getDTransType() << "\n");
      continue;
    }

    // Save the instruction and all its callers to the list of locations that
    // will need to be checked for being within loops.
    AllocPathMap[I->getFunction()].insert(std::make_pair(I, TyInfo));
    for (auto &FuncInstrPair : CallChain)
      AllocPathMap[FuncInstrPair.first].insert(
          std::make_pair(FuncInstrPair.second, TyInfo));

    Qualified.push_back(TyInfo);
  }

  CandidateTypes = std::move(Qualified);
  if (CandidateTypes.empty())
    return false;

  // Check the function's loops to see if the allocation (or call to the
  // allocation) instruction is within a loop
  for (auto &FuncToAllocPath : AllocPathMap) {
    Function *F = FuncToAllocPath.first;
    DominatorTree &DT = (GetDT)(*F);
    LoopInfo LI(DT);

    if (LI.size())
      for (auto &InstTypePair : FuncToAllocPath.second)
        if (LI.getLoopFor(InstTypePair.first->getParent())) {
          StructInfo *StInfo = InstTypePair.second;
          LLVM_DEBUG(dbgs() << "AOS-to-SOA rejecting -- Allocation in loop: "
                            << *StInfo->getDTransType()
                            << "\n  Function: " << F->getName() << "\n");
          auto *It =
              std::find(CandidateTypes.begin(), CandidateTypes.end(), StInfo);
          if (It != CandidateTypes.end())
            CandidateTypes.erase(It);
        }
  }

  return !CandidateTypes.empty();
}

bool AOSToSOAOPPass::qualifyInstructions(Module &M,
                                         StructInfoVecImpl &CandidateTypes,
                                         DTransSafetyInfo &DTInfo) {
  SmallPtrSet<DTransType *, 4> DTransCandidates;
  SmallPtrSet<DTransType *, 4> DisqualifiedTypes;
  for (auto *StInfo : CandidateTypes)
    DTransCandidates.insert(StInfo->getDTransType());

  PtrTypeAnalyzer &PTA = DTInfo.getPtrTypeAnalyzer();
  for (auto &F : M)
    for (auto &I : instructions(&F)) {
      // BitCast instructions are not expected when opaque pointers are in use.
      // When typed-pointers are in use, the PointerTypeAnalyzer results will
      // store types the original operand was used as, and the types the BitCast
      // result is used as, which prevents easy determination of the source
      // and destination types without looking at the pointer element type. For
      // this reason, we will disqualify any bitcasts seen other than for the
      // allocation, free calls or byte-flattened GEPs.
      if (auto *BC = dyn_cast<BitCastInst>(&I)) {
        if (!BC->getType()->isPointerTy())
          continue;

        if (SafeBitCasts.count(BC))
          continue;

        ValueTypeInfo *Info = PTA.getValueTypeInfo(BC);
        assert(Info && "Expected PTA to get info for all pointers");
        if (!Info->canAliasToAggregatePointer(ValueTypeInfo::VAT_Use))
          continue;

        for (auto *Ty : Info->getPointerTypeAliasSet(ValueTypeInfo::VAT_Use))
          if (Ty->isPointerTy() &&
              DTransCandidates.contains(Ty->getPointerElementType())) {

            bool NonByteGEP = false;
            for (auto *U : BC->users()) {
              if (auto *GEP = dyn_cast<GetElementPtrInst>(U)) {
                auto InfoPair = DTInfo.getByteFlattenedGEPElement(GEP);
                if (InfoPair.first)
                  continue;
              }
              NonByteGEP = true;
              break;
            }

            if (NonByteGEP) {
              LLVM_DEBUG(dbgs()
                         << "AOS-to-SOA rejecting -- Unsupported bitcasts: "
                         << *Ty->getPointerElementType() << "\n"
                         << *BC << " in: " << BC->getFunction()->getName()
                         << "\n");
              DisqualifiedTypes.insert(Ty->getPointerElementType());
            }
          }
      }
    }

  StructInfoVec Qualified;
  for (auto *StInfo : CandidateTypes)
    if (!DisqualifiedTypes.count(StInfo->getDTransType()))
      Qualified.push_back(StInfo);

  CandidateTypes = std::move(Qualified);
  return !CandidateTypes.empty();
}

// The result of the allocation call is a ptr type. Because we are going to be
// replacing the ultimate pointer assignment during the transformation with an
// integer type, and storing the result of the allocation into a compiler
// generated structure, we need to perform some additional checks to make sure
// the allocation can be replaced. Specifically, the following uses will be
// allowed, and all others rejected:
//   icmp eq ptr %call, null   [also allow inequality, and reversed operands]
//   bitcast i8* %call to %struct.type* [ to support typed-pointers]
//   store ptr %call, ptr %field
//   getelementptr %struct.type, ptr %call, i64 0, i32 22
//
// There could be other instructions that would be safe, but they are not supported
// by the transformation currently, so will be rejected.
bool AOSToSOAOPPass::checkAllocationUsers(Instruction *AllocCall,
                                          llvm::Type *StructTy,
                                          Value **Unsupported) {

  assert(isa<CallInst>(AllocCall) &&
         "Instruction expected to be allocation call");

  *Unsupported = nullptr;
  uint32_t BitCastCount = 0;
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

      if (!(I->getOperand(0) == AllocCall &&
            isa<ConstantPointerNull>(I->getOperand(1))) &&
          !(I->getOperand(1) == AllocCall &&
            isa<ConstantPointerNull>(I->getOperand(0)))) {
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
      // Permit at most 1 BitCast, which should be the bitcast to be
      // a pointer to the strucure type in the case that typed pointers
      // are in use.
      BitCastCount++;
      if (BitCastCount > 1) {
        *Unsupported = I;
        return false;
      }
      SafeBitCasts.insert(cast<BitCastInst>(I));
      break;

    case Instruction::GetElementPtr:
      // For the opaque pointer version, the allocated pointer can be
      // directly used as a pointer to a structure with GetElementPtr
      // instructions. The transformation needs to be supported for 1 and 2
      // index versions that are indexing using a structure type (we know the
      // structure type is correct because the safety checks have all passed).
      // However, a usage in a byte-flattened GEP form is currently not
      // supported because it would require additional logic for the
      // convertByteGEP routine to handle the instruction after the
      // convertAllocCall routine has processed it.
      if (cast<GetElementPtrInst>(I)->getNumIndices() > 2 ||
          !cast<GetElementPtrInst>(I)->getSourceElementType()->isStructTy()) {
        *Unsupported = I;
        return false;
      }
      break;
    }
  }

  return true;
}

// If there is a single call chain that reaches the instruction 'I',
// add the function to the 'CallChain' vector, and return 'true'. Otherwise,
// return 'false'.
bool AOSToSOAOPPass::collectCallChain(
    WholeProgramInfo &WPInfo, Instruction *I,
    SmallVectorImpl<std::pair<Function *, Instruction *>> &CallChain) {
  Function *F = I->getFunction();
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
    return WPInfo.getMainFunction() == F;

  CallChain.push_back(std::make_pair(Callsite->getFunction(), Callsite));
  return collectCallChain(WPInfo, Callsite, CallChain);
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
  return PA;
}

} // end namespace dtransOP
} // end namespace llvm
