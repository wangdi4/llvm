; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -print-before=hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll -hir-complete-unroll-function-ddref-threshold=8 2>&1 < %s | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -print-after=hir-pre-vec-complete-unroll -hir-complete-unroll-function-ddref-threshold=6 2>&1 < %s | FileCheck %s --check-prefix=NOUNROLL

; Verify that both loops get unrolled when ddref threshold for the function is big enough and we don't do any unroll even if the threshold is big enough to unroll the first loop. Only non-simplified ddrefs are counted. Since @A is a constant array which be replaced with constants, it is not counted. Total number of ddrefs in the unrolled loops for this function are 4 + 4 = 8.

; CHECK: Dump Before

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |   (%B)[i1] = (@A)[0][i1];
; CHECK: + END LOOP
; CHECK: END REGION

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |   (%C)[i1] = (@A)[0][i1];
; CHECK: + END LOOP
; CHECK: END REGION


; CHECK: Dump After

; CHECK-NOT: DO i1

; NOUNROLL: DO i1
; NOUNROLL: DO i1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = internal unnamed_addr constant [4 x i32] [i32 3, i32 2, i32 1, i32 0], align 16

define void @foo(ptr nocapture %B, ptr nocapture %C) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [4 x i32], ptr @A, i64 0, i64 %indvars.iv
  %t0 = load i32, ptr %arrayidx, align 4
  %arrayidx3 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  store i32 %t0, ptr %arrayidx3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  br label %for.body1

for.body1:                                         ; preds = %for.body1, %for.end
  %indvars.iv1 = phi i64 [ 0, %for.end ], [ %indvars.iv.next1, %for.body1 ]
  %arrayidx1 = getelementptr inbounds [4 x i32], ptr @A, i64 0, i64 %indvars.iv1
  %t1 = load i32, ptr %arrayidx1, align 4
  %arrayidx31 = getelementptr inbounds i32, ptr %C, i64 %indvars.iv1
  store i32 %t1, ptr %arrayidx31, align 4
  %indvars.iv.next1 = add nuw nsw i64 %indvars.iv1, 1
  %exitcond1 = icmp eq i64 %indvars.iv.next1, 4
  br i1 %exitcond1, label %for.end1, label %for.body1

for.end1:                                          ; preds = %for.body1
  ret void
}

