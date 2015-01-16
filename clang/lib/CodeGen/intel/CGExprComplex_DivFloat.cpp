    if (isa<llvm::Constant>(RHSi) && cast<llvm::Constant>(RHSi)->isNullValue()) {
      // (a+ib) / c = a/c + i(b/c)
      DSTr = Builder.CreateFDiv(LHSr, RHSr);
      DSTi = Builder.CreateFDiv(LHSi, RHSr);
      return ComplexPairTy(DSTr, DSTi);
    }
    else if (isa<llvm::Constant>(RHSr) && cast<llvm::Constant>(RHSr)->isNullValue()) {
      // (a+ib) / di = -ai/d + b/d
      DSTr = Builder.CreateFDiv(LHSi, RHSi);
      DSTi = Builder.CreateFNeg(Builder.CreateFDiv(LHSr, RHSi));
      return ComplexPairTy(DSTr, DSTi);
    }
