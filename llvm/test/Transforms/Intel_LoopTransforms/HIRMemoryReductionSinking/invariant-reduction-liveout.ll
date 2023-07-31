; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-framework>,hir-memory-reduction-sinking,print<hir-framework>" -disable-output 2>&1 < %s | FileCheck %s

; Check that memory sinking does not occur for the invariant memory reductions when %0 is liveout. TODO: We need to recompute the temp value to preserve correctness.

; Dump Before-

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %0 = (@A)[0][5];
; CHECK: |   (@A)[0][5] = %0 + 3;
; CHECK: + END LOOP

; CHECK-NOT: modified

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(i64 %t) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = load i32, ptr getelementptr inbounds ([100 x i32], ptr @A, i64 0, i64 5), align 4
  %add3 = add nsw i32 %0, 3
  store i32 %add3, ptr getelementptr inbounds ([100 x i32], ptr @A, i64 0, i64 5), align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %.lcssa = phi i32 [ %0, %for.body ]
  ret void
}

