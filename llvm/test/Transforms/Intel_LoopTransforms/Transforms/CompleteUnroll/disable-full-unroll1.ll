; Sanity Tests DisableHIRCompleteUnroll flag on a simple loop
; A[i] = B[i], unrolled 4 times
;
; ===-----------------------------------===
; *** Run0: Normal Complete Unrolling ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-complete-unroll -print-after=hir-complete-unroll \
; RUN: -S < %s  2>&1 |	FileCheck %s -check-prefix=UNROLL 
;
; ===-----------------------------------===
; *** Run1: Disable Complete Unrolling with -disable-hir-complete-unroll flag ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-complete-unroll -disable-hir-complete-unroll	\
; RUN: -print-after=hir-complete-unroll -HIRCG -S < %s  2>&1 | FileCheck %s -check-prefix=NOUNROLL 
;
; 
; ===---------------------------------------------===
; --- Tests for Run0: Normal Complete Unrolling ---
; ===---------------------------------------------===
;Check the loop is fully unrolled;
;
; UNROLL: BEGIN REGION { modified }
; UNROLL:   %0 = {al:4}(@B)[0][1];
; UNROLL:   {al:4}(@A)[0][1] = %0;
; UNROLL:   %0 = {al:4}(@B)[0][2];
; UNROLL:   {al:4}(@A)[0][2] = %0;
; UNROLL:   %0 = {al:4}(@B)[0][3];
; UNROLL:   {al:4}(@A)[0][3] = %0;
; UNROLL:   %0 = {al:4}(@B)[0][4];
; UNROLL:   {al:4}(@A)[0][4] = %0;
; UNROLL:  END REGION
;
; 
; ===---------------------------------------------===
; --- Tests for Run1: Disable Complete Unrolling ---
; ===---------------------------------------------===
;Check the loop is not unrolled at all;
;
; NOUNROLL: BEGIN REGION { }
; NOUNROLL:  DO i1 = 0, 3, 1   <DO_LOOP>
; NOUNROLL:   %0 = {al:4}(@B)[0][i1 + 1];
; NOUNROLL:   {al:4}(@A)[0][i1 + 1] = %0;
; NOUNROLL:  END LOOP
; NOUNROLL: END REGION
;
;
;Following is the testcase
;ModuleID = 'test.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = global [10 x i32] zeroinitializer, align 16
@B = global [10 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @_Z3foov() #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.05 = phi i64 [ 1, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %i.05
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx1 = getelementptr inbounds [10 x i32], [10 x i32]* @A, i64 0, i64 %i.05
  store i32 %0, i32* %arrayidx1, align 4, !tbaa !1
  %inc = add nuw nsw i64 %i.05, 1
  %exitcond = icmp eq i64 %inc, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %1 = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @A, i64 0, i64 2), align 8, !tbaa !1
  ret i32 %1
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 637) (llvm/branches/loopopt 655)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
