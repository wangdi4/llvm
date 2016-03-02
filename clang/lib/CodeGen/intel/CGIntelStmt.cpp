//===--- CGIntelStmt.cpp - Emit LLVM Code from Statements ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This contains code to emit Stmt nodes as LLVM code.
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"
#include "CGDebugInfo.h"
#include "CodeGenModule.h"
#include "TargetInfo.h"
#include "clang/AST/StmtVisitor.h"
#include "clang/Basic/PrettyStackTrace.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Sema/LoopHint.h"
#include "clang/Sema/SemaDiagnostic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Intrinsics.h"
#include "intel/CGCilkPlusRuntime.h"
#include "clang/Basic/CapturedStmt.h"
#include "llvm/IR/TypeBuilder.h"

using namespace clang;
using namespace CodeGen;

#if INTEL_SPECIFIC_CILKPLUS

void
CodeGenFunction::EmitCilkForGrainsizeStmt(const CilkForGrainsizeStmt &S) {
  const Expr *E = S.getGrainsize();
  assert(!E->getType()->isReferenceType() && "invalid type");
  llvm::Value *Grainsize = EmitAnyExpr(E).getScalarVal();
  EmitCilkForStmt(*cast<CilkForStmt>(S.getCilkFor()), Grainsize);
}

/// A simple RAII object to deal with CapturedStmtInfo of the given CGF
class CilkForRAII {
  CodeGenFunction::CGCapturedStmtInfo *OldCapturedStmtInfo;
  CodeGenFunction *CGF;

public:
  CilkForRAII(CodeGenFunction *CGF) : CGF(CGF){
    OldCapturedStmtInfo = CGF->CapturedStmtInfo;
  };
  ~CilkForRAII() {
    CGF->CapturedStmtInfo = OldCapturedStmtInfo;
  }
};

void
CodeGenFunction::EmitCilkForStmt(const CilkForStmt &S, llvm::Value *Grainsize) {
  // if (cond) {
  //   count = loop_count;
  //   grainsize = gs;
  //   initialize context
  //   __cilkrts_cilk_for_32(helper, captures, count, gs);
  // }
  RunCleanupsScope CilkForScope(*this);

  CGDebugInfo *DI = getDebugInfo();
  if (DI)
    DI->EmitLexicalBlockStart(Builder, S.getSourceRange().getBegin());

  // Evaluate the first part before the loop.
  EmitStmt(S.getInit());
  // Emit the helper function
  CGCilkForStmtInfo CSInfo(S);
  CodeGenFunction CGF(CGM, true);
  CGF.CapturedStmtInfo = &CSInfo;
  llvm::Function *Helper = CGF.GenerateCapturedStmtFunction(*S.getBody());

  llvm::BasicBlock *ThenBlock = createBasicBlock("if.then");
  llvm::BasicBlock *ContBlock = createBasicBlock("if.end");
  EmitBranchOnBoolExpr(S.getCond(), ThenBlock, ContBlock, 0);

  EmitBlock(ThenBlock);
  {
    CilkForRAII CfRAII (this);
    RunCleanupsScope Scope(*this);
    const Expr *LoopCountExpr = S.getLoopCount();
    if(!CapturedStmtInfo)
      CapturedStmtInfo = &CSInfo;
    llvm::Value *LoopCount = EmitAnyExpr(LoopCountExpr).getScalarVal();
    // Initialize the captured struct.
    LValue CapStruct = InitCapturedStruct(*S.getBody());
    llvm::LLVMContext &Ctx = CGM.getLLVMContext();
    // Get or insert the cilk_for abi function.
    llvm::Constant *CilkForABI = 0;
    llvm::FunctionType *FTy = 0;
    {
      llvm::Module &M = CGM.getModule();
      uint64_t SizeInBits =
          getContext().getTypeSize(LoopCountExpr->getType());
      if (SizeInBits <= 32u) {
        FTy = llvm::TypeBuilder<void(void(void *, uint32_t, uint32_t),
                                     void *, uint32_t, int), false>::get(Ctx);
        CilkForABI = M.getOrInsertFunction("__cilkrts_cilk_for_32", FTy);
      } else if (SizeInBits <= 64u) {
        FTy = llvm::TypeBuilder<void(void(void *, uint64_t, uint64_t),
                                     void *, uint64_t, int), false>::get(Ctx);
        CilkForABI = M.getOrInsertFunction("__cilkrts_cilk_for_64", FTy);
      } else
        llvm_unreachable("unexpected loop count type size");
    }

    // Call __cilkrts_cilk_for_*(helper, captures, count, grainsize);
    SmallVector<llvm::Value *, 4> Args(4);
    Args[0] = Builder.CreateBitCast(Helper, FTy->getParamType(0));
    Args[1] = Builder.CreatePointerCast(CapStruct.getAddress().getPointer(),
                                        FTy->getParamType(1));
    Args[2] = LoopCount;
    Args[3] = Grainsize ? Grainsize
                        : llvm::Constant::getNullValue(FTy->getParamType(3));

    EmitCallOrInvoke(CilkForABI, Args);

    // Update the Loop Control Variable
    if (!isa<DeclStmt>(S.getInit())) {
      Address LCVAddr = Address::invalid();
      auto I = LocalDeclMap.find(S.getLoopControlVar());
      if (I != LocalDeclMap.end())
        LCVAddr = I->second;
      else if (CGCilkForStmtInfo *CFCSI =
                   dyn_cast<CGCilkForStmtInfo>(CapturedStmtInfo))
        LCVAddr = CFCSI->getInnerLoopControlVarAddr();
      assert(LCVAddr.getPointer() &&
             "missing inner loop control variable address");
      llvm::Value *LCVValue = Builder.CreateLoad(LCVAddr);

      bool IsAdd = true;
      llvm::Value *Delta = 0;
      const Expr *IncExpr = S.getInc();
      if (const UnaryOperator *Inc = dyn_cast<UnaryOperator>(IncExpr)) {
        Delta = LoopCount;
        IsAdd = Inc->isIncrementOp();
      } else if (const BinaryOperator *Inc = dyn_cast<BinaryOperator>(IncExpr)) {
        llvm::Value *RHS = EmitScalarExpr(Inc->getRHS());
        assert(RHS->getType()->isIntegerTy() && "increment not integer type");
        RHS = Builder.CreateSExtOrTrunc(RHS, LoopCount->getType());
        Delta = Builder.CreateMul(RHS, LoopCount);
        IsAdd = (Inc->getOpcode() == BO_AddAssign);
      } else
        llvm_unreachable("unexpected increment expression");

      llvm::Value *Update = 0;
      if (LCVValue->getType()->isPointerTy()) {
        if (!IsAdd) Delta = Builder.CreateNeg(Delta);
        Update = Builder.CreateGEP(LCVValue, Delta);
      } else {
        Delta = Builder.CreateSExtOrTrunc(Delta, LCVValue->getType());
        llvm::Instruction::BinaryOps Op = (IsAdd) ? llvm::Instruction::Add
                                                  : llvm::Instruction::Sub;
        Update = Builder.CreateBinOp(Op, LCVValue, Delta);
      }
      Builder.CreateStore(Update, LCVAddr);
    }

    EmitBranch(ContBlock);
  }

  EmitBlock(ContBlock, true);
}

void CodeGenFunction::EmitCilkForHelperBody(const Stmt *S) {
  // The outlined function for a Cilk for statement looks like
  //
  // void helper(context, low, high) {
  //   for (index = low /*, other-initialization/;
  //        index < high;
  //        ++index /*, loop-increment*/) {
  //     /* loop-body */
  //   }
  // }
  //
  // This function is a simplified version of EmitForStmt with the partial
  // knowledge of the loop head structure. For example, the loop condition
  // always exists and there is no loop condition variable declaration.

  assert(CapturedStmtInfo && CapturedStmtInfo->getKind() == CR_CilkFor &&
           "codegen info expected");
  CGCilkForStmtInfo *CilkForInfo =
      reinterpret_cast<CGCilkForStmtInfo*>(CapturedStmtInfo);
  const CilkForStmt &CilkFor = CilkForInfo->getCilkForStmt();
  const CapturedDecl *CD = CilkFor.getBody()->getCapturedDecl();
  auto Low = LocalDeclMap.find(CD->getParam(1))->second;
  auto High = LocalDeclMap.find(CD->getParam(2))->second;

  // Find the type for the index variable.
  llvm::Type *VarType = Low.getPointer()->getType();
  assert(VarType->isPointerTy() && "pointer type expected");
  VarType = cast<llvm::PointerType>(VarType)->getElementType();
  assert((VarType->isIntegerTy(32) || VarType->isIntegerTy(64))
         && "unexpected type size");

  auto Index = CreateDefaultAlignTempAlloca(VarType, "__index.addr");
  High = Address(Builder.CreatePointerCast(High.getPointer(), Index.getType()),
                 High.getAlignment());

  JumpDest LoopExit = getJumpDestInCurrentScope("loop.end");
  RunCleanupsScope LoopScope(*this);

  // Emit the loop initialization.
  {
    // Initialize Low.
    Builder.CreateStore(Builder.CreateLoad(Low), Index);

    // Initialize the inner loop control variable.
    const VarDecl *D = CilkFor.getInnerLoopControlVar();
    AutoVarEmission Emission = EmitAutoVarAlloca(*D);
    EmitAutoVarInit(Emission);
    EmitAutoVarCleanups(Emission);

    // Keep track of the loop control variable and the address of its
    // corresponding inner copy so that any reference to the loop control
    // variable will reference its inner adjusted copy instead and this
    // correction allows nested _Cilk_for statements.
    auto Addr = Emission.getAllocatedAddress();
    CilkForInfo->setInnerLoopControlVarAddr(Addr);

    // Emit the adjustment on the inner loop control variable.
    EmitStmt(CilkFor.getInnerLoopVarAdjust());
  }

  // Emit the loop condition.
  llvm::BasicBlock *CondBlock = createBasicBlock("loop.cond");
  {
    EmitBlock(CondBlock);
    llvm::BasicBlock *ExitBlock = LoopExit.getBlock();

    LoopStack.setParallel();
    LoopStack.push(CondBlock);

    // If there are any cleanups between here and the loop-exit scope,
    // create a block to stage a loop exit along.
    if (LoopScope.requiresCleanups())
      ExitBlock = createBasicBlock("loop.cond.cleanup");

    // As long as the condition is true, iterate the loop.
    llvm::BasicBlock *LoopBody = createBasicBlock("loop.body");

    QualType CmpTy1 = CD->getParam(1)->getType();
    llvm::Value *CondVal = 0;
    if (CmpTy1->hasSignedIntegerRepresentation()) {
      CondVal = Builder.CreateICmpSLT(Builder.CreateLoad(Index),
                                      Builder.CreateLoad(High));
    } else {
      CondVal = Builder.CreateICmpULT(Builder.CreateLoad(Index),
                                      Builder.CreateLoad(High));
    }
    Builder.CreateCondBr(CondVal, LoopBody, ExitBlock);

    if (ExitBlock != LoopExit.getBlock()) {
      EmitBlock(ExitBlock);
      EmitBranchThroughCleanup(LoopExit);
    }

    EmitBlock(LoopBody);
  }

  JumpDest Continue = getJumpDestInCurrentScope("loop.inc");

  // Store the blocks to use for break and continue.
  BreakContinueStack.push_back(BreakContinue(LoopExit, Continue));

  // Emit the loop body.
  {
    // Create a separate cleanup scope for the body, in case it is not
    // a compound statement.
    RunCleanupsScope BodyScope(*this);
    EmitStmt(S);
  }

  // Emit the loop increment.
  {
    EmitBlock(Continue.getBlock());
    EmitStmt(CilkFor.getInc());

    // Increment the loop variable and branch back the loop condition.
    llvm::Value *Inc = Builder.CreateAdd(Builder.CreateLoad(Index),
                                         llvm::ConstantInt::get(VarType, 1));
    Builder.CreateStore(Inc, Index);
  }

  BreakContinueStack.pop_back();

  EmitBranch(CondBlock);

  LoopScope.ForceCleanup();

  LoopStack.pop();

  // Emit the fall-through block.
  EmitBlock(LoopExit.getBlock(), true);
}

void
CodeGenFunction::CGCilkSpawnInfo::EmitBody(CodeGenFunction &CGF, const Stmt *S) {
  // If there is a receiver, save its address.
  CGCilkSpawnInfo *Info = cast<CGCilkSpawnInfo>(CGF.CapturedStmtInfo);
  if (Info->getReceiverDecl()) {
    assert(CGF.CurFn->arg_size() >= 2);
    llvm::Function::arg_iterator A = CGF.CurFn->arg_begin();
    Info->setReceiverAddr(
        Address(&*(++A), CharUnits::fromQuantity(CGF.PointerAlignInBytes)));

    // Similarly, save the receiver temporary address if it exists.
    if (++A != CGF.CurFn->arg_end())
      Info->setReceiverTmp(
          Address(&*A, CharUnits::fromQuantity(CGF.PointerAlignInBytes)));
  }

  CGF.CGM.getCilkPlusRuntime().EmitCilkHelperStackFrame(CGF);
  CGCapturedStmtInfo::EmitBody(CGF, S);
}

void CodeGenFunction::EmitSIMDForHelperCall(llvm::Function *BodyFunc,
                                            LValue CapStruct, Address LoopIndex,
                                            bool IsLastIter) {
  // Emit call to the helper function.
  SmallVector<llvm::Value *, 3> HelperArgs;
  HelperArgs.push_back(CapStruct.getAddress().getPointer());
  HelperArgs.push_back(Builder.CreateLoad(LoopIndex));

  llvm::Value *LastIter = 0;
  if (IsLastIter)
    LastIter = llvm::ConstantInt::getTrue(BodyFunc->getContext());
  else
    LastIter = llvm::ConstantInt::getFalse(BodyFunc->getContext());
  HelperArgs.push_back(LastIter);

  disableExceptions();
  EmitCallOrInvoke(BodyFunc, HelperArgs);
  enableExceptions();
}

void CodeGenFunction::CGPragmaSimd::emitInit(CodeGenFunction &CGF,
                                             Address &LoopIndex,
                                             llvm::Value *&LoopCount) {
  // Emit the initialization.
  CGF.EmitStmt(this->getInit());

  // Load the current value as the initial value and cache the stride value.
  const VarDecl *LoopControlVar = cast<SIMDForStmt>(
      this->getStmt())->getLoopControlVar();
  assert(LoopControlVar && "invalid loop control variable");

  DeclRefExpr DRE(const_cast<VarDecl *>(LoopControlVar),
                  /*RefersToEnclosingVariableOrCapture=*/true,
                  LoopControlVar->getType().getNonReferenceType(), VK_LValue,
                  SourceLocation());
  auto ControlVarAddr = CGF.EmitDeclRefLValue(&DRE).getAddress();
  assert(ControlVarAddr.getPointer() &&
         "invalid loop control variable address");

  LCVAddr = ControlVarAddr;
  LCVInitVal = CGF.Builder.CreateLoad(ControlVarAddr, "__init");

  LCVStrideVal = CGF.EmitAnyExpr(
      cast<SIMDForStmt>(this->getStmt())->getLoopStride()).getScalarVal();
  assert(LCVStrideVal->getType()->isIntegerTy() && "invalid stride type");

  LoopCount = CGF.EmitAnyExpr(getLoopCount()).getScalarVal();
  LoopIndex = CGF.CreateMemTemp(getLoopCount()->getType(), "__index.addr");
}

void CodeGenFunction::CGPragmaSimd::emitIncrement(CodeGenFunction &CGF,
                                                  Address IndexVar) const {
  llvm::Value *Index = CGF.Builder.CreateLoad(IndexVar, "__index");
  llvm::Value *Stride
    = CGF.Builder.CreateSExtOrTrunc(LCVStrideVal, Index->getType());
  llvm::Value *Delta = CGF.Builder.CreateMul(Index, Stride, "__delta");

  //  lcv = init + index * stride;
  llvm::Value *NewIndex = 0;
  if (LCVInitVal->getType()->isPointerTy())
    NewIndex = CGF.Builder.CreateGEP(LCVInitVal, Delta, "__index.new");
  else {
    Delta = CGF.Builder.CreateSExtOrTrunc(Delta, LCVInitVal->getType());
    NewIndex = CGF.Builder.CreateAdd(LCVInitVal, Delta, "__index.new");
  }
  CGF.Builder.CreateStore(NewIndex, LCVAddr);
}

// Simd wrappers implementation for '#pragma simd'.
bool CodeGenFunction::CGPragmaSimd::emitSafelen(CodeGenFunction *CGF) const {
  bool SeparateLastIter = false;
  const ArrayRef<Attr *> &Attrs = SimdFor->getSIMDAttrs();
  for (unsigned i = 0, e = Attrs.size(); i < e; ++i) {
    switch (Attrs[i]->getKind()) {
      case clang::attr::SIMD:
        CGF->LoopStack.setParallel();
        CGF->LoopStack.setVectorizeEnable(true);
        break;
      case clang::attr::SIMDLength:
        {
          const SIMDLengthAttr *SIMDLength = 0;
          SIMDLength = static_cast<const SIMDLengthAttr*>(Attrs[i]);
          RValue Width = CGF->EmitAnyExpr(SIMDLength->getValueExpr(),
              AggValueSlot::ignored(), true);
          llvm::ConstantInt *C = dyn_cast<llvm::ConstantInt>(Width.getScalarVal());
          assert(C);
          CGF->LoopStack.setVectorizeWidth(C->getZExtValue());
          // In presence of finite 'safelen', it may be unsafe to mark all
          // the memory instructions parallel, because loop-carried
          // dependences of 'safelen' iterations are possible.
          CGF->LoopStack.setParallel(false);
        } break;
      case clang::attr::SIMDLastPrivate:
      case clang::attr::SIMDLinear:
        SeparateLastIter = true;
        break;
      default:
        // Not yet handled
        ;
    }
  }
  return SeparateLastIter;
}

#ifdef INTEL_SPECIFIC_IL0_BACKEND
/// Helper to build argument list for passing operands list into
/// backend (as the intel.pragma call arguments).
class IntelPragmaBuilder {
  llvm::SmallVector<llvm::Value *, 4> Ops;
  llvm::LLVMContext &C;

public:
  IntelPragmaBuilder(llvm::LLVMContext &Context) : C(Context) {}
  IntelPragmaBuilder &addop(StringRef S) {
    Ops.push_back(llvm::MetadataAsValue::get(C, llvm::MDString::get(C, S)));
    return *this;
  }
  IntelPragmaBuilder &addop(llvm::Value *V) {
    Ops.push_back(llvm::MetadataAsValue::get(C, llvm::ValueAsMetadata::get(V)));
    return *this;
  }
  IntelPragmaBuilder &addop(RValue RVal) {
    if (RVal.isScalar()) {
      addop("SCALAR").addop(RVal.getScalarVal());
    } else if (RVal.isComplex()) {
      addop("COMPLEX").addop(RVal.getComplexVal().first).addop(
          RVal.getComplexVal().second);
    } else {
      addop("AGGREGATE").addop(RVal.getAggregatePointer());
    }
    return *this;
  }
  IntelPragmaBuilder &addop(LValue LVal, CodeGenFunction *CGF) {
    if (LVal.isSimple()) {
      addop("SIMPLE").addop(LVal.getPointer());
    } else if (LVal.isVectorElt()) {
      addop("VECTOR_ELT").addop(LVal.getVectorPointer()).addop(
          LVal.getVectorIdx());
    } else if (LVal.isExtVectorElt()) {
      RValue RVal = CGF->EmitLoadOfExtVectorElementLValue(LVal);
      addop("EXT_VECTOR_ELT").addop(RVal);
    } else {
      RValue RVal = CGF->EmitLoadOfBitfieldLValue(LVal);
      addop("BITFIELD").addop(RVal);
    }
    return *this;
  }
  ArrayRef<llvm::Value *> getops() { return Ops; }
};

void CodeGenFunction::CGPragmaSimd::emitIntelIntrinsic(
    CodeGenFunction *CGF, CodeGenModule *CGM, Address LoopIndex,
    llvm::Value *LoopCount) const {
  IntelPragmaBuilder P(CGM->getLLVMContext());
  P.addop("SIMD_LOOP");
  const ArrayRef<Attr *> &Attrs = SimdFor->getSIMDAttrs();
  for (unsigned i = 0, e = Attrs.size(); i < e; ++i) {
    switch (Attrs[i]->getKind()) {
      case clang::attr::SIMDLength:
        {
          auto SIMDLength = static_cast<const SIMDLengthAttr*>(Attrs[i]);
          RValue Width = CGF->EmitAnyExpr(SIMDLength->getValueExpr(),
                                          AggValueSlot::ignored(), true);
          auto C = cast<llvm::ConstantInt>(Width.getScalarVal());
          P.addop("VECTORLENGTH");
          P.addop(C);
        } break;
      case clang::attr::SIMDAssert:
        P.addop("ASSERT");
        break;
      case clang::attr::SIMDReduction:
        {
          auto A = static_cast<const SIMDReductionAttr *>(Attrs[i]);
          std::string Op = "";
          switch (A->Operator) {
            case SIMDReductionAttr::max:      Op = "MAX"; break;
            case SIMDReductionAttr::min:      Op = "MIN"; break;
            case SIMDReductionAttr::plus:     Op = "+"; break;
            case SIMDReductionAttr::star:     Op = "*"; break;
            case SIMDReductionAttr::minus:    Op = "-"; break;
            case SIMDReductionAttr::amp:      Op = "&"; break;
            case SIMDReductionAttr::pipe:     Op = "|"; break;
            case SIMDReductionAttr::caret:    Op = "^"; break;
            case SIMDReductionAttr::ampamp:   Op = "&&"; break;
            case SIMDReductionAttr::pipepipe: Op = "||"; break;
          }
          for (SIMDReductionAttr::variables_iterator
               it = A->variables_begin(), end = A->variables_end();
               it != end; ++it) {
            P.addop("REDUCTION").addop(Op);
            LValue LVal = CGF->EmitLValue(*it);
            P.addop("LVALUE").addop(LVal, CGF);
          }
        } break;
      default:
        break;
    }
  }

  auto One = llvm::ConstantInt::get(LoopCount->getType(), 1);
  P.addop("LINEAR").addop(LoopIndex.getPointer()).addop(One);
  CGF->EmitRuntimeCall(CGM->getIntrinsic(llvm::Intrinsic::intel_pragma),
                       P.getops());
}
#endif  // INTEL_SPECIFIC_IL0_BACKEND

llvm::Function *CodeGenFunction::EmitSimdFunction(CGPragmaSimdWrapper &W) {
  const CapturedStmt &CS = *W.getAssociatedStmt();

  CGSIMDForStmtInfo CSInfo(W, LoopStack.getCurLoopID(),
                              LoopStack.getCurLoopParallel());
  CodeGenFunction CGF(CGM, true);
  CGF.CapturedStmtInfo = &CSInfo;
  CGF.disableExceptions();
  llvm::Function *BodyFunction = CGF.GenerateCapturedStmtFunction(CS);
  CGF.enableExceptions();
  // Always inline this function back to the call site.
  BodyFunction->addFnAttr(llvm::Attribute::AlwaysInline);
  return BodyFunction;
}

void CodeGenFunction::EmitPragmaSimd(CodeGenFunction::CGPragmaSimdWrapper &W) {
  RunCleanupsScope SIMDForScope(*this);

  // Emit 'safelen' clause and decide if we want to separate last iteration.
  bool SeparateLastIter = W.emitSafelen(this);

  // Update debug info.
  CGDebugInfo *DI = getDebugInfo();
  if (DI)
    DI->EmitLexicalBlockStart(Builder, W.getForLoc());

  // Emit the for-loop.
  Address LoopIndex = Address::invalid();
  llvm::Value *LoopCount = 0;

  // Emit the loop control variable and cache its initial value and the
  // stride value.
  // Also emit loop index and loop count, depending on stmt.

  W.emitInit(*this, LoopIndex, LoopCount);

  // Only run the SIMD loop if the loop condition is true
  llvm::BasicBlock *ContBlock = createBasicBlock("if.end");
  llvm::BasicBlock *ThenBlock = createBasicBlock("if.then");

  // The following condition is zero trip test to skip last iteration if
  // the loopcount is zero.
  // In the 'omp simd' we may have more than one loop counter due to
  // 'collapse', so we check loopcount instead of loop counter.
  EmitBranchOnBoolExpr(W.getCond(), ThenBlock, ContBlock, 0);
  EmitBlock(ThenBlock);

  // Initialize the captured struct.
  LValue CapStruct = InitCapturedStruct(*W.getAssociatedStmt());

#ifdef INTEL_SPECIFIC_IL0_BACKEND
  // Call intel_pragma intrinsic
  W.emitIntelIntrinsic(this, &CGM, LoopIndex, LoopCount);
#endif  // INTEL_SPECIFIC_IL0_BACKEND

  {
    JumpDest LoopExit = getJumpDestInCurrentScope("for.end");
    RunCleanupsScope ForScope(*this);

    Builder.CreateStore(llvm::ConstantInt::get(LoopCount->getType(), 0),
                        LoopIndex);

    if (SeparateLastIter)
      // Lastprivate or linear variable present, remove last iteration.
      LoopCount = Builder.CreateSub(
          LoopCount, llvm::ConstantInt::get(LoopCount->getType(), 1));

    // Start the loop with a block that tests the condition.
    // If there's an increment, the continue scope will be overwritten
    // later.
    JumpDest Continue = getJumpDestInCurrentScope("for.cond");
    llvm::BasicBlock *CondBlock = Continue.getBlock();
    LoopStack.push(CondBlock);

    EmitBlock(CondBlock);

    llvm::Value *BoolCondVal = 0;
    {
      // If the for statement has a condition scope, emit the local variable
      // declaration.
      llvm::BasicBlock *ExitBlock = LoopExit.getBlock();

      // If there are any cleanups between here and the loop-exit scope,
      // create a block to stage a loop exit along.
      if (ForScope.requiresCleanups())
        ExitBlock = createBasicBlock("for.cond.cleanup");

      // As long as the condition is true, iterate the loop.
      llvm::BasicBlock *ForBody = createBasicBlock("for.body");

      // Use LoopCount and LoopIndex for iteration.
      BoolCondVal = Builder.CreateICmpULT(Builder.CreateLoad(LoopIndex),
                                          LoopCount);

      // C99 6.8.5p2/p4: The first substatement is executed if the expression
      // compares unequal to 0.  The condition must be a scalar type.
      Builder.CreateCondBr(BoolCondVal, ForBody, ExitBlock);

      if (ExitBlock != LoopExit.getBlock()) {
        EmitBlock(ExitBlock);
        EmitBranchThroughCleanup(LoopExit);
      }

      EmitBlock(ForBody);
    }

    Continue = getJumpDestInCurrentScope("for.inc");

    // Store the blocks to use for break and continue.
    BreakContinueStack.push_back(BreakContinue(LoopExit, Continue));

    W.emitIncrement(*this, LoopIndex);

    // Emit the call to the loop body.
    llvm::Function *BodyFunction = EmitSimdFunction(W);
    EmitSIMDForHelperCall(BodyFunction, CapStruct, LoopIndex, false);

    // Emit the increment block.
    EmitBlock(Continue.getBlock());

    {
      llvm::Value *NewLoopIndex =
        Builder.CreateAdd(Builder.CreateLoad(LoopIndex),
                          llvm::ConstantInt::get(LoopCount->getType(), 1));
      Builder.CreateStore(NewLoopIndex, LoopIndex);
    }

    BreakContinueStack.pop_back();

    EmitBranch(CondBlock);

    ForScope.ForceCleanup();

    if (DI)
      DI->EmitLexicalBlockEnd(Builder, W.getSourceRange().getEnd());

    LoopStack.pop();

    // Emit the fall-through block.
    EmitBlock(LoopExit.getBlock(), true);

#ifdef INTEL_SPECIFIC_IL0_BACKEND
    {
      // Mark the end of #pragma simd - it's required for IL0 backend.
      IntelPragmaBuilder P(CGM.getLLVMContext());
      P.addop("SIMD_END_LOOP");
      EmitRuntimeCall(CGM.getIntrinsic(llvm::Intrinsic::intel_pragma),
                      P.getops());
    }
#endif // INTEL_SPECIFIC_IL0_BACKEND

    // Increment again, for last iteration.
    W.emitIncrement(*this, LoopIndex);

    if (SeparateLastIter) {
      // This helper call makes updates to linear or lastprivate variables.
      // In the case of openmp, only for lastprivate ones.
      EmitSIMDForHelperCall(BodyFunction, CapStruct, LoopIndex, true);
    }

    W.emitLinearFinal(*this);
  }

  EmitBlock(ContBlock, true);
}

void CodeGenFunction::EmitSIMDForStmt(const SIMDForStmt &S) {
  RunCleanupsScope ExecutedScope(*this);
  CGPragmaSimd Wrapper(&S);
  EmitPragmaSimd(Wrapper);
}

static Expr *GetLinearStep(const SIMDForStmt &SS, VarDecl *SIMDVar) {
  // Get the linear step from the SIMD Attributes list
  ArrayRef<Attr *> Attrs = SS.getSIMDAttrs();
  for (ArrayRef<Attr *>::iterator I = Attrs.begin(), E = Attrs.end();
       I != E; ++I) {
    Attr *A = *I;
    if (SIMDLinearAttr *LA = dyn_cast<SIMDLinearAttr>(A))
      // This is a Linear clause, iterate over its variables and steps.
      for (SIMDLinearAttr::linear_iterator V = LA->vars_begin(),
                                           S = LA->steps_begin(),
                                           SE = LA->steps_end();
           S != SE; ++S, ++V) {
        assert(isa<DeclRefExpr>(*V) && "Linear variable must be a DeclRefExpr");
        if (VarDecl *VD = cast<VarDecl>(cast<DeclRefExpr>(*V)->getDecl()))
          if (VD == SIMDVar)
            return *S;
      }
  }
  return 0;
}

static void EmitUpdateExpr(CodeGenFunction &CGF, const Expr *UpdateExpr,
                           QualType T, ArrayRef<VarDecl *> ArrayIndexes,
                           unsigned Index) {
  // Allocate (uninitialized) index variables for the first level.
  if (Index == 0)
    for (unsigned I = 0, N = ArrayIndexes.size(); I < N; ++I)
      CGF.EmitAutoVarDecl(*ArrayIndexes[I]);

  // Emit the update expression for the inner most level. If no loop is needed
  // then the update expression is emitted in the following block.
  if (Index == ArrayIndexes.size()) {
    CodeGenFunction::RunCleanupsScope Scope(CGF);
    CGF.EmitIgnoredExpr(UpdateExpr);
    return;
  }

  // Emit a loop for the current level of the array.
  const ConstantArrayType *Array = CGF.getContext().getAsConstantArrayType(T);
  assert(Array && "array update without the array type");
  auto IndexVar = CGF.GetAddrOfLocalVar(ArrayIndexes[Index]);
  assert(IndexVar.getPointer() && "Array index variable not loaded");

  // Initialize this index variable to zero.
  llvm::Value *Zero = llvm::Constant::getNullValue(CGF.SizeTy);
  CGF.Builder.CreateStore(Zero, IndexVar);

  llvm::BasicBlock *ForCond = CGF.createBasicBlock("for.cond");
  llvm::BasicBlock *ForBody = CGF.createBasicBlock("for.body");
  llvm::BasicBlock *ForInc = CGF.createBasicBlock("for.inc");
  llvm::BasicBlock *ForEnd = CGF.createBasicBlock("for.end");

  // Emit the condition.
  llvm::Value *Counter = 0;
  {
    CGF.EmitBlock(ForCond);

    Counter = CGF.Builder.CreateLoad(IndexVar);
    uint64_t NumElements = Array->getSize().getZExtValue();
    llvm::Value *NumElementsPtr = llvm::ConstantInt::get(Counter->getType(),
                                                         NumElements);
    llvm::Value *LessThan = CGF.Builder.CreateICmpULT(Counter, NumElementsPtr);
    CGF.Builder.CreateCondBr(LessThan, ForBody, ForEnd);
  }

  // Emit the body.
  CGF.EmitBlock(ForBody);
  {
    CodeGenFunction::RunCleanupsScope Cleanups(CGF);

    // Recurse to emit the inner loop.
    EmitUpdateExpr(CGF, UpdateExpr, Array->getElementType(), ArrayIndexes,
                   Index + 1);
  }

  // Emit the increment.
  {
    CGF.EmitBlock(ForInc);

    llvm::Value *NextVal = llvm::ConstantInt::get(Counter->getType(), 1);
    Counter = CGF.Builder.CreateLoad(IndexVar);
    NextVal = CGF.Builder.CreateAdd(Counter, NextVal, "inc");
    CGF.Builder.CreateStore(NextVal, IndexVar);

    // Branch back up to the next iteration.
    CGF.EmitBranch(ForCond);
  }

  // Emit the fall-through block.
  CGF.EmitBlock(ForEnd, true);
}

// Walker for '#pragma simd'
bool CodeGenFunction::CGPragmaSimd::walkLocalVariablesToEmit(
    CodeGenFunction *CGF, CGSIMDForStmtInfo *Info) const {
  const SIMDForStmt &SS = *(cast<SIMDForStmt>(Info->getStmt()));
  const CapturedDecl *CD = SS.getBody()->getCapturedDecl();
  ImplicitParamDecl *IPD =  CD->getParam(1);
  bool isSignedLoopIndex = IPD->getType()->hasSignedIntegerRepresentation();
  auto LoopIndex = CGF->LocalDeclMap.find(IPD)->second;
  bool RequiresUpdate = false;

  // Emit all SIMD local variables and update the codegen info.
  for (SIMDForStmt::simd_var_iterator I = SS.simd_var_begin(),
      E = SS.simd_var_end(); I != E; ++I) {
    if (!I->isPrivate() && !I->isFirstPrivate() && !I->isLastPrivate() &&
        !I->isLinear())
      continue;

    VarDecl *SIMDVar = I->getSIMDVar();
    VarDecl *LocalVar = I->getLocalVar();

    if (CGF->LocalDeclMap.count(LocalVar) > 0)
      CGF->LocalDeclMap.erase(LocalVar);
    CGF->EmitAutoVarDecl(*LocalVar);

    auto Addr = CGF->GetAddrOfLocalVar(LocalVar);
    assert(Addr.getPointer() && "null address");
    Info->updateLocalAddr(SIMDVar, Addr);

    if (I->isLinear()) {
      const FieldDecl *FD = Info->lookup(SIMDVar);
      assert(FD && "must have been captured");
      QualType TagType = CGF->getContext().getTagDeclType(FD->getParent());
      LValue LV = CGF->MakeNaturalAlignAddrLValue(
                          Info->getContextValue(), TagType);
      LV = CGF->EmitLValueForField(LV, FD);

      llvm::Value *Start = CGF->Builder.CreateLoad(LV.getAddress());
      llvm::Value *Index = CGF->Builder.CreateLoad(LoopIndex);
      llvm::Value *Result = 0;
      Expr *StepExpr = GetLinearStep(SS, SIMDVar);
      bool isSigned = isSignedLoopIndex; // by default we use sign of LoopIndex
      if (StepExpr) {
        DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(StepExpr);
        VarDecl* VD = DRE ? cast<VarDecl>(DRE->getDecl()) : nullptr;
        if (VD && Info->lookup(VD)){
          DeclRefExpr Step (VD,
                            /*RefersToEnclosingVariableOrCapture=*/true,
                            VD->getType().getNonReferenceType(),
                            VK_LValue, SourceLocation());
          Result = CGF->EmitAnyExpr(&Step).getScalarVal();
        }
        else
          Result = CGF->EmitAnyExpr(StepExpr).getScalarVal();
        isSigned = StepExpr->getType()->hasSignedIntegerRepresentation();
      }
      else
        Result = llvm::ConstantInt::get(Index->getType(), 1);
      // We could have different size of operands (Index*Result) here
      Result = CGF->Builder.CreateIntCast(Result, Index->getType(), isSigned);
      Result = CGF->Builder.CreateMul(Index, Result);
      if (LocalVar->getType()->isPointerType())
        Result = CGF->Builder.CreateGEP(Start, Result);
      else if (LocalVar->getType()->isRealFloatingType()) {
        if(isSigned)
          Result = CGF->Builder.CreateSIToFP(Result, Start->getType());
        else
          Result = CGF->Builder.CreateUIToFP(Result, Start->getType());
        Result = CGF->Builder.CreateFAdd(Start, Result);
      } else {
        Result = CGF->Builder.CreateIntCast(Result, Start->getType(), isSignedLoopIndex);
        Result = CGF->Builder.CreateAdd(Start, Result);
      }
      CGF->Builder.CreateStore(Result, Addr);
    }
    RequiresUpdate |= I->isLinear() || I->isLastPrivate();
  }

  return RequiresUpdate;
}

void CodeGenFunction::EmitSIMDForHelperBody(const Stmt *S) {
  assert(CapturedStmtInfo && "Should be only called inside a CapturedStmt");
  CGSIMDForStmtInfo *Info = cast<CGSIMDForStmtInfo>(CapturedStmtInfo);

  // Mark the loop body as an extended region of this SIMD loop.
  LoopStack.push(Info->getLoopID(), Info->getLoopParallel());
  {
    RunCleanupsScope Scope(*this);

    // Emit all SIMD local variables and update the codegen info.
    bool RequiresUpdate = Info->walkLocalVariablesToEmit(this);

    // Emit the SIMD for loop body.
    {
      RunCleanupsScope BodyScope(*this);
      // It is not allowed to have return or break in a SIMD loop body.
      // Continue statements are allowed and updates to the data
      // privatization variables will be emitted in a unified continue block.
      JumpDest LoopContinue = getJumpDestInCurrentScope("for.continue");
      BreakContinueStack.push_back(BreakContinue(JumpDest(), LoopContinue));

      EmitStmt(S);

      EmitBlock(LoopContinue.getBlock());

      // If an update is required, emit those update expressions to be run on
      // the last iteration of the loop.
      //
      // if (IsLastIter) {
      //   [[ Update Expressions ]]
      // }
      //
      // IsLastIter will only be true if this is a second output of the helper
      // body, after the intial for-loop.
      // Since IsLastIter is constant, it will be optimized out, and the if
      // statement will not be a part of the SIMD For loop, thus allowing
      // vectorization.
      if (RequiresUpdate) {
        const SIMDForStmt *SimdFor = cast<SIMDForStmt>(Info->getStmt());
        const CapturedDecl *CD = SimdFor->getBody()->getCapturedDecl();
        auto IsLastIter = LocalDeclMap.find(CD->getParam(2))->second;
        RunCleanupsScope UpdateScope(*this);
        llvm::BasicBlock *UpdateBody = createBasicBlock("update.body");
        llvm::BasicBlock *UpdateExit = createBasicBlock("update.exit");

        auto *Compare = Builder.CreateIsNotNull(Builder.CreateLoad(IsLastIter));
        Builder.CreateCondBr(Compare, UpdateBody, UpdateExit);

        EmitBlock(UpdateBody);
        // Update expressions update a SIMD variable, do not replace those uses
        // with the local copy's address.
        //
        Info->setShouldReplaceWithLocal(false);
        for (SIMDForStmt::simd_var_iterator I = SimdFor->simd_var_begin(),
            E = SimdFor->simd_var_end(); I != E; ++I) {
          // Emit the update for this variable. For lastprivate variables,
          // loops may be needed to complete the update.
          if (I->isLinear() || I->isLastPrivate()) {
            EmitUpdateExpr(*this, I->getUpdateExpr(),
                I->getLocalVar()->getType(),
                I->getArrayIndexVars(), 0);
          }
        }
        Info->setShouldReplaceWithLocal(true);
        EmitBlock(UpdateExit);
      }
      BreakContinueStack.pop_back();
    }
  }

  // Leave the loop body.
  LoopStack.pop();
}

namespace {
class NonCEANExprEmitter : public StmtVisitor<NonCEANExprEmitter, void> {
  CodeGenFunction &CGF;
  llvm::DenseMap<Expr *, Address> ExprTemps;
public:
  void VisitCEANIndexExpr(CEANIndexExpr *E) { }
  void VisitDeclRefExpr(DeclRefExpr *E) { }
  void VisitExpr(Expr *E) {
    QualType OrigTy = E->getType();
    QualType RefTy = E->isGLValue() ?
                            CGF.getContext().getLValueReferenceType(OrigTy) :
                            CGF.getContext().getRValueReferenceType(OrigTy);
    auto Temp = CGF.CreateMemTemp(RefTy);
    RValue RVal = CGF.EmitReferenceBindingToExpr(E);
    LValue LVal = CGF.MakeAddrLValue(Temp, RefTy);
    CGF.EmitStoreThroughLValue(RVal, LVal, false);
    ExprTemps.insert(std::make_pair(E, Temp));
  }
  void VisitStmt(Stmt *S) {
    for (Stmt::child_iterator I = S->child_begin(), E = S->child_end();
         I != E; ++I) {
      if (*I) Visit(*I);
    }
  }
  NonCEANExprEmitter(CodeGenFunction &CGF) : CGF(CGF) { }
};
}

static void EmitRecursiveCilkRankedStmt(CodeGenFunction &CGF,
                                        const CilkRankedStmt &S,
                                        unsigned Rank) {
  if (Rank < S.getRank()) {
    const DeclStmt *DS = cast<DeclStmt>(S.getVars()[Rank]);
    CGF.EmitStmt(DS);
    // Generate code for loop.
    const Expr *Length = S.getLengths()[Rank];
    bool isSigned = !Length->getType()->hasUnsignedIntegerRepresentation();
    llvm::Value *ValLength = CGF.EmitScalarExpr(Length);
    llvm::BasicBlock *CondBlock = CGF.createBasicBlock("cean.loop.cond");
    llvm::BasicBlock *MainLoop = CGF.createBasicBlock("cean.loop.body");
    llvm::BasicBlock *ExitLoop = CGF.createBasicBlock("cean.loop.exit");
    if (llvm::ConstantInt *Val = dyn_cast<llvm::ConstantInt>(ValLength)) {
      isSigned = Val->isZero() || Val->isNegative();
    }
    if (isSigned) {
      llvm::BasicBlock *EnterLoop = CGF.createBasicBlock("cean.loop.enter");
      CGF.Builder.CreateCondBr(
         CGF.Builder.CreateICmpSGT(
                         ValLength,
                         llvm::ConstantInt::getSigned(ValLength->getType(), 0)),
         EnterLoop, ExitLoop);
      CGF.EmitBlock(EnterLoop);
    }
    if (Rank == S.getRank() - 1) {
      Stmt::const_child_range Ch = S.getInits()->children();
      for (auto I = Ch.begin(), E = Ch.end(); I != E; ++I)
        CGF.EmitStmt(*I);
    }
#ifdef INTEL_SPECIFIC_IL0_BACKEND
    // Generate intel intrinsic.
    if (Rank == S.getRank() - 1) {
      IntelPragmaBuilder P(CGF.CGM.getLLVMContext());
      P.addop("CEAN_LOOP");
      CGF.EmitRuntimeCall(CGF.CGM.getIntrinsic(llvm::Intrinsic::intel_pragma),
                          P.getops());
    }
#endif  // INTEL_SPECIFIC_IL0_BACKEND
    CGF.EmitBranch(CondBlock);
    CGF.EmitBlock(CondBlock);
    CGF.LoopStack.setParallel();
    CGF.LoopStack.setVectorizeEnable(true);
    CGF.LoopStack.push(CondBlock);
    const VarDecl *VD = cast<VarDecl>(DS->getSingleDecl());
    CGF.Builder.CreateCondBr(
     CGF.Builder.CreateICmpNE(
                 CGF.Builder.CreateLoad(CGF.GetAddrOfLocalVar(VD)),
                 ValLength),
     MainLoop, ExitLoop);
    CGF.EmitBlock(MainLoop);
    EmitRecursiveCilkRankedStmt(CGF, S, Rank + 1);
    CGF.EmitStmt(S.getIncrements()[Rank]);
    CGF.EmitBranch(CondBlock);
    CGF.LoopStack.pop();
    CGF.EmitBlock(ExitLoop, true);
  } else {
    CodeGenFunction::RunCleanupsScope BodyScope(CGF);
    CGF.EmitStmt(S.getAssociatedStmt());
  }
}

void CodeGenFunction::EmitCilkRankedStmt(const CilkRankedStmt &S) {
  CodeGenFunction::LocalVarsDeclGuard Guard(*this);
  EmitRecursiveCilkRankedStmt(*this, S, 0);
}
#endif // INTEL_SPECIFIC_CILKPLUS

#ifdef INTEL_SPECIFIC_IL0_BACKEND
static StringRef CreateStringRef(const char *Str) {
  char *Data = new char[ ::strlen(Str) + 1];
  ::strcpy(Data, Str);
  return StringRef(Data);
}

static Expr *CreateStringExpr(const char *Str, ASTContext &Context,
                              SourceLocation Loc = SourceLocation()) {
  StringRef Par = CreateStringRef(Str);
  QualType Type = Context.getConstantArrayType(
      Context.CharTy, llvm::APInt(32, Par.size() + 1), ArrayType::Normal, 0);
  return (StringLiteral::Create(Context, Par, StringLiteral::Ascii, false, Type,
                                Loc));
}

static Expr *CreateDeclRefExpr(ValueDecl *Val, ASTContext &Context,
                               bool RefToEnclosing,
                               SourceLocation Loc = SourceLocation()) {
  return (DeclRefExpr::Create(Context, NestedNameSpecifierLoc(),
                              SourceLocation(), Val, RefToEnclosing,
                              SourceLocation(), Val->getType(), VK_LValue));
}

#include "llvm/ADT/DenseSet.h"
static llvm::DenseSet<llvm::BasicBlock *> VisitedBB;
static bool IsBBPredecessorForBB(llvm::BasicBlock *Pred,
                                 llvm::BasicBlock *Succ) {
  if (VisitedBB.find(Pred) != VisitedBB.end())
    return (false);
  VisitedBB.insert(Pred);
  if (Pred == Succ)
    return (true);
  llvm::TerminatorInst *TI = Pred->getTerminator();
  if (!TI)
    return (false);
  for (unsigned i = 0; i < TI->getNumSuccessors(); ++i) {
    if (IsBBPredecessorForBB(TI->getSuccessor(i), Succ))
      return (true);
  }
  return (false);
}

llvm::BasicBlock *
CodeGenFunction::CreateIPForInlineEnd(llvm::BasicBlock *InlineBB) {
  VisitedBB.insert(InlineBB);
  llvm::TerminatorInst *TI = InlineBB->getTerminator();
  if (!TI) {
    // Correct insertion point is set already
    return (NULL);
  }
  if (TI->getNumSuccessors() == 0) {
    // Set insertion point before TerminatorInst
    Builder.SetInsertPoint(TI);
    return (NULL);
  }
  llvm::BasicBlock *SuccBB = TI->getSuccessor(0);
  if (!SuccBB->getTerminator() || !SuccBB->getSinglePredecessor() ||
      VisitedBB.find(SuccBB) != VisitedBB.end()) {
    // Next BasicBlock does not have terminator or have several predecessors -
    // insert the new one and return address of the previous
    Builder.SetInsertPoint(InlineBB);
    llvm::BasicBlock *BB = createBasicBlock("intel.pragma.block");
    EmitBlock(BB);
    TI->setSuccessor(0, BB);
    return (SuccBB);
  }
  return (this->CreateIPForInlineEnd(SuccBB));
}

static llvm::SmallVector<llvm::BasicBlock *, 4> PreviousInlineBB;
void CodeGenFunction::EmitPragmaStmt(const PragmaStmt &S) {
  switch (S.getPragmaKind()) {
  case (IntelPragma_SPECCALL):
    if (S.getAttribs().size() > 0) {
      if (isa<CXXConstructExpr>(S.getAttribs()[0].Value)) {
        const CXXConstructExpr *CCE =
            cast<CXXConstructExpr>(S.getAttribs()[0].Value);
        const CXXConstructorDecl *CCD = CCE->getConstructor();
        RValue This = EmitAnyExprToTemp(S.getAttribs()[1].Value);
        EmitCXXConstructorCall(CCD, Ctor_Complete, false, false,
                               Address(This.getScalarVal(), getPointerAlign()),
                               CCE);
      } else if (isa<CXXMemberCallExpr>(S.getAttribs()[0].Value)) {
        const CXXMemberCallExpr *CMCE =
            cast<CXXMemberCallExpr>(S.getAttribs()[0].Value);
        if (isa<CXXDestructorDecl>(CMCE->getMethodDecl())) {
          const CXXDestructorDecl *CDD =
              cast<CXXDestructorDecl>(CMCE->getMethodDecl());
          RValue This = EmitAnyExprToTemp(S.getAttribs()[1].Value);
          EmitCXXDestructorCall(
              CDD, Dtor_Complete, false, false,
              Address(This.getScalarVal(), getPointerAlign()));
        } else {
          ReturnValueSlot RVS;
          EmitCXXMemberCallExpr(
              cast<CXXMemberCallExpr>(S.getAttribs()[0].Value), RVS);
        }
      } else {
        assert("Only destructor or constructor is allowed");
      }
    }
    break;
  case (IntelPragma_SPECCALLAGG):
    if (S.getAttribs().size() > 0) {
      DeclRefExpr *DRE;
      if (isa<CXXConstructExpr>(S.getAttribs()[0].Value)) {
        const CXXConstructExpr *CCE =
            cast<CXXConstructExpr>(S.getAttribs()[0].Value);
        const CXXConstructorDecl *CCD = CCE->getConstructor();
        RValue NumElements = EmitAnyExprToTemp(S.getAttribs()[2].Value);
        RValue ArrayPtr = EmitAnyExprToTemp(S.getAttribs()[1].Value);
        EmitCXXAggrConstructorCall(
            CCD, NumElements.getScalarVal(),
            Address(ArrayPtr.getScalarVal(), getPointerAlign()), CCE);
      } else if (isa<DeclRefExpr>(S.getAttribs()[0].Value)) {
        DRE = cast<DeclRefExpr>(S.getAttribs()[0].Value);
        if (isa<CXXDestructorDecl>(DRE->getDecl())) {
          LValue LV = EmitLValue(S.getAttribs()[1].Value);
          EmitCXXDestructorCall(cast<CXXDestructorDecl>(DRE->getDecl()),
                                Dtor_Complete, false, false, LV.getAddress());
        } else {
          assert("Only destructor or constructor is allowed");
        }
      } else {
        assert("Only destructor or constructor is allowed");
      }
    }
    break;
  case (IntelPragmaFloatControl):
    if (CurFn) {
      std::string Res;
      // if (S.getAttribs()[0].ExprKind == IntelPragmaExprConst &&
      // isa<StringLiteral>(S.getAttribs()[0].Value)) {
      //  Res +=
      //  cast<StringLiteral>(S.getAttribs()[0].Value)->getString().str();
      //}
      for (size_t i = 1; i < S.getAttribs().size(); ++i) {
        if (S.getAttribs()[i].ExprKind == IntelPragmaExprConst &&
            isa<StringLiteral>(S.getAttribs()[i].Value)) {
          Res = "INTEL:" +
                cast<StringLiteral>(S.getAttribs()[i].Value)->getString().str();
          ++i;
          if (S.getAttribs()[i].ExprKind == IntelPragmaExprConst &&
              isa<StringLiteral>(S.getAttribs()[i].Value)) {
            Res +=
                "-" +
                cast<StringLiteral>(S.getAttribs()[i].Value)->getString().str();
            CurFn->addFnAttr(CreateStringRef(Res.data()));
          }
        }
      }
    }
    break;
  case (IntelPragmaKindCommonOnOff):
    if (CurFn) {
      std::string Res;
      if (S.getAttribs()[0].ExprKind == IntelPragmaExprConst &&
          isa<StringLiteral>(S.getAttribs()[0].Value)) {
        Res = cast<StringLiteral>(S.getAttribs()[0].Value)->getString().str();
        if (Res == "FP_CONTRACT" || Res == "FENV_ACCESS") {
          Res = "INTEL:" + Res;
          for (size_t i = 1; i < S.getAttribs().size(); ++i) {
            if (S.getAttribs()[i].ExprKind == IntelPragmaExprConst &&
                isa<StringLiteral>(S.getAttribs()[i].Value)) {
              Res += "-" +
                     cast<StringLiteral>(S.getAttribs()[i].Value)
                         ->getString()
                         .str();
            }
          }
          CurFn->addFnAttr(CreateStringRef(Res.data()));
          break;
        }
      }
    }
  case (IntelAssumeAligned):
  case (IntelCommonAttribute):
  case (IntelPragmaIvdep):
  case (IntelPragmaNoVector):
  case (IntelPragmaDistribute):
  case (IntelPragmaInline):
  case (IntelPragmaInlineEnd):
  case (IntelPragmaLoopCount):
  case (IntelPragmaOptimize):
  case (IntelPragmaOptimizationLevel):
  case (IntelPragmaOptimizationLevelGCC):
  case (IntelPragmaNoParallel):
  case (IntelPragmaUnroll):
  case (IntelPragmaUnrollAndJam):
  case (IntelPragmaNoFusion):
  case (IntelPragmaVector):
  case (IntelPragmaOptimizationParameter):
  case (IntelPragmaParallel):
  case (IntelPragmaAllocSection):
  // alloc_text and code_seg, bss)seg, data_seg and const_seg does not require
  // codegen actions
  // They are generated as attributes on objects
  // case (IntelPragmaAllocText):
  // case (IntelPragmaBCCDSeg):
  case (IntelPragmaSection):
  case (IntelPragmaAutoInline):
  case (IntelPragmaCheckStack):
  case (IntelPragmaInitSeg): {
    assert(S.getAttribs().size() > 0 && "At least one item must be defined");
    assert(isa<StringLiteral>(S.getAttribs()[0].Value) &&
           "The first must be a string");
    IntelPragmaBuilder P(CGM.getLLVMContext());
    for (size_t i = 0; i < S.getAttribs().size(); ++i) {
      if (S.getAttribs()[i].ExprKind == IntelPragmaExprConst &&
          isa<StringLiteral>(S.getAttribs()[i].Value)) {
        P.addop(cast<StringLiteral>(S.getAttribs()[i].Value)->getString());
      } else if (S.getAttribs()[i].ExprKind == IntelPragmaExprConst) {
        auto res = CGM.EmitConstantExpr(S.getAttribs()[i].Value,
                                        S.getAttribs()[i].Value->getType());
        P.addop("CONSTANT").addop(res);
      } else if (S.getAttribs()[i].ExprKind == IntelPragmaExprRValue) {
        DeclRefExpr *DRE;
        if (isa<DeclRefExpr>(S.getAttribs()[i].Value) &&
            (DRE = cast<DeclRefExpr>(S.getAttribs()[i].Value)) &&
            isa<FunctionDecl>(DRE->getDecl())) {
          //P.addop("FUNCTION_REFERENCE");
          P.addop(CGM.GetAddrOfGlobal(cast<FunctionDecl>(DRE->getDecl())));
        } else {
          RValue RVal = EmitAnyExprToTemp(S.getAttribs()[i].Value);
          P.addop("RVALUE").addop(RVal);
        }
      } else {
        LValue LVal = EmitLValue(S.getAttribs()[i].Value);
        P.addop("LVALUE").addop(LVal, this);
      }
    }
    assert(!(S.getPragmaKind() == IntelPragmaInlineEnd &&
             PreviousInlineBB.empty()) &&
           "No previous INLINE for INLINEEND is found!");
    CGBuilderTy::InsertPoint SaveIP;
    llvm::BasicBlock *SpecBasicBlock = NULL;
    bool NeedToFindIP = false;
    // Special processing for pragma inline
    VisitedBB.clear();
    if (S.getPragmaKind() == IntelPragmaInlineEnd && PreviousInlineBB.back() &&
        (NeedToFindIP = !IsBBPredecessorForBB(PreviousInlineBB.back(),
                                              Builder.GetInsertBlock()))) {
      SaveIP = Builder.saveIP();
      VisitedBB.clear();
      SpecBasicBlock = this->CreateIPForInlineEnd(PreviousInlineBB.back());
    }
    Builder.CreateCall(CGM.getIntrinsic(llvm::Intrinsic::intel_pragma),
                       P.getops());
    // Special processing for pragma inline
    if (S.getPragmaKind() == IntelPragmaInline) {
      PreviousInlineBB.push_back(Builder.GetInsertBlock());
    } else if (S.getPragmaKind() == IntelPragmaInlineEnd) {
      PreviousInlineBB.pop_back();
      if (NeedToFindIP) {
        if (SpecBasicBlock) {
          EmitBranch(SpecBasicBlock);
        }
        Builder.restoreIP(SaveIP);
      }
    }
  } break;
  default:
    break;
  }
}

// Pass the type inside the sizeof expression \a QTy to the IL0 backend.
// LLVM backend needs only the constant size value \a C; IL0 uses this for
// early optimizations (mostly dtrans) and kfolds it out later.
llvm::Value *CodeGenFunction::EmitIntelSizeof(QualType QTy, llvm::Value *C) {
  llvm::Type *Ty = CGM.getTypes().ConvertType(QTy);
  if (!getLangOpts().IntelCompat || Ty == nullptr || Ty->isFunctionTy()) {
    return C;
  }
  // Sizeof may be represented as: (i64) gep (Ty*)null, 1
  // But if we generate a ptrtoint above GEP, the type we need is kfolded out.
  // Therefore we pass both GEP and size to llvm_fe through an intrinsic call.
  auto GEP = llvm::ConstantExpr::getGetElementPtr(
      Ty, llvm::ConstantExpr::getNullValue(llvm::PointerType::getUnqual(Ty)),
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ty->getContext()), 1));
  llvm::Value *Ops[2] = { C, GEP };
  llvm::Value *Callee =
      CGM.getIntrinsic(llvm::Intrinsic::intel_sizeof, C->getType());
  return Builder.CreateCall(Callee, Ops);
}

void CodeGenFunction::EmitPragmaDecl(const PragmaDecl &D) {
  switch (D.getStmt()->getPragmaKind()) {
  // case (IntelPragmaOptimize):
  // case (IntelPragmaOptimizationLevel):
  // case (IntelPragmaOptimizationLevelGCC):
  // case (IntelPragmaOptimizationParameter):
  case (IntelPragmaAllocSection):
  case (IntelPragmaSection):
  // case (IntelPragmaAutoInline):
  // alloc_text and code_seg, bss_seg, data_seg and const_seg does not require
  // codegen actions
  // They are generated as attributes on objects
  // case (IntelPragmaAllocText):
  // case (IntelPragmaBCCDSeg):
  // case (IntelPragmaCheckStack):
  case (IntelPragmaInitSeg):
  // case (IntelPragmaFloatControl):
  // case (IntelPragmaKindCommonOnOff):
  case (IntelCommonAttribute): {
    const CGFunctionInfo &FuncInfo = CGM.getTypes().arrangeNullaryFunction();
    llvm::FunctionType *FNType = CGM.getTypes().GetFunctionType(FuncInfo);
    llvm::Function *FN = llvm::Function::Create(
        FNType, llvm::GlobalValue::PrivateLinkage,
        CreateStringRef(".DIRECTIVE."), &CGM.getModule());
    FN->setDoesNotAccessMemory();
    FN->setDoesNotReturn();
    FN->setDoesNotThrow();
    FunctionArgList Args;
    StartFunction(GlobalDecl(), getContext().VoidTy, FN, FuncInfo, Args);
    EmitPragmaStmt(cast<PragmaStmt>(*D.getStmt()));
    FinishFunction();
  } break;
  default:
    break;
  }
}

void CodeGenFunction::EmitIntelAttribute(const Decl &D) {
  if (D.hasAttr<AvoidFalseShareAttr>()) {
    ASTContext &Context = CGM.getContext();
    PragmaStmt *stmt = new (Context) PragmaStmt(SourceLocation());
    (stmt->getAttribs())
        .push_back(IntelPragmaAttrib(
            CreateStringExpr("AVOID_FALSE_SHARE", Context, SourceLocation()),
            IntelPragmaExprConst));
    bool refersToEnclosingScope = D.getDeclContext()->isFunctionOrMethod();
    (stmt->getAttribs())
        .push_back(IntelPragmaAttrib(
            CreateDeclRefExpr(cast<ValueDecl>(const_cast<Decl *>(&D)), Context,
                              refersToEnclosingScope),
            IntelPragmaExprLValue));
    AvoidFalseShareAttr *AFSA = D.getAttr<AvoidFalseShareAttr>();
    StringRef Str = AFSA->getExceptionId();
    if (!Str.empty()) {
      (stmt->getAttribs())
          .push_back(IntelPragmaAttrib(
              CreateStringExpr("EXCEPTION_ID", Context, SourceLocation()),
              IntelPragmaExprConst));
      (stmt->getAttribs())
          .push_back(IntelPragmaAttrib(
              CreateStringExpr(Str.data(), Context, SourceLocation()),
              IntelPragmaExprConst));
    }
    stmt->setPragmaKind(IntelCommonAttribute);

    if (D.getDeclContext()->isFunctionOrMethod()) {
      EmitPragmaStmt(*stmt);
    } else {
      PragmaDecl *decl =
          PragmaDecl::Create(Context, const_cast<Decl *>(&D)->getDeclContext(),
                             stmt->getSemiLoc());

      stmt->setDecl();
      decl->setStmt(stmt);

      // D->getDeclContext()->addDecl(decl);
      EmitPragmaDecl(*decl);
    }
  }
}
#endif // INTEL_SPECIFIC_IL0_BACKEND
