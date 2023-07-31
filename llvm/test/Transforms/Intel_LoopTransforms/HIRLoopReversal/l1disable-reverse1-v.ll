; l1disable-reversal.ll
;
; Sanity Test on HIR Loop Reversal: check the DisableHIRLoopReversal flag
;
; *** Source Code ***
;
;[BEFORE LOOP REVERSAL]
;
;int foo(int A[100]) {
; int i;
;  for (i = 0; i <= 4; i++) {
;    A[100 - i] = i;
;  }
;  return A[1];
;}
;
;[AFTER LOOP REVERSAL]{Expected!}
;
;int foo(int A[100]) {
; int i;
;  for (i = 0; i <= 4; i++) {
;    A[96 + i] = 4 - i;
;  }
;  return A[1];
;}
;
; ===-----------------------------------===
; *** Run0: BEFORE HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-reversal" -aa-pipeline="basic-aa" -S 2>&1 < %s  | FileCheck %s -check-prefix=BEFORE
;
;
; ===-----------------------------------===
; *** Run1: AFTER HIR Loop Reversal, with Disable flag ON ***
; ===-----------------------------------===
; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-reversal,print<hir>" -aa-pipeline="basic-aa" -disable-hir-loop-reversal -S 2>&1 < %s  | FileCheck %s -check-prefix=AFTER
;
;
; === -------------------------------------- ===
; *** Tests0: W/O HIR Loop Reversal Output ***
; === -------------------------------------- ===
; Expected output before Loop Reversal
;
;          BEGIN REGION { }
;<12>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<5>          |   (%A)[-1 * i1 + 100] = i1;
;<12>         + END LOOP
;          END REGION
;
; BEFORE: BEGIN REGION { }
; BEFORE:  + DO i1 = 0, 4, 1   <DO_LOOP>
; BEFORE:   (%A)[-1 * i1 + 100] = i1;
; BEFORE:  + END LOOP
; BEFORE: END REGION
;
;
; === -------------------------------------- ===
; *** Tests1: With HIR Loop Reversal Disabled Output ***
; === -------------------------------------- ===
;
; Expected output after HIR Loop Reversal, with Disable flag ON
;
;          BEGIN REGION { }
;<12>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<5>          |   (%A)[-1 * i1 + 100] = i1;
;<12>         + END LOOP
;          END REGION
;
; AFTER: BEGIN REGION { }
; AFTER:  + DO i1 = 0, 4, 1   <DO_LOOP>
; AFTER:   (%A)[-1 * i1 + 100] = i1;
; AFTER:  + END LOOP
; AFTER: END REGION
;
;
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [100 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @foo(ptr nocapture %A) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = sub nuw nsw i64 100, %indvars.iv
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %0
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %arrayidx1 = getelementptr inbounds i32, ptr %A, i64 1
  %2 = load i32, ptr %arrayidx1, align 4, !tbaa !1
  ret i32 %2
}

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.start(i64, ptr nocapture) #1

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.end(i64, ptr nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind argmemonly }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1686) (llvm/branches/loopopt 1995)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
