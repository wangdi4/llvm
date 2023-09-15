//===--- HIRSymbaseAssignment.h - Assigns symbase to ddrefs ---*-- C++ --*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// This pass is responsible for initial assignment of symbases to memory ddrefs.
// Temps are assigned symbase by the dependent HIRParser pass which then returns
// the max symbase assigned to any temp. This max is used to initialize the
// range for memory ddref symbase.
// DDRefs sharing a symbase may alias with any other ddref with the same
// symbase but are guaranteed not to alias with a DDref with another
// symbase. Similar in many respects to LLVM's alias sets concept.
// Will probably use it for implementation in some form or fashion
// Temps and blobs should have their symbases assigned by parsing
//
// Current implementation operates by using AliasSetTracker to classify ld/sts
// into disjoint sets of memory accesses. This is still possible in HIR
// because this pass is run prior to any HIR optimization. We can still
// reliably map HIR to LLVM IR, and so should be able
// to find a representative ptr for each RegDDRef to use in
// AliasAnalysis.
//
// The usual caveats for AliasAnalysis apply here, namely that the
// effectiveness and repeatability of this pass depends on the specific
// AA implementation(s) used and their chain order. Also, AliasSetTracker
// is known to be non deterministic.
//===----------------------------------------------------------------------===//

#ifndef INTEL_LOOPANALYSIS_SYMBASE
#define INTEL_LOOPANALYSIS_SYMBASE

namespace llvm {

class AAResults;
class Function;
class raw_ostream;

namespace loopopt {

class HIRFramework;
class HIRParser;
class RegDDRef;

class HIRSymbaseAssignment {
  class HIRSymbaseAssignmentVisitor;

  AAResults &AA;
  HIRFramework &HIRF;
  HIRParser &HIRP;

public:
  HIRSymbaseAssignment(AAResults &AA, HIRFramework &HIRF, HIRParser &HIRP)
      : AA(AA), HIRF(HIRF), HIRP(HIRP) {}

  void run();
  void print(raw_ostream &OS) const;
};
}
}
#endif
