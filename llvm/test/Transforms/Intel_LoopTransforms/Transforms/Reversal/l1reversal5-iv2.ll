; Sanity Test(s) on HIR Loop Reversal: simple l1 loop that MAYBE reversed
; 
; l1reversal5-c.ll: 
; 1-level loop, sanity testcase1, potentially confusing case for in HIR LOOP Reversal
; 
; 
; [REASON]
; - cost model indicates the Reversal transformation will NOT be profitable!
; 
; [REASONS]
; - Applicalbe: YES, HAS valid (1) negative memory-access address; 
; - Profitable: NO
;   Analysis finds there is (1) constcoeff negative IV stride, and 1 constcoeff on positiv IV stride.
;   Since writes carries more weight than reads, the cost model returns negative.
; - Legal:      YES (NO loop-carried dependence)
; 
; 
; *** Source Code ***
;[BEFORE LOOP REVERSAL]
; 
;int foo(int A[100]) {
;  int i = 0;
;  for (i = 0; i <= 4; i++) {
;    A[i + 1] = B[20 - i] + i + 2;
;  }
;  return A[1] + B[1] + 1;
;}
;
;[AFTER LOOP REVERSAL]{Expect NO REVERSAL WILL HAPPEN}
;
;int foo(int A[100]) {
;  int i = 0;
;  for (i = 0; i <= 4; i++) {
;    A[i + 1] = B[20 - i] + i + 2;
;  }
;  return A[1] + B[1] + 1;
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
; *** Run1: AFTER HIR Loop Reversal ***
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
;         BEGIN REGION { }
;<16>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<4>          |   %1 = (%B)[-1 * i1 + 20];
;<10>         |   (%A)[i1 + 1] = i1 + %1 + 2;
;<16>         + END LOOP
;         END REGION
;
; BEFORE: BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 4, 1   <DO_LOOP>
; BEFORE:     |   %1 = (%B)[-1 * i1 + 20];
; BEFORE:     |   (%A)[i1 + 1] = i1 + %1 + 2;
; BEFORE:     + END LOOP
; BEFORE: END REGION
; 
; === -------------------------------------- ===
; *** Tests1: With HIR Loop Reversal Output ***
; === -------------------------------------- ===
;
; Expected output after Loop Reversal
; 
;         BEGIN REGION { }
;<16>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<4>          |   %1 = (%B)[-1 * i1 + 20];
;<10>         |   (%A)[i1 + 1] = i1 + %1 + 2;
;<16>         + END LOOP
;         END REGION
;
; AFTER: BEGIN REGION { }
; AFTER:     + DO i1 = 0, 4, 1   <DO_LOOP>
; AFTER:     |   %1 = (%B)[-1 * i1 + 20];
; AFTER:     |   (%A)[i1 + 1] = i1 + %1 + 2;
; AFTER:     + END LOOP
; AFTER: END REGION
; 
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo(i32* nocapture %A, i32* nocapture readonly %B) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = sub nuw nsw i64 20, %indvars.iv
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %0
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %2 = add nuw nsw i64 %indvars.iv, 2
  %3 = trunc i64 %2 to i32
  %add1 = add i32 %3, %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx4 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv.next
  store i32 %add1, i32* %arrayidx4, align 4, !tbaa !1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 1
  %4 = load i32, i32* %arrayidx5, align 4, !tbaa !1
  %arrayidx6 = getelementptr inbounds i32, i32* %B, i64 1
  %5 = load i32, i32* %arrayidx6, align 4, !tbaa !1
  %add7 = add i32 %4, 1
  %add8 = add i32 %add7, %5
  ret i32 %add8
}

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind argmemonly }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1686) (llvm/branches/loopopt 2039)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
