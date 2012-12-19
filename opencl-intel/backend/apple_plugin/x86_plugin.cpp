//===-- x86_plugin.cpp - OpenCL CPU Driver LLVM Interface -----------------===//
//
// Copyright:  (c) 2007-2011 by Apple, Inc., All Rights Reserved.
//
//===----------------------------------------------------------------------===//
//
// This file implements the plugin interface to CVMS for the OpenCL CPU device
// driver.
//
// Source code is compiled with clang into an LLVM module, which is then linked
// with a bitcode runtime library to produce a final linked module.  This module
// is emitted to a mach-o object file in memory, and then linked into a mach-o
// bundle, which is cached by CVMS.
//
// For information on how CVMS returns data from the server to the client, see
// allocateCVMSReturnData().
//
//===----------------------------------------------------------------------===//
#define GL_TARGET_GL
#include <OpenGL/gl.h>
#include <OpenGL/cl_driver_types.h>
#include <dlfcn.h>
#include <sys/cdefs.h>
#include "llvm/CallingConv.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/GlobalVariable.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/OwningPtr.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringMap.h"

#include "llvm/Analysis/Verifier.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Version.h"

#include "llvm/Support/TargetRegistry.h"

#include <string>
#include <utility>
#include <vector>
#include "x86_archive.h"
#include "x86_cvms.h"
#include <cvms/plugin.h>

#include "TargetArch.h"
#include "CPUDetect.h"
#include "Optimizer.h"
#include "MetaDataApi.h"
#include "VolcanoWrapper/VecConfig.h"
#include "cl_device_api.h"
#include "CompilationUtils.h"
#include "TypeAlignment.h"

#ifdef CLD_ASSERT
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

using namespace llvm;

__BEGIN_DECLS

namespace Intel {
  unsigned int getBarrierBufferStrideSize(OpenCL::DeviceBackend::Optimizer& optimizer, const Function& func) {
    std::map<std::string, unsigned int> bufferStrideMap;
    optimizer.getPrivateMemorySize(bufferStrideMap);
    return bufferStrideMap[func.getName().str()];
  }
}

int Link(std::vector<std::string*>& objs, const char *path, CFDataRef dict,
         std::vector<unsigned char> &dylib, std::string &log);

int cld_link(Module *M, Module *Runtime);

__END_DECLS

Intel::CPUId selectCPU();

/// Used to keep the plugin instance specific data
/// allocated by cvmsPluginServiceInitialize
struct CPUPluginPrivateData
{
  Intel::CPUId cpuId;
};

extern "C" void fillNoBarrierPathSet(llvm::Module *M, std::set<std::string>& noBarrierPath);

/// alloc_kernel_info - Create a dictionary containing argument info for all
/// kernels in the program.
static
int alloc_kernels_info(CFMutableDictionaryRef *info,
                       ConstantArray *init,
                       Module *M, Intel::OpenCL::DeviceBackend::Optimizer &optimizer) {

  // Aquire named metadata node for kernel arg info. Right now only type info
  // is recorded.
  Intel::MetaDataUtils mdUtils(M);
  unsigned numKernels = mdUtils.size_Kernels();
  // Create the info dictionary which will store CFString program names as
  // the keys, and CFArrays of CFNumbers for the size|flags fields.
  CFMutableDictionaryRef d;
  d = CFDictionaryCreateMutable(NULL, numKernels,
                                &kCFTypeDictionaryKeyCallBacks,
                                &kCFTypeDictionaryValueCallBacks);
  if (!d)
    return -1;
  
  *info = d;
  if (!numKernels) {
    return 0;
  }
  // Create the TargetData structure from the Module's arch info.
  TargetData TD(M);

  Intel::OpenCL::DeviceBackend::FunctionWidthVector vectorFunc;
  optimizer.GetVectorizedFunctions(vectorFunc);
  std::vector<Intel::OpenCL::DeviceBackend::FunctionWidthPair>::const_iterator vecIter = vectorFunc.begin();
  std::vector<Intel::OpenCL::DeviceBackend::FunctionWidthPair>::const_iterator vecEnd = vectorFunc.end();

  std::set<std::string> noBarrierPath;
  fillNoBarrierPathSet(M, noBarrierPath);

  Intel::KernelMetaDataHandle kmd;

  NamedMDNode *OpenCLKernelWrapperMD = M->getNamedMetadata("opencl.wrappers");
  Intel::MetaDataUtils::KernelsList::const_iterator iter = mdUtils.begin_Kernels(), end = mdUtils.end_Kernels();
  for(unsigned int i=0; iter != end; ++iter, ++i) {
    Intel::KernelMetaDataHandle kmd = (*iter);
    // In case the cast is wrong an assertion failure will be thrown
    llvm::Function *pFunc = (*iter)->getFunction();
    // The wrapper function that receives a single buffer as argument is the last node in the metadata
    MDNode *wrapperN = OpenCLKernelWrapperMD->getOperand(i);
    llvm::Function *pWrapperFunc = llvm::cast<llvm::Function>(wrapperN->getOperand(0)->stripPointerCasts());

#if !1 //reqd_work_group_size
    MDNode *KernelRWGsize = 0; // MDNode with kernel required work group size.
    // Search the rest of the operands to find the MDNode we are interested in.
    for (unsigned j = 1; j < kernelN->getNumOperands(); ++j) {
      MDNode *data = dyn_cast<MDNode>(kernelN->getOperand(j));
      if (data) {
        else if (key->getString() == "reqd_work_group_size")
          KernelRWGsize = data;
      }
    }
#endif

    // Get the kernel arguments info
    std::vector<cl_kernel_argument> arguments;
    Intel::OpenCL::DeviceBackend::CompilationUtils::parseKernelArguments(M,  pFunc, arguments);

    CFMutableArrayRef kf_argsizes = CFArrayCreateMutable(NULL,
        arguments.size(), &kCFTypeArrayCallBacks);

    CFMutableArrayRef kf_argalignments = CFArrayCreateMutable(NULL,
        arguments.size(), &kCFTypeArrayCallBacks);

    //Calculate the size and alignment of each argument from the arguments info
    for(unsigned int j=0; j<arguments.size(); ++j) {
        cl_kernel_argument arg = arguments[j];
    
        // Create a CFNumber to hold the size value.
        int size = ((int)Intel::OpenCL::DeviceBackend::TypeAlignment::getSize(arg));
        const void *vptrsize = (const void *)&size;
        CFNumberRef valsizeref = CFNumberCreate(NULL, kCFNumberIntType, vptrsize);

        // Append size value to the function arg sizes array.
        CFArrayAppendValue(kf_argsizes, valsizeref);
        CFRelease(valsizeref);

        // Create a CFNumber to hold the alignment value.
        int alignment = ((int)Intel::OpenCL::DeviceBackend::TypeAlignment::getAlignment(arg));
        const void *vptralignment = (const void *)&alignment;
        CFNumberRef valalignmentref = CFNumberCreate(NULL, kCFNumberIntType, vptralignment);

        // Append size value to the function arg sizes array.
        CFArrayAppendValue(kf_argalignments, valalignmentref);
        CFRelease(valalignmentref);
    }

    ConstantStruct *elt = cast<ConstantStruct>(init->getOperand(i));
    Function *kf = cast<Function>(elt->getOperand(0)->stripPointerCasts());
    //assert(kf->getName().str() == pWrapperFunc->getName().str() && "annotcation has differemt kernel order than metadata!");

    // This is a function, pull out the arg info string.
    Value *field1 = elt->getOperand(1)->stripPointerCasts();
    GlobalVariable *sgv = cast<GlobalVariable>(field1);

    // Get the description of the arguments that the front end generated, so
    // that we can put the read and write information for images in the arg
    // description structure.
    std::string args_desc;
    if (ConstantDataArray *f1arr = dyn_cast<ConstantDataArray>(sgv->getInitializer()))
    if (f1arr->isString())
      args_desc = f1arr->getAsString();

    // Create and fill in the kernel args description structure.
    CFMutableArrayRef kfinfo = CFArrayCreateMutable(NULL, kf->arg_size(),
                                                    &kCFTypeArrayCallBacks);

    CFMutableArrayRef kf_argnames = CFArrayCreateMutable(NULL, kf->arg_size(),
                                                         &kCFTypeArrayCallBacks);

    CFMutableArrayRef kf_argtypes = CFArrayCreateMutable(NULL,
                                                         kf->arg_size(),
                                                         &kCFTypeArrayCallBacks);

    CFMutableArrayRef kf_argtypequals = CFArrayCreateMutable(NULL,
                                                          kf->arg_size(),
                                                          &kCFTypeArrayCallBacks);

    unsigned j = 0;
    for (Function::arg_iterator ai = kf->arg_begin(), ae = kf->arg_end(); ai != ae; ++ai, ++j) {
    Type *Ty = ai->getType();

    // If this is a byval argument, it is always a pointer type.  We actually
    // care about the type of the argument being pointed to by the byval arg.
    if (ai->hasByValAttr())
      Ty = cast<PointerType>(Ty)->getElementType();

    unsigned desc = args_desc.empty() ? 0 : args_desc[j];

    // The size of the argument in bytes, if requested.
    uint32_t size = TD.getTypeAllocSize(Ty);

    // If the argument is a pointer, it is a stream. If the pointer type has
    // an address space qualifier of 3 (local), then set the local flag on the
    // stream as well.
    unsigned flags = 0;
    switch (desc)  {
      case '1': flags = CLD_ARGS_FLAGS_GLOBAL | CLD_ARGS_FLAGS_RD; break;
      case '2': flags = CLD_ARGS_FLAGS_GLOBAL | CLD_ARGS_FLAGS_WR; break;
      case '4': flags = CLD_ARGS_FLAGS_SAMPLER; break;
      case '5': flags = CLD_ARGS_FLAGS_IMAGE | CLD_ARGS_FLAGS_RD; break;
      case '6': flags = CLD_ARGS_FLAGS_IMAGE | CLD_ARGS_FLAGS_WR; break;
      case '8': flags = CLD_ARGS_FLAGS_CONST; break;
      case '9': flags = CLD_ARGS_FLAGS_LOCAL; break;
      case 'a': flags = CLD_ARGS_FLAGS_IMAGE | CLD_ARGS_FLAGS_RD | CLD_ARGS_FLAGS_WR; break;
      default: break;
    }

    assert(sizeof(uint64_t) == 2 * sizeof(size));
    assert(sizeof(uint64_t) == 2 * sizeof(flags));

    // Create a CFNumber to hold the values we calculated.
    uint64_t val = ((uint64_t)size) << 32 | (uint64_t)flags;
    const void *vptr = (const void *)&val;
    CFNumberRef valref = CFNumberCreate(NULL, kCFNumberLongLongType, vptr);

    // Append to info value to the function arg info array.
    CFArrayAppendValue(kfinfo, valref);
    CFRelease(valref);

    // Append the argument name to kf_argnames
    if (kmd->isArgNamesHasValue()) {
      std::string argname = kmd->getArgNamesItem(j);
      CFStringRef cfargname = CFStringCreateWithCString(NULL, argname.c_str(),
                                                        kCFStringEncodingUTF8);
      CFArrayAppendValue(kf_argnames, cfargname);
      CFRelease(cfargname);
    }

    // Remember the type of the argument.
    if (kmd->isArgTypesHasValue()) {
      std::string tyname = kmd->getArgTypesItem(j);
      CFStringRef cfargtype = CFStringCreateWithCString(NULL, tyname.c_str(),
                                                          kCFStringEncodingUTF8);
      CFArrayAppendValue(kf_argtypes, cfargtype);
      CFRelease(cfargtype);
    }

    // Remember the type qual of the arg.
    if (kmd->isArgTypeQualsHasValue()) {
      std::string quals = kmd->getArgTypeQualsItem(j);

      // Given the string, convert to numerical representation.
      unsigned qualflags = 0;
      if (quals.find("const") != std::string::npos)
        qualflags |= CLD_ARGS_FLAGS_TYPE_QUAL_CONST;
      if (quals.find("volatile") != std::string::npos)
        qualflags |= CLD_ARGS_FLAGS_TYPE_QUAL_VOLATILE;
      if (quals.find("restrict") != std::string::npos)
        qualflags |= CLD_ARGS_FLAGS_TYPE_QUAL_RESTRICT;

      // Create a CFNumber to hold the values we calculated.
      uint64_t val = (uint64_t)qualflags;
      const void *vptr = (const void *)&val;
      valref = CFNumberCreate(NULL, kCFNumberLongLongType, vptr);

      // Append to info value to the function arg info array.
      CFArrayAppendValue(kf_argtypequals, valref);
      CFRelease(valref);
    }
  }

  // Insert work group dimensions.
  const unsigned max_wg_dim = 3;
  CFMutableArrayRef kf_wg_dims = CFArrayCreateMutable(NULL, max_wg_dim,
                                                      &kCFTypeArrayCallBacks);

#if 1 // reqd_work_group_size
    // TODO: Remove this code when "llvm.global.annotations" is deprecated
    if (!args_desc.empty() && args_desc[j] == 'R') {
      // Get Required work group size.
      assert(args_desc[j+1] == 'W' && args_desc[j+2] == 'G' &&
             "Expected required work group size");
    
      char* ValStart = &args_desc[j+3];
      char* ValEnd;
      for (unsigned k = 0; k < max_wg_dim; ++k) {
        assert(*ValStart != '\0');
        uint64_t val = strtol(ValStart, &ValEnd, 10);
        const void *vptr = (const void *)&val;
        CFNumberRef valref = CFNumberCreate(NULL, kCFNumberLongLongType, vptr);
      
        // Append to info value to the function arg info array.
        CFArrayAppendValue(kf_wg_dims, valref);
        CFRelease(valref);
      
        ValStart = *ValEnd == '\0' ? ValEnd : ValEnd+1;
      }
    } else {
      uint64_t val = 0;
      const void *vptr = (const void *)&val;
      for (unsigned k = 0; k < max_wg_dim; ++k) {
        CFNumberRef valref = CFNumberCreate(NULL, kCFNumberLongLongType, vptr);
        // Append to info value to the function arg info array.
        CFArrayAppendValue(kf_wg_dims, valref);
        CFRelease(valref);
      }
    }
#else // reqd_work_group_size
    if (KernelRWGsize) {
      // Get Required work group size.
      for (unsigned k = 0; k < max_wg_dim; ++k) {
        // With this MDNode, there is a key followed by max_wg_dim number of type WG sizes 
        // The first element is the key, so offset arg index by one.
        uint64_t val = llvm::dyn_cast<llvm::ConstantInt>(MDRWGS->getOperand(i+1))->getValue().getZExtValue();
        const void *vptr = (const void *)&val;
        CFNumberRef valref = CFNumberCreate(NULL, kCFNumberLongLongType, vptr);
        // Append to info value to the function arg info array.
        CFArrayAppendValue(kf_wg_dims, valref);
        CFRelease(valref);
      }
    } else {
      uint64_t val = 0;
      const void *vptr = (const void *)&val;
      for (unsigned k = 0; k < max_wg_dim; ++k) {
        CFNumberRef valref = CFNumberCreate(NULL, kCFNumberLongLongType, vptr);
        // Append to info value to the function arg info array.
        CFArrayAppendValue(kf_wg_dims, valref);
        CFRelease(valref);
      }
    }
#endif
    // Scalar name, which we will use to dlsym the wrapper out of the object.
    CFStringRef sname = CFStringCreateWithCString(NULL,
                                                  pWrapperFunc->getName().str().c_str(),
                                                  kCFStringEncodingUTF8);

    // Vector name, which we will use to dlsym the vector wrapper out of the object.
    std::string VKernelName = "";
    int VWidthMax = 0;
    if (vecIter != vecEnd && vecIter->first != NULL) {
      VKernelName = vecIter->first->getName().str();
      VWidthMax = vecIter->second;
      
    }
    CFStringRef vname = CFStringCreateWithCString(NULL, VKernelName.c_str(),
                                                  kCFStringEncodingUTF8);
    const void *vwmaxptr = (const void *)&VWidthMax;
    CFNumberRef vwmax = CFNumberCreate(NULL, kCFNumberIntType, vwmaxptr);
  
    //Add also: hasBarrier, SLM size, barrier buffer stride.
    int hasBarrier = 0 == noBarrierPath.count(pWrapperFunc->getName().str());
    const void *hbarrierptr = (const void *)&hasBarrier;
    CFNumberRef hbarrier = CFNumberCreate(NULL, kCFNumberIntType, hbarrierptr);

    unsigned int barrierBufferSTride = Intel::getBarrierBufferStrideSize(optimizer, *pWrapperFunc);
    const void *bbsptr = (const void *)&barrierBufferSTride;
    CFNumberRef bbs = CFNumberCreate(NULL, kCFNumberIntType, bbsptr);

    const void *entry[] = { kfinfo, kf_argsizes, kf_argalignments, sname, vname, vwmax, hbarrier, bbs,
      kf_wg_dims, kf_argnames, kf_argtypes, kf_argtypequals };
    CFArrayRef arrayref = CFArrayCreate(NULL, entry, 12, &kCFTypeArrayCallBacks);
  
    CFRelease(kfinfo);
    CFRelease(kf_argsizes);
    CFRelease(kf_argalignments);
    CFRelease(sname);
    CFRelease(vname);
    CFRelease(vwmax);
    CFRelease(hbarrier);
    CFRelease(bbs);
    CFRelease(kf_wg_dims);
    CFRelease(kf_argnames);
    CFRelease(kf_argtypes);
    CFRelease(kf_argtypequals);
  
    // Create a CFString from the function name to use as the key.
    CFStringRef kfstr = CFStringCreateWithCString(NULL, pWrapperFunc->getName().str().c_str(),
                                                  kCFStringEncodingUTF8);
  
    // Insert the info array into the dictionary.
    CFDictionaryAddValue(*info, kfstr, arrayref);
    CFRelease(kfstr);
    CFRelease(arrayref);
  }
  return 0;
}

static void cld_replace_uses(Value* From, Value* To, Instruction* AfterPos) {
  // This utility function is used to replace the global variable used to
  // mark __local variables in a kernel with an address passed into the kernel
  // through a hidden argument.  We can't use Value::replaceAllUsesWith because
  // it will fail when From is a constant (pointer to global) used in a
  // constant expression and To is a non constant expression (pointer argument).
  // For this case, we examine the constant expression and replace it with the
  // equivalent non constant expression in the instruction used to access
  // the __local storage.
  // ConstantExpr *CE = dyn_cast<ConstantExpr>(From);

  for (Value::use_iterator ui = From->use_begin(), ue = From->use_end();
       ui != ue; ) {
    User* UserOp = *ui++;

    if (ConstantExpr* ExprVal = dyn_cast<ConstantExpr>(UserOp)) {
      if (ExprVal->getOpcode() == Instruction::GetElementPtr) {
        SmallVector<Value*, 8> Idxs(ExprVal->getNumOperands() - 1);
        User::op_iterator OI = ExprVal->op_begin(), OE = ExprVal->op_end();
        unsigned OIdx = 0;
        for (++OI; OI != OE; ++OI, ++OIdx)
          Idxs[OIdx] = *OI;
        Instruction* NewGEP = GetElementPtrInst::Create(To, llvm::makeArrayRef(Idxs), "larg");
        NewGEP->insertAfter(AfterPos);
        cld_replace_uses(ExprVal, NewGEP, NewGEP);
      } else if (ExprVal->getOpcode() == Instruction::BitCast) {
        Instruction* NewBit = CastInst::Create(Instruction::BitCast, To,
                                               ExprVal->getType(), "bcast");
        NewBit->insertAfter(AfterPos);
        cld_replace_uses(ExprVal, NewBit, NewBit);
      }
      else if (ExprVal->getOpcode() == Instruction::PtrToInt) {
        Instruction* NewPtoI = new PtrToIntInst(To, ExprVal->getType(), "ptoi");
        NewPtoI->insertAfter(AfterPos);
        cld_replace_uses(ExprVal, NewPtoI, NewPtoI);
      } else {
        assert(0 && "Unhandled constant expression");
        abort();
      }
    } else if (ConstantVector *CV = dyn_cast<ConstantVector>(UserOp)) {
      Value *V = UndefValue::get(CV->getType());
      LLVMContext &Ctx = CV->getType()->getContext();
      InsertElementInst *IE = 0;
      for (unsigned vi = 0, ve = CV->getNumOperands(); vi != ve; ++vi) {
        Value *Idx = ConstantInt::get(Type::getInt32Ty(Ctx), vi);
        Value *Op = CV->getOperand(vi);
        if (Op == From)
          Op = To;
        IE = InsertElementInst::Create(V, Op, Idx, "");
        IE->insertAfter(AfterPos);
        V = AfterPos = IE;
      }
      cld_replace_uses(CV, IE, IE);
    } else if (isa<ConstantArray>(UserOp) ||
               isa<ConstantStruct>(UserOp)) {
      // ignore
    } else if (Instruction *UseI = dyn_cast<Instruction>(UserOp)) {
      if (UseI->getParent()->getParent() !=
          cast<Instruction>(To)->getParent()->getParent())
        continue;

      // Non constant expression; just replace.
      UserOp->replaceUsesOfWith(From, To);
    } else {
      assert(0 && "Unhandled subtype of User");
      abort();
    }
  }
}

static
Function *cld_genwrapper(Module *M, TargetData &TD, Function *kf,
                         FunctionType *WTy, ConstantArray *LGVs,
                         bool debug, bool vector) {
  LLVMContext &CTX = M->getContext();
  Type *I32Ty = Type::getInt32Ty(CTX);
  Type *SizeTy = IntegerType::get(CTX, TD.getPointerSizeInBits());

  // Create a wrapper function which will loop over the kernel in the
  // x-dimension, with the following prototype:
  //
  // @wrapper ( i8 **args, i32/64 *gtid, i32/64 x, size_t stride )
  SmallString<64> wname;
  wname += kf->getName();
  wname += "_wrapper";
  Constant *wc = M->getOrInsertFunction(wname, WTy);

  Function *wf = cast<Function>(wc);

  // Set the calling convention and say that we don't throw.
  wf->setCallingConv(CallingConv::C);
  wf->addFnAttr(Attribute::NoUnwind);

  // set the argument names. This is not strictly necessary, but makes looking
  // at the generated IR more pleasant.
  Function::arg_iterator wai = wf->arg_begin();
  Value *al = wai;
  al->setName("args_list");
  ++wai;
  Value *gtid = wai;
  gtid->setName("gtid");
  ++wai;
  Value *xend = wai;
  xend->setName("xend");
  ++wai;
  Value *stride;
  if (vector) {
    stride = wai;
    stride->setName("stride");
  }
  else {
    stride = ConstantInt::get(SizeTy, 1);
  }

  // In the entry block, we will unpack all the arguments, and then test our
  // loop against early termination.  The pseudo-IR is:
  //
  // Entry:
  //   unpack args
  //   %0 = load gtid
  //   %1 = icmp eq %0, %x
  //   br %1, Return, Loop
  BasicBlock *entrybb = BasicBlock::Create(CTX, "entry", wf);
  BasicBlock *loopcd = BasicBlock::Create(CTX, "loop.cond", wf);
  BasicBlock *loopbb = BasicBlock::Create(CTX, "loop", wf);
  BasicBlock *returnbb = BasicBlock::Create(CTX, "return", wf);
  IRBuilder<> builder(entrybb);

  // Vector to collect the args to the actual function in
  SmallVector<Value*, 16> argvector;

  // Now that we have created an empty function, we need to load the
  // arguments, which are of type void *, out of the kernel's argument array,
  // of type void **. The process for loading these arguments is:
  //
  // 1. insert a load from the corresponding slot args_list, the first
  //    argument to the wrapper function we're creating.
  // 2. cast the result to a pointer of the pointer type to the corresponding
  //    argument of the kernel function we're calling.
  // 3. load from the resulting pointer, and push the Value onto the args
  //    vector we will use to call the kernel function with.
  unsigned i = 0;
  for (Function::arg_iterator ai = kf->arg_begin(), ae = kf->arg_end();
       ai != ae; ++i, ++ai) {
    Type *Ty = ai->getType();

    // gep args list, i32 n
    Value *GEP = builder.CreateGEP(al, ConstantInt::get(I32Ty, i), "arglist");
    // If this is a by val argument, it is always a pointer type.  We actually
    // care about the type of the argument being pointed to by the byval arg.
    if (ai->hasByValAttr())
      Ty = cast<PointerType>(Ty)->getElementType();

    // load the pointer at that slot in the args list.  If this is a stream,
    // it is a pointer to the stream data, and can be passed as is.  If this
    // is a scalar, we must create a bit cast to pointer of the scalar type,
    // and load the scalar as an argument.
    Value *VP = isa<PointerType>(Ty) ? GEP : builder.CreateLoad(GEP,"argptr");
    VP = builder.CreateBitCast(VP, PointerType::getUnqual(Ty), "ptrtoarg");
    if (!ai->hasByValAttr())
      VP = builder.CreateLoad(VP, "arg");
    argvector.push_back(VP);
  }

  LoadInst *V0 = builder.CreateLoad(gtid);
  LoadInst *V1 = builder.CreateLoad(builder.CreateGEP(gtid, ConstantInt::get(I32Ty, 1), "gtid_1"));
  builder.CreateBr(loopcd);

  // Loop.cond:
  //  iv = phi [ x, entry ], [ xinc, loop ]
  //  cmp = icmp eq iv, xend
  //  br cmp, return, loop
  builder.SetInsertPoint(loopcd);
  PHINode *Phi = builder.CreatePHI(SizeTy, 2, "iv");
  Phi->addIncoming(V0, entrybb);
  Value *Cmp = builder.CreateICmpEQ(Phi, xend);
  builder.CreateCondBr(Cmp, returnbb, loopbb);

  // In the loop block, we will call our kernel function, update the global id
  // for the x-dimension, and then loop if not done.   The pseudo-IR is:
  //
  // Loop:
  //   tail call void func (args)
  //   %inc = add %iv, %stride
  //   store %inc, %gtid
  //   br Loop.cond
  builder.SetInsertPoint(loopbb);
  CallInst *CI = builder.CreateCall(kf, llvm::makeArrayRef(argvector));
  Value *Inc = builder.CreateAdd(Phi, stride);
  Phi->addIncoming(Inc, loopbb);
  builder.CreateStore(Inc, gtid);
  builder.CreateBr(loopcd);

  // In the return block, store the original starting gtid in the x-dimension
  // so that we can restart iteration at that point for the next y, and then
  // return from the wrapper.  The pseudo-IR is:
  //
  // Return:
  //   store %0, %gtid
  //   ret void
  //
  builder.SetInsertPoint(returnbb);
  builder.CreateStore(V0, gtid);
  builder.CreateRetVoid();

  // Inline the callee into the wrapper
  if (!debug) {
    InlineFunctionInfo IFI;
    InlineFunction(CI, IFI);
  }

  // FIXME: replace all uses of get_global_id 0 / 1 / 2 with our induction variable.
  Function *F = M->getFunction("get_global_id");
  if (F)
  {
    for (Value::use_iterator ui = F->use_begin(), ue = F->use_end();
         ui != ue; ) {
      User* UserOp = *ui++;
      if (!isa<CallInst>(UserOp))
        continue;

      CallInst *call = cast<CallInst>(UserOp);
      if (call->getParent()->getParent() != wf)
        continue;

      if (ConstantInt *argCI = dyn_cast<ConstantInt>(call->getArgOperand(0)))
      {
        if (argCI->getZExtValue() == 0)
          call->replaceAllUsesWith(Phi);
        if (argCI->getZExtValue() == 1)
          call->replaceAllUsesWith(V1);
      }
    }
  }

  // If the function defines any variables in the kernel that uses the
  // __local address space, add it to the list of parameters because we will
  // pass a pointer for the __local address.
  if (LGVs) {
    builder.SetInsertPoint(entrybb, V0);

    for (unsigned gi = 0, ge = LGVs->getType()->getNumElements(); gi != ge;
         ++gi) {
      Value *LGVI = LGVs->getOperand(gi)->stripPointerCasts();
      GlobalVariable *G = cast<GlobalVariable>(LGVI);
      G->removeDeadConstantUsers();
      PointerType *Ty = G->getType();
     // replace with alloca.
      Value *LGVL = builder.CreateAlloca(Ty->getElementType(),
                                         ConstantInt::get(I32Ty, 1), "localgv");
      // bitcast to appropriate address space.
      LGVL = builder.CreateBitCast(LGVL, Ty);

      cld_replace_uses(G, LGVL, V0);
    }
  }
  return wf;
}




// HeaderStruct used internally to pass header information.

__BEGIN_DECLS

/// cl2module - The compiler's entry point.
extern "C" {
  struct HeaderStruct {
    const char*  hdr;
    unsigned     hdr_size;
    const char*  hdr_name;
  };

void *cl2module(LLVMContext &LCTX, cl_bitfield opt, const char* options,
                const char *source, int debug, char **compile_log,
                std::vector<HeaderStruct>* hdrs);
}
__END_DECLS


/// compileProgram -  called by cmvsModularBuilder.  Turns a program
/// source string into IR, and records the info dictionary for the program.
static int compileProgram(
  cvms_plugin_service_t objects,
  CFDictionaryRef *info,
  const void *source, size_t source_size,
  std::vector<HeaderStruct>& hdrs,
  cld_comp_opt opt, const char *options, char **log,
  formatted_raw_ostream &os,
  bool link_multiple,
  bool& has_kernel)
{
  // Get the runtime module, which contains all the OpenCL runtime functions.
  Module *Runtime = objects->llvm_module;

  // Set the triple string based on the architecture we're compiling for.
  std::string triple = Runtime->getTargetTriple();

  // Prepend "-triple {$TRIPLE} " onto the options string.
  std::string options_str;
  options_str = std::string("-triple ") + triple + " " + options;

  // Determine if optimizations are disabled, in which case we should also turn
  // on debug info.
  bool disable_opt = (opt & CLD_COMP_OPT_FLAGS_DISABLE) != 0;
  bool debug = (opt & CLD_COMP_OPT_FLAGS_DEBUG) != 0;
  Module *SM = NULL;

  if (opt & CLD_COMP_OPT_FLAGS_PORT_BINARY) {
    // Load LLVM IR; don't include the null byte in size.
    StringRef ref((const char*)source, source_size-1);
    MemoryBuffer *memory_buffer = MemoryBuffer::getMemBuffer(ref);
    if (!memory_buffer) {
      if (log) {
       *log = strdup("Cannot load portable binary");
      }
      return -9;
    }
    std::string ErrMsg;
    SM = ParseBitcodeFile(memory_buffer, objects->llvm_module->getContext(),&ErrMsg);
    if (!SM && log)
      *log = strdup(ErrMsg.c_str());
    delete memory_buffer;

    // If the portable binary has a compiler option associated with it, grab
    // the value and override the options to be the same as what we compiled.
    if (SM) {
      if (GlobalVariable *OptionGV =
            SM->getGlobalVariable("opencl.com.compiler.option", true)) {
        assert(OptionGV->hasInitializer() && "OptionGV has no initializer");
        ConstantInt *init = dyn_cast<ConstantInt>(OptionGV->getInitializer());
        opt = init->getValue().getZExtValue();
        OptionGV->eraseFromParent();
      }
    }
  }
  else {
    // Add global variable with local linkage.
    SM = (Module *)cl2module(
                     objects->llvm_module->getContext(), opt,
                     options_str.c_str(), (const char*)source,
                     debug, log, &hdrs);
  }
  if (!SM)
    return -2;

  OwningPtr<Module> OM(SM);

  if (getenv("CL_DUMP_IR"))
    errs() << *SM << '\n';

  // If we are building a portable binary, return the bitcode file after
  // adding a variable to hold the compiler options for this compile.
  if (opt & CLD_COMP_OPT_FLAGS_BUILD_PORT_BINARY) {
    Type* IntTy = Type::getIntNTy(SM->getContext(), sizeof(opt)*8);
    (void) new GlobalVariable(*SM, IntTy, true,
           GlobalValue::InternalLinkage,
           ConstantInt::get(IntTy, opt & ~CLD_COMP_OPT_FLAGS_BUILD_PORT_BINARY),
                              "opencl.compiler.option", NULL, GlobalVariable::NotThreadLocal, 0);
    WriteBitcodeToFile(SM, os);
    os.flush();
    return 0;
  }

  // Generate the list of kernel names
  GlobalVariable *annotation = SM->getGlobalVariable("llvm.global.annotations");
  bool has_kernel_annotation = annotation && annotation->hasInitializer();
  if (!has_kernel_annotation && !link_multiple) {
    if (log)
      *log = strdup("No kernels or only kernel prototypes found when build executable.");
    return -3;
  }

  ConstantArray *init = 0;
  if (has_kernel_annotation) {
    // Make sure that llvm.global.annotations has a constant initializer.
    init = dyn_cast<ConstantArray>(annotation->getInitializer());
    if (!init)
      return -4;

    // We have at least one kernel.
    has_kernel = true;
    //annotation->eraseFromParent();
  } else {
    // We have no kernels so we must be linking multiple files.  Create an
    // empty kernel data dictionary.
    CFMutableDictionaryRef d;
    d = CFDictionaryCreateMutable(NULL, 0,
                                  &kCFTypeDictionaryKeyCallBacks,
                                  &kCFTypeDictionaryValueCallBacks);
    if (!d)
      return -15;
    *info = d;
  }

  // Optimize the whole module
  int vectorWidth = 1;

  if (!(opt & CLD_COMP_OPT_FLAGS_AUTO_VECTORIZE_DISABLE) && !debug && !disable_opt) {
    // FIXME: replace with sysctl check for avx when available.
    vectorWidth = 4;
    if (getenv("CL_VWIDTH_8"))
      vectorWidth = 8;
    }

  CPUPluginPrivateData* pluginData = (CPUPluginPrivateData*)objects->private_service;
  Intel::CPUId cpuId;

  if( pluginData != NULL)
    cpuId = pluginData->cpuId;
  else
    cpuId = selectCPU();

  intel::OptimizerConfig optimizerConfig(cpuId,
                                         vectorWidth,
                                         std::vector<int>(),
                                         std::vector<int>(),
                                         "",    // dump IR - disabled
                                         debug, // debug mode
                                         false, // profiling - disabled
                                         disable_opt, // optimizations on/off
                                         false, // relaxed_math
                                         false, // create library only
                                         false  // produce heuristics IR
                                         );

  Intel::OpenCL::DeviceBackend::Optimizer optimizer(SM, Runtime, &optimizerConfig);
  optimizer.Optimize();
  
  if (has_kernel_annotation) {
    alloc_kernels_info((CFMutableDictionaryRef*)info, init, SM, optimizer);
    annotation->eraseFromParent();
  }
  // Create the target machine for this module, and add passes to emit an object
  // file of this module's architecture. We can't get it from SM since we
  // may be loading a portable binary.
  std::string target_err;
  const Target *T = TargetRegistry::lookupTarget(triple, target_err);
  if (!T) {
    if (log)
      *log = strdup("x86 llvm target machine not found!");
    return -7;
  }

  TargetOptions toptions;
  
  OwningPtr<TargetMachine> Target(T->createTargetMachine(triple, "","", toptions,
                                                         Reloc::PIC_,
                                                         CodeModel::Default));

  PassManager KernelPasses;

  if (Target->addPassesToEmitFile(KernelPasses, os,
                                  TargetMachine::CGFT_ObjectFile,
                                  CodeGenOpt::Default))
    return -8;

  if (getenv("CL_DUMP_IR"))
    errs() << *SM << '\n';

  // Run the pass manager, emitting an object file to 'os'
  KernelPasses.run(*SM);

  if (getenv("CL_DUMP_IR"))
    errs() << *SM << '\n';

  os.flush();

  return 0;
}

/// pack_kernel_info - Serialize the four input dictionaries as binary plists
/// and return the CFWriteStream to which they have been written.
static int pack_kernel_info(CFWriteStreamRef *writeStreamP,
                            SmallVector<CFDictionaryRef, 8>& InfoVec) {
  if (InfoVec.size() == 0)
    return -1;
  CFDictionaryRef p_info;
  if (InfoVec.size() < 2) {
    p_info = *(InfoVec.begin());
  } else {
    // Merge the info dictionaries.
    CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0,
                                          &kCFTypeDictionaryKeyCallBacks,
                                          &kCFTypeDictionaryValueCallBacks);
    SmallVector<CFDictionaryRef, 8>::iterator iter = InfoVec.begin();
    SmallVector<CFDictionaryRef, 8>::iterator end = InfoVec.end();
    for (; iter != end; ++iter) {
      // Merge all p_info togethers.
      CFIndex n_keys = CFDictionaryGetCount(*iter);
      const void **keys = (const void **)alloca(n_keys * sizeof *keys);
      const void **values = (const void **)alloca(n_keys * sizeof *values);
      CFDictionaryGetKeysAndValues(*iter, keys, values);
      for (int i= 0; i < n_keys; ++i)
        CFDictionaryAddValue(d, keys[i], values[i]);
      CFRelease(*iter);
    }
    p_info = d;
  }

  CFWriteStreamRef writeStream;
  writeStream = CFWriteStreamCreateWithAllocatedBuffers(kCFAllocatorDefault,
                                                        kCFAllocatorDefault);
  assert(writeStream);

  CFWriteStreamOpen(writeStream);
  CFPropertyListWriteToStream(p_info, writeStream, kCFPropertyListBinaryFormat_v1_0, NULL);
  CFWriteStreamClose(writeStream);

  *writeStreamP = writeStream;

  CFRelease(p_info);
  return 0;
}

static void allocateCVMSReturnData(
                                   cvms_plugin_element_t llvm_func,
                                   char *log, int err,
                                   std::vector<unsigned char> &dylib,
                                   std::string &ofile) {
  if (!log)
    log = strdup("");

  cvmsCPUReturnData *rd;
  size_t log_len = strlen(log) + 1;
  size_t rdlen = sizeof *rd + log_len + ofile.size() + dylib.size();

  // offset[0]: char *log
  // offset[1]: char *object_file
  rd = (cvmsCPUReturnData *) (*llvm_func->service->content_allocate)(llvm_func, rdlen);
  rd->err = err;
  rd->offsets[0] = log_len;
  rd->offsets[1] = rd->offsets[0] + dylib.size();
  rd->offsets[2] = rd->offsets[1] + ofile.size();
  strlcpy((char *)rd->data, log, rd->offsets[0]);
  memcpy(rd->data + rd->offsets[0], &dylib[0], dylib.size());
  memcpy(rd->data + rd->offsets[1], ofile.data(), ofile.size());

#ifdef CLD_ASSERT
  char* ocl_file_name = getenv("CL_DUMP_KERNEL");
  if (ocl_file_name) {
    char filename[1024];
    sprintf(filename, "/tmp/%s", ocl_file_name);
    int fd = open(filename, O_CREAT | O_TRUNC | O_RDWR,
                  S_IRUSR | S_IWUSR | S_IRGRP);
    if (fd != -1) {
      write(fd, &dylib[0], dylib.size());
      close(fd);
    }
  }
#endif

  free(log);
}

////////////////////////////////////////////////////////////////////////////////
// Archive library support
////////////////////////////////////////////////////////////////////////////////
// Build the list of sources file from llvm_func.
static int buildSrcFiles(
                         cvms_plugin_element_t llvm_func,
                         std::vector<SrcLenStruct>& SrcPtrs,
                         char **log)
{
  cvmsStrings *strings = (cvmsStrings *) llvm_func->source_addrs[cvmsSrcIdxData];
  int numSrcs = strings->offsets[cvmsStringNumSource];
  char *source = &strings->data[strings->offsets[cvmsStringOffsetSource]];
  size_t source_size = strings->offsets[cvmsStringOffsetOptions] -
                       strings->offsets[cvmsStringOffsetSource];

  // In the case of loading sources files, the CVMS string has the
  // the following structure
  //    num_sources
  //    1st cvmsStringOffsetSource
  //    linker option string
  //    data:
  //      num_sources cvmsStringOffsetSources if num_sources > 1
  //      num_sources bitcode file
  int idx = 1; // used when num_srcs > 1
  do {
    StringRef ref((const char*)source, source_size-1);
    MemoryBuffer *memory_buffer = MemoryBuffer::getMemBuffer(ref);
    if (!memory_buffer) {
      if (log)
        *log = strdup("Cannot load source file");
      return -1;
    }

    unsigned char *BufPtr = (unsigned char *)memory_buffer->getBufferStart();
    unsigned char *BufEnd = BufPtr + memory_buffer->getBufferSize();
    if (llvm::isRawBitcode(BufPtr, BufEnd) ||
        llvm::isBitcodeWrapper(BufPtr, BufEnd)) {
      // Add to this to the list of pointers
      SrcPtrs.push_back(SrcLenStruct(source, source_size));
    } else if (CLArchive::isArchive(BufPtr, BufEnd)) {
      CLArchive::readArchive(BufPtr, BufEnd, log, SrcPtrs);
    } else {
      if (numSrcs == 1) {
        // This can be a regular source file we are trying to compile.
        SrcPtrs.push_back(SrcLenStruct(source, source_size));
      } else {
        // If we have more than one source, we must be linking so we must have
        // bitcode files or archive files.
        if (log)
          *log = strdup("Unsupported file format for linking an executable");
        return -2;
      }
    }
    delete memory_buffer;

    // Advance to the next source.
    if (idx < numSrcs) {
      size_t offset = strings->offsets[cvmsStringOffsetOptions+idx];
      size_t next_offset = strings->offsets[cvmsStringOffsetOptions+idx+1];
      source = &strings->data[offset];
      source_size = next_offset - offset;
    }
    ++idx;
  } while (idx <= numSrcs);
  return 0;
}

static int buildSrcHeaderFiles(
             cvms_plugin_element_t llvm_func,
             SrcLenStruct& Source,
             std::vector<HeaderStruct>& HdrPtrs,
             char **log)
{
  // In the case of loading a source file with headers, the CVMS string
  // has the following structure
  //    num_header +1 for the source
  //    1st cvmsStringOffsetSource
  //    compiler option string
  //    data:
  //      2 * num_headers cvmsStringOffsetSources if num_sources > 1
  //      num_header header files
  //      num_header header names

  // First, process the source.
  cvmsStrings *strings = (cvmsStrings *) llvm_func->source_addrs[cvmsSrcIdxData];
  int numSrcs = strings->offsets[cvmsStringNumSource];
  char *source = &strings->data[strings->offsets[cvmsStringOffsetSource]];
  size_t source_size = strings->offsets[cvmsStringOffsetOptions] -
  strings->offsets[cvmsStringOffsetSource];

  {
    StringRef ref((const char*)source, source_size-1);
    MemoryBuffer *memory_buffer = MemoryBuffer::getMemBuffer(ref);
    if (!memory_buffer) {
      if (log)
        *log = strdup("Cannot load source file for compile");
      return -1;
    }
    unsigned char *BufPtr = (unsigned char *)memory_buffer->getBufferStart();
    unsigned char *BufEnd = BufPtr + memory_buffer->getBufferSize();
    if (CLArchive::isArchive(BufPtr, BufEnd)) {
      if (log)
        *log = strdup("Illegal source file type: can not compile a library");
      return -3;
    }
    Source.src = source;
    Source.src_size = source_size;
    delete memory_buffer;
  }

  // Process each header
  int numHdrs = numSrcs-1;
  for (int i = 0; i < numHdrs; ++i) {
    HeaderStruct HStruct;
    size_t offset = strings->offsets[cvmsStringOffsetOptions + i + 1];
    size_t next_offset = strings->offsets[cvmsStringOffsetOptions + i + 2];
    source = &strings->data[offset];
    source_size = next_offset - offset;

    StringRef ref((const char*)source, source_size-1);
    MemoryBuffer *memory_buffer = MemoryBuffer::getMemBuffer(ref);
    unsigned char *BufPtr = (unsigned char *)memory_buffer->getBufferStart();
    unsigned char *BufEnd = BufPtr + memory_buffer->getBufferSize();

    if (llvm::isRawBitcode(BufPtr, BufEnd) ||
        llvm::isBitcodeWrapper(BufPtr, BufEnd) ||
        CLArchive::isArchive(BufPtr, BufEnd)) {
      if (log)
        *log = strdup("Headers can not be bitcode files or archives");
      return -4;
    }
    HStruct.hdr = (const char*) source;
    HStruct.hdr_size = source_size;
    HdrPtrs.push_back(HStruct);
    delete memory_buffer;
  }

  for (int i = 0; i < numHdrs; ++i) {
    size_t offset = strings->offsets[cvmsStringOffsetOptions + numHdrs+i+1];
    source = &strings->data[offset];
    HdrPtrs[i].hdr_name = (const char*)source;
  }
  return 0;
}


static int buildArchive(cvms_plugin_element_t llvm_func)
{
  // Build list of bitcode file
  std::vector<SrcLenStruct> SrcPtrs;
  char* log = NULL;
  int err = buildSrcFiles(llvm_func, SrcPtrs, &log);

  if (err || SrcPtrs.size() == 1) {
    if (!log)
      log = strdup("");
    // Return the original source.
    char *source = (char*) SrcPtrs[0].src;
    size_t source_size = SrcPtrs[0].src_size;
    cvmsCPUReturnData *rd;
    size_t log_len = strlen(log) + 1;
    size_t rdlen = sizeof *rd + log_len + source_size;
    rd = (cvmsCPUReturnData *) (*llvm_func->service->content_allocate)(llvm_func, rdlen);
    rd->err = 0;
    rd->offsets[0] = log_len;
    rd->offsets[1] = rd->offsets[0] + 0;
    rd->offsets[2] = rd->offsets[1] + source_size;
    strlcpy((char *)rd->data, log, rd->offsets[0]);
    memcpy(rd->data + rd->offsets[1], source, source_size);
    free(log);
    return CVMS_ERROR_NONE;
  }

  // Create the archive
  std::string library_file;
  raw_string_ostream ofile_stream(library_file);
  formatted_raw_ostream os(ofile_stream);
  CLArchive::createArchive(SrcPtrs, &log, os);
  std::vector<unsigned char> dylib;
  allocateCVMSReturnData(llvm_func, log, 0, dylib, library_file);
  free(log);
  return CVMS_ERROR_NONE;
}




unsigned int SelectCpuFeatures( unsigned int cpuId, const std::vector<std::string>& forcedFeatures)
{
  unsigned int  cpuFeatures = Intel::CFS_SSE2;

  // Add standard features
  if( cpuId >= (unsigned int)Intel::CPUId::GetCPUByName("corei7") )
  {
    cpuFeatures |= Intel::CFS_SSE41 | Intel::CFS_SSE42;
  }

  if( cpuId >= (unsigned int)Intel::CPUId::GetCPUByName("sandybridge"))
  {
    cpuFeatures |= Intel::CFS_AVX1;
  }

  if( cpuId >= (unsigned int)Intel::CPUId::GetCPUByName("core-avx2"))
  {
    cpuFeatures |= Intel::CFS_AVX1;
    cpuFeatures |= Intel::CFS_AVX2;
    //cpuFeatures |= CFS_FMA;
  }

  // Add forced features
  if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "+sse41" ) != forcedFeatures.end())
  {
    cpuFeatures |= Intel::CFS_SSE41;
  }

  if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "+avx2" ) != forcedFeatures.end())
  {
    cpuFeatures |= Intel::CFS_AVX2;
    cpuFeatures |= Intel::CFS_AVX1;
  }

  if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "+fma3" ) != forcedFeatures.end())
  {
    cpuFeatures |= Intel::CFS_FMA;
  }

  if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "+avx" ) != forcedFeatures.end())
  {
    cpuFeatures |= Intel::CFS_AVX1;
  }

  if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "-sse41" ) != forcedFeatures.end())
  {
    cpuFeatures &= ~(Intel::CFS_SSE41 | Intel::CFS_SSE42);
  }

  if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "-avx2" ) != forcedFeatures.end())
  {
    cpuFeatures &= ~Intel::CFS_AVX2;
    cpuFeatures &= ~Intel::CFS_FMA;
  }

  if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "-avx" ) != forcedFeatures.end())
  {
    cpuFeatures &= ~Intel::CFS_AVX1;
    cpuFeatures &= ~Intel::CFS_AVX2;
    cpuFeatures &= ~Intel::CFS_FMA;
  }
  if( std::find( forcedFeatures.begin(), forcedFeatures.end(), "-fma3" ) != forcedFeatures.end())
  {
    cpuFeatures &= ~Intel::CFS_FMA;
  }

  return cpuFeatures;

}

Intel::CPUId selectCPU()
{
  Intel::ECPU selectedCpuId = Intel::OpenCL::DeviceBackend::Utils::CPUDetect::GetInstance()->GetCPUId().GetCPU();
  std::vector< std::string > forcedCpuFeatures;

  if (selectedCpuId == Intel::CPU_SANDYBRIDGE)
    forcedCpuFeatures.push_back("+avx");

  if (selectedCpuId == Intel::CPU_HASWELL) {
    forcedCpuFeatures.push_back("+avx2");
    forcedCpuFeatures.push_back("+f16c");
    forcedCpuFeatures.push_back("+fma3");
  }

  unsigned int selectedCpuFeatures = SelectCpuFeatures( selectedCpuId, forcedCpuFeatures );
  return Intel::CPUId(selectedCpuId, selectedCpuFeatures, sizeof(void*)==8);
}

/// cvmsPluginElementBuild - Plugin interface to CVMS invoked when CVMS has been
/// asked to compile program source.
///
/// cvmsModularBuilder handles 4 cases:
///    - A single source/portable binary (bitcode) with zero or more header
///      files to be built into a compile object (portable binary)
///      (clCompileProgram). This case occurs when
///      CLD_COMP_OPT_FLAGS_BUILD_PORT_BINARY is set.
///    - Multiple portable binaries create a library (an archive of bitcodes)
///      (clLinkProgram). This is the case when CLD_COMP_LINK_CREATE_LIBRARY
///      is set.
///    - A single source/portable binary (bitcode) will be built into an
///      executable. (clBuildProgram).  This is the case when the above two
///      conditions are not met and we have a single source being passed.
///    - Multiple portable binaries used to build an executable. (clLinkProgram)
///      This occurs when none of the above conditions are met.
cvms_private_service_t *cvmsPluginServiceInitialize(cvms_plugin_service_t service)
{
  CPUPluginPrivateData* privateData = new CPUPluginPrivateData();
  privateData->cpuId = selectCPU();
  return (cvms_private_service_t *)privateData;
}

void cvmsPluginServiceTerminate(cvms_plugin_service_t service)
{
  if( service->private_service )
    delete (CPUPluginPrivateData*)service->private_service;
}

cvms_error_t cvmsPluginElementBuild(cvms_plugin_element_t llvm_func)
{
  char *options = NULL;
  char *log = NULL;
  int32_t err;

  // setup the fake device/program for the call to cld_createkernels
  cvmsKeys *keys = (cvmsKeys *) llvm_func->source_addrs[cvmsSrcIdxKeys];
  // If we are building a library, build it and return.
  if (keys->opt & CLD_COMP_LINK_CREATE_LIBRARY) {
    return buildArchive(llvm_func);
  }

  cvmsStrings *strings = (cvmsStrings *) llvm_func->source_addrs[cvmsSrcIdxData];
  options = &strings->data[strings->offsets[cvmsStringOffsetOptions]];
/*
  llvm::NoFramePointerElim = true;

  // Set llvm options for our optimization phases and also set them in
  // llvm_func so CVMS will set them properly when jitting.
  llvm::UnsafeFPMath =
    (keys->opt & CLD_COMP_OPT_FLAGS_UNSAFE_MATH_OPTIMIZATIONS) ||
    (keys->opt & CLD_COMP_OPT_FLAGS_FAST_RELAXED_MATH);
  llvm::NoInfsFPMath =
     (keys->opt & CLD_COMP_OPT_FLAGS_FINITE_MATH_ONLY) ||
    (keys->opt & CLD_COMP_OPT_FLAGS_FAST_RELAXED_MATH);
  llvm::NoNaNsFPMath =
   (keys->opt & CLD_COMP_OPT_FLAGS_FINITE_MATH_ONLY) ||
     (keys->opt & CLD_COMP_OPT_FLAGS_FAST_RELAXED_MATH);
  llvm::LessPreciseFPMADOption = keys->opt & CLD_COMP_OPT_FLAGS_MAD_ENABLE;
*/
  // Turn the source passed to us by cvms into IR.  Collect the list of called
  // functions that survive inlining in spec_funcs, so we can be sure to remove
  // the code that gets generated for them in the JIT.
  std::vector<unsigned char> dylib;
  std::vector<HeaderStruct> HdrPtrs;
  std::vector<std::string*> ObjFileVec;
  SmallVector<CFDictionaryRef, 8> InfoVec;
  bool has_kernel = false;

  if (keys->opt & CLD_COMP_OPT_FLAGS_BUILD_PORT_BINARY) {
    // We are only compiling a source.  Grab the source file and headers to
    // generate a bitcode file.
    SrcLenStruct Source;
    err = buildSrcHeaderFiles(llvm_func, Source, HdrPtrs, &log);

    if (!err) {
      // Compiling a program.
      CFDictionaryRef info;
      std::string *object_file = new std::string();
      raw_string_ostream ofile_stream(*object_file);
      formatted_raw_ostream os(ofile_stream);
      err = compileProgram(
              llvm_func->service,
              &info, Source.src, Source.src_size,
              HdrPtrs, keys->opt, options, &log, os,
              false, has_kernel);
      if (!err) {
        ObjFileVec.push_back(object_file);
        InfoVec.push_back(info);
      }
    }
  } else {
    // Building an executable so first get the list of sources
    std::vector<SrcLenStruct> SrcPtrs;
    std::vector<const char*> HdrNames;
    err = buildSrcFiles(llvm_func, SrcPtrs, &log);

    // Compile each program seperately since we may have different options
    // (if this was not the case, we would have done an LLVM link instead).
    std::vector<SrcLenStruct>::iterator iter = SrcPtrs.begin();
    std::vector<SrcLenStruct>::iterator end = SrcPtrs.end();
    bool link_multiple = SrcPtrs.size() > 1;
    for (; iter != end; ++iter) {
      CFDictionaryRef info;
      std::string *object_file = new std::string();
      raw_string_ostream ofile_stream(*object_file);
      formatted_raw_ostream os(ofile_stream);
      bool local_has_kernel;
      err = compileProgram(
              llvm_func->service,
              &info, iter->src, iter->src_size,
              HdrPtrs, keys->opt, options, &log, os,
              link_multiple, local_has_kernel);
      if (err) {
        break;
      }
      ObjFileVec.push_back(object_file);
      InfoVec.push_back(info);
      if (local_has_kernel)
        has_kernel = true;

    }

    if (!err && !has_kernel) {
      log = strdup("No kernels or only kernel prototypes found.");
      err = -3;
    }

    // pack the kernel info into a writestream for later transfer
    CFDataRef strData = NULL;
    if (!err) {
      CFWriteStreamRef writeStream = NULL;
      if (pack_kernel_info(&writeStream, InfoVec)) {
        log = strdup("Internal error pack_kernel_info failed.");
        err = -20;
        if (writeStream)
          CFRelease(writeStream);
      }
      strData = (CFDataRef) CFWriteStreamCopyProperty(writeStream,
                                                 kCFStreamPropertyDataWritten);
      CFRelease(writeStream);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Single-file object -> bundle linker.
    ////////////////////////////////////////////////////////////////////////////
    std::string linklog;
    if (!err) {
      char *source = (char*) SrcPtrs[0].src;
      bool debug = (keys->opt & CLD_COMP_OPT_FLAGS_DEBUG) != 0;
      const char *path = debug ? source : NULL;

      err = Link(ObjFileVec, path, strData, dylib, linklog);
      CFRelease(strData);

      if (err && !linklog.empty()) {
        if (!log)
          log = strdup(linklog.c_str());
        else {
          size_t new_len = strlen(log) + linklog.length() + 1;
          char* new_log = (char*) malloc(strlen(log) + linklog.length());
          if (new_log) {
            strlcpy(new_log, log, new_len);
            strlcat(new_log, linklog.c_str(), new_len);
            free(log);
            log = new_log;
          }
        }
      }
    }

    // If debugging is disabled, do not return the original object file.
    if ((keys->opt & CLD_COMP_OPT_FLAGS_DEBUG) == 0) {
      std::vector<std::string*>::iterator bciter = ObjFileVec.begin();
      std::vector<std::string*>::iterator bcend = ObjFileVec.end();
      for (; bciter != bcend; ++bciter)
        (*bciter)->clear();
    }
  }
  std::string dummy;
  std::string *object_file = ObjFileVec.size() == 0 ? &dummy : ObjFileVec[0];
  allocateCVMSReturnData(llvm_func, log, err, dylib, *object_file);

  // Reset llvm options back as CVMS clients are not aware of certain options.
  /*
  llvm::UnsafeFPMath = false;
  llvm::NoInfsFPMath = false;
  llvm::NoNaNsFPMath = false;
  llvm::LessPreciseFPMADOption = false;
  */
  std::vector<std::string*>::iterator bciter = ObjFileVec.begin();
  std::vector<std::string*>::iterator bcend = ObjFileVec.end();
  for (; bciter != bcend; ++bciter)
    free(*bciter);

  return CVMS_ERROR_NONE;
}

