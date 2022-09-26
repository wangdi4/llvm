; Test HIR decomposition of memory operands for simple vector addition.
; Input LLVM-IR generated for below C code with command: icx -O2 -print-module-before-loopopt

; int*  foo(int *a, int *b, int *c)
; {
;     int index;
;
; #pragma vector always
; #pragma ivdep
;     for (index = 0; index < 1024; index++) {
;         a[index] = b[index] + c[index];
;     }
;
;     return a;
; }

; Input HIR
; <15>      + DO i1 = 0, 1023, 1   <DO_LOOP> <vectorize> <ivdep>
; <3>       |   %0 = (%b)[i1];
; <5>       |   %1 = (%c)[i1];
; <8>       |   (%a)[i1] = %0 + %1;
; <15>      + END LOOP


; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-print-after-plain-cfg -vplan-dump-subscript-details -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -vplan-print-after-plain-cfg -vplan-dump-subscript-details -disable-output < %s 2>&1 | FileCheck %s

; Check decomposed VPInstructions
; CHECK: i64 [[I1:%vp.*]] = phi
; CHECK-NEXT: i32* [[ADDR1:%vp.*]] = subscript inbounds i32* %b {i64 0 : i64 [[I1]] : i64 4 : i32*(i32)}
; CHECK-NEXT: i32 [[LOAD1:%vp.*]] = load i32* [[ADDR1]]
; CHECK-NEXT: i32* [[ADDR2:%vp.*]] = subscript inbounds i32* %c {i64 0 : i64 [[I1]] : i64 4 : i32*(i32)}
; CHECK-NEXT: i32 [[LOAD2:%vp.*]] = load i32* [[ADDR2]]
; CHECK-NEXT: i32 [[ADD:%vp.*]] = add i32 [[LOAD1]] i32 [[LOAD2]]
; CHECK-NEXT: i32* [[ADDR3:%vp.*]] = subscript inbounds i32* %a {i64 0 : i64 [[I1]] : i64 4 : i32*(i32)}
; CHECK-NEXT: store i32 [[ADD]] i32* [[ADDR3]]


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local i32* @foo(i32* returned %a, i32* nocapture readonly %b, i32* nocapture readonly %c) local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %arrayidx2 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4, !tbaa !2
  %add = add nsw i32 %1, %0
  %arrayidx4 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  store i32 %add, i32* %arrayidx4, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !6

for.end:                                          ; preds = %for.body
  ret i32* %a
}


!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7, !8, !9}
!7 = !{!"llvm.loop.vectorize.ivdep_back"}
!8 = !{!"llvm.loop.vectorize.ignore_profitability"}
!9 = !{!"llvm.loop.vectorize.enable", i1 true}
