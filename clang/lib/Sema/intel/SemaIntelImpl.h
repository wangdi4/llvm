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
void Sema::HLSAddOneConstantValueAttr(Decl *D, const AttributeCommonInfo &CI,
                                      Expr *E) {
  AttrType TmpAttr(Context, CI, E);

  if (!E->isValueDependent()) {
    ExprResult ICE;
    if (checkRangedIntegralArgument<AttrType>(E, &TmpAttr, ICE))
      return;
    E = ICE.get();
  }

  if (IntelFPGAMaxReplicatesAttr::classof(&TmpAttr) ||
      (MaxConcurrencyAttr::classof(&TmpAttr) && isa<VarDecl>(D))) {
    if (!D->hasAttr<IntelFPGAMemoryAttr>())
      D->addAttr(IntelFPGAMemoryAttr::CreateImplicit(
          Context, IntelFPGAMemoryAttr::Default));
  }

  D->addAttr(::new (Context)
                 AttrType(Context, CI, E));
}

