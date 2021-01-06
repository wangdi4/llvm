//===- HIROptVarPredicate.h --- Optimization of predicates containing IVs -===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallSet.h"

namespace llvm {

namespace loopopt {

class HIRFramework;

class HLLoop;

class HIROptVarPredicateInterface {
public:
  static std::unique_ptr<HIROptVarPredicateInterface>
  create(HIRFramework &HIRF);

  virtual bool processLoop(HLLoop *Loop, bool SetRegionModified,
                   SmallVectorImpl<HLLoop *> *OutLoops) = 0;

  virtual const SmallPtrSetImpl<HLNode *> &getNodesToInvalidate() const = 0;

  virtual ~HIROptVarPredicateInterface() {}
};

}

}
