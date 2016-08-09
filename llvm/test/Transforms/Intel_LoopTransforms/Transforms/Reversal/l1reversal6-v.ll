; Sanity Test(s) on HIR Loop Reversal: simple reversable level-1 (l1) loop test
; 
; l1reversal6-v.ll
; (1-level loop, sanity testcase6, valid reversal case)
;
; Multiple DDRefs, both on LHS and RHS, and mixing with IV without Symbase
;
; [REASONS]
; - Applicalbe: YES, HAS valid (2) negative memory-access addresses, and 2 positive memory-access addresses; 
; - Profitable: YES
;   Accumulated negative weight is higher than accumulated positive weight;
; - Legal:      YES ()
;
; 
; *** Source Code ***
;
; [BEFORE LOOP REVERSAL]
;int foo(int *restrict A, int *restrict B) {
;  for (int i = 1; i <= 20; i++) {
;    A[50 - i] = B[i]+1;
;    B[30-i] = A[i+1] + 1;
;  }
;  return A[1] + B[1] + 1;
;}
;
; [AFTER LOOP REVERSAL]
;int foo(int *restrict A, int *restrict B) {
;  for (int i = 1; i <= 20; i++) {
;    A[50 - i] = A[i] + B[i - 1] + 1;
;    B[i] = A[20 - i] + A[30 - i] + 1;
;  }
;  return A[1] + B[1] + 1;
;}
;
; ===-----------------------------------===
; *** Run0: WITHOUT HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-before=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=BEFORE 
;
; 
; ===-----------------------------------===
; *** Run1: WITH HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-after=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=AFTER 
;
; 
; === -------------------------------------- ===
; *** Tests0: W/O HIR Loop Reversal Output ***
; === -------------------------------------- ===
; Expected output BEFORE Loop Reversal
; 
;          BEGIN REGION { }
;<20>         + DO i1 = 0, 19, 1   <DO_LOOP>
;<3>          |   %2 = (%B)[i1 + 1];
;<7>          |   (%A)[-1 * i1 + 199] = %2 + 1;
;<10>         |   %4 = (%A)[i1 + 2];
;<14>         |   (%B)[-1 * i1 + 999] = %4 + 2;
;<20>         + END LOOP
;          END REGION
;
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 19, 1   <DO_LOOP>
; BEFORE:     |   %2 = (%B)[i1 + 1];
; BEFORE:     |   (%A)[-1 * i1 + 199] = %2 + 1;
; BEFORE:     |   %4 = (%A)[i1 + 2];
; BEFORE:     |   (%B)[-1 * i1 + 999] = %4 + 2;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; === -------------------------------------- ===
; *** Tests1: With HIR Loop Reversal Output ***
; === -------------------------------------- ===
; Expected output AFTER Loop Reversal
; 
;          BEGIN REGION { modified }
;<20>         + DO i1 = 0, 19, 1   <DO_LOOP>
;<3>          |   %2 = (%B)[-1 * i1 + 20];
;<7>          |   (%A)[i1 + 180] = %2 + 1;
;<10>         |   %4 = (%A)[-1 * i1 + 21];
;<14>         |   (%B)[i1 + 980] = %4 + 2;
;<20>         + END LOOP
;          END REGION
;
; AFTER:   BEGIN REGION { modified }
; AFTER:      + DO i1 = 0, 19, 1   <DO_LOOP>
; AFTER:      |   %2 = (%B)[-1 * i1 + 20];
; AFTER:      |   (%A)[i1 + 180] = %2 + 1;
; AFTER:      |   %4 = (%A)[-1 * i1 + 21];
; AFTER:      |   (%B)[i1 + 980] = %4 + 2;
; AFTER:      + END LOOP
; AFTER:   END REGION
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i32* noalias nocapture %A, i32* noalias nocapture %B) #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %arrayidx10 = getelementptr inbounds i32, i32* %A, i64 1
  %0 = load i32, i32* %arrayidx10, align 4, !tbaa !1
  %arrayidx11 = getelementptr inbounds i32, i32* %B, i64 1
  %1 = load i32, i32* %arrayidx11, align 4, !tbaa !1
  %add12 = add i32 %0, 1
  %add13 = add i32 %add12, %1
  ret i32 %add13

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 1, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %2, 1
  %3 = sub nuw nsw i64 200, %indvars.iv
  %arrayidx2 = getelementptr inbounds i32, i32* %A, i64 %3
  store i32 %add, i32* %arrayidx2, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv.next
  %4 = load i32, i32* %arrayidx5, align 4, !tbaa !1
  %add6 = add nsw i32 %4, 2
  %5 = sub nuw nsw i64 1000, %indvars.iv
  %arrayidx9 = getelementptr inbounds i32, i32* %B, i64 %5
  store i32 %add6, i32* %arrayidx9, align 4, !tbaa !1
  %exitcond = icmp eq i64 %indvars.iv.next, 21
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 6818) (llvm/branches/loopopt 9722)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
