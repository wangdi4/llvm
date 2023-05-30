; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -debug-only=hir-opt-predicate -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the If in the nested loops was identified that
; won't increase code size since the else branch in the inner loop won't have
; a side effect (no temp live out, no side effect instructions, no stores).
; It was created from the following test in C++:

; void foo(int *a, int n, int m, int l) {
;   for (int i = 0; i < n; i++) {
;     int temp1 = 0;
;     for(int j = 0; j < m; j++) {
;       if (l == 5) {
;         a[i * n + j] = temp1;
;       } else {
;         temp1++;
;       }
;     }
;   }
; }

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |      %temp1.017 = 0;
;       |   + DO i2 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   |   if (%l == 5)
;       |   |   {
;       |   |      (%a)[zext.i32.i64(%n) * i1 + i2] = %temp1.017;
;       |   |   }
;       |   |   else
;       |   |   {
;       |   |      %temp1.017 = %temp1.017  +  1;
;       |   |   }
;       |   + END LOOP
;       + END LOOP
; END REGION

; Debug trace

; CHECK: Opportunity: <12>         if (%l == 5) --> Level 0, Candidate: Yes
; CHECK:   - Code size will NOT increase, thresholds NOT needed

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%l == 5)
; CHECK:       {
; CHECK:          + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:          |      %temp1.017 = 0;
; CHECK:          |   + DO i2 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:          |   |   (%a)[zext.i32.i64(%n) * i1 + i2] = %temp1.017;
; CHECK:          |   + END LOOP
; CHECK:          + END LOOP
; CHECK:       }
; CHECK: END REGION

;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(argmem: write) uwtable
define dso_local void @_Z3fooPiiii(ptr nocapture noundef writeonly %a, i32 noundef %n, i32 noundef %m, i32 noundef %l) local_unnamed_addr #0 {
entry:
  %cmp19 = icmp sgt i32 %n, 0
  br i1 %cmp19, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp216 = icmp sgt i32 %m, 0
  %cmp5 = icmp eq i32 %l, 5
  %0 = zext i32 %n to i64
  %wide.trip.count = zext i32 %m to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup3
  %indvars.iv22 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next23, %for.cond.cleanup3 ]
  br i1 %cmp216, label %for.body4.lr.ph, label %for.cond.cleanup3

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader
  %1 = mul nsw i64 %indvars.iv22, %0
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.cond.cleanup3.loopexit:                       ; preds = %for.inc
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.cond1.preheader
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond26.not = icmp eq i64 %indvars.iv.next23, %0
  br i1 %exitcond26.not, label %for.cond.cleanup.loopexit, label %for.cond1.preheader, !llvm.loop !3

for.body4:                                        ; preds = %for.body4.lr.ph, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %temp1.017 = phi i32 [ 0, %for.body4.lr.ph ], [ %temp1.1, %for.inc ]
  br i1 %cmp5, label %if.then, label %if.else

if.then:                                          ; preds = %for.body4
  %2 = add nuw nsw i64 %indvars.iv, %1
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %2
  store i32 %temp1.017, ptr %arrayidx, align 4, !tbaa !5
  br label %for.inc

if.else:                                          ; preds = %for.body4
  %inc = add nsw i32 %temp1.017, 1
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %temp1.1 = phi i32 [ %temp1.017, %if.then ], [ %inc, %if.else ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup3.loopexit, label %for.body4, !llvm.loop !9
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind memory(argmem: write) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.mustprogress"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = distinct !{!9, !4}
