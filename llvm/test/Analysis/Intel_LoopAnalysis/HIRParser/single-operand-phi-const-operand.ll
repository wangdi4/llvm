; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check that the test is compiled successfully. We should be able to handle non-instruction operand in single operand phis.

; CHECK: + DO i1 = 0, 12, 1   <DO_LOOP>
; CHECK: |   %mul62 = undef  *  0;
; CHECK: + END LOOP


; ModuleID = 'bugpoint-reduced-simplified.bc'
source_filename = "bugpoint-output-a66ffbc.bc"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: nounwind
define void @main() local_unnamed_addr {
entry:
  br label %for.body36.lr.ph

for.body36.lr.ph:                                 ; preds = %for.inc76, %entry
  %0 = phi i32 [ %sub21, %for.inc76 ], [ 14, %entry ]
  %sub21 = add nsw i32 %0, -1
  br label %for.body36.for.cond64.preheader_crit_edge

for.body36.for.cond64.preheader_crit_edge:        ; preds = %for.body36.lr.ph
  %split = phi i32 [ 0, %for.body36.lr.ph ]
  %mul62 = mul i32 undef, %split
  br label %for.inc76

for.inc76:                                        ; preds = %for.body36.for.cond64.preheader_crit_edge
  %cmp17 = icmp ugt i32 %sub21, 1
  br i1 %cmp17, label %for.body36.lr.ph, label %for.cond16.for.inc79_crit_edge

for.cond16.for.inc79_crit_edge:                   ; preds = %for.inc76
  %mul62.lcssa = phi i32 [ %mul62, %for.inc76 ]
  ret void
}

