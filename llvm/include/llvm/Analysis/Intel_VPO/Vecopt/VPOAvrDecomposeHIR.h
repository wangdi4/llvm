//===-- VPOAvrDecomposeHIR.h ------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the AVRDecomposeHIR Pass. This pass decomposes any
/// AVRValueHIR node that encapsulates non-unitary HIR CanonExprs. The result of
/// the decomposition is an AVRExpression tree that explicitly represents each
/// unitary operation/operand in the CanonExpr. A CanonExpr is said unitary if
/// it represents a single temp (%t) or constant (5) operand without any
/// implicit or explicit operation on it. A CanonExpr representing a nested
/// blob, memory operation, non-standalone IV, hidden data type conversion, or
/// any kind or arithmetic or logical operation is considered non-unitary and,
/// thus, subject to be decomposed.
///
/// The decomposition process creates the following kind of nodes:
///    - AVRValueHIR nodes for operands that have HIR-specific representation
///    (blobs and IVs). The HIR information is available from the DDRef
///    (BlobDDRef) or IVValueInfo members of the new AVRValueHIR.
///    - AVRValue nodes for Constant operands. They do not have HIR-specific
///    representation.
///    - AVRExpression nodes for any kind of implicit/explicit operation. They
///    do not have HIR-specific representation.
///
/// After the decomposition, the resulting AVRExpression is accessible from the
/// DecompTree member of the original AVRValueHIR that has been decomposed. The
/// original AVRValueHIR is not replaced or removed from the AVR in order not to
/// penalize the performance of those AVR-based analysis/transformations that do
/// not need decomposed AVRValueHIR nodes. These passes can process the original
/// AVRValueHIR node and ignore the decomposed AVRExpression tree. Algorithms
/// that do need decomposition can access the decomposed AVRExpression on demand
/// using the DecompTree member.
///
/// Rational:
/// ---------
/// This approach is an attempt to minimize the differences in the vectorizer
/// representation when the AVR is built from LLVM-IR and HIR. The main goal is
/// to reduce as much as possible the IR-specific code of vectorizer
/// analyses/transformations that currently is leading to implementing and
/// maintaining two versions of each AVR-based analysis/transformation. The
/// problem and potential solutions were presented and discussed in 7/12/2016
/// US/IDP LLVM Vectorizer Team meeting.
/// More information can be found in
/// S:\cmplr\HPO\Vectorizer\LLVM\AVRValueHIR_issue.pptx
//
/// Usage:
/// ------
/// The decomposition of AVRValueHIR nodes has been designed to be as
/// transparent as possible to clients. AVRVisitor has been extended to
/// automatically traverse the decomposed AVRExpression of those AVRValuesHIR
/// that have been decomposed. This traversal can be enabled or disabled on
/// demand in AVRVisitor with the new flag RecurseInsideValues. If
/// RecurseInsideValues is false, AVRVisitor will not traverse the decomposed
/// AVRExpression of AVRValueHIRs. If it is true, AVRVisitor will first visit
/// the original AVRValueHIR, and then the decomposed AVRExpression. The
/// equivalent behavior applies to post visits.
///
/// Extensions:
/// -----------
/// If clients need a more sophisticated traversal, we can extend the AVRVisitor
/// interface.
///
/// Example 1: some analyses like cost model may want to skip the visit of the
/// original AVRValueHIR and visit only the decomposed AVRExpression. We could
/// support this behavior with a new flag (SkipWrapperValue) in AVRVisitor. A
/// bit mask that represents different behaviors is another option.
///
/// Example 2: some transformation like code gen may want to visit only the
/// decomposed AVRExpression of some particular AVRValueHIR. For example, only
/// AVRValueHIR containing nested blobs. If this is the case, we could add a
/// functor to the interface that would be used as predicate to determine if the
/// decomposed AVRExpression has to be visited or not.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_DECOMPOSE_H
#define LLVM_ANALYSIS_VPO_AVR_DECOMPOSE_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrGenerate.h"

using namespace llvm::loopopt;

namespace llvm { // LLVM Namespace
namespace vpo {  // VPO Vectorizer namespace

class AVRDecomposeHIR : public FunctionPass {

protected:
  AVRGenerateHIR *AVRG;

public:
  static char ID;

  AVRDecomposeHIR();

  bool runOnFunction(Function &F);
  bool runOnAvr(AVR *ANode, const DataLayout &DL);
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void print(raw_ostream &OS, unsigned Depth = 1,
             VerbosityLevel VLevel = PrintBase) const;
  void dump(VerbosityLevel VLevel = PrintBase) const;
  void releaseMemory();
};

// Interface function to decompose a blob. We can get rid of this function
// in future when the decomposer stores a map of <blob, decomposed_tree> to
// avoid duplicating blobs more than once.
AVR *decomposeBlob(RegDDRef *RDDR, unsigned BlobIdx, int64_t BlobCoeff,
                   const DataLayout &DL);
} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_AVR_DECOMPOSE_H
