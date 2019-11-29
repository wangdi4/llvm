; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -print-after=hir-last-value-computation < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
; The original HIR created for this test case shown below is wrong. The liveout
; value of %i.01 should be i1 instead of (i1 + 1). The new HIR does not trigger
; the transformation due to issues which need to be investigated.
; XFAIL: *

;*** IR Dump Before HIR Last Value Computation ***
;
;<0>       BEGIN REGION { }
;<18>            + DO i1 = 0, 39, 1   <DO_MULTI_EXIT_LOOP>
;<2>             |   %t.02.out = %t.02;
;<8>             |   if ((@A)[0][i1] < 0)
;<8>             |   {
;<9>             |      goto for.end;
;<8>             |   }
;<13>            |   %i.01 = i1 + 1;
;<14>            |   %t.02 = i1;
;<18>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Last Value Computation ***
;
; CHECK:   BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 39, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:        |   %t.02.out = %t.02;
; CHECK:        |   if ((@A)[0][i1] < 0)
; CHECK:        |   {
; CHECK:        |      goto for.end;
; CHECK:        |   }
; CHECK:        |   %t.02 = i1;
; CHECK:        + END LOOP
; CHECK:           %i.01 = 40;
; CHECK:   END REGION
;
; ModuleID = 'last.ll'
source_filename = "last.ll"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [50 x i32] zeroinitializer, align 16

define dso_local i32 @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.cond
  %t.02 = phi i32 [ 0, %entry ], [ %t.0, %for.cond ]
  %i.01 = phi i32 [ 0, %entry ], [ %i.0, %for.cond ]
  %0 = zext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds [50 x i32], [50 x i32]* @A, i64 0, i64 %0
  %1 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp slt i32 %1, 0
  %inc = add nuw nsw i32 %i.01, 1
  br i1 %cmp1, label %for.end, label %for.cond

for.cond:                                         ; preds = %for.body
  %i.0 = phi i32 [ %inc, %for.body ]
  %t.0 = phi i32 [ %i.01, %for.body ]
  %cmp = icmp ult i32 %i.0, 40
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body, %for.cond
  %t.0.lcssa = phi i32 [ %t.02, %for.body ], [ %t.0, %for.cond ]
  ret i32 %t.0.lcssa
}
