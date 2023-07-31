; Sanity Test(s) on HIR Loop Reversal: simple l1 loop that MAYBE reversed
;
; l1reversal1-c.ll:
; 1-level loop, sanity testcase1, potentially confusing case1 for in HIR LOOP Reversal
; linear stride on both LHS and RHS;
;
; [REASONS]
; - Applicalbe: NO valid negative memory-access address;
; - Profitable: N/A
; - Legal:      N/A
;
; *** Source Code ***
;
; [BEFORE LOOP REVERSAL]
;
;int foo(int A[100], int B[100]) {
;  int i;
;  for (i = 0; i <= 4; i++) {
;    A[100 - i] = B[i];
;  }
;  return A[1] + B[1];
;}
;
; [AFTER LOOP REVERSAL: potential output 1 -- Not Reversed]
;
;int foo(int A[100], int B[100]) {
;  int i;
;  for (i = 0; i <= 4; i++) {
;    A[100 - i] = B[i];
;  }
;  return A[1] + B[1];
;}
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
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-reversal" -aa-pipeline="basic-aa" -S 2>&1 < %s  | FileCheck %s -check-prefix=BEFORE
;
;
; ===-----------------------------------===
; *** Run1: AFTER HIR Loop Reversal, DOESN'T REVERSE anything ***
; ===-----------------------------------===
; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-reversal,print<hir>" -aa-pipeline="basic-aa" -S 2>&1 < %s  | FileCheck %s -check-prefix=AFTER
;
;
; === -------------------------------------- ===
; *** Tests0: W/O HIR Loop Reversal Output ***
; === -------------------------------------- ===
; Expected output before Loop Reversal
;
;          BEGIN REGION { }
;<13>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<3>          |   %0 = (%B)[i1];
;<6>          |   (%A)[-1 * i1 + 100] = %0;
;<13>         + END LOOP
;          END REGION
;
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 4, 1   <DO_LOOP>
; BEFORE:     |   %0 = (%B)[i1];
; BEFORE:     |   (%A)[-1 * i1 + 100] = %0;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
;
; === -------------------------------------- ===
; *** Tests1: AFTER HIR Loop Reversal Output ***
; *** NEED TO TUNE THE COST MODEL !!!        ***
; === -------------------------------------- ===
; Expected output AFTER	 Loop Reversal
;
;          BEGIN REGION { modified }
;<13>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<3>          |   %0 = (%B)[-1 * i1 + 4];
;<6>          |   (%A)[i1 + 96] = %0;
;<13>         + END LOOP
;          END REGION
;
;
; AFTER:   BEGIN REGION { modified }
; AFTER:      + DO i1 = 0, 4, 1   <DO_LOOP>
; AFTER:      |   %0 = (%B)[-1 * i1 + 4];
; AFTER:      |   (%A)[i1 + 96] = %0;
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
define i32 @foo(ptr noalias nocapture %A, ptr noalias nocapture readonly %B) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !1
  %1 = sub nuw nsw i64 100, %indvars.iv
  %arrayidx2 = getelementptr inbounds i32, ptr %A, i64 %1
  store i32 %0, ptr %arrayidx2, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, ptr %A, i64 1
  %2 = load i32, ptr %arrayidx3, align 4, !tbaa !1
  %arrayidx4 = getelementptr inbounds i32, ptr %B, i64 1
  %3 = load i32, ptr %arrayidx4, align 4, !tbaa !1
  %add = add nsw i32 %3, %2
  ret i32 %add
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, ptr nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 6818) (llvm/branches/loopopt 9722)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
