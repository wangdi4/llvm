//==--- SemaIntelImpl.h - Definitions of Intel Sema templates. -*- C++ -*---==//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

using namespace clang;

template <typename AttrType>
void Sema::HLSAddOneConstantValueAttr(SourceRange AttrRange, Decl *D, Expr *E,
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
      MaxReplicatesAttr::classof(&TmpAttr) ||
      (MaxConcurrencyAttr::classof(&TmpAttr) && isa<VarDecl>(D))) {
    if (!D->hasAttr<IntelFPGAMemoryAttr>())
      D->addAttr(IntelFPGAMemoryAttr::CreateImplicit(
          Context, IntelFPGAMemoryAttr::Default));
  }

  D->addAttr(::new (Context)
                 AttrType(AttrRange, Context, E, SpellingListIndex));
}

template <typename AttrType>
void Sema::HLSAddOneConstantPowerTwoValueAttr(SourceRange AttrRange, Decl *D,
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
    if (IntelFPGANumBanksAttr::classof(&TmpAttr)) {
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

  if (!D->hasAttr<IntelFPGAMemoryAttr>())
    D->addAttr(IntelFPGAMemoryAttr::CreateImplicit(
        Context, IntelFPGAMemoryAttr::Default));

  // We are adding a user NumBanks, drop any implicit default.
  if (IntelFPGANumBanksAttr::classof(&TmpAttr)) {
    if (auto *NBA = D->getAttr<IntelFPGANumBanksAttr>())
      if (NBA->isImplicit())
        D->dropAttr<IntelFPGANumBanksAttr>();
  }

  D->addAttr(::new (Context)
                 AttrType(AttrRange, Context, E, SpellingListIndex));
}
