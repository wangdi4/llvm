//==--- SemaIntelImpl.h - Definitions of Intel Sema templates. -*- C++ -*---==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

using namespace clang;

template <typename AttrType>
bool Sema::checkRangedIntegralArgument(Expr *E, const AttrType *TmpAttr,
                                       ExprResult &Result) {
  llvm::APSInt Value;
  Result = VerifyIntegerConstantExpression(E, &Value);
  if (Result.isInvalid())
    return true;

  if (Value < AttrType::getMinValue() || Value > AttrType::getMaxValue()) {
    Diag(TmpAttr->getRange().getBegin(),
         diag::err_attribute_argument_out_of_range)
        << TmpAttr << AttrType::getMinValue() << AttrType::getMaxValue()
        << E->getSourceRange();
    return true;
  }
  return false;
}

template <typename AttrType>
void Sema::AddOneConstantValueAttr(SourceRange AttrRange, Decl *D, Expr *E,
                                   unsigned SpellingListIndex) {
  AttrType TmpAttr(AttrRange, Context, E, SpellingListIndex);

  if (!E->isValueDependent()) {
    ExprResult ICE;
    if (checkRangedIntegralArgument<AttrType>(E, &TmpAttr, ICE))
      return;
    E = ICE.get();
  }

  if (NumReadPortsAttr::classof(&TmpAttr) ||
      NumWritePortsAttr::classof(&TmpAttr) ||
      (MaxConcurrencyAttr::classof(&TmpAttr) && isa<VarDecl>(D))) {
    if (!D->hasAttr<MemoryAttr>())
      D->addAttr(MemoryAttr::CreateImplicit(Context, MemoryAttr::Default));
  }

  D->addAttr(::new (Context)
                 AttrType(AttrRange, Context, E, SpellingListIndex));
}

template <typename AttrType>
void Sema::AddOneConstantPowerTwoValueAttr(SourceRange AttrRange, Decl *D,
                                           Expr *E,
                                           unsigned SpellingListIndex) {
  AttrType TmpAttr(AttrRange, Context, E, SpellingListIndex);

  if (!E->isValueDependent()) {
    ExprResult ICE;
    if (checkRangedIntegralArgument<AttrType>(E, &TmpAttr, ICE))
      return;
    Expr::EvalResult Result;
    E->EvaluateAsInt(Result, Context);
    llvm::APSInt Value = Result.Val.getInt();
    if (!Value.isPowerOf2()) {
      Diag(AttrRange.getBegin(), diag::err_attribute_argument_not_power_of_two)
          << &TmpAttr;
      return;
    }
    if (NumBanksAttr::classof(&TmpAttr)) {
      if (auto *BBA = D->getAttr<BankBitsAttr>()) {
        unsigned NumBankBits = BBA->args_size();
        if (NumBankBits != Value.ceilLogBase2()) {
          Diag(AttrRange.getBegin(), diag::err_bankbits_numbanks_conflicting);
          return;
        }
      }
    }
    E = ICE.get();
  }

  if (!D->hasAttr<MemoryAttr>())
    D->addAttr(MemoryAttr::CreateImplicit(Context, MemoryAttr::Default));

  // We are adding a user NumBanks, drop any implicit default.
  if (NumBanksAttr::classof(&TmpAttr)) {
    if (auto *NBA = D->getAttr<NumBanksAttr>())
      if (NBA->isImplicit())
        D->dropAttr<NumBanksAttr>();
  }

  D->addAttr(::new (Context)
                 AttrType(AttrRange, Context, E, SpellingListIndex));
}
