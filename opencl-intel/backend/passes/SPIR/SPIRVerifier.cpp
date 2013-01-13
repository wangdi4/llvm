//===-- SpirVerifier.cpp - Implement the SPIR Verifier -----------*- C++ -*-==//
// 
// Copyright (c) 2012 The Khronos Group Inc.  All rights reserved.
//
// NOTICE TO KHRONOS MEMBER:
//
// AMD has assigned the copyright for this object code to Khronos.
// This object code is subject to Khronos ownership rights under U.S. and
// international Copyright laws.
//
// Permission is hereby granted, free of charge, to any Khronos Member
// obtaining a copy of this software and/or associated documentation files
// (the "Materials"), to use, copy, modify and merge the Materials in object
// form only and to publish, distribute and/or sell copies of the Materials
// solely in object code form as part of conformant OpenCL API implementations,
// subject to the following conditions:
//
// Khronos Members shall ensure that their respective ICD implementation,
// that is installed over another Khronos Members' ICD implementation, will
// continue to support all OpenCL devices (hardware and software) supported
// by the replaced ICD implementation. For the purposes of this notice, "ICD"
// shall mean a library that presents an implementation of the OpenCL API for
// the purpose routing API calls to different vendor implementation.
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Materials.
//
// KHRONOS AND AMD MAKE NO REPRESENTATION ABOUT THE SUITABILITY OF THIS
// SOURCE CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
// IMPLIED WARRANTY OF ANY KIND.  KHRONOS AND AMD DISCLAIM ALL WARRANTIES
// WITH REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
// IN NO EVENT SHALL KHRONOS OR AMD BE LIABLE FOR ANY SPECIAL, INDIRECT,
// INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
// FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
// NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH
// THE USE OR PERFORMANCE OF THIS SOURCE CODE.
//
// U.S. Government End Users.   This source code is a "commercial item" as
// that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting of
// "commercial computer software" and "commercial computer software
// documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995)
// and is provided to the U.S. Government only as a commercial end item.
// Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
// 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
// source code with only those rights set forth herein.
// 
//===----------------------------------------------------------------------===//
//
// This file defines the function verifier interface, that can be used for some
// sanity checking of input SPIR to the system.
//
// This does not provide LLVM style verification. It instead assumes that the
// LLVM verifier has already been run and the IR is well formed.
//
// In lightweight mode:
//    *All extensions in the spir.extensions metadata are supported by the
//     vendor and device combination.
//    *Optional core features specified in the spir.used.optional.core.features
//     are valid for the vendor and device combination.
//    *The SPIR version is valid for the vendor.
//    *The OpenCL version is valid for the specific SPIR version.
//    *The target triple is the SPIR target triple.
//    *The target data layout matches the SPIR data layout.
//
// In heavyweight mode:
//    *The lightweight mode checks are executed.
//    *Only the data types that are specified by the SPIR specification are
//     used.
//    *Only the intrinsics specified in the SPIR specification are used.
//    *Only the instructions specified in the SPIR specification are used.
//    *Only alwaysinline, inlinehint, noinline, nounwind, readnone, readonly
//     are specified as function attributes.
//    *Only zeroext, signext, byval, sret and nocapture parameter attributes
//     are used.
//    *No visibility types other than 'default' is used.
//    *Only private, available_externally, internal and external linkage modes
//     are used.
//    *All functions are marked with SPIR_KERNEL or SPIR_FUNC calling conventions.
//    *All intrinsics begin with @llvm.spir or @spir.
//    *All functions have the function attribute nounwind.
//    *No inline assembly is used.
//    *Only sampler values as specified in Table14 of the spec are used.
//    *Only llvm.spir.get.null.N() are used to assign NULL to a pointer.
//    *All alignments comply with the OpenCL spec.
//    *Only address spaces 0 through 5 are valid.
//    *All functions in the spir.kernels metdata have the SPIR_KERNEL calling
//     convention.
//    *All functions marked with SPIR_KERNEL calling convention exist in the
//     spir.kernels metadata.
//    *Work group size hint metadata must have 3 integer values.
//    *Reqd work group size metadata must have 3 integer values.
//    *vector type hint metadata can only have the string types in section
//     2.4.2.2 of the SPIR spec.
//      *It can only be attached to non-kernel functions.
//      *double[N] types are only valid if cl_double is specified as an optional
//       core feature.
//    *Kernel arg metadata only contains values specified in section 2.5.
//    *Only constant, no alias and volatile are valid type qualifiers.
//    *Only compiler options as specified in the SPIR spec are valid.
//    *All functions are properly name mangled.
//    *Only the size_t functions in the SPIR spec are used and are valid.
//    *All load/store operations have an alignment valid.
//
//===----------------------------------------------------------------------===//

#include "SPIRVerifier.h"
#include "llvm/CallingConv.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/GlobalValue.h"
#include "llvm/Intrinsics.h"
#include "llvm/InlineAsm.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Metadata.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstVisitor.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cstdarg>
using namespace llvm;

bool isOpaqueNamedStruct(const Type *t, StringRef name);

bool isAddressSpace(const Type*, unsigned);
namespace {

  enum ErrorCodes {
    InvalidType,
    RequiresDouble,
    RequiresImages,
    InvalidVectorSize,
    InvalidAddressSpace,
    BooleanVector,
    InvalidInstr,
    NoAtomicLoadStore,
    UnalignedLoadStore,
    VectorConversion,
    InvalidPtrInst,
    NoTailCall,
    BadCallingConv,
    NoNounwind,
    InvalidFuncAttribute,
    InvalidArgAttribute,
    InlineAsm,
    IndirectCall,
    InvalidTriple,
    InvalidDataLayout,
    InvalidSPIRVersion,
    InvalidOCLVersion,
    InvalidOptionalCore,
    InvalidKHRExtension,
    UnsupportedSPIRVersion,
    UnsupportedOCLVersion,
    UnsupportedOptionalCore,
    UnsupportedKHRExtension,
    InvalidLinkage,
    InvalidVisibility,
    InvalidVarArg,
    InvalidGarbageCollection,
    InvalidNameMangling,
    InvalidIntrRet,
    InvalidIntrArg,
    InvalidIntrFunc,
    IllegalAShrExact,
    InvalidCompilerOpts,
    InvalidExtCompilerOpts,
    InvalidSamplerMD,
    InvalidFunctionMD
  } ;


  struct SPIRVerifier : public FunctionPass,
  public InstVisitor<SPIRVerifier> {
    static char ID; // Pass ID, replacement for typeid
    bool Broken;          // Is this module found to be broken?
    bool RealPass;        // Are we not being run by a PassManager?
    bool LightWeight;     // Are we running the lightweight pass?
    bool HasDouble;       // Do we support doubles?
    bool HasImages;       // Do we support images?
    bool HasHalf;         // Do we support half type?
    VerifierFailureAction action;
                          // What to do if verification fails.
    Module *Mod;          // Module we are verifying right now
    LLVMContext *Context; // Context within which we are verifying
    std::string OptCore;  // List of space seperated support optional core features.
    std::string KhrExt;   // List of space seperated support KHR extensions.
    unsigned SPIRVer[2];  // SPIR version information.
    unsigned OCLVer[2];   // OpenCL version information.

    std::string Messages;
    raw_string_ostream MessagesStr;

    SPIRVerifier()
      : FunctionPass(ID), Broken(false), RealPass(true), LightWeight(true),
        HasDouble(false), HasImages(false), HasHalf(false),
        action(PrintMessageAction), Mod(0), Context(0),
        MessagesStr(Messages) {
      initializeSPIRVerifierPass(*PassRegistry::getPassRegistry());
    }
    explicit SPIRVerifier(VerifierFailureAction ctn, bool lw)
      : FunctionPass(ID), Broken(false), RealPass(true), LightWeight(lw),
        HasDouble(false), HasImages(false), HasHalf(false),
        action(ctn), Mod(0), Context(0), MessagesStr(Messages) {
      initializeSPIRVerifierPass(*PassRegistry::getPassRegistry());
    }

    bool doInitialization(Module &M)
    {
      Mod = &M;
      Context = &M.getContext();
      visitSPIRHeader(M);

      // If this is a real pass, in a pass manager, we must abort before
      // returning back to the pass manager, or else the pass manager may try to
      // run other passes on the broken module.
      if (RealPass)
        return abortIfBroken();
      return false;
    }

    bool runOnFunction(Function &F)
    {
      Mod = F.getParent();
      if (!Context) Context = &F.getContext();

      if (!LightWeight) {
        visit(F);
      }

      // If this is a real pass, in a pass manager, we must abort before
      // returning back to the pass manager, or else the pass manager may try to
      // run other passes on the broken module.
      if (RealPass)
        return abortIfBroken();

      return false;
    }

    bool doFinalization(Module &M)
    {
      // Scan through, checking all of the external function's linkage now...
      for (Module::iterator I = M.begin(), E = M.end(); I != E; ++I) {
        visitGlobalValue(*I);

        // Check to make sure function prototypes are okay.
        if (I->isDeclaration()) visitFunction(*I);
      }

      for (Module::named_metadata_iterator I = M.named_metadata_begin(),
           E = M.named_metadata_end(); I != E; ++I)
        visitNamedMDNode(*I);

      // If the module is broken, abort at this time.
      return abortIfBroken();
    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const
    {
      AU.setPreservesAll();
    }

    virtual const char *getPassName() const
    {
      return "Verification pass";
    }

    /// abortIfBroken - If the module is broken and we are supposed to abort on
    /// this condition, do so.
    ///
    bool abortIfBroken()
    {
      if (!Broken) return false;
      MessagesStr << "Broken SPIR module found, ";
      switch (action) {
      default: llvm_unreachable("Unknown action");
      case AbortProcessAction:
        MessagesStr << "compilation aborted!\n";
        dbgs() << MessagesStr.str();
        // Client should choose different reaction if abort is not desired
        abort();
      case PrintMessageAction:
        MessagesStr << "verification continues.\n";
        dbgs() << MessagesStr.str();
        return false;
      case ReturnStatusAction:
        MessagesStr << "compilation terminated.\n";
        return true;
      }
    }


    void visitSPIRHeader(Module &M);
    // Verification methods...
    void visitGlobalValue(GlobalValue &GV);
    void visitFunction(Function &F);
    void visitNamedMDNode(NamedMDNode &NMD);
    using InstVisitor<SPIRVerifier>::visit;

    void visit(Instruction &I);
    void visitInstruction(Instruction &I);

    bool isValidType(const Type *t);
    bool isValidType(const VectorType *t);
    bool isValidType(const ArrayType *t);
    bool isValidType(const StructType *t);
    bool isValidType(const PointerType *t);
    bool isValidType(const FunctionType *t);
    bool isIntrEncodingValid(const Function *f);
    bool isSamplerValid(const Function *f);
    bool isSizeIntrValid(const Function* f);

    template <class T>
      bool verifyOperands(Instruction &I);
    const char *ErrorMessages(unsigned err_code);

    void WriteInst(const Instruction *V)
    {
      if (!V) return;
      MessagesStr << *V << '\n';
    }

    void WriteValue(const Value *V)
    {
      if (!V) return;
      if (isa<Instruction>(V)) {
        MessagesStr << *V << '\n';
      } else {
        WriteAsOperand(MessagesStr, V, true, Mod);
        MessagesStr << '\n';
      }
    }

    void WriteType(Type *T)
    {
      if (!T) return;
      MessagesStr << ' ' << *T;
    }

    void WriteType(const Type *T)
    {
      if (!T) return;
      WriteType(const_cast<Type*>(T));
    }


    // CheckFailed - A check failed, so print out the condition and the message
    // that failed.  This provides a nice place to put a breakpoint if you want
    // to see why something is not correct.
    void CheckFailed(const Twine &Message,
                     const Twine &str)
    {
      MessagesStr << Message.str() << "\n";
      MessagesStr << str.str() << "\n";
      Broken = true;
    }

    void CheckFailed(const Twine &Message,
                     const Instruction *V1 = 0,
                     const Instruction *V2 = 0,
                     const Instruction *V3 = 0,
                     const Instruction *V4 = 0)
    {
      MessagesStr << Message.str() << "\n";
      WriteInst(V1);
      WriteInst(V2);
      WriteInst(V3);
      WriteInst(V4);
      Broken = true;
    }
    void CheckFailed(const Twine &Message,
                     const Type *V1 = 0, const Type *V2 = 0,
                     const Type *V3 = 0, const Type *V4 = 0)
    {
      MessagesStr << Message.str() << "\n";
      WriteType(V1);
      WriteType(V2);
      WriteType(V3);
      WriteType(V4);
      Broken = true;
    }

    void CheckFailed(const Twine &Message,
                     const Value *V1 = 0, const Value *V2 = 0,
                     const Value *V3 = 0, const Value *V4 = 0)
    {
      MessagesStr << Message.str() << "\n";
      WriteValue(V1);
      WriteValue(V2);
      WriteValue(V3);
      WriteValue(V4);
      Broken = true;
    }

    void CheckFailed(const Twine &Message, const Value *V1,
                     Type *T2, const Value *V3 = 0)
    {
      MessagesStr << Message.str() << "\n";
      WriteValue(V1);
      WriteType(T2);
      WriteValue(V3);
      Broken = true;
    }

    void CheckFailed(const Twine &Message, Type *T1,
                     Type *T2 = 0, Type *T3 = 0)
    {
      MessagesStr << Message.str() << "\n";
      WriteType(T1);
      WriteType(T2);
      WriteType(T3);
      Broken = true;
    }
    static const std::string SPIR_DATA_LAYOUT;
    private:
    void visitOptionalCoreMD(NamedMDNode*);
    void visitUsedExtensionsMD(NamedMDNode*);
    void visitSamplerConstructorMD(NamedMDNode*);
    void visitSPIRVersionMD(NamedMDNode*);
    void visitOCLVersionMD(NamedMDNode*);
    void visitCompilerExtOptsMD(NamedMDNode*);
    void visitCompilerOptsMD(NamedMDNode*);
    void visitUserFuncsMD(NamedMDNode*);
    void visitKernelsMD(NamedMDNode*);
  };
} // End anonymous namespace

const std::string SPIRVerifier::SPIR_DATA_LAYOUT =
  "i1:8-i8:8-i16:16-i32:32-i64:64-f32:32-f64:64-v16:16"
  "-v24:32-v32:32-v48:64-v64:64-v96:128-v128:128-v192:256"
  "-v256:256-v512:512-v1024:1024";
char SPIRVerifier::ID = 0;
INITIALIZE_PASS_BEGIN(SPIRVerifier, "spirverify", "SPIR Module Verifier", false, false)
INITIALIZE_PASS_END(SPIRVerifier, "spirverify", "SPIR Module Verifier", false, false)

// Assert - We know that cond should be true, if not print an error message.
#define Assert(C, M) \
  do { if (!(C)) { CheckFailed(M); return; } } while (0)
#define Assert1(C, M, V1) \
  do { if (!(C)) { CheckFailed(M, V1); return; } } while (0)
#define Assert2(C, M, V1, V2) \
  do { if (!(C)) { CheckFailed(M, V1, V2); return; } } while (0)
#define Assert3(C, M, V1, V2, V3) \
  do { if (!(C)) { CheckFailed(M, V1, V2, V3); return; } } while (0)
#define Assert4(C, M, V1, V2, V3, V4) \
  do { if (!(C)) { CheckFailed(M, V1, V2, V3, V4); return; } } while (0)

void
SPIRVerifier::visit(Instruction &I)
{
  for (unsigned i = 0, e = I.getNumOperands(); i != e; ++i)
    Assert1(I.getOperand(i) != 0, "Operand is null", &I);
  InstVisitor<SPIRVerifier>::visit(I);
}

// Verify that the module is a SPIR module and that it has
// the basic information correct. This is the lightweight check.
void
SPIRVerifier::visitSPIRHeader(Module &M)
{
  if (M.getTargetTriple() != "spir") {
    CheckFailed(ErrorMessages(InvalidTriple), M.getTargetTriple());
    return;
  }

  if (M.getDataLayout() != SPIR_DATA_LAYOUT) {
    CheckFailed(ErrorMessages(InvalidDataLayout), M.getDataLayout());
    return;
  }

  NamedMDNode *SPIRVersion = M.getNamedMetadata("!spir.version");
  visitSPIRVersionMD(SPIRVersion);

  NamedMDNode *OCLVersion = M.getNamedMetadata("!spir.ocl.version");
  visitOCLVersionMD(OCLVersion);

  NamedMDNode *OptFeat =
    M.getNamedMetadata("!spir.used.optional.core.features");
  visitOptionalCoreMD(OptFeat);

  NamedMDNode *KHRExt = M.getNamedMetadata("!spir.used.extensions");
  visitUsedExtensionsMD(KHRExt);


}

const char *
SPIRVerifier::ErrorMessages(unsigned err_code)
{
  switch(err_code) {
    default:
      return "Unknown error.";
    case InvalidType:
      return "Invalid type, SPIR Section 2.1.1.";
    case RequiresDouble:
      return "Requires cl_doubles optional core feature,"
        " SPIR section 2.12.2.";
    case RequiresImages:
      return "Requires cl_images optional core feature,"
        " SPIR section 2.12.2.";
    case InvalidVectorSize:
      return "Invalid vector size, SPIR Section 2.1.2.";
    case InvalidAddressSpace:
      return "Invalid address space, SPIR section 2.2.";
    case BooleanVector:
      return "Vector of booleans invalid, SPIR Section 2.1.2.";
    case InvalidInstr:
      return "Invalid instruction, SPIR Section 3.3.";
    case NoAtomicLoadStore:
      return "Invalid atomic attribute, SPIR section 3.3.";
    case UnalignedLoadStore:
      return "Unaligned load/store, SPIR section 2.1.4.";
    case VectorConversion:
      return "Invalid vector conversion, SPIR Section 3.3.";
    case InvalidPtrInst:
      return "Invalid Pointer Instruction, SPIR section 3.3.";
    case NoTailCall:
      return "No tail calls allowed, SPIR section 3.10.";
    case BadCallingConv:
      return "Invalid calling convention, SPIR section 3.6.";
    case NoNounwind:
      return "Nounwind must be specified, SPIR section 3.10.";
    case InvalidFuncAttribute:
      return "Invalid function attribute, SPIR section 3.10.";
    case InvalidArgAttribute:
      return "Invalid argument attribute, SPIR section 3.8.";
    case InlineAsm:
      return "Invalid inline assembly, SPIR section 3.12.";
    case IndirectCall:
      return "Invalid indirect call, SPIR section 3.3.";
    case InvalidTriple:
      return "Invalid target triple, SPIR section 3.1.";
    case InvalidDataLayout:
      return "Invalid data layout, SPIR section 3.2.";
    case InvalidSPIRVersion:
      return "Invalid SPIR version, SPIR section 2.13.";
    case InvalidOCLVersion:
      return "Invalid OCL version, SPIR section 2.14.";
    case InvalidOptionalCore:
      return "Invalid optional core features, SPIR section 2.12.1.";
    case InvalidKHRExtension:
      return "Invalid KHR Extensions, SPIR section 2.12.2.";
    case UnsupportedSPIRVersion:
      return "Unsupported SPIR version.";
    case UnsupportedOCLVersion:
      return "Unsupported OCL version.";
    case UnsupportedOptionalCore:
      return "Unsupported optional core features.";
    case UnsupportedKHRExtension:
      return "Unsupported KHR Extensions.";
    case InvalidLinkage:
      return "Invalid linkage mode, SPIR section 3.5.";
    case InvalidVisibility:
      return "Invalid visibility, SPIR section 3.7.";
    case InvalidVarArg:
      return "Var Args are illegal, SPIR section 3.3.";
    case InvalidGarbageCollection:
      return "Garbage colletion is illegal, SPIR section 3.9.";
    case InvalidNameMangling:
      return "Name mangling is invalid, SPIR section section 2.11.1.";
    case InvalidIntrRet:
      return "Return value for intrincis is invalid, SPIR spec 2.1.1.X/2.11.1.";
    case InvalidIntrArg:
      return "Argument for intrincis is invalid, SPIR spec 2.1.1.X/2.11.1.";
    case InvalidIntrFunc:
      return "The intrinsic function is invalid, SPIR spec 2.1.1.X/2.11.1.";
    case IllegalAShrExact:
      return "The AShr instruction cannot have 'exact', SPIR spec 3.3.";
    case InvalidCompilerOpts:
      return "Invalid compiler options metadata, SPIR spec 2.9.";
    case InvalidExtCompilerOpts:
      return "Invalid extended compiler options metadata, SPIR spec 2.9.";
    case InvalidSamplerMD:
      return "Invalid sampler MD detected, SPIR spec 2.1.3.1.";
    case InvalidFunctionMD:
      return "Function metadata is malformed, SPIR spec 2.4.1/2.4.2.";
  }
}
// A pointer type is valid if the address spaces are valid.
bool
SPIRVerifier::isValidType(const PointerType *t)
{
  switch (t->getAddressSpace()) {
    default:
      CheckFailed(ErrorMessages(InvalidAddressSpace), t);
      return false;
    case SPIRAS_PRIVATE:
    case SPIRAS_GLOBAL:
    case SPIRAS_CONSTANT:
    case SPIRAS_LOCAL:
    case SPIRAS_GLOBAL_HOST:
    case SPIRAS_CONSTANT_HOST:
      return true;
  }
  // FIXME: Should we verify that we are pointing to a valid
  // type? Things get tricky in this case when we try to
  // verify a pointer in structure with a loop in the
  // pointer chain.
}

// An array type is valid if the element types are all valid.
bool
SPIRVerifier::isValidType(const ArrayType *type)
{
  for (Type::subtype_iterator sti = type->subtype_begin(),
      ste = type->subtype_end(); sti != ste; ++sti) {
    if (!isValidType(*sti)) {
      CheckFailed(ErrorMessages(InvalidType), type);
      return false;
    }
  }
  return true;
}

// A vector type is valid if the element types are valid
// and the size is 2, 3, 4, 8 or 16 elements.
bool
SPIRVerifier::isValidType(const VectorType *type)
{
  unsigned numEle = type->getNumElements();
  switch (numEle) {
    default:
      CheckFailed(ErrorMessages(InvalidVectorSize), type);
      return false;
    case 2:
    case 3:
    case 4:
    case 8:
    case 16:
      break;
  }
  for (llvm::Type::subtype_iterator sti = type->subtype_begin(),
      ste = type->subtype_end(); sti != ste; ++sti) {
    if ((*sti)->isIntegerTy(1)) {
      CheckFailed(ErrorMessages(BooleanVector), type);
      return false;
    }
    if (!isValidType(*sti)) {
      CheckFailed(ErrorMessages(InvalidType), type);
      return false;
    }
  }
  return true;
}

// A struct type is valid if all of its element types are valid
// or if it is an opaque type with a reserved SPIR name.
bool SPIRVerifier::isValidType(const StructType *type)
{
  if (type->isOpaque()) {
    if (type->hasName()) {
      StringRef name = type->getName();
      if (name.startswith("spir.image1d_t")) return HasImages;
      if (name.startswith("spir.image1d_array_t")) return HasImages;
      if (name.startswith("spir.image1d_buffer_t")) return HasImages;
      if (name.startswith("spir.image2d_t")) return HasImages;
      if (name.startswith("spir.image2d_array_t")) return HasImages;
      if (name.startswith("spir.image3d_t")) return HasImages;
      if (name.startswith("spir.sampler_t")) return true;
      if (name.startswith("spir.event_t")) return true;
      if (name.startswith("spir.half_t")) return true;
      if (name.startswith("spir.size_t")) return true;
    }
    CheckFailed(ErrorMessages(InvalidType), type);
    return false;
  }
  for (Type::subtype_iterator sti = type->subtype_begin(),
      ste = type->subtype_end(); sti != ste; ++sti) {
    if (!isValidType(*sti)) {
      CheckFailed(ErrorMessages(InvalidType), type);
      return false;
    }
  }
  return true;
}

// A Function type is valid if all of its operands are valid,
// it has a valid return type and is not a variable argument.
bool
SPIRVerifier::isValidType(const FunctionType *type)
{
  for (FunctionType::param_iterator pib = type->param_begin(),
      pie = type->param_end(); pib != pie; ++pib) {
    if (!isValidType(*pib)) {
      CheckFailed(ErrorMessages(InvalidType), *pib);
      return false;
    }
  }
  if (!isValidType(type->getReturnType())) {
      CheckFailed(ErrorMessages(InvalidType), type);
      return false;
  }
  // FIXME: What do we do about printf?
  return !type->isVarArg();
}
bool
SPIRVerifier::isValidType(const Type *type)
{
  // Verify the scalar types
  if (type->isIntegerTy(1) || type->isIntegerTy(8)
   || type->isIntegerTy(16) || type->isIntegerTy(32)
   || type->isIntegerTy(64) || type->isVoidTy()
   || type->isFloatTy() || type->isLabelTy())  return true;
  if (type->isDoubleTy()) {
    if (HasDouble) return true;
    CheckFailed(ErrorMessages(RequiresDouble), type);
    return false;
  }
  if (type->isVectorTy())
    return isValidType(dyn_cast<VectorType>(type));
  if (type->isArrayTy())
    return isValidType(dyn_cast<ArrayType>(type));
  if (type->isPointerTy())
    return isValidType(dyn_cast<PointerType>(type));
  if (type->isStructTy())
    return isValidType(dyn_cast<StructType>(type));
  if (type->isFunctionTy())
    return isValidType(dyn_cast<FunctionType>(type));
  if (type->isMetadataTy()) return true;

  CheckFailed(ErrorMessages(InvalidType), type);
  return false;
}

// Verify that the optional core features are valid.
// SPIR Spec section 2.12.1
void
SPIRVerifier::visitOptionalCoreMD(NamedMDNode *OptFeat)
{
  // Lets check the Optional features metadata for both valid
  // features and supported features. This has to be parsed
  // before the extensions since we require to know if
  // images are supported to determine if some image extensions
  // are also supported.
  if (!OptFeat) return;
  for (unsigned x = 0, y = OptFeat->getNumOperands(); x < y; ++x) {
    MDString *ptr = dyn_cast<MDString>(OptFeat->getOperand(x));
    if (!ptr) {
      CheckFailed(ErrorMessages(InvalidOptionalCore), OptFeat->getName());
      return;
    }
    if (ptr->getString() == "cl_doubles") {
      HasDouble = true;
      if (OptCore.find("cl_doubles") == std::string::npos) {
        CheckFailed(ErrorMessages(UnsupportedOptionalCore), ptr);
        return;
      }
    } else if (ptr->getString() == "cl_images") {
      HasImages = true;
      if (OptCore.find("cl_images") == std::string::npos) {
        CheckFailed(ErrorMessages(UnsupportedOptionalCore), ptr);
        return;
      }
    } else {
      CheckFailed(ErrorMessages(InvalidOptionalCore), ptr);
    }
  }
}

// Verify that the list of used extensions is valid and
// only includes the KHR extensions.
// SPIR Spec 2.12.2
void
SPIRVerifier::visitUsedExtensionsMD(NamedMDNode *KHRExt)
{
  if (!KHRExt) return;
  const char *valid12_extensions[14] = {
    "cl_khr_int64_base_atomics",
    "cl_khr_int64_extended_atomics",
    "cl_khr_fp16",
    "cl_khr_gl_sharing",
    "cl_khr_gl_event",
    "cl_khr_d3d10_sharing",
    "cl_khr_media_sharing",
    "cl_khr_d3d11_sharing",
    "cl_khr_global_int32_base_atomics",
    "cl_khr_global_int32_extended_atomics",
    "cl_khr_local_int32_base_atomics",
    "cl_khr_local_int32_extended_atomics",
    "cl_khr_byte_addressable_store",
    "cl_khr_3d_image_writes"
  };
  // TODO: since the list of valid extensions is known, should
  // we generate a perfect hash function for this to speed up
  // detection?
  // Lets check the list of 1.2 KHR extensions to make sure that the
  // ones declared in the module are valid and also check to
  // make sure that the ones declared in the module are supported
  // by the vendor/device.
  for (unsigned x = 0, y = KHRExt->getNumOperands(); x < y; ++x) {
    MDString *ptr = dyn_cast<MDString>(KHRExt->getOperand(x));
    if (!ptr) {
      CheckFailed(ErrorMessages(InvalidKHRExtension), KHRExt->getOperand(x));
      continue;
    }
    StringRef ext = ptr->getString();
    bool found = false;
    for (unsigned z = 0;
        z < sizeof(valid12_extensions) / sizeof(valid12_extensions[0]);
        ++z) {
      if (ext == valid12_extensions[z]) {
        found = true;
        break;
      }
    }
    if (!found) {
      CheckFailed(ErrorMessages(InvalidKHRExtension), ptr);
      return;
    }
    if (ext == "cl_khr_3d_image_writes" && !HasImages) {
      CheckFailed(ErrorMessages(UnsupportedKHRExtension), ptr);
      return;
    }
    if (KhrExt.find(ext) == std::string::npos) {
      CheckFailed(ErrorMessages(UnsupportedKHRExtension), ptr);
      return;
    }
  }
}

// Verify that the OCL version metadata follows the pattern:
// !spir.ocl.version = {i32, i32}
// SPIR Spec 2.14
void
SPIRVerifier::visitOCLVersionMD(NamedMDNode *OCLVersion)
{
  if (!OCLVersion) return;
  for (unsigned x = 0, y = OCLVersion->getNumOperands(); x < y; ++x) {
    MDNode *ptr = OCLVersion->getOperand(x);
    if (!ptr || ptr->getNumOperands() != 2) {
      CheckFailed(ErrorMessages(InvalidOCLVersion), ptr);
      return;
    }
    ConstantInt *major = dyn_cast<ConstantInt>(ptr->getOperand(0));
    ConstantInt *minor = dyn_cast<ConstantInt>(ptr->getOperand(1));
    if (0 == major || 0 == minor) {
      CheckFailed(ErrorMessages(InvalidOCLVersion), ptr);
      return;
    }
    if (major->getZExtValue() > OCLVer[0]
        || major->getZExtValue() > OCLVer[1]) {
      CheckFailed(ErrorMessages(UnsupportedOCLVersion), ptr);
      return;
    }
  }
}

// Verify that the SPIR version metadata follows the pattern:
// !spir.version = {i32, i32}
// SPIR Spec 2.13
void
SPIRVerifier::visitSPIRVersionMD(NamedMDNode *SPIRVersion)
{
  if (!SPIRVersion) return;
  for (unsigned x = 0, y = SPIRVersion->getNumOperands(); x < y; ++x) {
    MDNode *ptr = SPIRVersion->getOperand(x);
    if (ptr->getNumOperands() != 2) {
      CheckFailed(ErrorMessages(InvalidSPIRVersion), ptr);
      return;
    }
    ConstantInt *major = dyn_cast<ConstantInt>(ptr->getOperand(0));
    ConstantInt *minor = dyn_cast<ConstantInt>(ptr->getOperand(1));
    if (0 == major || 0 == minor) {
      CheckFailed(ErrorMessages(InvalidSPIRVersion), ptr);
      return;
    }
    if (major->getZExtValue() > SPIRVer[0]
        || major->getZExtValue() > SPIRVer[1]) {
      CheckFailed(ErrorMessages(UnsupportedSPIRVersion), ptr);
      return;
    }
  }
}

// Verify that the Sampler constructor follows the pattern:
// !spir.sampler.constructors = !{!0, !1}
// Where each item in the list is defined as:
//!0 = metadata !{Pointer to GlobalValue, i32 init_value}
//!1 = metadata !{Pointer to GlobalValue, i32 init_value}
void
SPIRVerifier::visitSamplerConstructorMD(NamedMDNode *Sampler)
{
  if (!Sampler) return;
  for (unsigned x = 0, y = Sampler->getNumOperands(); x < y; ++x) {
    MDNode *ptr = Sampler->getOperand(x);
    if (ptr->getNumOperands() != 2) {
      CheckFailed(ErrorMessages(InvalidSamplerMD), ptr);
      continue;
    }
    if (!dyn_cast<GlobalValue>(ptr->getOperand(0))) {
      CheckFailed(ErrorMessages(InvalidSamplerMD), ptr);
      continue;
    }
    if (!dyn_cast<ConstantInt>(ptr->getOperand(1))) {
      CheckFailed(ErrorMessages(InvalidSamplerMD), ptr);
      continue;
    }
    if (!ptr->getOperand(1)->getType()->isIntegerTy(32)) {
      CheckFailed(ErrorMessages(InvalidSamplerMD), ptr);
      continue;
    }
  }
}

// Format is:
// !spir.compiler.ext.options =
// metadata !{metadata !"opt1", ..., metadata !"optN"}
void
SPIRVerifier::visitCompilerExtOptsMD(NamedMDNode *ExtOpts)
{
  if (!ExtOpts) return;
  for (unsigned x = 0, y = ExtOpts->getNumOperands(); x < y; ++x) {
    MDString *ptr = dyn_cast<MDString>(ExtOpts->getOperand(x));
    if (!ptr) {
      CheckFailed(ErrorMessages(InvalidExtCompilerOpts),
          ExtOpts->getOperand(x));
      continue;
    }
  }
}
// Format is:
// !spir.compiler.options = metadata !{metadata !"opt1", ..., metadata !"optN"}
void
SPIRVerifier::visitCompilerOptsMD(NamedMDNode *CompOpts)
{
  if (!CompOpts) return;
  const char *valid12_options[] = {
    "-cl-single-precision-constant",
    "-cl-denorms-are-zero",
    "-cl-fp32-correctly-rounded-divide-sqrt",
    "-cl-opt-disable",
    "-cl-mad-enable",
    "-cl-no-signed-zeros",
    "-cl-unsafe-math-optimizations",
    "-cl-finite-math-only",
    "-cl-fast-relaxed-math",
    "-w",
    "-Werror",
    "-cl-kernel-arg-info",
    "-create-library",
    "-enable-link-options"
  };
  // Lets check the options to make sure that they are the ones that
  // are allowed by the SPIR spec section 2.9.
  for (unsigned x = 0, y = CompOpts->getNumOperands(); x < y; ++x) {
    MDString *ptr = dyn_cast<MDString>(CompOpts->getOperand(x));
    if (!ptr) {
      CheckFailed(ErrorMessages(InvalidCompilerOpts), CompOpts->getOperand(x));
      continue;
    }
    StringRef ext = ptr->getString();
    bool found = false;
    for (unsigned z = 0;
        z < sizeof(valid12_options) / sizeof(valid12_options[0]); ++z) {
      if (ext == valid12_options[z]) {
        found = true;
        break;
      }
    }
    if (!found) {
      CheckFailed(ErrorMessages(InvalidCompilerOpts), ptr);
      return;
    }
  }
}

void
SPIRVerifier::visitUserFuncsMD(NamedMDNode *funcs)
{
  if (!funcs) return;
  for (unsigned x = 0, y = funcs->getNumOperands(); x < y; ++x) {
    MDNode *ptr = funcs->getOperand(x);
    if (ptr->getNumOperands() == 0) {
      CheckFailed(ErrorMessages(InvalidFunctionMD), ptr);
      continue;
    }
    Function *F = dyn_cast<Function>(ptr->getOperand(0));
    if (!F) {
      CheckFailed(ErrorMessages(InvalidFunctionMD), ptr);
      continue;
    }
    for (unsigned w = 1, z = ptr->getNumOperands(); w < z; ++w) {
      MDNode *mdptr = dyn_cast<MDNode>(ptr->getOperand(w));
      if (!mdptr || mdptr->getNumOperands() == 0) {
        CheckFailed(ErrorMessages(InvalidFunctionMD), ptr->getOperand(w));
        continue;
      }
      MDString *mdname = dyn_cast<MDString>(mdptr->getOperand(0));
      if (!mdname) {
        CheckFailed(ErrorMessages(InvalidFunctionMD), mdptr->getOperand(0));
        continue;
      }
    }
  }
}

void
SPIRVerifier::visitKernelsMD(NamedMDNode *funcs)
{
  if (!funcs) return;
  // A kernel is everything that is in user funcs plus more.
  for (unsigned x = 0, y = funcs->getNumOperands(); x < y; ++x) {
    MDNode *ptr = funcs->getOperand(x);
    if (ptr->getNumOperands() == 0) {
      CheckFailed(ErrorMessages(InvalidFunctionMD), ptr);
      continue;
    }
    Function *F = dyn_cast<Function>(ptr->getOperand(0));
    if (!F) {
      CheckFailed(ErrorMessages(InvalidFunctionMD), ptr);
      continue;
    }
    FunctionType *FT = F->getFunctionType();
    for (unsigned w = 1, z = ptr->getNumOperands(); w < z; ++w) {
      MDNode *mdptr = dyn_cast<MDNode>(ptr->getOperand(w));
      if (!mdptr || mdptr->getNumOperands() == 0) {
        CheckFailed(ErrorMessages(InvalidFunctionMD), ptr->getOperand(w));
        continue;
      }
      MDString *mdname = dyn_cast<MDString>(mdptr->getOperand(0));
      if (!mdname) {
        CheckFailed(ErrorMessages(InvalidFunctionMD), mdptr->getOperand(0));
        continue;
      }
      StringRef id = mdname->getString();
      if (id == "work_group_size_hint" || id == "reqd_work_group_size") {
        if (mdptr->getNumOperands() == 4
            && mdptr->getOperand(1)->getType()->isIntegerTy(32)
            && mdptr->getOperand(2)->getType()->isIntegerTy(32)
            && mdptr->getOperand(3)->getType()->isIntegerTy(32))
          continue;
      } else if (id == "vec_type_hint") {
        if (mdptr->getNumOperands() == 3
            && isValidType(mdptr->getOperand(1)->getType())
            && mdptr->getOperand(2)->getType()->isIntegerTy(32)
            && dyn_cast<ConstantInt>(mdptr->getOperand(2))
            && dyn_cast<ConstantInt>(mdptr->getOperand(2))->getZExtValue() < 2)
          continue;
      } else if (id == "address_qualifier") {
        if (mdptr->getNumOperands() == 2
            && mdptr->getOperand(1)->getType()->isIntegerTy(32)
            && dyn_cast<ConstantInt>(mdptr->getOperand(1))
            && dyn_cast<ConstantInt>(mdptr->getOperand(1))->getZExtValue() 
            <= SPIRAS_CONSTANT_HOST)
          continue;
      } else if (id == "access_qualifier") {
        bool fail = false;
        if (FT->getNumParams() != mdptr->getNumOperands() - 1) {
          CheckFailed(ErrorMessages(InvalidFunctionMD), mdptr->getOperand(0));
          continue;
        }
        for (unsigned b = 1, c = mdptr->getNumOperands(); b < c; ++b) {
          if (!mdptr->getOperand(b)->getType()->isIntegerTy(32)
           || !dyn_cast<ConstantInt>(mdptr->getOperand(b))) {
            fail = true;
            break;
          }
          uint64_t val = dyn_cast<ConstantInt>(mdptr->getOperand(b))
            ->getZExtValue();
          if (val >= 4) {
            fail = true;
            break;
          }
          if (val != 0 
              && !isOpaqueNamedStruct(FT->getParamType(b-1), "spir.image")) {
            fail = true;
            break;
          }
        }
        if (!fail) continue;
      } else if (id == "arg_type_name") {
        if (mdptr->getNumOperands() == 2
            && dyn_cast<MDString>(mdptr->getOperand(1)))
          continue;
      } else if (id == "arg_type_qualifier") {
        if (mdptr->getNumOperands() == 2
            && mdptr->getOperand(1)->getType()->isIntegerTy(32)
            && dyn_cast<ConstantInt>(mdptr->getOperand(1))
            && dyn_cast<ConstantInt>(mdptr->getOperand(1))->getZExtValue() < 8)
          continue;
      } else if (id == "arg_name") {
        if (mdptr->getNumOperands() == 2
            && dyn_cast<MDString>(mdptr->getOperand(1)))
          continue;
      }
      CheckFailed(ErrorMessages(InvalidFunctionMD), mdname);
    }
  }
}
void
SPIRVerifier::visitNamedMDNode(NamedMDNode &NMD)
{
  StringRef name = NMD.getName();
  // We only need to investigate spir related named
  // metadata nodes.
  if (!name.startswith("spir.")) return;
  if (name == "spir.kernels") {
    visitKernelsMD(&NMD);
  } else if (name == "spir.user.functions") {
    visitUserFuncsMD(&NMD);
  } else if (name == "spir.compiler.options") {
    visitCompilerOptsMD(&NMD);
  } else if (name == "spir.compiler.ext.options") {
    visitCompilerExtOptsMD(&NMD);
  } else if (name == "spir.ocl.version") {
    visitOCLVersionMD(&NMD);
  } else if (name == "spir.version") {
    visitSPIRVersionMD(&NMD);
  } else if (name == "spir.used.extensions") {
    visitUsedExtensionsMD(&NMD);
  } else if (name == "spir.used.optional.core.features") {
    visitOptionalCoreMD(&NMD);
  } else if (name == "spir.sampler.constructors") {
    visitSamplerConstructorMD(&NMD);
  }

}
void
SPIRVerifier::visitGlobalValue(GlobalValue &GV)
{
  if (!GV.hasExternalLinkage()
      && !GV.hasAvailableExternallyLinkage()
      && !GV.hasPrivateLinkage()
      && !GV.hasInternalLinkage()) {
    CheckFailed(ErrorMessages(InvalidLinkage), &GV);
  }
  if (!GV.hasDefaultVisibility()) {
    CheckFailed(ErrorMessages(InvalidVisibility), &GV);
  }
  if (!isValidType(GV.getType())) {
    CheckFailed(ErrorMessages(InvalidType), &GV);
  }
}
void
SPIRVerifier::visitFunction(Function &F) {
  Function *func = &F;
  if (!isValidType(func->getReturnType())) {
    CheckFailed(ErrorMessages(InvalidType), func->getReturnType());
  }
  if (!isValidType(func->getFunctionType())) {
    CheckFailed(ErrorMessages(InvalidType), func->getFunctionType());
  }
  if (func->isVarArg()) {
    CheckFailed(ErrorMessages(InvalidVarArg), func);
  }	
  if (func->getCallingConv() != CallingConv::SPIR_KERNEL
      && func->getCallingConv() != CallingConv::SPIR_FUNC) {
    CheckFailed(ErrorMessages(BadCallingConv), func);
  }
  if (func->getAttributes().getFnAttributes()
      & ~(Attribute::NoUnwind
        | Attribute::AlwaysInline
        | Attribute::InlineHint
        | Attribute::NoInline
        | Attribute::ReadNone
        | Attribute::ReadOnly)) {
    CheckFailed(ErrorMessages(InvalidFuncAttribute), func);
  }
  if (func->getAttributes().getRetAttributes()
      & ~(Attribute::ZExt
        | Attribute::SExt
        | Attribute::ByVal
        | Attribute::StructRet
        | Attribute::NoCapture)) {
    CheckFailed(ErrorMessages(InvalidArgAttribute), func);
  }
  for (Function::const_arg_iterator caib = func->arg_begin(),
      caie = func->arg_end(); caib != caie; ++caib) {
    if (caib->hasNestAttr()) {
      const Argument *arg = caib;
      CheckFailed(ErrorMessages(InvalidArgAttribute), arg);
    }
  }
  if (func->hasGC()) {
    CheckFailed(ErrorMessages(InvalidGarbageCollection), func);
  }

  if (func->isIntrinsic()) {
    //    unsigned id = func->getIntrinsicID();
    if (!isIntrEncodingValid(func)) {
      CheckFailed(ErrorMessages(InvalidNameMangling), func->getFunctionType());
    }
  }
}

// Verify that the intrinsic follows the pattern:
// spir.sampler_t llvm.spir.sampler.initialize(i32, i32, i32)
// SPIR Spec 2.1.3.1
bool
SPIRVerifier::isSamplerValid(const Function *func)
{
  const StructType *type = dyn_cast<StructType>(func->getReturnType());
  if (!func || !type->isOpaque()
      || !type->hasName()
      || (type->hasName()
        && !type->getName().startswith("spir.sampler_t"))) {
    CheckFailed(ErrorMessages(InvalidIntrRet), func);
    return false;
  }
  FunctionType *samp = func->getFunctionType();
  if (samp->getNumParams() != 3) {
    CheckFailed(ErrorMessages(InvalidIntrFunc), func);
    return false;
  }
  for (FunctionType::param_iterator pib = samp->param_begin(),
      pie = samp->param_end(); pib != pie; ++pib) {
    if (!(*pib)->isIntegerTy(32)) {
      CheckFailed(ErrorMessages(InvalidIntrArg), (*pib));
      return false;
    }
  }
  return true;
}
bool isTypeEncodingCorrect(Type *p, const char* ptr)
{
  return (p->isIntegerTy(1) && !strncmp(ptr, "i1", 2)) //< i1  check
    || (p->isIntegerTy(8) && !strncmp(ptr, "i8", 2))   //< i8  check
    || (p->isIntegerTy(16) && !strncmp(ptr, "i16", 3)) //< i16 check
    || (p->isIntegerTy(32) && !strncmp(ptr, "i32", 3)) //< i32 check
    || (p->isIntegerTy(64) && !strncmp(ptr, "i64", 3)) //< i64 check
    || (p->isFloatTy() && !strncmp(ptr, "fp32", 4))    //< f32 check
    || (p->isDoubleTy() && !strncmp(ptr, "fp64", 4))   //< f64 check
    || (isOpaqueNamedStruct(p, "spir.half_t")        //< f16 check
        && !strncmp(ptr, "fp16", 4))
    || (p->isPointerTy()                             //< p[0-5] check
        && isAddressSpace(p, (*(ptr + 1) - '0')));
}
  bool
SPIRVerifier::isSizeIntrValid(const Function *func)
{
  const std::string &name = func->getName().str();
  // Lets skip llvm.spir.sizet. since we should have
  // already verified that the incoming function starts
  // with it.
  assert(!strncmp(name.c_str(), "llvm.spir.sizet.", 16)
      && "Only an intrinsic starting with llvm.spir.size. should be"
      " passed into this function.");
  const char* ptr = name.c_str() + 16;
  FunctionType *FT = func->getFunctionType();
  if (!strncmp(ptr, "convert.from.", 13)) {
    ptr += 13;
    if (!isOpaqueNamedStruct(FT->getReturnType(), "spir.size_t")) {
      CheckFailed(ErrorMessages(InvalidIntrRet), FT->getReturnType());
      return false;
    }
    if (FT->getNumParams() != 1) {
      CheckFailed(ErrorMessages(InvalidIntrFunc), FT);
      return false;
    }
    Type *p = FT->getParamType(0);
    if (!isTypeEncodingCorrect(p, ptr)) {
      CheckFailed(ErrorMessages(InvalidIntrArg), p);
      return false;
    }
  } else if (!strncmp(ptr, "convert.to.", 11)) {
    ptr += 11;
    if (FT->getNumParams() != 1) {
      CheckFailed(ErrorMessages(InvalidIntrFunc), FT);
      return false;
    }
    if (!isOpaqueNamedStruct(FT->getParamType(0), "spir.size_t")) {
      CheckFailed(ErrorMessages(InvalidIntrArg), FT->getReturnType());
      return false;
    }
    Type *p = FT->getReturnType();
    if (!isTypeEncodingCorrect(p, ptr)) {
      CheckFailed(ErrorMessages(InvalidIntrRet), p);
      return false;
    }
  } else if (!strncmp(ptr, "not", 3)) {
    if (FT->getNumParams() != 1) {
      CheckFailed(ErrorMessages(InvalidIntrFunc), FT);
      return false;
    }
    if (!isOpaqueNamedStruct(FT->getReturnType(), "spir.size_t")) {
      CheckFailed(ErrorMessages(InvalidIntrRet), FT->getReturnType());
      return false;
    }
    if (!isOpaqueNamedStruct(FT->getParamType(0), "spir.size_t")) {
      CheckFailed(ErrorMessages(InvalidIntrArg), FT->getParamType(0));
      return false;
    }
  } else if (!strncmp(ptr, "add.p", 5)) {
    // The check for the pointer add needs to come before the check
    // for the size_t add.
    ptr += 4;
    if (FT->getNumParams() != 2) {
      CheckFailed(ErrorMessages(InvalidIntrFunc), FT);
      return false;
    }
    Type *p = FT->getReturnType();
    if (!isTypeEncodingCorrect(p, ptr)) {
      CheckFailed(ErrorMessages(InvalidIntrRet), p);
      return false;
    }
    p = FT->getParamType(0);
    if (!isTypeEncodingCorrect(p, ptr)) {
      CheckFailed(ErrorMessages(InvalidIntrArg), p);
      return false;
    }
    p = FT->getParamType(1);
    if (!isOpaqueNamedStruct(p, "spir.size_t")) {
      CheckFailed(ErrorMessages(InvalidIntrArg), p);
      return false;
    }
  } else if (!strncmp(ptr, "add", 3)
      || !strncmp(ptr, "sub", 3)
      || !strncmp(ptr, "mul", 3)
      || !strncmp(ptr, "and", 3)
      || !strncmp(ptr, "xor", 3)
      || !strncmp(ptr, "shl", 3)
      || !strncmp(ptr, "shr", 3)
      || !strncmp(ptr, "udiv", 4)
      || !strncmp(ptr, "sdiv", 4)
      || !strncmp(ptr, "urem", 4)
      || !strncmp(ptr, "srem", 4)
      || !strncmp(ptr, "shar", 4)
      || !strncmp(ptr, "or", 2)
      ) {
    if (FT->getNumParams() != 2) {
      CheckFailed(ErrorMessages(InvalidIntrFunc), FT);
      return false;
    }
    if (!isOpaqueNamedStruct(FT->getReturnType(), "spir.size_t")) {
      CheckFailed(ErrorMessages(InvalidIntrRet), FT->getReturnType());
      return false;
    }
    if (!isOpaqueNamedStruct(FT->getParamType(0), "spir.size_t")) {
      CheckFailed(ErrorMessages(InvalidIntrArg), FT->getParamType(0));
      return false;
    }
    if (!isOpaqueNamedStruct(FT->getParamType(1), "spir.size_t")) {
      CheckFailed(ErrorMessages(InvalidIntrArg), FT->getParamType(1));
      return false;
    }
  } else if (!strncmp(ptr, "cmp", 3)) {
    if (FT->getNumParams() != 3) {
      CheckFailed(ErrorMessages(InvalidIntrFunc), FT);
      return false;
    }
    if (!FT->getReturnType()->isIntegerTy(1)) {
      CheckFailed(ErrorMessages(InvalidIntrRet), FT->getReturnType());
      return false;
    }
    if (!isOpaqueNamedStruct(FT->getParamType(0), "spir.size_t")) {
      CheckFailed(ErrorMessages(InvalidIntrArg), FT->getParamType(0));
      return false;
    }
    if (!isOpaqueNamedStruct(FT->getParamType(1), "spir.size_t")) {
      CheckFailed(ErrorMessages(InvalidIntrArg), FT->getParamType(1));
      return false;
    }
    if (!FT->getParamType(2)->isIntegerTy(32)) {
      CheckFailed(ErrorMessages(InvalidIntrArg), FT->getParamType(2));
      return false;
    }
  } else if (!strncmp(ptr, "alloca", 6)) {
    if (FT->getNumParams() != 2) {
      CheckFailed(ErrorMessages(InvalidIntrFunc), FT);
      return false;
    }
    if (!isOpaqueNamedStruct(FT->getReturnType(), "spir.size_t")) {
      CheckFailed(ErrorMessages(InvalidIntrRet), FT->getReturnType());
      return false;
    }
    if (!FT->getParamType(0)->isIntegerTy(32)) {
      CheckFailed(ErrorMessages(InvalidIntrArg), FT->getParamType(0));
      return false;
    }
    if (!FT->getParamType(1)->isIntegerTy(32)) {
      CheckFailed(ErrorMessages(InvalidIntrArg), FT->getParamType(1));
      return false;
    }
  } else if (!strncmp(ptr, "of.size.t", 9)) {
    if (FT->getNumParams() != 0) {
      CheckFailed(ErrorMessages(InvalidIntrFunc), FT);
      return false;
    }
    if (!isOpaqueNamedStruct(FT->getReturnType(), "spir.size_t")) {
      CheckFailed(ErrorMessages(InvalidIntrRet), FT->getReturnType());
      return false;
    }
  } else if (!strncmp(ptr, "of.pointer", 10)) {
    if (FT->getNumParams() != 0) {
      CheckFailed(ErrorMessages(InvalidIntrFunc), FT);
      return false;
    }
    if (!isOpaqueNamedStruct(FT->getReturnType(), "spir.size_t")) {
      CheckFailed(ErrorMessages(InvalidIntrRet), FT->getReturnType());
      return false;
    }
  } else {
    CheckFailed(ErrorMessages(InvalidIntrArg), func);
    return false;
  }
  return true;
}

// This function returns true if the number in the
// tok StringRef is equal to the vector number of
// elements.
bool isVectorTokenEqual(StringRef tok, const Type *T)
{
  const VectorType *VT = dyn_cast<VectorType>(T);
  return VT && (tok == itostr(VT->getNumElements()));
}

// This function returns true if the type is an
// opaque structure with a name that is equal
// to the name passed in the second argument.
bool isOpaqueNamedStruct(const Type *t, StringRef name)
{
  const StructType *ST = dyn_cast<StructType>(t);
  return ST && ST->isOpaque() && ST->hasName()
  && ST->getName().startswith(name);
}

bool isAddressSpace(const Type *t, unsigned as)
{
  const PointerType *PT = dyn_cast<PointerType>(t);
  return PT && PT->getAddressSpace() == as;
}

bool
SPIRVerifier::isIntrEncodingValid(const Function *func)
{
  assert(func->isIntrinsic() && "Must be an intrinsic to enter this function!");
  const std::string &name = func->getName().str();
  if (strncmp(name.c_str(), "llvm.spir.", 10)
      && strncmp(name.c_str(), "spir.", 5)) return false;
  unsigned offset = 10;
  const char* ptr = name.c_str() + offset;
  const FunctionType *FT = func->getFunctionType();
  if (!strncmp(ptr + offset, "builtin.", 7)) {
    offset += 7;
  } else if (!strncmp(ptr + offset, "sizet.", 6)) {
    return isSizeIntrValid(func);
  } else if (!strncmp(ptr + offset, "sampler.initialize", 18)) {
    return isSamplerValid(func);
  } else if (!strncmp(ptr + offset, "get.null.p", 10)) {
    return isAddressSpace(FT->getReturnType(), *(ptr + offset + 10) - '0');
  } else if (!strncmp(ptr + offset, "opencl.version", 14)
      || !strncmp(ptr + offset, "endian.little", 13)
      || !strncmp(ptr + offset, "image.support", 13)) {
    return FT->getReturnType()->isIntegerTy(32);
  } else if (!strncmp(ptr + offset, "memcpy.", 7)) {
    offset += 7;
  }
  SmallVector<StringRef, 16> tokens;
  StringRef src = name;
  SplitString(src, tokens, ".\n");
  for (unsigned x = FT->getNumParams(), y = tokens.size();
      x > 0 && y > 0; --x, --y) {
    Type *curParam = FT->getParamType(x - 1);
    StringRef curTok = tokens[y - 1];
    if (curTok == "i1") {
      if (curParam->isIntegerTy(1)) continue;
    } else if (curTok == "i8" || curTok == "u8") {
      if (curParam->isIntegerTy(8)) continue;
      if ((curParam->isVectorTy()
          && dyn_cast<VectorType>(curParam)->
            getElementType()->isIntegerTy(8))
          && isVectorTokenEqual(tokens[y-2], curParam)) {
        // Lets skip the next token since we have already
        // validated it.
        --y;
        continue;
      }
    } else if (curTok == "i16" || curTok == "u16") {
      if (curParam->isIntegerTy(16)) continue;
      if ((curParam->isVectorTy()
          && dyn_cast<VectorType>(curParam)->
            getElementType()->isIntegerTy(16))
          && isVectorTokenEqual(tokens[y-2], curParam)) {
        // Lets skip the next token since we have already
        // validated it.
        --y;
        continue;
      }
    } else if (curTok == "i32" || curTok == "u32") {
      if (curParam->isIntegerTy(32)) continue;
      if ((curParam->isVectorTy()
          && dyn_cast<VectorType>(curParam)->
            getElementType()->isIntegerTy(32))
          && isVectorTokenEqual(tokens[y-2], curParam)) {
        // Lets skip the next token since we have already
        // validated it.
        --y;
        continue;
      }
    } else if (curTok == "i64" || curTok == "u64") {
      if (curParam->isIntegerTy(64) ) continue;
      if ((curParam->isVectorTy()
          && dyn_cast<VectorType>(curParam)->
            getElementType()->isIntegerTy(64))
          && isVectorTokenEqual(tokens[y-2], curParam)) {
        // Lets skip the next token since we have already
        // validated it.
        --y;
        continue;
      }
    } else if (curTok == "f") {
      if (curParam->isFloatTy()) continue;
      if ((curParam->isVectorTy()
          && dyn_cast<VectorType>(curParam)->
            getElementType()->isFloatTy())
          && isVectorTokenEqual(tokens[y-2], curParam)) {
        // Lets skip the next token since we have already
        // validated it.
        --y;
        continue;
      }
    } else if (curTok == "d") {
      if (curParam->isDoubleTy()) continue;
      if ((curParam->isVectorTy()
          && dyn_cast<VectorType>(curParam)->
            getElementType()->isDoubleTy())
          && isVectorTokenEqual(tokens[y-2], curParam)) {
        // Lets skip the next token since we have already
        // validated it.
        --y;
        continue;
      }
    } else if (curTok == "h"
        && isOpaqueNamedStruct(curParam, "spir.half_t")) {
      continue;
    } else if ((curTok == "ptrdifft" || curTok == "sizet")
        && isOpaqueNamedStruct(curParam, "spir.size_t")) {
      continue;
    } else if (curTok == "img1d"
        && isOpaqueNamedStruct(curParam, "spir.image1d_t")) {
      continue;
    } else if (curTok == "img1darr"
        && isOpaqueNamedStruct(curParam, "spir.image1d_array_t")) {
      continue;
    } else if (curTok == "img1dbuf"
        && isOpaqueNamedStruct(curParam, "spir.image1d_buffer_t")) {
      continue;
    } else if (curTok == "img2d"
        && isOpaqueNamedStruct(curParam, "spir.image2d_t")) {
      continue;
    } else if (curTok == "img2darr"
        && isOpaqueNamedStruct(curParam, "spir.image2d_array_t")) {
      continue;
    } else if (curTok == "img3d"
        && isOpaqueNamedStruct(curParam, "spir.image3d_t")) {
      continue;
    } else if (curTok == "evt"
        && isOpaqueNamedStruct(curParam, "spir.event_t")) {
      continue;
    } else if (curTok == "smpl"
        && isOpaqueNamedStruct(curParam, "spir.sampler_t")) {
      continue;
    } else if (curTok == "p0" && isAddressSpace(curParam, SPIRAS_PRIVATE)) {
        continue;
    } else if (curTok == "p1" && isAddressSpace(curParam, SPIRAS_GLOBAL)) {
        continue;
    } else if (curTok == "p2" && isAddressSpace(curParam, SPIRAS_CONSTANT)) {
        continue;
    } else if (curTok == "p3" && isAddressSpace(curParam, SPIRAS_LOCAL)) {
        continue;
    } else if (curTok == "p4" && isAddressSpace(curParam, SPIRAS_GLOBAL_HOST)) {
        continue;
    } else if (curTok == "p5"
        && isAddressSpace(curParam, SPIRAS_CONSTANT_HOST)) {
        continue;
    }
    CheckFailed(ErrorMessages(InvalidIntrArg), curParam);
    return false;
  }
  return true;
}

// SPIR Spec section 3.3
template <class T>
bool SPIRVerifier::verifyOperands(Instruction &I)
{

    for (typename T::op_iterator oib = static_cast<T&>(I).op_begin(),
      oie = static_cast<T&>(I).op_end();
      oib != oie; ++oib) {
    if (!isValidType(oib->get()->getType())) {
      CheckFailed(ErrorMessages(InvalidType), *oib);
      return false;
    }
  }

  return true;
}

void
SPIRVerifier::visitInstruction(Instruction &I)
{
  switch(I.getOpcode()) {
    case Instruction::Br:
    case Instruction::Ret:
    case Instruction::Switch:
    case Instruction::Unreachable:
      if (!verifyOperands<TerminatorInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::Add:
    case Instruction::Sub:
    case Instruction::Mul:
    case Instruction::UDiv:
    case Instruction::URem:
    case Instruction::SDiv:
    case Instruction::SRem:
    case Instruction::FAdd:
    case Instruction::FSub:
    case Instruction::FMul:
    case Instruction::FDiv:
    case Instruction::FRem:
    case Instruction::Shl:
    case Instruction::LShr:
    case Instruction::AShr:
    case Instruction::And:
    case Instruction::Or:
    case Instruction::Xor:
      if (!verifyOperands<BinaryOperator>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      if (I.getOpcode() == Instruction::AShr) {
        if (static_cast<BinaryOperator&>(I).isExact()) {
          CheckFailed(ErrorMessages(IllegalAShrExact), &I);
        }
      }
      break;
    case Instruction::ExtractElement:
      if (!verifyOperands<ExtractElementInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::InsertElement:
      if (!verifyOperands<InsertElementInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::ShuffleVector:
      if (!verifyOperands<ShuffleVectorInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::ExtractValue:
      if (!verifyOperands<ExtractValueInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::InsertValue:
      if (!verifyOperands<InsertValueInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::Load:
      if (!verifyOperands<LoadInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      if (dyn_cast<LoadInst>(&I)->isAtomic()) {
        CheckFailed(ErrorMessages(NoAtomicLoadStore), &I);
      }
      if (!dyn_cast<LoadInst>(&I)->getAlignment()) {
        CheckFailed(ErrorMessages(UnalignedLoadStore), &I);
      }
      break;
    case Instruction::Store:
      if (!verifyOperands<StoreInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      if (dyn_cast<StoreInst>(&I)->isAtomic()) {
        CheckFailed(ErrorMessages(NoAtomicLoadStore), &I);
      }
      if (!dyn_cast<StoreInst>(&I)->getAlignment()) {
        CheckFailed(ErrorMessages(UnalignedLoadStore), &I);
      }
      break;
    case Instruction::GetElementPtr:
      if (!verifyOperands<GetElementPtrInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::Alloca:
      if (!verifyOperands<AllocaInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::Trunc:
    case Instruction::ZExt:
    case Instruction::SExt:
    case Instruction::FPToSI:
    case Instruction::SIToFP:
    case Instruction::FPToUI:
    case Instruction::UIToFP:
    case Instruction::FPTrunc:
    case Instruction::FPExt:
      if (!verifyOperands<UnaryInstruction>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      for (UnaryInstruction::op_iterator oib = I.op_begin(),
          oie = I.op_end(); oib != oie; ++oie) {
        if (oib->get()->getType()->isVectorTy()) {
          CheckFailed(ErrorMessages(VectorConversion), &I);
        }
      }
      break;
    case Instruction::ICmp:
    case Instruction::FCmp:
      if (!verifyOperands<CmpInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::BitCast:
      if (!verifyOperands<BitCastInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::PHI:
      if (!verifyOperands<PHINode>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::Select:
      if (!verifyOperands<SelectInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
    case Instruction::Call:
      if (!verifyOperands<CallInst>(I))
        CheckFailed(ErrorMessages(InvalidInstr), &I);
      {
        CallInst &CI = static_cast<CallInst&>(I);
        if (CI.isTailCall()) {
          CheckFailed(ErrorMessages(NoTailCall), &I);
        }
        if (CI.getCallingConv() != CallingConv::SPIR_KERNEL
            && CI.getCallingConv() != CallingConv::SPIR_FUNC) {
          CheckFailed(ErrorMessages(BadCallingConv), &I);
        }
        if (!CI.paramHasAttr(~0U, Attribute::NoUnwind)) {
          CheckFailed(ErrorMessages(NoNounwind), &I);
        }
        if (CI.getAttributes().getFnAttributes()
            & ~(Attribute::NoUnwind
              | Attribute::AlwaysInline
              | Attribute::InlineHint
              | Attribute::NoInline
              | Attribute::ReadNone
              | Attribute::ReadOnly)) {
          CheckFailed(ErrorMessages(InvalidFuncAttribute), &I);
        }
        if (CI.getAttributes().getRetAttributes()
            & ~(Attribute::ZExt
              | Attribute::SExt
              | Attribute::ByVal
              | Attribute::StructRet
              | Attribute::NoCapture)) {
          CheckFailed(ErrorMessages(InvalidArgAttribute), I.getOperand(0));
        }

        if (CI.isInlineAsm()) {
          CheckFailed(ErrorMessages(InlineAsm), &I);
        }
        Function *fn = CI.getCalledFunction();
        if (0 == fn) {
          CheckFailed(ErrorMessages(IndirectCall), &I);
        }
        for (unsigned x = 0, y = CI.getNumArgOperands(); x < y; ++x) {
          if (CI.getAttributes().getParamAttributes(x + 1)
              & ~(Attribute::ZExt
                | Attribute::SExt
                | Attribute::ByVal
                | Attribute::StructRet
                | Attribute::NoCapture)) {
            CheckFailed(ErrorMessages(InvalidArgAttribute), I.getOperand(x + 1));
          }
        }
      }
      break;
    case Instruction::IntToPtr:
    case Instruction::PtrToInt:
      CheckFailed(ErrorMessages(InvalidPtrInst), &I);
      break;
    default:
      CheckFailed(ErrorMessages(InvalidInstr), &I);
      break;
  }
}

//===----------------------------------------------------------------------===//
//  Implement the public interfaces to this file...
//===----------------------------------------------------------------------===//

FunctionPass *
llvm::createLightweightSPIRVerifierPass(VerifierFailureAction action,
    std::string CoreFeat, std::string KhrFeat,
    unsigned SPIRMajor, unsigned SPIRMinor,
    unsigned OCLMajor, unsigned OCLMinor)
{
  SPIRVerifier *V = new SPIRVerifier(action, true);
  V->OptCore = CoreFeat;
  V->KhrExt = KhrFeat;
  V->SPIRVer[0] = SPIRMajor;
  V->SPIRVer[1] = SPIRMinor;
  V->OCLVer[0] = OCLMajor;
  V->OCLVer[1] = OCLMinor;
  return V;
}

FunctionPass *
llvm::createHeavyweightSPIRVerifierPass(VerifierFailureAction action,
    std::string CoreFeat, std::string KhrFeat,
    unsigned SPIRMajor, unsigned SPIRMinor,
    unsigned OCLMajor, unsigned OCLMinor)
{
  SPIRVerifier *V = new SPIRVerifier(action, false);
  V->OptCore = CoreFeat;
  V->KhrExt = KhrFeat;
  V->SPIRVer[0] = SPIRMajor;
  V->SPIRVer[1] = SPIRMinor;
  V->OCLVer[0] = OCLMajor;
  V->OCLVer[1] = OCLMinor;
  return V;
}

