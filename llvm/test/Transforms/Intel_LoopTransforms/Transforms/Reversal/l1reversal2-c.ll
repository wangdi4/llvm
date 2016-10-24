; Sanity Test(s) on HIR Loop Reversal: simple l1 loop that MAYBE reversed
; 
; l1reversal2-c.ll: 
; -1-level loop, sanity test, potentially confusing case2 for in HIR LOOP Reversal;
; 
; [REASONS]
; - Applicalbe: HAS valid negative memory-access address; 
; - Profitable: NO 
;     Revised cost model indicates the accumulated value on positive IV outweights the accumulated value
;     on negative IV(s). So, the loop won't be reversed.
; - Legal:      N/A
; 
; 
; *** Source Code ***
;
; [BEFORE LOOP REVERSAL]
;
;int foo(int * strict A, int * strict B) {
;  int i;
;  for (i = 0; i <= 4; i++) {
;    A[100 - 2 * i] = B[i];
;  }
;  return A[1] + B[1];
;}
;
; [AFTER LOOP REVERSAL: potential output 1 -- Not Reversed]
; 
;int foo(int A[100], int B[100]) {
;  int i;
;  for (i = 0; i <= 4; i++) {
;    A[100 - 2 * i] = B[i];
;  }
;  return A[1] + B[1];
;}
;
;
; [AFTER LOOP REVERSAL: potential output 2 -- Reversed]
; 
;int foo(int A[100], int B[100]) {
;  int i;
;  for (i = 0; i <= 4; i++) {
;    A[96 + i] = B[4-i];
;  }
;  return A[1] + B[1];
;}
; 
; ===-----------------------------------===
; *** Run0: BEFORE HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-before=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=BEFORE 
;
;
; ===-----------------------------------===
; *** Run1: AFTER HIR Loop Reversal, DOESN'T REVERSE anything ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-after=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=AFTER 
;
;
; === -------------------------------------- ===
; *** Tests0: W/O HIR Loop Reversal Output ***
; === -------------------------------------- ===
; Expected output before Loop Reversal
; 
;          BEGIN REGION { }
;<14>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<3>          |   %0 = (%B)[i1];
;<7>          |   (%A)[-2 * i1 + 100] = %0;
;<14>         + END LOOP
;          END REGION
;
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 4, 1   <DO_LOOP>
; BEFORE:     |   %0 = (%B)[i1];
; BEFORE:     |   (%A)[-2 * i1 + 100] = %0;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; === -------------------------------------- ===
; *** Tests1: AFTER HIR Loop Reversal Output ***
; *** THOUGHT NOTHING IS REVERSED !!!        ***
; === -------------------------------------- ===
; Expected output after Loop Reversal
; 
;          BEGIN REGION { }
;<14>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<3>          |   %0 = (%B)[i1];
;<7>          |   (%A)[-2 * i1 + 100] = %0;
;<14>         + END LOOP
;          END REGION
;
; AFTER:   BEGIN REGION { }
; AFTER:      + DO i1 = 0, 4, 1   <DO_LOOP>
; AFTER:      |   %0 = (%B)[i1];
; AFTER:      |   (%A)[-2 * i1 + 100] = %0;
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
define i32 @foo(i32* noalias nocapture %A, i32* noalias nocapture readonly %B) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %1 = shl nsw i64 %indvars.iv, 1
  %2 = sub nuw nsw i64 100, %1
  %arrayidx2 = getelementptr inbounds i32, i32* %A, i64 %2
  store i32 %0, i32* %arrayidx2, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %A, i64 1
  %3 = load i32, i32* %arrayidx3, align 4, !tbaa !1
  %arrayidx4 = getelementptr inbounds i32, i32* %B, i64 1
  %4 = load i32, i32* %arrayidx4, align 4, !tbaa !1
  %add = add nsw i32 %4, %3
  ret i32 %add
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
