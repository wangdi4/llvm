; Sanity Test(s) on HIR Loop Reversal: simple l1 loop that CAN'T be reversed
;
; l1reversal4-iv.ll:
; 1-level loop, sanity testcase4, invalid reversal case
;
; [REASONS]
; - Applicalbe: NO, HAS NO valid (0) negative memory-access address;
; - Profitable: NO
;   Analysis finds there is (0) constcoeff negative IV stride, and 2 constcoeff on positiv IV stride.
;   So the cost model returns negative.
; - Legal:      NO (loop-carried ANTI dependence)
;
; *** Source Code ***
;
; [BEFORE LOOP REVERSAL]
;
;int foo(int A[100]) {
;  int i;
;  for (i = 0; i <= 4; i++) {
;    A[i] = A[i+1];
;  }
;  return A[1];
;}
;
; [AFTER LOOP REVERSAL]
;
;int foo(int A[100]) {
;  int i;
;  for (i = 0; i <= 4; i++) {
;    A[i] = A[i+1];
;  }
;  return A[1];
;}
;
; ===-----------------------------------===
; *** Run0: BEFORE HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-before=hir-loop-reversal -S 2>&1 < %s  |	FileCheck %s -check-prefix=BEFORE
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-reversal" -aa-pipeline="basic-aa" -S 2>&1 < %s  | FileCheck %s -check-prefix=BEFORE
;
;
; ===-----------------------------------===
; *** Run1: AFTER HIR Loop Reversal, DOESN'T REVERSE anything ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-after=hir-loop-reversal -S 2>&1 < %s  |	FileCheck %s -check-prefix=AFTER
; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-reversal,print<hir>" -aa-pipeline="basic-aa" -S 2>&1 < %s  | FileCheck %s -check-prefix=AFTER
;
;
; === -------------------------------------- ===
; *** Tests0: W/O HIR Loop Reversal Output ***
; === -------------------------------------- ===
; Expected output before Loop Reversal
;
;          BEGIN REGION { }
;<12>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<4>          | %0 = (%A)[i1 + 1];
;<6>          | (%A)[i1] = %0;
;<12>         + END LOOP
;          END REGION
;
; BEFORE:  BEGIN REGION { }
; BEFORE:    + DO i1 = 0, 4, 1   <DO_LOOP>
; BEFORE:    | %0 = (%A)[i1 + 1];
; BEFOER:    | (%A)[i1] = %0;
; BEFORE:    + END LOOP
; BEFORE:  END REGION
;
;
; === -------------------------------------- ===
; *** Tests1: AFTER HIR Loop Reversal Output ***
; *** THOUGHT NOTHING IS REVERSED !!!        ***
; === -------------------------------------- ===
; Expected output AFTER	 Loop Reversal
;
;          BEGIN REGION { }
;<12>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<4>          | %0 = (%A)[i1 + 1];
;<6>          | (%A)[i1] = %0;
;<12>         + END LOOP
;          END REGION
;
; AFTER:  BEGIN REGION { }
; AFTER:    + DO i1 = 0, 4, 1   <DO_LOOP>
; AFTER:    | %0 = (%A)[i1 + 1];
; AFTER:    | (%A)[i1] = %0;
; AFTER:    + END LOOP
; AFTER:  END REGION
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo(i32* nocapture %A) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv.next
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx2 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 %0, i32* %arrayidx2, align 4, !tbaa !1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %A, i64 1
  %1 = load i32, i32* %arrayidx3, align 4, !tbaa !1
  ret i32 %1
}

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind argmemonly }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1686) (llvm/branches/loopopt 1995)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
