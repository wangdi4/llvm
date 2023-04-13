; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the transformation was applied since the there is
; no an else branch for the If condition. Since there is no else branche, the
; if won't have a side effect, therefore the code size won't increase. It was
; created from the following test in C++:

; void foo(int *a, int *b, int n, int m, int p, int p1) {
;   for (int i = 0; i < n; i++ ) {
;     int v1 = i + b[i];
;     if (p1 != 0) {
;       for (int j = 0; j < m; j++) {
;         for (int k =0; k < n; k++) {
;          for (int l = 0; l < p; l++) {
;            a[i * j * l + k] = v1;
;          }
;         }
;       }
;     }
;   }
; }

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   %0 = (%b)[i1];
;       |   if (%p1 != 0)
;       |   {
;       |      + DO i2 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |      |   %2 = i1  *  i2;
;       |      |   
;       |      |   + DO i3 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |      |   |   + DO i4 = 0, zext.i32.i64(%p) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |      |   |   |   (%a)[i3 + %2 * i4] = i1 + %0;
;       |      |   |   + END LOOP
;       |      |   + END LOOP
;       |      + END LOOP
;       |   }
;       + END LOOP
; END REGION


; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       if (%p1 != 0)
; CHECK:       {
; CHECK:          + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:          |   %0 = (%b)[i1];
; CHECK:          |   
; CHECK:          |   + DO i2 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:          |   |   %2 = i1  *  i2;
; CHECK:          |   |   
; CHECK:          |   |   + DO i3 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:          |   |   |   + DO i4 = 0, zext.i32.i64(%p) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:          |   |   |   |   (%a)[i3 + %2 * i4] = i1 + %0;
; CHECK:          |   |   |   + END LOOP
; CHECK:          |   |   + END LOOP
; CHECK:          |   + END LOOP
; CHECK:          + END LOOP
; CHECK:       }
; CHECK: END REGION


;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooPiS_iiii(ptr nocapture noundef writeonly %a, ptr nocapture noundef readonly %b, i32 noundef %n, i32 noundef %m, i32 noundef %p, i32 noundef %p1) {
entry:
  %cmp48 = icmp sgt i32 %n, 0
  br i1 %cmp48, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp1.not = icmp eq i32 %p1, 0
  %cmp346 = icmp sgt i32 %m, 0
  %cmp1142 = icmp sgt i32 %p, 0
  %wide.trip.count64 = zext i32 %n to i64
  %wide.trip.count60 = zext i32 %m to i64
  %wide.trip.count = zext i32 %p to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %if.end
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %if.end
  %indvars.iv62 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next63, %if.end ]
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %indvars.iv62
  %0 = load i32, ptr %arrayidx
  %1 = trunc i64 %indvars.iv62 to i32
  %add = add nsw i32 %0, %1
  br i1 %cmp1.not, label %if.end, label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.body
  br i1 %cmp346, label %for.cond10.preheader.lr.ph.preheader, label %if.end

for.cond10.preheader.lr.ph.preheader:             ; preds = %for.cond2.preheader
  br label %for.cond10.preheader.lr.ph

for.cond10.preheader.lr.ph:                       ; preds = %for.cond10.preheader.lr.ph.preheader, %for.cond.cleanup8
  %indvars.iv57 = phi i64 [ %indvars.iv.next58, %for.cond.cleanup8 ], [ 0, %for.cond10.preheader.lr.ph.preheader ]
  %2 = mul nuw nsw i64 %indvars.iv62, %indvars.iv57
  br label %for.cond10.preheader

for.cond10.preheader:                             ; preds = %for.cond10.preheader.lr.ph, %for.cond.cleanup12
  %indvars.iv53 = phi i64 [ 0, %for.cond10.preheader.lr.ph ], [ %indvars.iv.next54, %for.cond.cleanup12 ]
  br i1 %cmp1142, label %for.body13.preheader, label %for.cond.cleanup12

for.body13.preheader:                             ; preds = %for.cond10.preheader
  br label %for.body13

for.cond.cleanup8:                                ; preds = %for.cond.cleanup12
  %indvars.iv.next58 = add nuw nsw i64 %indvars.iv57, 1
  %exitcond61.not = icmp eq i64 %indvars.iv.next58, %wide.trip.count60
  br i1 %exitcond61.not, label %if.end.loopexit, label %for.cond10.preheader.lr.ph

for.cond.cleanup12.loopexit:                      ; preds = %for.body13
  br label %for.cond.cleanup12

for.cond.cleanup12:                               ; preds = %for.cond.cleanup12.loopexit, %for.cond10.preheader
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond56.not = icmp eq i64 %indvars.iv.next54, %wide.trip.count64
  br i1 %exitcond56.not, label %for.cond.cleanup8, label %for.cond10.preheader

for.body13:                                       ; preds = %for.body13.preheader, %for.body13
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body13 ], [ 0, %for.body13.preheader ]
  %3 = mul nuw nsw i64 %2, %indvars.iv
  %4 = add nuw nsw i64 %3, %indvars.iv53
  %arrayidx17 = getelementptr inbounds i32, ptr %a, i64 %4
  store i32 %add, ptr %arrayidx17
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup12.loopexit, label %for.body13

if.end.loopexit:                                  ; preds = %for.cond.cleanup8
  br label %if.end

if.end:                                           ; preds = %if.end.loopexit, %for.cond2.preheader, %for.body
  %indvars.iv.next63 = add nuw nsw i64 %indvars.iv62, 1
  %exitcond65.not = icmp eq i64 %indvars.iv.next63, %wide.trip.count64
  br i1 %exitcond65.not, label %for.cond.cleanup.loopexit, label %for.body
}
