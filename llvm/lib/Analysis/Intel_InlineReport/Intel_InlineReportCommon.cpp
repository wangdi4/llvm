//===--------- Intel_InlineReportCommon.cpp - Inlining Reporti utils  ----===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements inline report utils.
//
//===----------------------------------------------------------------------===//
//
#include "llvm/Transforms/IPO/Intel_InlineReportCommon.h"

using namespace llvm;

///
/// \brief Print 'indentCount' indentations
///
void llvm::printIndentCount(raw_ostream &OS, unsigned indentCount) {
  OS.indent(indentCount * 3);
}

/// \brief Inlining report level option
///
/// Specified with -inline-report=N
///   N is a bit mask with the following interpretation of the bits
///   0x0000: No inlining report
///   0x0001: Simple inlining report
///   0x0002: Add inlining reasons
///   0x0004: Put the inlining reasons on the same line as the call sites
///   0x0008: Print the line and column info for each call site if available
///   0x0010: Print the file for each call site
///   0x0020: Print linkage info for each function and call site
///   0x0040: Print the early exit cost vs. threshold info, where applicable
///   0x0080: Create metadata-based inline report
///   0x0100: Create composite inline report for an -flto compilation.
///   0x0200: Create the inlining report info for the special intrinsic call
///           sites
///   0x0400: Print the source language: 'C' for C/C++ and 'F' for Fortran
///   0x0800: Print the inlining option values
///   0x1000: Print both early exit and real inlining costs
///   0x2000: Print dead static functions
///   0x4000: Print external function callsites
///   0x8000: Print indirect function callsites
///   0x10000: Demangle C++ names
///   0x20000: Use compact representation of nested inlines
///
///  The driver will be set to emit the following settings when -qopt-report=X
///  is used:
///    -qopt-report=1  0x0019
///    -qopt-report=2  0x2819 (default)
///    -qopt-report=3  0xf859
///
cl::opt<unsigned> llvm::IntelInlineReportLevel("inline-report", cl::Hidden,
                                               cl::init(0), cl::Optional,
                                               cl::desc("Print inline report"));

///
/// If the callee being inlined has already been inlined into this number
/// of times, the inlining of this callee will be reported in compact form.
/// As an example, consider the following test case:
///
/// define void @mynoinline() #0 {
/// entry:
///   ret void
/// }
///
/// define i32 @foo() {
/// entry:
///   call void @mynoinline()
///   ret i32 5
/// }
///
/// define i32 @boo() {
/// entry:
///   ret i32 5
/// }
///
/// define i32 @goo() {
/// entry:
///   %call0 = call i32 @foo()
///   %call1 = call i32 @foo()
///   %call2 = sub i32 %call0, %call1
///   %call3 = call i32 @foo()
///   %call4 = add i32 %call2, %call3
///   ret i32 %call4
/// }
///
/// define i32 @hoo() {
/// entry:
///  %call0 = call i32 @boo()
///  %call1 = call i32 @boo()
///  %call2 = sub i32 %call0, %call1
///  ret i32 %call2
/// }
///
/// define i32 @main() {
/// entry:
///   call void @mynoinline()
///   %call0 = call i32 @goo()
///   %call1 = call i32 @goo()
///   %call2 = add i32 %call0, %call1
///   %call3 = call i32 @hoo()
///   %call4 = add i32 %call2, %call3
///   %call5 = call i32 @foo()
///   %call6 = add i32 %call4, %call5
///   ret i32 %call6
/// }
///
/// attributes #0 = { noinline }
///
/// With normal settings, the inlining report with -inline-report=0xf847
/// will casue the inline report for @main be printed as:
///
/// COMPILE FUNC: main
///   -> mynoinline [[Callee has noinline attribute]]
///   -> INLINE: goo (60<=337) <<Callee is single basic block>>
///      -> INLINE: foo (0<=337) <<Callee is single basic block>>
///         -> mynoinline [[Callee has noinline attribute]]
///      -> INLINE: foo (0<=337) <<Callee is single basic block>>
///         -> mynoinline [[Callee has noinline attribute]]
///      -> INLINE: foo (0<=337) <<Callee is single basic block>>
///         -> mynoinline [[Callee has noinline attribute]]
///   -> INLINE: goo (60<=337) <<Callee is single basic block>>
///      -> INLINE: foo (0<=337) <<Callee is single basic block>>
///         -> mynoinline [[Callee has noinline attribute]]
///      -> INLINE: foo (0<=337) <<Callee is single basic block>>
///         -> mynoinline [[Callee has noinline attribute]]
///      -> INLINE: foo (0<=337) <<Callee is single basic block>>
///         -> mynoinline [[Callee has noinline attribute]]
///   -> INLINE: hoo (-30<=337) <<Callee is single basic block>>
///      -> INLINE: boo (-30<=337) <<Callee is single basic block>>
///      -> INLINE: boo (-30<=337) <<Callee is single basic block>>
///   -> INLINE: foo (0<=337) <<Callee is single basic block>>
///      -> mynoinline [[Callee has noinline attribute]]
///
/// But adding -inline-report-compact-theshold=1 causes it to be printed as:
///
/// COMPILE FUNC: main
///   -> mynoinline [[Callee has noinline attribute]]
///   -> <C> INLINE: goo (60<=337) <<Callee is single basic block>>
///      -> mynoinline [[Callee has noinline attribute]]
///      -> mynoinline [[Callee has noinline attribute]]
///      -> mynoinline [[Callee has noinline attribute]]
///   -> <C> INLINE: goo (60<=337) <<Callee is single basic block>>
///      -> mynoinline [[Callee has noinline attribute]]
///      -> mynoinline [[Callee has noinline attribute]]
///      -> mynoinline [[Callee has noinline attribute]]
///   -> <C> INLINE: hoo (-30<=337) <<Callee is single basic block>>
///   -> INLINE: foo (0<=337) <<Callee is single basic block>>
///      -> mynoinline [[Callee has noinline attribute]]
///  SUMMARIZED INLINED CALL SITE COUNTS
///         6 foo
///         2 boo
///
/// Here the nested inlines of @foo into @goo and @boo into @hoo have been
/// omitted and a summary of them is given at the end of the inline report
/// for @main. The <C> indicates that the nested inlines for the callsite
/// have been summarized.
///
/// It is possible also to select that nested inlinings always be summarized
/// by setting the 0x20000 (compact) bit in -inline-report. For example, use
/// -inline-report=0x2f847 rather than -inline-report=0xf847.
///
/// The primary motivation for implementuing this is to speed up compile
/// times and decrease memory usage when the inlining report is generated
/// for C++ programs with very deep inlining.
///
cl::opt<unsigned> llvm::IntelInlineReportCompactThreshold(
    "inline-report-compact-threshold", cl::Hidden, cl::init(100), cl::Optional,
    cl::desc("Switch to compact form if more than this many inlines"));

StringRef llvm::getOpStr(Metadata *Node, StringRef Front) {
  assert(Node && "Empty metadata");
  MDString *StrMD = nullptr;
  MDNode *StrNode = dyn_cast<MDNode>(Node);
  if (StrNode)
    StrMD = cast<MDString>(StrNode->getOperand(0));
  else
    StrMD = cast<MDString>(Node);

  StringRef Res = StrMD->getString();
  bool RV = Res.consume_front(Front);
  (void)RV;
  assert(RV && "Expecting string on front");
  return Res;
}

void llvm::getOpVal(Metadata *Node, StringRef Front, int64_t *Val) {
  assert(Val && "Empty value storage");
  StringRef Res = getOpStr(Node, Front);
  assert(!Res.empty() && "Incomplete inlining report metadata");
  bool Failed = Res.getAsInteger(10, *Val);
  (void)Failed;
  assert(!Failed && "Expecting integer value");
}

// Print the inlining option values
void llvm::printOptionValues(raw_ostream &OS, unsigned OptLevel,
                             unsigned SizeLevel) {
  InlineParams Params;
  if (!OptLevel && !SizeLevel)
    Params = llvm::getInlineParams();
  else
    Params = llvm::getInlineParams(OptLevel, SizeLevel);
  OS << "Option Values:\n";
  OS << "  inline-threshold: " << Params.DefaultThreshold << "\n";
  OS << "  inlinehint-threshold: "
     << (Params.HintThreshold.has_value() ? Params.HintThreshold.value() : 0)
     << "\n";
  OS << "  inlinecold-threshold: "
     << (Params.ColdThreshold.has_value() ? Params.ColdThreshold.value() : 0)
     << "\n";
  OS << "  inlineoptsize-threshold: "
     << (Params.OptSizeThreshold.has_value() ? Params.OptSizeThreshold.value()
                                            : 0)
     << "\n";
  OS << "\n";
}

// Skip some llvm-specific intrinsics to make inline report shorter.
bool llvm::shouldSkipIntrinsic(IntrinsicInst *II) {
  if (!II)
    return false;
  Intrinsic::ID Intrin = II->getIntrinsicID();
  switch (Intrin) {
  default:
    return false;
  case Intrinsic::dbg_declare:
  case Intrinsic::dbg_value:
  case Intrinsic::intel_subscript:
  case Intrinsic::intel_subscript_nonexact:
  case Intrinsic::lifetime_end:
  case Intrinsic::lifetime_start:
  case Intrinsic::ptr_annotation:
  case Intrinsic::var_annotation:
  case Intrinsic::assume:
  case Intrinsic::type_test:
    return true;
  }
  return false;
}
