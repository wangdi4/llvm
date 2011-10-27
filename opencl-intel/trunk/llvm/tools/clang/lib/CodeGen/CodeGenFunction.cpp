//===--- CodeGenFunction.cpp - Emit LLVM Code from ASTs for a Function ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This coordinates the per-function state used while generating code.
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include "CGCXXABI.h"
#include "CGDebugInfo.h"
#include "CGException.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/AST/APValue.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/StmtCXX.h"
#include "clang/Frontend/CodeGenOptions.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Intrinsics.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/SmallVector.h"

#include <sstream>

using namespace clang;
using namespace CodeGen;

CodeGenFunction::CodeGenFunction(CodeGenModule &cgm)
  : BlockFunction(cgm, *this, Builder), CGM(cgm),
    Target(CGM.getContext().Target),
    Builder(cgm.getModule().getContext()),
    NormalCleanupDest(0), EHCleanupDest(0), NextCleanupDestIndex(1),
    ExceptionSlot(0), DebugInfo(0), IndirectBranch(0),
    SwitchInsn(0), CaseRangeBlock(0),
    DidCallStackSave(false), UnreachableBlock(0),
    CXXThisDecl(0), CXXThisValue(0), CXXVTTDecl(0), CXXVTTValue(0),
    ConditionalBranchLevel(0), TerminateLandingPad(0), TerminateHandler(0),
    TrapBB(0) {
      
  // Get some frequently used types.
  LLVMPointerWidth = Target.getPointerWidth(0);
  llvm::LLVMContext &LLVMContext = CGM.getLLVMContext();
  IntPtrTy = llvm::IntegerType::get(LLVMContext, LLVMPointerWidth);
  Int32Ty  = llvm::Type::getInt32Ty(LLVMContext);
  Int64Ty  = llvm::Type::getInt64Ty(LLVMContext);
      
  Exceptions = getContext().getLangOptions().Exceptions;
  CatchUndefined = getContext().getLangOptions().CatchUndefined;
  CGM.getCXXABI().getMangleContext().startNewFunction();
}

ASTContext &CodeGenFunction::getContext() const {
  return CGM.getContext();
}


const llvm::Type *CodeGenFunction::ConvertTypeForMem(QualType T) {
  return CGM.getTypes().ConvertTypeForMem(T);
}

const llvm::Type *CodeGenFunction::ConvertType(QualType T) {
  return CGM.getTypes().ConvertType(T);
}

bool CodeGenFunction::hasAggregateLLVMType(QualType T) {
  return T->isRecordType() || T->isArrayType() || T->isAnyComplexType() ||
    T->isObjCObjectType();
}

void CodeGenFunction::EmitReturnBlock() {
  // For cleanliness, we try to avoid emitting the return block for
  // simple cases.
  llvm::BasicBlock *CurBB = Builder.GetInsertBlock();

  if (CurBB) {
    LLVM_ASSERT(!CurBB->getTerminator() && "Unexpected terminated block.");

    // We have a valid insert point, reuse it if it is empty or there are no
    // explicit jumps to the return block.
    if (CurBB->empty() || ReturnBlock.getBlock()->use_empty()) {
      ReturnBlock.getBlock()->replaceAllUsesWith(CurBB);
      delete ReturnBlock.getBlock();
    } else
      EmitBlock(ReturnBlock.getBlock());
    return;
  }

  // Otherwise, if the return block is the target of a single direct
  // branch then we can just put the code in that block instead. This
  // cleans up functions which started with a unified return block.
  if (ReturnBlock.getBlock()->hasOneUse()) {
    llvm::BranchInst *BI =
      dyn_cast<llvm::BranchInst>(*ReturnBlock.getBlock()->use_begin());
    if (BI && BI->isUnconditional() &&
        BI->getSuccessor(0) == ReturnBlock.getBlock()) {
      // Reset insertion point, including debug location, and delete the branch.
      Builder.SetCurrentDebugLocation(BI->getDebugLoc());
      Builder.SetInsertPoint(BI->getParent());
      BI->eraseFromParent();
      delete ReturnBlock.getBlock();
      return;
    }
  }

  // FIXME: We are at an unreachable point, there is no reason to emit the block
  // unless it has uses. However, we still need a place to put the debug
  // region.end for now.

  EmitBlock(ReturnBlock.getBlock());
}

static void EmitIfUsed(CodeGenFunction &CGF, llvm::BasicBlock *BB) {
  if (!BB) return;
  if (!BB->use_empty())
    return CGF.CurFn->getBasicBlockList().push_back(BB);
  delete BB;
}

void CodeGenFunction::FinishFunction(SourceLocation EndLoc) {
  LLVM_ASSERT(BreakContinueStack.empty() &&
         "mismatched push/pop in break/continue stack!");

  // Emit function epilog (to return).
  EmitReturnBlock();

  EmitFunctionInstrumentation("__cyg_profile_func_exit");

  // Emit debug descriptor for function end.
  if (CGDebugInfo *DI = getDebugInfo()) {
    DI->setLocation(EndLoc);
    DI->EmitFunctionEnd(Builder);
  }

  EmitFunctionEpilog(*CurFnInfo);
  EmitEndEHSpec(CurCodeDecl);

  LLVM_ASSERT(EHStack.empty() &&
         "did not remove all scopes from cleanup stack!");

  // If someone did an indirect goto, emit the indirect goto block at the end of
  // the function.
  if (IndirectBranch) {
    EmitBlock(IndirectBranch->getParent());
    Builder.ClearInsertionPoint();
  }
  
  // Remove the AllocaInsertPt instruction, which is just a convenience for us.
  llvm::Instruction *Ptr = AllocaInsertPt;
  AllocaInsertPt = 0;
  Ptr->eraseFromParent();
  
  // If someone took the address of a label but never did an indirect goto, we
  // made a zero entry PHI node, which is illegal, zap it now.
  if (IndirectBranch) {
    llvm::PHINode *PN = cast<llvm::PHINode>(IndirectBranch->getAddress());
    if (PN->getNumIncomingValues() == 0) {
      PN->replaceAllUsesWith(llvm::UndefValue::get(PN->getType()));
      PN->eraseFromParent();
    }
  }

  EmitIfUsed(*this, RethrowBlock.getBlock());
  EmitIfUsed(*this, TerminateLandingPad);
  EmitIfUsed(*this, TerminateHandler);
  EmitIfUsed(*this, UnreachableBlock);

  if (CGM.getCodeGenOpts().EmitDeclMetadata)
    EmitDeclMetadata();
}

/// ShouldInstrumentFunction - Return true if the current function should be
/// instrumented with __cyg_profile_func_* calls
bool CodeGenFunction::ShouldInstrumentFunction() {
  if (!CGM.getCodeGenOpts().InstrumentFunctions)
    return false;
  if (CurFuncDecl->hasAttr<NoInstrumentFunctionAttr>())
    return false;
  return true;
}

/// EmitFunctionInstrumentation - Emit LLVM code to call the specified
/// instrumentation function with the current function and the call site, if
/// function instrumentation is enabled.
void CodeGenFunction::EmitFunctionInstrumentation(const char *Fn) {
  if (!ShouldInstrumentFunction())
    return;

  const llvm::PointerType *PointerTy;
  const llvm::FunctionType *FunctionTy;
  std::vector<const llvm::Type*> ProfileFuncArgs;

  // void __cyg_profile_func_{enter,exit} (void *this_fn, void *call_site);
  PointerTy = llvm::Type::getInt8PtrTy(VMContext);
  ProfileFuncArgs.push_back(PointerTy);
  ProfileFuncArgs.push_back(PointerTy);
  FunctionTy = llvm::FunctionType::get(
    llvm::Type::getVoidTy(VMContext),
    ProfileFuncArgs, false);

  llvm::Constant *F = CGM.CreateRuntimeFunction(FunctionTy, Fn);
  llvm::CallInst *CallSite = Builder.CreateCall(
    CGM.getIntrinsic(llvm::Intrinsic::returnaddress, 0, 0),
    llvm::ConstantInt::get(Int32Ty, 0),
    "callsite");

  Builder.CreateCall2(F,
                      llvm::ConstantExpr::getBitCast(CurFn, PointerTy),
                      CallSite);
}

void CodeGenFunction::StartFunction(GlobalDecl GD, QualType RetTy,
                                    llvm::Function *Fn,
                                    const FunctionArgList &Args,
                                    SourceLocation StartLoc) {
  const Decl *D = GD.getDecl();
  
  DidCallStackSave = false;
  CurCodeDecl = CurFuncDecl = D;
  FnRetTy = RetTy;
  CurFn = Fn;
  LLVM_ASSERT(CurFn->isDeclaration() && "Function already has body?");

  // Pass inline keyword to optimizer if it appears explicitly on any
  // declaration.
  if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D)) {
    for (FunctionDecl::redecl_iterator RI = FD->redecls_begin(),
           RE = FD->redecls_end(); RI != RE; ++RI)
      if (RI->isInlineSpecified()) {
        Fn->addFnAttr(llvm::Attribute::InlineHint);
        break;
      }
    if(FD->hasAttr<LLVMAlignStackAttr>()) {
      unsigned alignment = FD->getAttr<LLVMAlignStackAttr>()->getN();
      Fn->addFnAttr(llvm::Attribute::constructStackAlignmentFromInt(alignment));
    }

    if(FD->hasAttr<LLVMAlwaysInlineAttr>()) Fn->addFnAttr(llvm::Attribute::AlwaysInline);
//    if(FD->hasAttr<LLVMHotpatchAttr>()) Fn->addFnAttr(llvm::Attribute::);
//    if(FD->hasAttr<LLVMNonLazyBindAttr>()) Fn->addFnAttr(llvm::Attribute::);
    if(FD->hasAttr<LLVMInlineHintAttr>()) Fn->addFnAttr(llvm::Attribute::InlineHint);
    if(FD->hasAttr<LLVMNakedAttr>()) Fn->addFnAttr(llvm::Attribute::Naked);
    if(FD->hasAttr<LLVMNoImplicitFloatAttr>()) Fn->addFnAttr(llvm::Attribute::NoImplicitFloat);
    if(FD->hasAttr<LLVMNoInlineAttr>()) Fn->addFnAttr(llvm::Attribute::NoInline);
    if(FD->hasAttr<LLVMNoRedZoneAttr>()) Fn->addFnAttr(llvm::Attribute::NoRedZone);
    if(FD->hasAttr<LLVMNoReturnAttr>()) Fn->addFnAttr(llvm::Attribute::NoReturn);
    if(FD->hasAttr<LLVMNoUnwindAttr>()) Fn->addFnAttr(llvm::Attribute::NoUnwind);
    if(FD->hasAttr<LLVMOptSizeAttr>()) Fn->addFnAttr(llvm::Attribute::OptimizeForSize);
    if(FD->hasAttr<LLVMReadNoneAttr>()) Fn->addFnAttr(llvm::Attribute::ReadNone);
    if(FD->hasAttr<LLVMReadOnlyAttr>()) Fn->addFnAttr(llvm::Attribute::ReadOnly);
    if(FD->hasAttr<LLVMSSPAttr>()) Fn->addFnAttr(llvm::Attribute::StackProtect);
    if(FD->hasAttr<LLVMSSPReqAttr>()) Fn->addFnAttr(llvm::Attribute::StackProtectReq);
  }

  if (getContext().getLangOptions().OpenCL) {
    // Add a metadata for a kernel entry.
    if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D))
	{
      if (FD->hasAttr<OpenCLKernelAttr>()) {

		std::string kernel_param_types;
		for (unsigned int i = 0; i < FD->getNumParams(); i++) {
			if(i > 0) kernel_param_types = kernel_param_types + ", ";

			const ParmVarDecl *param = FD->getParamDecl(i);
			std::string tmp(param->getType().getAsString());

			if(param->hasAttr<AnnotateAttr>())
			{
				const AnnotateAttr *AA = param->getAttr<AnnotateAttr>();
				
				tmp = AA->getAnnotation().str() + " " + tmp;
			}

			kernel_param_types += tmp;
		}

        llvm::NamedMDNode *OpenCLMetadata = 
          CGM.getModule().getOrInsertNamedMetadata("opencl.kernels");
        
		int reqx = 0, reqy = 0, reqz = 0;

		if(FD->hasAttr<ReqdWorkGroupSizeAttr>())
		{
			reqx = FD->getAttr<ReqdWorkGroupSizeAttr>()->getXDim();
			reqy = FD->getAttr<ReqdWorkGroupSizeAttr>()->getYDim();
			reqz = FD->getAttr<ReqdWorkGroupSizeAttr>()->getZDim();
		}

		int hintx = 0, hinty = 0, hintz = 0;

		if(FD->hasAttr<WorkGroupSizeHintAttr>())
		{
			hintx = FD->getAttr<WorkGroupSizeHintAttr>()->getXDim();
			hinty = FD->getAttr<WorkGroupSizeHintAttr>()->getYDim();
			hintz = FD->getAttr<WorkGroupSizeHintAttr>()->getZDim();
		}

		std::string vecTypeHint("");
		if(FD->hasAttr<VecTypeHintAttr>())
		{
			clang::QualType Ty = FD->getAttr<VecTypeHintAttr>()->getVecType();
			vecTypeHint = Ty.getAsString();
		}

		llvm::Value *ReqWG[] = {
			llvm::ConstantInt::get(llvm::Type::getInt32Ty(VMContext), reqx),
			llvm::ConstantInt::get(llvm::Type::getInt32Ty(VMContext), reqy),
			llvm::ConstantInt::get(llvm::Type::getInt32Ty(VMContext), reqz)
		};

		llvm::Value *WGHint[] = {
			llvm::ConstantInt::get(llvm::Type::getInt32Ty(VMContext), hintx),
			llvm::ConstantInt::get(llvm::Type::getInt32Ty(VMContext), hinty),
			llvm::ConstantInt::get(llvm::Type::getInt32Ty(VMContext), hintz)
		};

		std::string LocalsAnchorName = std::string("opencl_") + Fn->getNameStr() + std::string("_locals_anchor");

    llvm::SmallVector<llvm::Value *, 5> ArgAddrSpaces;
    llvm::SmallVector<llvm::Value *, 5> ArgAccess;
    llvm::SmallVector<llvm::Value *, 5> ArgTypes;
    llvm::SmallVector<llvm::Value *, 5> ArgNames;

		for (unsigned int i = 0; i < FD->getNumParams(); i++) {
			const ParmVarDecl *param = FD->getParamDecl(i);
			std::string ArgType(param->getType().getAsString());

      int addrSpace = 0;

      if(param->getType()->isPointerType()) {
        addrSpace = param->getType()->getAs<PointerType>()->getPointeeType().getAddressSpace();
      }

      ArgAddrSpaces.push_back(
          llvm::ConstantInt::get(llvm::Type::getInt32Ty(VMContext), addrSpace));

      QualType Ty = param->getOriginalType();
      std::string str = Ty.getAsString();

      std::string::size_type aspos = str.find("__attribute__((address_space(");

      if(aspos != std::string::npos) {
        str.erase(aspos-1, 35); 
      }

      int access = 3;

			if(str == "image2d_t" || str == "image3d_t" || str == "image2d_array_t")
      {
        access = 2;

        if(param->hasAttr<AnnotateAttr>())
			  {
				  const AnnotateAttr *AA = param->getAttr<AnnotateAttr>();
  				
				  std::string annotate = AA->getAnnotation().str();

          if(annotate == "__rd") {
            access = 0;
          } else if(annotate == "__wr") {
            access = 1;
          }
			  }
      }

      ArgAccess.push_back(
          llvm::ConstantInt::get(llvm::Type::getInt32Ty(VMContext), access));

      ArgTypes.push_back(llvm::MDString::get(VMContext, llvm::StringRef(str)));

      ArgNames.push_back(llvm::MDString::get(VMContext,param->getName()));
		}

    std::stringstream KernelAttributes; 

		if(FD->hasAttr<ReqdWorkGroupSizeAttr>())
		{
      KernelAttributes << "__attribute__((reqd_work_group_size(";
      KernelAttributes << reqx << "," << reqy << "," << reqz << ")))";
		}

		if(FD->hasAttr<WorkGroupSizeHintAttr>())
		{
      if(!KernelAttributes.str().empty()) KernelAttributes << " ";
      KernelAttributes << "__attribute__((work_group_size_hint(";
      KernelAttributes << hintx << "," << hinty << "," << hintz << ")))";
		}

		if(FD->hasAttr<VecTypeHintAttr>())
		{
      if(!KernelAttributes.str().empty()) KernelAttributes << " ";
      KernelAttributes << "__attribute__((vec_type_hint(";
      KernelAttributes << vecTypeHint << ")))";
		}

		llvm::Value *Elts[] = {
			Fn,
			llvm::MDNode::get(VMContext, &ReqWG[0], 3),
			llvm::MDNode::get(VMContext, &WGHint[0], 3),
			llvm::MDString::get(VMContext, llvm::StringRef(vecTypeHint)),
			llvm::MDString::get(VMContext, llvm::StringRef(kernel_param_types)),
			llvm::MDString::get(VMContext, llvm::StringRef(LocalsAnchorName)),
      llvm::MDNode::get(VMContext, ArgAddrSpaces.data(), ArgAddrSpaces.size()),
      llvm::MDNode::get(VMContext, ArgAccess.data(), ArgAccess.size()),
      llvm::MDNode::get(VMContext, ArgTypes.data(), ArgTypes.size()),
      llvm::MDNode::get(VMContext, ArgNames.data(), ArgNames.size()),
			llvm::MDString::get(VMContext, llvm::StringRef(KernelAttributes.str()))
		};

        OpenCLMetadata->addOperand(llvm::MDNode::get(VMContext, &Elts[0], sizeof(Elts)/sizeof(Elts[0])));
	  }
    }
  }

  llvm::BasicBlock *EntryBB = createBasicBlock("entry", CurFn);

  // Create a marker to make it easy to insert allocas into the entryblock
  // later.  Don't create this with the builder, because we don't want it
  // folded.
  llvm::Value *Undef = llvm::UndefValue::get(Int32Ty);
  AllocaInsertPt = new llvm::BitCastInst(Undef, Int32Ty, "", EntryBB);
  if (Builder.isNamePreserving())
    AllocaInsertPt->setName("allocapt");

  ReturnBlock = getJumpDestInCurrentScope("return");

  Builder.SetInsertPoint(EntryBB);

  QualType FnType = getContext().getFunctionType(RetTy, 0, 0, false, 0,
                                                 false, false, 0, 0,
                                                 /*FIXME?*/
                                                 FunctionType::ExtInfo());

  // Emit subprogram debug descriptor.
  if (CGDebugInfo *DI = getDebugInfo()) {
    DI->setLocation(StartLoc);
    DI->EmitFunctionStart(GD, FnType, CurFn, Builder);
  }

  EmitFunctionInstrumentation("__cyg_profile_func_enter");

  // FIXME: Leaked.
  // CC info is ignored, hopefully?
  CurFnInfo = &CGM.getTypes().getFunctionInfo(FnRetTy, Args,
                                              FunctionType::ExtInfo());

  if (RetTy->isVoidType()) {
    // Void type; nothing to return.
    ReturnValue = 0;
  } else if (CurFnInfo->getReturnInfo().getKind() == ABIArgInfo::Indirect &&
             hasAggregateLLVMType(CurFnInfo->getReturnType())) {
    // Indirect aggregate return; emit returned value directly into sret slot.
    // This reduces code size, and affects correctness in C++.
    ReturnValue = CurFn->arg_begin();
  } else {
    ReturnValue = CreateIRTemp(RetTy, "retval");
  }

  EmitStartEHSpec(CurCodeDecl);
  EmitFunctionProlog(*CurFnInfo, CurFn, Args);

  if (D && isa<CXXMethodDecl>(D) && cast<CXXMethodDecl>(D)->isInstance())
    CGM.getCXXABI().EmitInstanceFunctionProlog(*this);

  // If any of the arguments have a variably modified type, make sure to
  // emit the type size.
  for (FunctionArgList::const_iterator i = Args.begin(), e = Args.end();
       i != e; ++i) {
    QualType Ty = i->second;

    if (Ty->isVariablyModifiedType())
      EmitVLASize(Ty);
  }
}

void CodeGenFunction::EmitFunctionBody(FunctionArgList &Args) {
  const FunctionDecl *FD = cast<FunctionDecl>(CurGD.getDecl());
  LLVM_ASSERT(FD->getBody());
  EmitStmt(FD->getBody());
}

/// Tries to mark the given function nounwind based on the
/// non-existence of any throwing calls within it.  We believe this is
/// lightweight enough to do at -O0.
static void TryMarkNoThrow(llvm::Function *F) {
  // LLVM treats 'nounwind' on a function as part of the type, so we
  // can't do this on functions that can be overwritten.
  if (F->mayBeOverridden()) return;

  for (llvm::Function::iterator FI = F->begin(), FE = F->end(); FI != FE; ++FI)
    for (llvm::BasicBlock::iterator
           BI = FI->begin(), BE = FI->end(); BI != BE; ++BI)
      if (llvm::CallInst *Call = dyn_cast<llvm::CallInst>(&*BI))
        if (!Call->doesNotThrow())
          return;
  F->setDoesNotThrow(true);
}

void CodeGenFunction::GenerateCode(GlobalDecl GD, llvm::Function *Fn) {
  const FunctionDecl *FD = cast<FunctionDecl>(GD.getDecl());
  
  // Check if we should generate debug info for this function.
  if (CGM.getDebugInfo() && !FD->hasAttr<NoDebugAttr>())
    DebugInfo = CGM.getDebugInfo();

  FunctionArgList Args;
  QualType ResTy = FD->getResultType();

  CurGD = GD;
  if (isa<CXXMethodDecl>(FD) && cast<CXXMethodDecl>(FD)->isInstance())
    CGM.getCXXABI().BuildInstanceFunctionParams(*this, ResTy, Args);

  if (FD->getNumParams()) {
    const FunctionProtoType* FProto = FD->getType()->getAs<FunctionProtoType>();
    LLVM_ASSERT(FProto && "Function def must have prototype!");

    for (unsigned i = 0, e = FD->getNumParams(); i != e; ++i)
      Args.push_back(std::make_pair(FD->getParamDecl(i),
                                    FProto->getArgType(i)));
  }

  SourceRange BodyRange;
  if (Stmt *Body = FD->getBody()) BodyRange = Body->getSourceRange();

  // Emit the standard function prologue.
  StartFunction(GD, ResTy, Fn, Args, BodyRange.getBegin());

  // Generate the body of the function.
  if (isa<CXXDestructorDecl>(FD))
    EmitDestructorBody(Args);
  else if (isa<CXXConstructorDecl>(FD))
    EmitConstructorBody(Args);
  else
    EmitFunctionBody(Args);

  // Emit the standard function epilogue.
  FinishFunction(BodyRange.getEnd());

  // If we haven't marked the function nothrow through other means, do
  // a quick pass now to see if we can.
  if (!CurFn->doesNotThrow())
    TryMarkNoThrow(CurFn);
}

/// ContainsLabel - Return true if the statement contains a label in it.  If
/// this statement is not executed normally, it not containing a label means
/// that we can just remove the code.
bool CodeGenFunction::ContainsLabel(const Stmt *S, bool IgnoreCaseStmts) {
  // Null statement, not a label!
  if (S == 0) return false;

  // If this is a label, we have to emit the code, consider something like:
  // if (0) {  ...  foo:  bar(); }  goto foo;
  if (isa<LabelStmt>(S))
    return true;

  // If this is a case/default statement, and we haven't seen a switch, we have
  // to emit the code.
  if (isa<SwitchCase>(S) && !IgnoreCaseStmts)
    return true;

  // If this is a switch statement, we want to ignore cases below it.
  if (isa<SwitchStmt>(S))
    IgnoreCaseStmts = true;

  // Scan subexpressions for verboten labels.
  for (Stmt::const_child_iterator I = S->child_begin(), E = S->child_end();
       I != E; ++I)
    if (ContainsLabel(*I, IgnoreCaseStmts))
      return true;

  return false;
}


/// ConstantFoldsToSimpleInteger - If the sepcified expression does not fold to
/// a constant, or if it does but contains a label, return 0.  If it constant
/// folds to 'true' and does not contain a label, return 1, if it constant folds
/// to 'false' and does not contain a label, return -1.
int CodeGenFunction::ConstantFoldsToSimpleInteger(const Expr *Cond) {
  // FIXME: Rename and handle conversion of other evaluatable things
  // to bool.
  Expr::EvalResult Result;
  if (!Cond->Evaluate(Result, getContext()) || !Result.Val.isInt() ||
      Result.HasSideEffects)
    return 0;  // Not foldable, not integer or not fully evaluatable.

  if (CodeGenFunction::ContainsLabel(Cond))
    return 0;  // Contains a label.

  return Result.Val.getInt().getBoolValue() ? 1 : -1;
}


/// EmitBranchOnBoolExpr - Emit a branch on a boolean condition (e.g. for an if
/// statement) to the specified blocks.  Based on the condition, this might try
/// to simplify the codegen of the conditional based on the branch.
///
void CodeGenFunction::EmitBranchOnBoolExpr(const Expr *Cond,
                                           llvm::BasicBlock *TrueBlock,
                                           llvm::BasicBlock *FalseBlock) {
  if (const ParenExpr *PE = dyn_cast<ParenExpr>(Cond))
    return EmitBranchOnBoolExpr(PE->getSubExpr(), TrueBlock, FalseBlock);

  if (const BinaryOperator *CondBOp = dyn_cast<BinaryOperator>(Cond)) {
    // Handle X && Y in a condition.
    if (CondBOp->getOpcode() == BO_LAnd) {
      // If we have "1 && X", simplify the code.  "0 && X" would have constant
      // folded if the case was simple enough.
      if (ConstantFoldsToSimpleInteger(CondBOp->getLHS()) == 1) {
        // br(1 && X) -> br(X).
        return EmitBranchOnBoolExpr(CondBOp->getRHS(), TrueBlock, FalseBlock);
      }

      // If we have "X && 1", simplify the code to use an uncond branch.
      // "X && 0" would have been constant folded to 0.
      if (ConstantFoldsToSimpleInteger(CondBOp->getRHS()) == 1) {
        // br(X && 1) -> br(X).
        return EmitBranchOnBoolExpr(CondBOp->getLHS(), TrueBlock, FalseBlock);
      }

      // Emit the LHS as a conditional.  If the LHS conditional is false, we
      // want to jump to the FalseBlock.
      llvm::BasicBlock *LHSTrue = createBasicBlock("land.lhs.true");
      EmitBranchOnBoolExpr(CondBOp->getLHS(), LHSTrue, FalseBlock);
      EmitBlock(LHSTrue);

      // Any temporaries created here are conditional.
      BeginConditionalBranch();
      EmitBranchOnBoolExpr(CondBOp->getRHS(), TrueBlock, FalseBlock);
      EndConditionalBranch();

      return;
    } else if (CondBOp->getOpcode() == BO_LOr) {
      // If we have "0 || X", simplify the code.  "1 || X" would have constant
      // folded if the case was simple enough.
      if (ConstantFoldsToSimpleInteger(CondBOp->getLHS()) == -1) {
        // br(0 || X) -> br(X).
        return EmitBranchOnBoolExpr(CondBOp->getRHS(), TrueBlock, FalseBlock);
      }

      // If we have "X || 0", simplify the code to use an uncond branch.
      // "X || 1" would have been constant folded to 1.
      if (ConstantFoldsToSimpleInteger(CondBOp->getRHS()) == -1) {
        // br(X || 0) -> br(X).
        return EmitBranchOnBoolExpr(CondBOp->getLHS(), TrueBlock, FalseBlock);
      }

      // Emit the LHS as a conditional.  If the LHS conditional is true, we
      // want to jump to the TrueBlock.
      llvm::BasicBlock *LHSFalse = createBasicBlock("lor.lhs.false");
      EmitBranchOnBoolExpr(CondBOp->getLHS(), TrueBlock, LHSFalse);
      EmitBlock(LHSFalse);

      // Any temporaries created here are conditional.
      BeginConditionalBranch();
      EmitBranchOnBoolExpr(CondBOp->getRHS(), TrueBlock, FalseBlock);
      EndConditionalBranch();

      return;
    }
  }

  if (const UnaryOperator *CondUOp = dyn_cast<UnaryOperator>(Cond)) {
    // br(!x, t, f) -> br(x, f, t)
    if (CondUOp->getOpcode() == UO_LNot)
      return EmitBranchOnBoolExpr(CondUOp->getSubExpr(), FalseBlock, TrueBlock);
  }

  if (const ConditionalOperator *CondOp = dyn_cast<ConditionalOperator>(Cond)) {
    // Handle ?: operator.

    // Just ignore GNU ?: extension.
    if (CondOp->getLHS()) {
      // br(c ? x : y, t, f) -> br(c, br(x, t, f), br(y, t, f))
      llvm::BasicBlock *LHSBlock = createBasicBlock("cond.true");
      llvm::BasicBlock *RHSBlock = createBasicBlock("cond.false");
      EmitBranchOnBoolExpr(CondOp->getCond(), LHSBlock, RHSBlock);
      EmitBlock(LHSBlock);
      EmitBranchOnBoolExpr(CondOp->getLHS(), TrueBlock, FalseBlock);
      EmitBlock(RHSBlock);
      EmitBranchOnBoolExpr(CondOp->getRHS(), TrueBlock, FalseBlock);
      return;
    }
  }

  // Emit the code with the fully general case.
  llvm::Value *CondV = EvaluateExprAsBool(Cond);
  Builder.CreateCondBr(CondV, TrueBlock, FalseBlock);
}

/// ErrorUnsupported - Print out an error that codegen doesn't support the
/// specified stmt yet.
void CodeGenFunction::ErrorUnsupported(const Stmt *S, const char *Type,
                                       bool OmitOnError) {
  CGM.ErrorUnsupported(S, Type, OmitOnError);
}

void
CodeGenFunction::EmitNullInitialization(llvm::Value *DestPtr, QualType Ty) {
  // Ignore empty classes in C++.
  if (getContext().getLangOptions().CPlusPlus) {
    if (const RecordType *RT = Ty->getAs<RecordType>()) {
      if (cast<CXXRecordDecl>(RT->getDecl())->isEmpty())
        return;
    }
  }

  // Cast the dest ptr to the appropriate i8 pointer type.
  unsigned DestAS =
    cast<llvm::PointerType>(DestPtr->getType())->getAddressSpace();
  const llvm::Type *BP =
    llvm::Type::getInt8PtrTy(VMContext, DestAS);
  if (DestPtr->getType() != BP)
    DestPtr = Builder.CreateBitCast(DestPtr, BP, "tmp");

  // Get size and alignment info for this aggregate.
  std::pair<uint64_t, unsigned> TypeInfo = getContext().getTypeInfo(Ty);
  uint64_t Size = TypeInfo.first;
  unsigned Align = TypeInfo.second;

  // Don't bother emitting a zero-byte memset.
  if (Size == 0)
    return;

  llvm::ConstantInt *SizeVal = llvm::ConstantInt::get(IntPtrTy, Size / 8);
  llvm::ConstantInt *AlignVal = Builder.getInt32(Align / 8);

  // If the type contains a pointer to data member we can't memset it to zero.
  // Instead, create a null constant and copy it to the destination.
  if (!CGM.getTypes().isZeroInitializable(Ty)) {
    llvm::Constant *NullConstant = CGM.EmitNullConstant(Ty);

    llvm::GlobalVariable *NullVariable = 
      new llvm::GlobalVariable(CGM.getModule(), NullConstant->getType(),
                               /*isConstant=*/true, 
                               llvm::GlobalVariable::PrivateLinkage,
                               NullConstant, llvm::Twine());
    llvm::Value *SrcPtr =
      Builder.CreateBitCast(NullVariable, Builder.getInt8PtrTy());

    // FIXME: variable-size types?

    // Get and call the appropriate llvm.memcpy overload.
    llvm::Constant *Memcpy =
      CGM.getMemCpyFn(DestPtr->getType(), SrcPtr->getType(), IntPtrTy);
    Builder.CreateCall5(Memcpy, DestPtr, SrcPtr, SizeVal, AlignVal,
                        /*volatile*/ Builder.getFalse());
    return;
  } 
  
  // Otherwise, just memset the whole thing to zero.  This is legal
  // because in LLVM, all default initializers (other than the ones we just
  // handled above) are guaranteed to have a bit pattern of all zeros.

  // FIXME: Handle variable sized types.
  Builder.CreateCall5(CGM.getMemSetFn(BP, IntPtrTy), DestPtr,
                      Builder.getInt8(0),
                      SizeVal, AlignVal, /*volatile*/ Builder.getFalse());
}

llvm::BlockAddress *CodeGenFunction::GetAddrOfLabel(const LabelStmt *L) {
  // Make sure that there is a block for the indirect goto.
  if (IndirectBranch == 0)
    GetIndirectGotoBlock();
  
  llvm::BasicBlock *BB = getJumpDestForLabel(L).getBlock();
  
  // Make sure the indirect branch includes all of the address-taken blocks.
  IndirectBranch->addDestination(BB);
  return llvm::BlockAddress::get(CurFn, BB);
}

llvm::BasicBlock *CodeGenFunction::GetIndirectGotoBlock() {
  // If we already made the indirect branch for indirect goto, return its block.
  if (IndirectBranch) return IndirectBranch->getParent();
  
  CGBuilderTy TmpBuilder(createBasicBlock("indirectgoto"));
  
  const llvm::Type *Int8PtrTy = llvm::Type::getInt8PtrTy(VMContext);

  // Create the PHI node that indirect gotos will add entries to.
  llvm::Value *DestVal = TmpBuilder.CreatePHI(Int8PtrTy, "indirect.goto.dest");
  
  // Create the indirect branch instruction.
  IndirectBranch = TmpBuilder.CreateIndirectBr(DestVal);
  return IndirectBranch->getParent();
}

llvm::Value *CodeGenFunction::GetVLASize(const VariableArrayType *VAT) {
  llvm::Value *&SizeEntry = VLASizeMap[VAT->getSizeExpr()];

  LLVM_ASSERT(SizeEntry && "Did not emit size for type");
  return SizeEntry;
}

llvm::Value *CodeGenFunction::EmitVLASize(QualType Ty) {
  LLVM_ASSERT(Ty->isVariablyModifiedType() &&
         "Must pass variably modified type to EmitVLASizes!");

  EnsureInsertPoint();

  if (const VariableArrayType *VAT = getContext().getAsVariableArrayType(Ty)) {
    llvm::Value *&SizeEntry = VLASizeMap[VAT->getSizeExpr()];

    if (!SizeEntry) {
      const llvm::Type *SizeTy = ConvertType(getContext().getSizeType());

      // Get the element size;
      QualType ElemTy = VAT->getElementType();
      llvm::Value *ElemSize;
      if (ElemTy->isVariableArrayType())
        ElemSize = EmitVLASize(ElemTy);
      else
        ElemSize = llvm::ConstantInt::get(SizeTy,
            getContext().getTypeSizeInChars(ElemTy).getQuantity());

      llvm::Value *NumElements = EmitScalarExpr(VAT->getSizeExpr());
      NumElements = Builder.CreateIntCast(NumElements, SizeTy, false, "tmp");

      SizeEntry = Builder.CreateMul(ElemSize, NumElements);
    }

    return SizeEntry;
  }

  if (const ArrayType *AT = dyn_cast<ArrayType>(Ty)) {
    EmitVLASize(AT->getElementType());
    return 0;
  }

  const PointerType *PT = Ty->getAs<PointerType>();
  LLVM_ASSERT(PT && "unknown VM type!");
  EmitVLASize(PT->getPointeeType());
  return 0;
}

llvm::Value* CodeGenFunction::EmitVAListRef(const Expr* E) {
  if (CGM.getContext().getBuiltinVaListType()->isArrayType())
    return EmitScalarExpr(E);
  return EmitLValue(E).getAddress();
}

/// Pops cleanup blocks until the given savepoint is reached.
void CodeGenFunction::PopCleanupBlocks(EHScopeStack::stable_iterator Old) {
  LLVM_ASSERT(Old.isValid());

  while (EHStack.stable_begin() != Old) {
    EHCleanupScope &Scope = cast<EHCleanupScope>(*EHStack.begin());

    // As long as Old strictly encloses the scope's enclosing normal
    // cleanup, we're going to emit another normal cleanup which
    // fallthrough can propagate through.
    bool FallThroughIsBranchThrough =
      Old.strictlyEncloses(Scope.getEnclosingNormalCleanup());

    PopCleanupBlock(FallThroughIsBranchThrough);
  }
}

static llvm::BasicBlock *CreateNormalEntry(CodeGenFunction &CGF,
                                           EHCleanupScope &Scope) {
  LLVM_ASSERT(Scope.isNormalCleanup());
  llvm::BasicBlock *Entry = Scope.getNormalBlock();
  if (!Entry) {
    Entry = CGF.createBasicBlock("cleanup");
    Scope.setNormalBlock(Entry);
  }
  return Entry;
}

static llvm::BasicBlock *CreateEHEntry(CodeGenFunction &CGF,
                                       EHCleanupScope &Scope) {
  LLVM_ASSERT(Scope.isEHCleanup());
  llvm::BasicBlock *Entry = Scope.getEHBlock();
  if (!Entry) {
    Entry = CGF.createBasicBlock("eh.cleanup");
    Scope.setEHBlock(Entry);
  }
  return Entry;
}

/// Transitions the terminator of the given exit-block of a cleanup to
/// be a cleanup switch.
static llvm::SwitchInst *TransitionToCleanupSwitch(CodeGenFunction &CGF,
                                                   llvm::BasicBlock *Block) {
  // If it's a branch, turn it into a switch whose default
  // destination is its original target.
  llvm::TerminatorInst *Term = Block->getTerminator();
  LLVM_ASSERT(Term && "can't transition block without terminator");

  if (llvm::BranchInst *Br = dyn_cast<llvm::BranchInst>(Term)) {
    LLVM_ASSERT(Br->isUnconditional());
    llvm::LoadInst *Load =
      new llvm::LoadInst(CGF.getNormalCleanupDestSlot(), "cleanup.dest", Term);
    llvm::SwitchInst *Switch =
      llvm::SwitchInst::Create(Load, Br->getSuccessor(0), 4, Block);
    Br->eraseFromParent();
    return Switch;
  } else {
    return cast<llvm::SwitchInst>(Term);
  }
}

/// Attempts to reduce a cleanup's entry block to a fallthrough.  This
/// is basically llvm::MergeBlockIntoPredecessor, except
/// simplified/optimized for the tighter constraints on cleanup blocks.
///
/// Returns the new block, whatever it is.
static llvm::BasicBlock *SimplifyCleanupEntry(CodeGenFunction &CGF,
                                              llvm::BasicBlock *Entry) {
  llvm::BasicBlock *Pred = Entry->getSinglePredecessor();
  if (!Pred) return Entry;

  llvm::BranchInst *Br = dyn_cast<llvm::BranchInst>(Pred->getTerminator());
  if (!Br || Br->isConditional()) return Entry;
  LLVM_ASSERT(Br->getSuccessor(0) == Entry);

  // If we were previously inserting at the end of the cleanup entry
  // block, we'll need to continue inserting at the end of the
  // predecessor.
  bool WasInsertBlock = CGF.Builder.GetInsertBlock() == Entry;
  LLVM_ASSERT(!WasInsertBlock || CGF.Builder.GetInsertPoint() == Entry->end());

  // Kill the branch.
  Br->eraseFromParent();

  // Merge the blocks.
  Pred->getInstList().splice(Pred->end(), Entry->getInstList());

  // Kill the entry block.
  Entry->eraseFromParent();

  if (WasInsertBlock)
    CGF.Builder.SetInsertPoint(Pred);

  return Pred;
}

static void EmitCleanup(CodeGenFunction &CGF,
                        EHScopeStack::Cleanup *Fn,
                        bool ForEH) {
  if (ForEH) CGF.EHStack.pushTerminate();
  Fn->Emit(CGF, ForEH);
  if (ForEH) CGF.EHStack.popTerminate();
  LLVM_ASSERT(CGF.HaveInsertPoint() && "cleanup ended with no insertion point?");
}

/// Pops a cleanup block.  If the block includes a normal cleanup, the
/// current insertion point is threaded through the cleanup, as are
/// any branch fixups on the cleanup.
void CodeGenFunction::PopCleanupBlock(bool FallthroughIsBranchThrough) {
  LLVM_ASSERT(!EHStack.empty() && "cleanup stack is empty!");
  LLVM_ASSERT(isa<EHCleanupScope>(*EHStack.begin()) && "top not a cleanup!");
  EHCleanupScope &Scope = cast<EHCleanupScope>(*EHStack.begin());
  LLVM_ASSERT(Scope.getFixupDepth() <= EHStack.getNumBranchFixups());
  LLVM_ASSERT(Scope.isActive() && "cleanup was still inactive when popped!");

  // Check whether we need an EH cleanup.  This is only true if we've
  // generated a lazy EH cleanup block.
  bool RequiresEHCleanup = Scope.hasEHBranches();

  // Check the three conditions which might require a normal cleanup:

  // - whether there are branch fix-ups through this cleanup
  unsigned FixupDepth = Scope.getFixupDepth();
  bool HasFixups = EHStack.getNumBranchFixups() != FixupDepth;

  // - whether there are branch-throughs or branch-afters
  bool HasExistingBranches = Scope.hasBranches();

  // - whether there's a fallthrough
  llvm::BasicBlock *FallthroughSource = Builder.GetInsertBlock();
  bool HasFallthrough = (FallthroughSource != 0);

  bool RequiresNormalCleanup = false;
  if (Scope.isNormalCleanup() &&
      (HasFixups || HasExistingBranches || HasFallthrough)) {
    RequiresNormalCleanup = true;
  }

  // If we don't need the cleanup at all, we're done.
  if (!RequiresNormalCleanup && !RequiresEHCleanup) {
    EHStack.popCleanup(); // safe because there are no fixups
    LLVM_ASSERT(EHStack.getNumBranchFixups() == 0 ||
           EHStack.hasNormalCleanups());
    return;
  }

  // Copy the cleanup emission data out.  Note that SmallVector
  // guarantees maximal alignment for its buffer regardless of its
  // type parameter.
  llvm::SmallVector<char, 8*sizeof(void*)> CleanupBuffer;
  CleanupBuffer.reserve(Scope.getCleanupSize());
  memcpy(CleanupBuffer.data(),
         Scope.getCleanupBuffer(), Scope.getCleanupSize());
  CleanupBuffer.set_size(Scope.getCleanupSize());
  EHScopeStack::Cleanup *Fn =
    reinterpret_cast<EHScopeStack::Cleanup*>(CleanupBuffer.data());

  // We want to emit the EH cleanup after the normal cleanup, but go
  // ahead and do the setup for the EH cleanup while the scope is still
  // alive.
  llvm::BasicBlock *EHEntry = 0;
  llvm::SmallVector<llvm::Instruction*, 2> EHInstsToAppend;
  if (RequiresEHCleanup) {
    EHEntry = CreateEHEntry(*this, Scope);

    // Figure out the branch-through dest if necessary.
    llvm::BasicBlock *EHBranchThroughDest = 0;
    if (Scope.hasEHBranchThroughs()) {
      LLVM_ASSERT(Scope.getEnclosingEHCleanup() != EHStack.stable_end());
      EHScope &S = *EHStack.find(Scope.getEnclosingEHCleanup());
      EHBranchThroughDest = CreateEHEntry(*this, cast<EHCleanupScope>(S));
    }

    // If we have exactly one branch-after and no branch-throughs, we
    // can dispatch it without a switch.
    if (!Scope.hasEHBranchThroughs() &&
        Scope.getNumEHBranchAfters() == 1) {
      LLVM_ASSERT(!EHBranchThroughDest);

      // TODO: remove the spurious eh.cleanup.dest stores if this edge
      // never went through any switches.
      llvm::BasicBlock *BranchAfterDest = Scope.getEHBranchAfterBlock(0);
      EHInstsToAppend.push_back(llvm::BranchInst::Create(BranchAfterDest));
    
    // Otherwise, if we have any branch-afters, we need a switch.
    } else if (Scope.getNumEHBranchAfters()) {
      // The default of the switch belongs to the branch-throughs if
      // they exist.
      llvm::BasicBlock *Default =
        (EHBranchThroughDest ? EHBranchThroughDest : getUnreachableBlock());

      const unsigned SwitchCapacity = Scope.getNumEHBranchAfters();

      llvm::LoadInst *Load =
        new llvm::LoadInst(getEHCleanupDestSlot(), "cleanup.dest");
      llvm::SwitchInst *Switch =
        llvm::SwitchInst::Create(Load, Default, SwitchCapacity);

      EHInstsToAppend.push_back(Load);
      EHInstsToAppend.push_back(Switch);

      for (unsigned I = 0, E = Scope.getNumEHBranchAfters(); I != E; ++I)
        Switch->addCase(Scope.getEHBranchAfterIndex(I),
                        Scope.getEHBranchAfterBlock(I));

    // Otherwise, we have only branch-throughs; jump to the next EH
    // cleanup.
    } else {
      LLVM_ASSERT(EHBranchThroughDest);
      EHInstsToAppend.push_back(llvm::BranchInst::Create(EHBranchThroughDest));
    }
  }

  if (!RequiresNormalCleanup) {
    EHStack.popCleanup();
  } else {
    // As a kindof crazy internal case, branch-through fall-throughs
    // leave the insertion point set to the end of the last cleanup.
    bool HasPrebranchedFallthrough =
      (HasFallthrough && FallthroughSource->getTerminator());
    LLVM_ASSERT(!HasPrebranchedFallthrough ||
           FallthroughSource->getTerminator()->getSuccessor(0)
             == Scope.getNormalBlock());

    // If we have a fallthrough and no other need for the cleanup,
    // emit it directly.
    if (HasFallthrough && !HasPrebranchedFallthrough &&
        !HasFixups && !HasExistingBranches) {

      // Fixups can cause us to optimistically create a normal block,
      // only to later have no real uses for it.  Just delete it in
      // this case.
      // TODO: we can potentially simplify all the uses after this.
      if (Scope.getNormalBlock()) {
        Scope.getNormalBlock()->replaceAllUsesWith(getUnreachableBlock());
        delete Scope.getNormalBlock();
      }

      EHStack.popCleanup();

      EmitCleanup(*this, Fn, /*ForEH*/ false);

    // Otherwise, the best approach is to thread everything through
    // the cleanup block and then try to clean up after ourselves.
    } else {
      // Force the entry block to exist.
      llvm::BasicBlock *NormalEntry = CreateNormalEntry(*this, Scope);

      // If there's a fallthrough, we need to store the cleanup
      // destination index.  For fall-throughs this is always zero.
      if (HasFallthrough && !HasPrebranchedFallthrough)
        Builder.CreateStore(Builder.getInt32(0), getNormalCleanupDestSlot());

      // Emit the entry block.  This implicitly branches to it if we
      // have fallthrough.  All the fixups and existing branches should
      // already be branched to it.
      EmitBlock(NormalEntry);

      bool HasEnclosingCleanups =
        (Scope.getEnclosingNormalCleanup() != EHStack.stable_end());

      // Compute the branch-through dest if we need it:
      //   - if there are branch-throughs threaded through the scope
      //   - if fall-through is a branch-through
      //   - if there are fixups that will be optimistically forwarded
      //     to the enclosing cleanup
      llvm::BasicBlock *BranchThroughDest = 0;
      if (Scope.hasBranchThroughs() ||
          (HasFallthrough && FallthroughIsBranchThrough) ||
          (HasFixups && HasEnclosingCleanups)) {
        LLVM_ASSERT(HasEnclosingCleanups);
        EHScope &S = *EHStack.find(Scope.getEnclosingNormalCleanup());
        BranchThroughDest = CreateNormalEntry(*this, cast<EHCleanupScope>(S));
      }

      llvm::BasicBlock *FallthroughDest = 0;
      llvm::SmallVector<llvm::Instruction*, 2> InstsToAppend;

      // If there's exactly one branch-after and no other threads,
      // we can route it without a switch.
      if (!Scope.hasBranchThroughs() && !HasFixups && !HasFallthrough &&
          Scope.getNumBranchAfters() == 1) {
        LLVM_ASSERT(!BranchThroughDest);

        // TODO: clean up the possibly dead stores to the cleanup dest slot.
        llvm::BasicBlock *BranchAfter = Scope.getBranchAfterBlock(0);
        InstsToAppend.push_back(llvm::BranchInst::Create(BranchAfter));

      // Build a switch-out if we need it:
      //   - if there are branch-afters threaded through the scope
      //   - if fall-through is a branch-after
      //   - if there are fixups that have nowhere left to go and
      //     so must be immediately resolved
      } else if (Scope.getNumBranchAfters() ||
                 (HasFallthrough && !FallthroughIsBranchThrough) ||
                 (HasFixups && !HasEnclosingCleanups)) {

        llvm::BasicBlock *Default =
          (BranchThroughDest ? BranchThroughDest : getUnreachableBlock());

        // TODO: base this on the number of branch-afters and fixups
        const unsigned SwitchCapacity = 10;

        llvm::LoadInst *Load =
          new llvm::LoadInst(getNormalCleanupDestSlot(), "cleanup.dest");
        llvm::SwitchInst *Switch =
          llvm::SwitchInst::Create(Load, Default, SwitchCapacity);

        InstsToAppend.push_back(Load);
        InstsToAppend.push_back(Switch);

        // Branch-after fallthrough.
        if (HasFallthrough && !FallthroughIsBranchThrough) {
          FallthroughDest = createBasicBlock("cleanup.cont");
          Switch->addCase(Builder.getInt32(0), FallthroughDest);
        }

        for (unsigned I = 0, E = Scope.getNumBranchAfters(); I != E; ++I) {
          Switch->addCase(Scope.getBranchAfterIndex(I),
                          Scope.getBranchAfterBlock(I));
        }

        if (HasFixups && !HasEnclosingCleanups)
          ResolveAllBranchFixups(Switch);
      } else {
        // We should always have a branch-through destination in this case.
        LLVM_ASSERT(BranchThroughDest);
        InstsToAppend.push_back(llvm::BranchInst::Create(BranchThroughDest));
      }

      // We're finally ready to pop the cleanup.
      EHStack.popCleanup();
      LLVM_ASSERT(EHStack.hasNormalCleanups() == HasEnclosingCleanups);

      EmitCleanup(*this, Fn, /*ForEH*/ false);

      // Append the prepared cleanup prologue from above.
      llvm::BasicBlock *NormalExit = Builder.GetInsertBlock();
      for (unsigned I = 0, E = InstsToAppend.size(); I != E; ++I)
        NormalExit->getInstList().push_back(InstsToAppend[I]);

      // Optimistically hope that any fixups will continue falling through.
      for (unsigned I = FixupDepth, E = EHStack.getNumBranchFixups();
           I < E; ++I) {
        BranchFixup &Fixup = CGF.EHStack.getBranchFixup(I);
        if (!Fixup.Destination) continue;
        if (!Fixup.OptimisticBranchBlock) {
          new llvm::StoreInst(Builder.getInt32(Fixup.DestinationIndex),
                              getNormalCleanupDestSlot(),
                              Fixup.InitialBranch);
          Fixup.InitialBranch->setSuccessor(0, NormalEntry);
        }
        Fixup.OptimisticBranchBlock = NormalExit;
      }
      
      if (FallthroughDest)
        EmitBlock(FallthroughDest);
      else if (!HasFallthrough)
        Builder.ClearInsertionPoint();

      // Check whether we can merge NormalEntry into a single predecessor.
      // This might invalidate (non-IR) pointers to NormalEntry.
      llvm::BasicBlock *NewNormalEntry =
        SimplifyCleanupEntry(*this, NormalEntry);

      // If it did invalidate those pointers, and NormalEntry was the same
      // as NormalExit, go back and patch up the fixups.
      if (NewNormalEntry != NormalEntry && NormalEntry == NormalExit)
        for (unsigned I = FixupDepth, E = EHStack.getNumBranchFixups();
               I < E; ++I)
          CGF.EHStack.getBranchFixup(I).OptimisticBranchBlock = NewNormalEntry;
    }
  }

  LLVM_ASSERT(EHStack.hasNormalCleanups() || EHStack.getNumBranchFixups() == 0);

  // Emit the EH cleanup if required.
  if (RequiresEHCleanup) {
    CGBuilderTy::InsertPoint SavedIP = Builder.saveAndClearIP();

    EmitBlock(EHEntry);
    EmitCleanup(*this, Fn, /*ForEH*/ true);

    // Append the prepared cleanup prologue from above.
    llvm::BasicBlock *EHExit = Builder.GetInsertBlock();
    for (unsigned I = 0, E = EHInstsToAppend.size(); I != E; ++I)
      EHExit->getInstList().push_back(EHInstsToAppend[I]);

    Builder.restoreIP(SavedIP);

    SimplifyCleanupEntry(*this, EHEntry);
  }
}

/// Terminate the current block by emitting a branch which might leave
/// the current cleanup-protected scope.  The target scope may not yet
/// be known, in which case this will require a fixup.
///
/// As a side-effect, this method clears the insertion point.
void CodeGenFunction::EmitBranchThroughCleanup(JumpDest Dest) {
  LLVM_ASSERT(Dest.getScopeDepth().encloses(EHStack.getInnermostNormalCleanup())
         && "stale jump destination");

  if (!HaveInsertPoint())
    return;

  // Create the branch.
  llvm::BranchInst *BI = Builder.CreateBr(Dest.getBlock());

  // Calculate the innermost active normal cleanup.
  EHScopeStack::stable_iterator
    TopCleanup = EHStack.getInnermostActiveNormalCleanup();

  // If we're not in an active normal cleanup scope, or if the
  // destination scope is within the innermost active normal cleanup
  // scope, we don't need to worry about fixups.
  if (TopCleanup == EHStack.stable_end() ||
      TopCleanup.encloses(Dest.getScopeDepth())) { // works for invalid
    Builder.ClearInsertionPoint();
    return;
  }

  // If we can't resolve the destination cleanup scope, just add this
  // to the current cleanup scope as a branch fixup.
  if (!Dest.getScopeDepth().isValid()) {
    BranchFixup &Fixup = EHStack.addBranchFixup();
    Fixup.Destination = Dest.getBlock();
    Fixup.DestinationIndex = Dest.getDestIndex();
    Fixup.InitialBranch = BI;
    Fixup.OptimisticBranchBlock = 0;

    Builder.ClearInsertionPoint();
    return;
  }

  // Otherwise, thread through all the normal cleanups in scope.

  // Store the index at the start.
  llvm::ConstantInt *Index = Builder.getInt32(Dest.getDestIndex());
  new llvm::StoreInst(Index, getNormalCleanupDestSlot(), BI);

  // Adjust BI to point to the first cleanup block.
  {
    EHCleanupScope &Scope =
      cast<EHCleanupScope>(*EHStack.find(TopCleanup));
    BI->setSuccessor(0, CreateNormalEntry(*this, Scope));
  }

  // Add this destination to all the scopes involved.
  EHScopeStack::stable_iterator I = TopCleanup;
  EHScopeStack::stable_iterator E = Dest.getScopeDepth();
  if (E.strictlyEncloses(I)) {
    while (true) {
      EHCleanupScope &Scope = cast<EHCleanupScope>(*EHStack.find(I));
      LLVM_ASSERT(Scope.isNormalCleanup());
      I = Scope.getEnclosingNormalCleanup();

      // If this is the last cleanup we're propagating through, tell it
      // that there's a resolved jump moving through it.
      if (!E.strictlyEncloses(I)) {
        Scope.addBranchAfter(Index, Dest.getBlock());
        break;
      }

      // Otherwise, tell the scope that there's a jump propoagating
      // through it.  If this isn't new information, all the rest of
      // the work has been done before.
      if (!Scope.addBranchThrough(Dest.getBlock()))
        break;
    }
  }
  
  Builder.ClearInsertionPoint();
}

void CodeGenFunction::EmitBranchThroughEHCleanup(UnwindDest Dest) {
  // We should never get invalid scope depths for an UnwindDest; that
  // implies that the destination wasn't set up correctly.
  LLVM_ASSERT(Dest.getScopeDepth().isValid() && "invalid scope depth on EH dest?");

  if (!HaveInsertPoint())
    return;

  // Create the branch.
  llvm::BranchInst *BI = Builder.CreateBr(Dest.getBlock());

  // Calculate the innermost active cleanup.
  EHScopeStack::stable_iterator
    InnermostCleanup = EHStack.getInnermostActiveEHCleanup();

  // If the destination is in the same EH cleanup scope as us, we
  // don't need to thread through anything.
  if (InnermostCleanup.encloses(Dest.getScopeDepth())) {
    Builder.ClearInsertionPoint();
    return;
  }
  LLVM_ASSERT(InnermostCleanup != EHStack.stable_end());

  // Store the index at the start.
  llvm::ConstantInt *Index = Builder.getInt32(Dest.getDestIndex());
  new llvm::StoreInst(Index, getEHCleanupDestSlot(), BI);

  // Adjust BI to point to the first cleanup block.
  {
    EHCleanupScope &Scope =
      cast<EHCleanupScope>(*EHStack.find(InnermostCleanup));
    BI->setSuccessor(0, CreateEHEntry(*this, Scope));
  }
  
  // Add this destination to all the scopes involved.
  for (EHScopeStack::stable_iterator
         I = InnermostCleanup, E = Dest.getScopeDepth(); ; ) {
    LLVM_ASSERT(E.strictlyEncloses(I));
    EHCleanupScope &Scope = cast<EHCleanupScope>(*EHStack.find(I));
    LLVM_ASSERT(Scope.isEHCleanup());
    I = Scope.getEnclosingEHCleanup();

    // If this is the last cleanup we're propagating through, add this
    // as a branch-after.
    if (I == E) {
      Scope.addEHBranchAfter(Index, Dest.getBlock());
      break;
    }

    // Otherwise, add it as a branch-through.  If this isn't new
    // information, all the rest of the work has been done before.
    if (!Scope.addEHBranchThrough(Dest.getBlock()))
      break;
  }
  
  Builder.ClearInsertionPoint();
}

/// All the branch fixups on the EH stack have propagated out past the
/// outermost normal cleanup; resolve them all by adding cases to the
/// given switch instruction.
void CodeGenFunction::ResolveAllBranchFixups(llvm::SwitchInst *Switch) {
  llvm::SmallPtrSet<llvm::BasicBlock*, 4> CasesAdded;

  for (unsigned I = 0, E = EHStack.getNumBranchFixups(); I != E; ++I) {
    // Skip this fixup if its destination isn't set or if we've
    // already treated it.
    BranchFixup &Fixup = EHStack.getBranchFixup(I);
    if (Fixup.Destination == 0) continue;
    if (!CasesAdded.insert(Fixup.Destination)) continue;

    Switch->addCase(Builder.getInt32(Fixup.DestinationIndex),
                    Fixup.Destination);
  }

  EHStack.clearFixups();
}

void CodeGenFunction::ResolveBranchFixups(llvm::BasicBlock *Block) {
  LLVM_ASSERT(Block && "resolving a null target block");
  if (!EHStack.getNumBranchFixups()) return;

  LLVM_ASSERT(EHStack.hasNormalCleanups() &&
         "branch fixups exist with no normal cleanups on stack");

  llvm::SmallPtrSet<llvm::BasicBlock*, 4> ModifiedOptimisticBlocks;
  bool ResolvedAny = false;

  for (unsigned I = 0, E = EHStack.getNumBranchFixups(); I != E; ++I) {
    // Skip this fixup if its destination doesn't match.
    BranchFixup &Fixup = EHStack.getBranchFixup(I);
    if (Fixup.Destination != Block) continue;

    Fixup.Destination = 0;
    ResolvedAny = true;

    // If it doesn't have an optimistic branch block, LatestBranch is
    // already pointing to the right place.
    llvm::BasicBlock *BranchBB = Fixup.OptimisticBranchBlock;
    if (!BranchBB)
      continue;

    // Don't process the same optimistic branch block twice.
    if (!ModifiedOptimisticBlocks.insert(BranchBB))
      continue;

    llvm::SwitchInst *Switch = TransitionToCleanupSwitch(*this, BranchBB);

    // Add a case to the switch.
    Switch->addCase(Builder.getInt32(Fixup.DestinationIndex), Block);
  }

  if (ResolvedAny)
    EHStack.popNullFixups();
}

/// Activate a cleanup that was created in an inactivated state.
void CodeGenFunction::ActivateCleanup(EHScopeStack::stable_iterator C) {
  LLVM_ASSERT(C != EHStack.stable_end() && "activating bottom of stack?");
  EHCleanupScope &Scope = cast<EHCleanupScope>(*EHStack.find(C));
  LLVM_ASSERT(!Scope.isActive() && "double activation");

  // Calculate whether the cleanup was used:
  bool Used = false;

  //   - as a normal cleanup
  if (Scope.isNormalCleanup()) {
    bool NormalUsed = false;
    if (Scope.getNormalBlock()) {
      NormalUsed = true;
    } else {
      // Check whether any enclosed cleanups were needed.
      for (EHScopeStack::stable_iterator
             I = EHStack.getInnermostNormalCleanup(); I != C; ) {
        LLVM_ASSERT(C.strictlyEncloses(I));
        EHCleanupScope &S = cast<EHCleanupScope>(*EHStack.find(I));
        if (S.getNormalBlock()) {
          NormalUsed = true;
          break;
        }
        I = S.getEnclosingNormalCleanup();
      }
    }

    if (NormalUsed)
      Used = true;
    else
      Scope.setActivatedBeforeNormalUse(true);
  }

  //  - as an EH cleanup
  if (Scope.isEHCleanup()) {
    bool EHUsed = false;
    if (Scope.getEHBlock()) {
      EHUsed = true;
    } else {
      // Check whether any enclosed cleanups were needed.
      for (EHScopeStack::stable_iterator
             I = EHStack.getInnermostEHCleanup(); I != C; ) {
        LLVM_ASSERT(C.strictlyEncloses(I));
        EHCleanupScope &S = cast<EHCleanupScope>(*EHStack.find(I));
        if (S.getEHBlock()) {
          EHUsed = true;
          break;
        }
        I = S.getEnclosingEHCleanup();
      }
    }

    if (EHUsed)
      Used = true;
    else
      Scope.setActivatedBeforeEHUse(true);
  }
  
  llvm::AllocaInst *Var = EHCleanupScope::activeSentinel();
  if (Used) {
    Var = CreateTempAlloca(Builder.getInt1Ty());
    InitTempAlloca(Var, Builder.getFalse());
  }
  Scope.setActiveVar(Var);
}

llvm::Value *CodeGenFunction::getNormalCleanupDestSlot() {
  if (!NormalCleanupDest)
    NormalCleanupDest =
      CreateTempAlloca(Builder.getInt32Ty(), "cleanup.dest.slot");
  return NormalCleanupDest;
}

llvm::Value *CodeGenFunction::getEHCleanupDestSlot() {
  if (!EHCleanupDest)
    EHCleanupDest =
      CreateTempAlloca(Builder.getInt32Ty(), "eh.cleanup.dest.slot");
  return EHCleanupDest;
}

void CodeGenFunction::EmitDeclRefExprDbgValue(const DeclRefExpr *E, 
                                              llvm::ConstantInt *Init) {
  assert (Init && "Invalid DeclRefExpr initializer!");
  if (CGDebugInfo *Dbg = getDebugInfo())
    Dbg->EmitGlobalVariable(E->getDecl(), Init, Builder);
}
