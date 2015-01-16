    if (isa<llvm::Constant>(RHSi) && cast<llvm::Constant>(RHSi)->isNullValue()) {
      // (a+ib) / c = a/c + i(b/c)
      if (Op.Ty->getAs<ComplexType>()->getElementType()->isUnsignedIntegerType()) {
        DSTr = Builder.CreateUDiv(LHSr, RHSr);
        DSTi = Builder.CreateUDiv(LHSi, RHSr);
      } else {
        DSTr = Builder.CreateSDiv(LHSr, RHSr);
        DSTi = Builder.CreateSDiv(LHSi, RHSr);
      }
      return ComplexPairTy(DSTr, DSTi);
    }
    else if (isa<llvm::Constant>(RHSr) && cast<llvm::Constant>(RHSr)->isNullValue()) {
      // (a+ib) / di = -ai/d + b/d
      if (Op.Ty->getAs<ComplexType>()->getElementType()->isUnsignedIntegerType()) {
        DSTr = Builder.CreateUDiv(LHSi, RHSi);
        DSTi = Builder.CreateUDiv(Builder.CreateNeg(LHSr), RHSi);
      } else {
        DSTr = Builder.CreateSDiv(LHSi, RHSi);
        DSTi = Builder.CreateSDiv(Builder.CreateNeg(LHSr), RHSi);
      }
      return ComplexPairTy(DSTr, DSTi);
    }
