; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-normalize-casts,print<hir>" %s -disable-output 2>&1 | FileCheck %s

; This test case checks that the zext casting in the loops upperbound was
; converted into sext since the operands were found in a memref with sext.
; In this case all the nested loops have the same upper bound (%n) and there
; is only one memref for %n. It was created from the following C++ code, but
; the casting was added in the IR:

; unsigned foo(unsigned *a, int n, int m) {
;   unsigned res = 0;
;   for (int i = 0; i < n; i++ ) {
;     for (int j = 0; j < n; j++) {
;       res += a[m * i + n *j];
;     }
;   }
;   return res;
; }

; HIR before transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   |   %5 = (%a)[zext.i32.i64(%m) * i1 + sext.i32.i64(%n) * i2];
; CHECK:       |   |   %res.024 = %5  +  %res.024;
; CHECK:       |   + END LOOP
; CHECK:       |
; CHECK:       |   %res.024.out = %res.024;
; CHECK:       + END LOOP
; CHECK: END REGION

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   |   %5 = (%a)[zext.i32.i64(%m) * i1 + sext.i32.i64(%n) * i2];
; CHECK:       |   |   %res.024 = %5  +  %res.024;
; CHECK:       |   + END LOOP
; CHECK:       |
; CHECK:       |   %res.024.out = %res.024;
; CHECK:       + END LOOP
; CHECK: END REGION

;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind readonly willreturn uwtable
define dso_local noundef i32 @_Z3fooPjii(ptr nocapture noundef readonly %a, i32 noundef %n, i32 noundef %m) {
entry:
  %cmp23 = icmp sgt i32 %n, 0
  br i1 %cmp23, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp220 = icmp sgt i32 %n, 0
  %0 = sext i32 %n to i64
  %1 = zext i32 %m to i64
  %ub0 = zext i32 %n to i64
  %ub1 = zext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup3
  %indvars.iv29 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next30, %for.cond.cleanup3 ]
  %res.024 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %res.1.lcssa, %for.cond.cleanup3 ]
  br i1 %cmp220, label %for.body4.lr.ph, label %for.cond.cleanup3

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader
  %2 = mul nsw i64 %indvars.iv29, %1
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  %res.1.lcssa.lcssa = phi i32 [ %res.1.lcssa, %for.cond.cleanup3 ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %res.0.lcssa = phi i32 [ 0, %entry ], [ %res.1.lcssa.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %res.0.lcssa

for.cond.cleanup3.loopexit:                       ; preds = %for.body4
  %add6.lcssa = phi i32 [ %add6, %for.body4 ]
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.cond1.preheader
  %res.1.lcssa = phi i32 [ %res.024, %for.cond1.preheader ], [ %add6.lcssa, %for.cond.cleanup3.loopexit ]
  %indvars.iv.next30 = add nuw nsw i64 %indvars.iv29, 1
  %exitcond33.not = icmp eq i64 %indvars.iv.next30, %ub1
  br i1 %exitcond33.not, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4.lr.ph, %for.body4
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %for.body4 ]
  %res.121 = phi i32 [ %res.024, %for.body4.lr.ph ], [ %add6, %for.body4 ]
  %3 = mul nsw i64 %indvars.iv, %0
  %4 = add nsw i64 %3, %2
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %4
  %5 = load i32, ptr %arrayidx
  %add6 = add i32 %5, %res.121
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %ub0
  br i1 %exitcond.not, label %for.cond.cleanup3.loopexit, label %for.body4
}
