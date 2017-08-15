; Sanity Test(s) on HIR Loop Reversal: simple reversable level-1 (l1) loop test
; 
; l1reversal5-v.ll
; (1-level loop, sanity testcase5, valid reversal case)
; 
; [REASONS]
; - Applicalbe: YES, HAS valid (1) negative memory-access address; 
; - Profitable: YES
;   Analysis finds there is (1) constcoeff negative IV stride, and 1 constcoeff on positiv IV stride.
;   Since writes carries more weight than reads, the cost model returns negative.
; - Legal:      YES (NO loop-carried dependence)
; 
; 
; *** Source Code ***
;
; [BEFORE LOOP REVERSAL]
;int foo(int *restrict A, int *restrict B) {
;  int i = 0;
;  for (i = 0; i <= 4; i++) {
;    B[50 - 3 * i] = A[2 * i];
;  }
;  return A[1] + B[2] + 1;
;}
;
; [AFTER LOOP REVERSAL]{Expected!}
; 
;int foo(int *restrict A, int *restrict B) {
;  int i = 0;
;  for (i = 0; i <= 4; i++) {
;    B[38 + 3 * i] = A[8 - 2 * i];
;  }
;  return A[1] + B[2] + 1;
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
;<15>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<4>          |   %1 = (%A)[2 * i1];
;<8>          |   (%B)[-3 * i1 + 50] = %1;
;<15>         + END LOOP
;          END REGION
;
; BEFORE:  BEGIN REGION { }
; BEFORE:      + DO i1 = 0, 4, 1   <DO_LOOP>
; BEFORE:     |   %1 = (%A)[2 * i1];
; BEFORE:     |   (%B)[-3 * i1 + 50] = %1;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
;
; === -------------------------------------- ===
; *** Tests1: With HIR Loop Reversal Output ***
; === -------------------------------------- ===
; Expected output AFTER Loop Reversal
; 
;          BEGIN REGION { modified }
;<15>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<4>          |   %1 = (%A)[-2 * i1 + 8];
;<8>          |   (%B)[3 * i1 + 38] = %1;
;<15>         + END LOOP
;          END REGION
;
; AFTER:   BEGIN REGION { modified }
; AFTER:      + DO i1 = 0, 4, 1   <DO_LOOP>
; AFTER:      |   %1 = (%A)[-2 * i1 + 8];
; AFTER:      |   (%B)[3 * i1 + 38] = %1;
; AFTER:      + END LOOP
; AFTER:   END REGION
;
; 
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i32* noalias nocapture readonly %A, i32* noalias nocapture %B) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = shl nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %0
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %2 = mul nsw i64 %indvars.iv, -3
  %3 = add nsw i64 %2, 50
  %arrayidx3 = getelementptr inbounds i32, i32* %B, i64 %3
  store i32 %1, i32* %arrayidx3, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %arrayidx4 = getelementptr inbounds i32, i32* %A, i64 1
  %4 = load i32, i32* %arrayidx4, align 4, !tbaa !1
  %arrayidx5 = getelementptr inbounds i32, i32* %B, i64 2
  %5 = load i32, i32* %arrayidx5, align 4, !tbaa !1
  %add = add i32 %4, 1
  %add6 = add i32 %add, %5
  ret i32 %add6
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 9725) (llvm/branches/loopopt 9729)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
