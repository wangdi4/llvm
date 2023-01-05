; Sanity Test(s) on HIR Loop Reversal: simple l1 loop that CAN'T be reversed
;
; l1reversal7-iv.ll:
; 1-level loop, sanity testcase7, invalid reversal case
;
; [REASONS]
; - Applicalbe: NO
;   non linear arithmatic on IV;
;   (E.g. i*i appears on CanonExpr on Array's sub index)
; - Profitable: N/A
; - Legal:      N/A
;
;
; *** Source Code ***
;
; [BEFORE LOOP REVERSAL]
;
;int foo(int A[100], int B[100]) {
;  int i = 0;
;  for (i = 0; i <= 4; i++) {
;    A[100 - i * i] = B[80 - 2 * i];
;  }
;  return A[1] + B[1]+1;
;}
;
; [AFTER LOOP REVERSAL: NO REVERSAL WILL HAPPEN!]
;
;int foo(int A[100], int B[100]) {
;  int i = 0;
;  for (i = 0; i <= 4; i++) {
;    A[100 - i * i] = B[80 - 2 * i];
;  }
;  return A[1] + B[1]+1;
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
;<16>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<5>          |   %2 = (%B)[-2 * i1 + 80];
;<6>          |   %3 = i1  *  i1;
;<7>          |   %4 = 100  -  %3;
;<9>          |   (%A)[%4] = %2;
;<16>         + END LOOP
;          END REGION
;
;          BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 4, 1   <DO_LOOP>
; BEFORE:     |   %2 = (%B)[-2 * i1 + 80];
; BEFORE:     |   %3 = i1  *  i1;
; BEFORE:     |   %4 = 100  -  %3;
; BEFORE:     |   (%A)[%4] = %2;
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
;          BEGIN REGION { }
;<16>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<5>          |   %2 = (%B)[-2 * i1 + 80];
;<6>          |   %3 = i1  *  i1;
;<7>          |   %4 = 100  -  %3;
;<9>          |   (%A)[%4] = %2;
;<16>         + END LOOP
;          END REGION
;
;          BEGIN REGION { }
; AFTER:     + DO i1 = 0, 4, 1   <DO_LOOP>
; AFTER:     |   %2 = (%B)[-2 * i1 + 80];
; AFTER:     |   %3 = i1  *  i1;
; AFTER:     |   %4 = 100  -  %3;
; AFTER:     |   (%A)[%4] = %2;
; AFTER:     + END LOOP
; AFTER:  END REGION
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i32* nocapture %A, i32* nocapture readonly %B) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = shl nsw i64 %indvars.iv, 1
  %1 = sub nuw nsw i64 80, %0
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %1
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %3 = mul nsw i64 %indvars.iv, %indvars.iv
  %4 = sub nsw i64 100, %3
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
  %add = add i32 %5, 1
  %add7 = add i32 %add, %6
  ret i32 %add7
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 2039) (llvm/branches/loopopt 2048)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
