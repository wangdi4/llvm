; REQUIRES: asserts
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -debug-only=hir-dd-test -aa-pipeline="basic-aa" -disable-output 2>&1 | FileCheck %s

; This test checks that the direction vector between (%a)[i1][i2] and
; (%a)[0][i2 + 1] is identified as (<= >). Then, the vector is selected as
; candidate for bidirection, which will generate (< >) as forward DV and
; (= <) as backward DV.

; HIR produced

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 99>  <LEGAL_MAX_TC = 2147483647>
;       |   |   (%a)[i1][i2] = (%a)[0][i2 + 1];
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK:  Src, Dst DDRefs
; CHECK: (%a)[0][i2 + 1],  (%a)[i1][i2]

; CHECK:  DV (<= >)
; CHECK:  DV reversed
; CHECK: BiDirection needed!

; CHECK: forward DV: (<= >)
; CHECK: backward DV: (= <)

; CHECK-DAG: (%a)[i1][i2] --> (%a)[0][i2 + 1] FLOW (= <)
; CHECK-DAG: (%a)[0][i2 + 1] --> (%a)[i1][i2] ANTI (<= >)

;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable
define dso_local void @_Z3fooPPiii(ptr nocapture noundef readonly %a, i32 noundef %n, i32 noundef %m) local_unnamed_addr #0 {
entry:
  %cmp21 = icmp sgt i32 %n, 0
  br i1 %cmp21, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp219 = icmp sgt i32 %n, 0
  %wide.trip.count = zext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup3
  %indvars.iv23 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next24, %for.cond.cleanup3 ]
  %indvars.iv.next24 = add nuw nsw i64 %indvars.iv23, 1
  br i1 %cmp219, label %for.body4.lr.ph, label %for.cond.cleanup3

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.cond.cleanup3.loopexit:                       ; preds = %for.body4
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.cond1.preheader
  %exitcond26.not = icmp eq i64 %indvars.iv.next24, %wide.trip.count
  br i1 %exitcond26.not, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4.lr.ph, %for.body4
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %for.body4 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds [100 x i32], ptr %a, i64 0, i64 %indvars.iv.next
  %0 = load i32, ptr %arrayidx5
  %arrayidx9 = getelementptr inbounds [100 x i32], ptr %a, i64 %indvars.iv23, i64 %indvars.iv
  store i32 %0, ptr %arrayidx9
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup3.loopexit, label %for.body4
}