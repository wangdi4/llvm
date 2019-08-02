; Sanity Test(s) on HIR Loop Reversal: simple reversable level-1 (l1) loop test
;
; l1reversal3-v.ll
; (1-level loop, sanity testcase3, valid reversal case)
;
; [REASONS]
; - Applicalbe: YES, HAS valid (3) negative memory-access address;
; - Profitable: YES
;   Analysis finds there is (2) constcoeff negative IV stride, and 0 constcoeff on positiv IV stride.
;   So the cost model returns positive.
; - Legal:      YES (NO loop-carried dependence)
;
; *** Source Code ***
;
; [BEFORE LOOP REVERSAL]
;int foo(int * strict A, int * strict B){
;  int i = 0;
;  for (i = 0; i <= 4; i++) {
;    A[100 - 3 * i] = B[80 - 2 * i];
;  }
;  return A[1] + B[1];
;}
;
; [AFTER LOOP REVERSAL]{Expected!}
;int foo(int * strict A, int * strict B){
;  int i = 0;
;  for (i = 0; i <= 4; i++) {
;    A[88 + 3 * i] = B[72 + 2 * i];
;  }
;  return A[1] + B[1];
;}
;
; ===-----------------------------------===
; *** Run0: WITHOUT HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-before=hir-loop-reversal -S 2>&1 < %s  |	FileCheck %s -check-prefix=BEFORE
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-reversal" -aa-pipeline="basic-aa" -S 2>&1 < %s  | FileCheck %s -check-prefix=BEFORE
;
;
; ===-----------------------------------===
; *** Run1: WITH HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-after=hir-loop-reversal -S 2>&1 < %s  |	FileCheck %s -check-prefix=AFTER
; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-reversal,print<hir>" -aa-pipeline="basic-aa" -S 2>&1 < %s  | FileCheck %s -check-prefix=AFTER
;
;
; === -------------------------------------- ===
; *** Tests0: W/O HIR Loop Reversal Output ***
; === -------------------------------------- ===
; Expected output BEFORE Loop Reversal
;
;          BEGIN REGION { }
;<16>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<5>          |   %2 = (%B)[-2 * i1 + 80];
;<9>          |   (%A)[-3 * i1 + 100] = %2;
;<16>         + END LOOP
;          END REGION
;
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 4, 1   <DO_LOOP>
; BEFORE:     |   %2 = (%B)[-2 * i1 + 80];
; BEFORE:     |   (%A)[-3 * i1 + 100] = %2;
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
;<16>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<5>          |   %2 = (%B)[2 * i1 + 72];
;<9>          |   (%A)[3 * i1 + 88] = %2;
;<16>         + END LOOP
;          END REGION
;
; AFTER:   BEGIN REGION { modified }
; AFTER:      + DO i1 = 0, 4, 1   <DO_LOOP>
; AFTER:      |   %2 = (%B)[2 * i1 + 72];
; AFTER:      |   (%A)[3 * i1 + 88] = %2;
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
define i32 @foo(i32* noalias nocapture %A, i32* noalias nocapture readonly %B) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = shl nsw i64 %indvars.iv, 1
  %1 = sub nuw nsw i64 80, %0
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %1
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %3 = mul nsw i64 %indvars.iv, -3
  %4 = add nsw i64 %3, 100
  %arrayidx4 = getelementptr inbounds i32, i32* %A, i64 %4
  store i32 %2, i32* %arrayidx4, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 1
  %5 = load i32, i32* %arrayidx5, align 4, !tbaa !1
  %arrayidx6 = getelementptr inbounds i32, i32* %B, i64 1
  %6 = load i32, i32* %arrayidx6, align 4, !tbaa !1
  %add = add nsw i32 %6, %5
  ret i32 %add
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
