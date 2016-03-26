//==-- VPOParoptUtils.cpp - Utilities for VPO Paropt Transforms -*- C++ -*--==//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// Authors:
// --------
// Xinmin Tian (xinmin.tian@intel.com)
//
// Major Revisions:
// ----------------
// Nov 2015: Initial Implementation of OpenMP runtime APIs (Xinmin Tian)
//
//==------------------------------------------------------------------------==//
///
/// \file
/// This file provides a set of utilities for VPO Paropt Transformations to
/// generate OpenMP runtime API call instructions.
///
//==------------------------------------------------------------------------==//

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptUtils.h"

#include <string>

#define DEBUG_TYPE "VPOParoptUtils"

using namespace llvm;
using namespace llvm::vpo;

// This function generates a runtime library call to __kmpc_begin(&loc, 0)
CallInst *VPOParoptUtils::genRTLKmpcBeginCall(Function *F, Instruction *AI,
                                              StructType *IdentTy) {
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  BasicBlock &B = F->getEntryBlock();
  BasicBlock &E = B;

  int Flags = KMP_IDENT_KMPC;

  AllocaInst *KmpcLoc = genKmpcLocfromDebugLoc(F, AI, IdentTy, Flags, &B, &E);

  ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);

  Constant *FnC = M->getOrInsertFunction("__kmpc_begin", Type::getVoidTy(C),
                                         PointerType::getUnqual(IdentTy),
                                         Type::getInt32Ty(C), NULL);

  Function *FnKmpcBegin = cast<Function>(FnC);

  FnKmpcBegin->setCallingConv(CallingConv::C);

  std::vector<Value *> FnKmpcBeginArgs;
  FnKmpcBeginArgs.push_back(KmpcLoc);
  FnKmpcBeginArgs.push_back(ValueZero);

  CallInst *KmpcBeginCall = CallInst::Create(FnKmpcBegin, FnKmpcBeginArgs);
  KmpcBeginCall->setCallingConv(CallingConv::C);

  return KmpcBeginCall;
}

// This function generates a runtime library call to __kmpc_end(&loc)
CallInst *VPOParoptUtils::genRTLKmpcEndCall(Function *F, Instruction *AI,
                                            StructType *IdentTy) {
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  BasicBlock &B = F->getEntryBlock();
  BasicBlock &E = B;

  int Flags = KMP_IDENT_KMPC;

  AllocaInst *KmpcLoc = genKmpcLocfromDebugLoc(F, AI, IdentTy, Flags, &B, &E);

  Constant *FnC = M->getOrInsertFunction("__kmpc_end", Type::getVoidTy(C),
                                         PointerType::getUnqual(IdentTy), NULL);

  Function *FnKmpcEnd = cast<Function>(FnC);

  FnKmpcEnd->setCallingConv(CallingConv::C);

  std::vector<Value *> FnKmpcEndArgs;
  FnKmpcEndArgs.push_back(KmpcLoc);

  CallInst *KmpcEndCall = CallInst::Create(FnKmpcEnd, FnKmpcEndArgs);
  KmpcEndCall->setCallingConv(CallingConv::C);

  return KmpcEndCall;
}

// This function generates a runtime library call to get global OpenMP thread
// ID - __kmpc_global_thread_num(&loc)
CallInst *VPOParoptUtils::genRTLKmpcGlobalThreadNumCall(Function *F,
                                                        Instruction *AI,
                                                        StructType *IdentTy) {
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();

  BasicBlock &B = F->getEntryBlock();
  BasicBlock &E = B;

  int Flags = KMP_IDENT_KMPC;

  AllocaInst *KmpcLoc = genKmpcLocfromDebugLoc(F, AI, IdentTy, Flags, &B, &E);

  FunctionType *FnGetTidTy = FunctionType::get(
      Type::getInt32Ty(C), PointerType::getUnqual(IdentTy), false);

  Function *FnGetTid = M->getFunction("__kmpc_global_thread_num");

  if (!FnGetTid) {
    FnGetTid = Function::Create(FnGetTidTy, GlobalValue::ExternalLinkage,
                                "__kmpc_global_thread_num", M);
    FnGetTid->setCallingConv(CallingConv::C);
  }

  std::vector<Value *> FnGetTidArgs;
  FnGetTidArgs.push_back(KmpcLoc);

  CallInst *GetTidCall = CallInst::Create(FnGetTid, FnGetTidArgs, "valtid");
  GetTidCall->setCallingConv(CallingConv::C);
  GetTidCall->setTailCall(true);

  return GetTidCall;
}

// This function collects path, file name, line, column information for
// generating kmpc_location struct needed for OpenMP runtime library
AllocaInst *VPOParoptUtils::genKmpcLocfromDebugLoc(Function *F, Instruction *AI,
                                                   StructType *IdentTy,
                                                   int Flags, BasicBlock *BS,
                                                   BasicBlock *BE) {
  Module *M = F->getParent();
  LLVMContext &C = F->getContext();
  std::string KmpLoc;

  StringRef Path("");
  StringRef File("unknown");
  StringRef FnName("unknown");
  unsigned SLine = 0;
  unsigned ELine = 0;

  int VpoEmitSourceLocation = 1;

  for (int K = 0; K < 2; ++K) {
    BasicBlock::iterator I = (K == 0) ? BS->begin() : BE->begin();
    if (Instruction *Inst = dyn_cast<Instruction>(&*I)) {
      if (DILocation *Loc = Inst->getDebugLoc()) {
        if (K == 0) {
          Path = Loc->getDirectory();
          File = Loc->getFilename();
          FnName = Loc->getScope()->getSubprogram()->getName();
          SLine = Loc->getLine();
        } else {
          ELine = Loc->getLine();
        }
      }
    }
  }

  // Source location string for OpenMP runtime library call
  // KmpLoc = ";pathfilename;routinename;sline;eline;;"
  switch (VpoEmitSourceLocation) {

  case 0:
    KmpLoc = ";unknown;unknown;0;0;;\00";
    break;

  case 1:
    KmpLoc = ";unknown;" + FnName.str() + ";" + std::to_string(SLine) + ";" +
             std::to_string(ELine) + ";;\00";
    break;

  case 2:
    KmpLoc = ";" + Path.str() + "/" + File.str() + ";" + FnName.str() + ";" +
             std::to_string(SLine) + ";" + std::to_string(ELine) + ";;\00";
    break;
  default:
    KmpLoc = ";unknown;unknown;0;0;;\00";
    break;
  }

  StringRef Loc = StringRef(KmpLoc);

  // Type Definitions
  ArrayType *LocStrTy = ArrayType::get(Type::getInt8Ty(C), Loc.str().size());

  // String Constant Definitions
  Constant *LocStrDef = ConstantDataArray::getString(C, Loc.str(), false);

  // Global Variable Definitions
  Constant *VarLoc = new GlobalVariable(
      *M, LocStrTy, false, GlobalValue::PrivateLinkage, LocStrDef,
      ".KmpcLoc." + std::to_string(SLine) + '.' + std::to_string(ELine));

  // Constant Definitions
  ConstantInt *ValueZero = ConstantInt::get(Type::getInt32Ty(C), 0);
  ConstantInt *ValueOne = ConstantInt::get(Type::getInt32Ty(C), 1);
  ConstantInt *ValueFour = ConstantInt::get(Type::getInt32Ty(C), 4);

  ConstantInt *ValueFlags = ConstantInt::get(Type::getInt32Ty(C), Flags);

  DEBUG(dbgs() << "\nSource Location Info: " << Loc << "\n");

  Constant *Zeros[] = {ValueZero, ValueZero};
  Constant *LocStrRef = ConstantExpr::getGetElementPtr(LocStrTy, VarLoc, Zeros);

  // Global Variable Definitions
  // VarLoc->setInitializer(LocStrRef);

  AllocaInst *KmpcLoc = new AllocaInst(IdentTy, "loc." + std::to_string(SLine) +
                                                    "." + std::to_string(ELine),
                                       AI);
  KmpcLoc->setAlignment(8);

  GetElementPtrInst *FlagsPtr = GetElementPtrInst::Create(
      IdentTy, KmpcLoc, {ValueZero, ValueOne},
      "flags." + std::to_string(SLine) + "." + std::to_string(ELine), AI);

  StoreInst *InitFlags = new StoreInst(ValueFlags, FlagsPtr, false, AI);

  InitFlags->setAlignment(4);

  GetElementPtrInst *PSrcPtr = GetElementPtrInst::Create(
      IdentTy, KmpcLoc, {ValueZero, ValueFour},
      "psource." + std::to_string(SLine) + "." + std::to_string(ELine), AI);

  StoreInst *InitPsource = new StoreInst(LocStrRef, PSrcPtr, false, AI);
  InitPsource->setAlignment(8);
  return KmpcLoc;
}

// Generate source location information for Explicit barrier
AllocaInst *VPOParoptUtils::genKmpcLocforExplicitBarrier(Function *F,
                                                         Instruction *AI,
                                                         StructType *IdentTy,
                                                         BasicBlock *BB) {
  int Flags = KMP_IDENT_KMPC | KMP_IDENT_BARRIER_EXPL; // bits 0x2 | 0x20

#if 0
  if (VPOParopt_openmp_dvsm)
    flags |= KMP_IDENT_CLOMP;  // bit 0x4
#endif

  AllocaInst *KmpcLoc =
      VPOParoptUtils::genKmpcLocfromDebugLoc(F, AI, IdentTy, Flags, BB, BB);
  return KmpcLoc;
}

// Generate source location information for Implicit barrier
AllocaInst *VPOParoptUtils::genKmpcLocforImplicitBarrier(WRegionNode *W,
                                                         Function *F,
                                                         Instruction *AI,
                                                         StructType *IdentTy,
                                                         BasicBlock *BB) {
  int Flags = 0;

  switch (W->getWRegionKindID()) {

  case WRegionNode::WRNParallelLoop:
  case WRegionNode::WRNWksLoop:
    Flags = KMP_IDENT_BARRIER_IMPL_FOR;
    break;

  case WRegionNode::WRNParallelSections:
  case WRegionNode::WRNWksSections:
    Flags = KMP_IDENT_BARRIER_IMPL_SECTIONS;
    break;

  case WRegionNode::WRNTask:
  case WRegionNode::WRNTaskLoop:
    break;

  case WRegionNode::WRNSingle:
    Flags = KMP_IDENT_BARRIER_IMPL_SINGLE;
    break;

  default:
    Flags = KMP_IDENT_BARRIER_IMPL;
    break;
  }

  Flags |= KMP_IDENT_KMPC; // bit 0x2

#if 0
  if (PAROPT_openmp_dvsm)
    Flags |= KMP_IDENT_CLOMP;  // bit 0x4
#endif

  AllocaInst *KmpcLoc =
      VPOParoptUtils::genKmpcLocfromDebugLoc(F, AI, IdentTy, Flags, BB, BB);
  return KmpcLoc;
}
