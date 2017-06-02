; RUN: opt < %s -hir-ssa-deconstruction -hir-cg -force-hir-cg -S | FileCheck %s


; Check that CG leaves HIR metadata in the old region as they become unreachable and will be removed later by simplifycfg.
; CHECK: in.de.ssa
; CHECK: live.range.de.ssa


; Verify that the gep for %A is CG'd correctly.
; CHECK: region.0:
; CHECK: {{loop.[0-9]+:}}
; CHECK: [[IVLOAD1:%.*]] = load i64, i64* %i1.i64
; CHECK-NEXT: [[IVLOAD2:%.*]] = load i64, i64* %i1.i64
; CHECK-NEXT: getelementptr inbounds [64 x i32], [64 x i32]* %A, i64 [[IVLOAD1]], i64 [[IVLOAD2]]


; Check that HIR metadata is cleaned up by CG when we do not generate code for the region. 
; RUN: opt < %s -hir-ssa-deconstruction -hir-cg -S | FileCheck %s -check-prefix=NOCG

; NOCG-NOT: in.de.ssa
; NOCG-NOT: live.range.de.ssa


; ModuleID = 'array-param.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readonly uwtable
define i32 @foo([64 x i32]* nocapture readonly %A) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret i32 %add

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %sum.09 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %arrayidx2 = getelementptr inbounds [64 x i32], [64 x i32]* %A, i64 %indvars.iv, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx2, align 4
  %add = add nsw i32 %0, %sum.09
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

