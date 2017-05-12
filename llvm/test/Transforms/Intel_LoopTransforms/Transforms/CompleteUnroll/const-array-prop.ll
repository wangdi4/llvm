; REQUIRES: asserts

; RUN: opt -hir-ssa-deconstruction -hir-pre-vec-complete-unroll -debug-only=hir-complete-unroll -print-before=hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll 2>&1 < %s | FileCheck %s


; Verify that constant array propagation of @A makes this loop profitable for pre-vec complete unroll.

; CHECK: Before HIR PreVec Complete Unroll

; CHECK: + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |   %0 = (@A)[0][i1];
; CHECK: |   %1 = (%B)[i1];
; CHECK: |   (%B)[i1] = %0 + %1;
; CHECK: + END LOOP

; Check that we recognize occurence of (@A)[0][i1] as GEP savings.
; CHECK: GEPSavings: 8

; CHECK: After HIR PreVec Complete Unroll

; CHECK: %0 = (@A)[0][0];
; CHECK: %1 = (%B)[0];
; CHECK: (%B)[0] = %0 + %1;
; CHECK: %0 = (@A)[0][1];
; CHECK: %1 = (%B)[1];
; CHECK: (%B)[1] = %0 + %1;
; CHECK: %0 = (@A)[0][2];
; CHECK: %1 = (%B)[2];
; CHECK: (%B)[2] = %0 + %1;
; CHECK: %0 = (@A)[0][3];
; CHECK: %1 = (%B)[3];
; CHECK: (%B)[3] = %0 + %1;


;Module Before HIR; ModuleID = 'const-array-prop.c'
source_filename = "const-array-prop.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = internal unnamed_addr constant [4 x i32] [i32 1, i32 2, i32 3, i32 4], align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %B) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [4 x i32], [4 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4
  %add = add nsw i32 %1, %0
  store i32 %add, i32* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

