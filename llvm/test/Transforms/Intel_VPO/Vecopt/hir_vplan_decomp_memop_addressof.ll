; Test HIR decomposition of memory operands for addressof operation.
; Input LLVM-IR generated for below C code with command: icx -O2 -print-module-before-loopopt

; int**  foo(int **a, int *b)
; {
;     int index;
;
; #pragma vector always
; #pragma ivdep
;     for (index = 0; index < 1024; index++) {
;         a[index] = &b[index];
;     }
;
;     return a;
; }

; Input HIR
; <11>      + DO i1 = 0, 1023, 1   <DO_LOOP> <vectorize> <ivdep>
; <4>       |   (%a)[i1] = &((%b)[i1]);
; <11>      + END LOOP


; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -vplan-print-after-plain-cfg -vplan-dump-subscript-details -disable-output < %s 2>&1 | FileCheck %s

; Check decomposed VPInstructions
; CHECK: i64 [[I1:%vp.*]] = phi
; CHECK-NEXT: ptr [[ADDR1:%vp.*]] = subscript inbounds ptr %b {i64 0 : i64 [[I1]] : i64 4 : ptr(i32)}
; CHECK-NEXT: ptr [[ADDR2:%vp.*]] = subscript inbounds ptr %a {i64 0 : i64 [[I1]] : i64 8 : ptr(ptr)}
; CHECK-NEXT: store ptr [[ADDR1]] ptr [[ADDR2]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local ptr @foo(ptr returned %a, ptr %b) local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  %arrayidx2 = getelementptr inbounds ptr, ptr %a, i64 %indvars.iv
  store ptr %arrayidx, ptr %arrayidx2, align 8, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !6

for.end:                                          ; preds = %for.body
  ret ptr %a
}


!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7, !8, !9}
!7 = !{!"llvm.loop.vectorize.ivdep_back"}
!8 = !{!"llvm.loop.vectorize.ignore_profitability"}
!9 = !{!"llvm.loop.vectorize.enable", i1 true}
