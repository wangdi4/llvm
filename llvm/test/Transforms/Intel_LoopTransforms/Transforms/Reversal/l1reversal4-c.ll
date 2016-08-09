; Sanity Test(s) on HIR Loop Reversal: simple l1 loop that MAYBE reversed
; 
; l1reversal4-c.ll: 
; 1-level loop, sanity testcase1, potentially confusing case4 for in HIR LOOP Reversal:
; 
; [REASONS]
; - Applicalbe: YES, HAS valid (1) negative memory-access address; 
; - Profitable: YES
;   Analysis finds there is constcoeff (2) negative IV stride, and constcoeff (3) on positiv IV stride.
;   So the cost model returns positive.
; - Legal:      YES (no dependence: A and B are not aliased)
; 
; 
; *** Source Code ***
;
; [BEFORE LOOP REVERSAL]
;
;int foo(int * strict A, int * strict B){
;  int i;
;  for (i = 0; i <= 4; i++) {
;    A[100 - 2*i] = B[3*i];
;  }
;  return A[1] + B[1];
;}
;
;
; [AFTER LOOP REVERSAL: potential output 2 -- Reversed]
; 
;int foo(int * strict A, int * strict B){
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
;<15>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<4>          | %1 = (%B)[3 * i1];
;<8>          | (%A)[-2 * i1 + 100] = %1;
;<15>         + END LOOP
;          END REGION
;
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 4, 1   <DO_LOOP>
; BEFORE:     | %1 = (%B)[3 * i1];
; BEFORE:     | (%A)[-2 * i1 + 100] = %1;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
;
; === -------------------------------------- ===
; *** Tests1: AFTER HIR Loop Reversal Output ***
; *** THOUGHT NOTHING IS REVERSED !!!        ***
; === -------------------------------------- ===
; Expected output AFTER	 Loop Reversal
; 
;         BEGIN REGION { modified }
;<15>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<4>          |   %1 = (%B)[-3 * i1 + 12];
;<8>          |   (%A)[2 * i1 + 92] = %1;
;<15>         + END LOOP
;         END REGION
;
;AFTER:   BEGIN REGION { modified }
;AFTER:       + DO i1 = 0, 4, 1   <DO_LOOP>
;AFTER:       |   %1 = (%B)[-3 * i1 + 12];
;AFTER:       |   (%A)[2 * i1 + 92] = %1;
;AFTER:       + END LOOP
;AFTER:   END REGION
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
  %0 = mul nuw nsw i64 %indvars.iv, 3
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %0
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %2 = shl nsw i64 %indvars.iv, 1
  %3 = sub nuw nsw i64 100, %2
  %arrayidx3 = getelementptr inbounds i32, i32* %A, i64 %3
  store i32 %1, i32* %arrayidx3, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %arrayidx4 = getelementptr inbounds i32, i32* %A, i64 1
  %4 = load i32, i32* %arrayidx4, align 4, !tbaa !1
  %arrayidx5 = getelementptr inbounds i32, i32* %B, i64 1
  %5 = load i32, i32* %arrayidx5, align 4, !tbaa !1
  %add = add nsw i32 %5, %4
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
