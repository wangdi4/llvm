; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -debug-only=hir-opt-predicate -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the side-effect analysis produces the correct
; results. The outer If should be identified as won't increase code size,
; while the inner should. It was created from the following test in C++:

; void foo(int *a, int *b, int n, int m, int p1, int p2) {
;   for (int i = 0; i < n; i++ ) {
;     int v1 = i + b[i];
;     int v2 = v1 * m;
;     if (p1 != 0) {
;       if (p2 == 3) {
;         for (int j = 0; j < m; j++) {
;           for (int k =0; k < n; k++) {
;             a[i * j + k] = v1;
;           }
;         }
;       } else {
;         for (int j = 0; j < m; j++) {
;           a[i * j] = b[i + j] + v2;
;         }
;       }
;     }
;   }
; }

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   %0 = (%b)[i1];
;       |   %1 = trunc.i64.i32(i1);
;       |   if (%p1 != 0)
;       |   {
;       |      if (%p2 == 3)
;       |      {
;       |         + DO i2 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |         |   + DO i3 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |         |   |   (%a)[zext.i32.i64(%1) * i2 + i3] = i1 + %0;
;       |         |   + END LOOP
;       |         + END LOOP
;       |      }
;       |      else
;       |      {
;       |         + DO i2 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |         |   %5 = (%b)[i1 + i2];
;       |         |   (%a)[zext.i32.i64(%1) * i2] = %5 + ((%0 + %1) * %m);
;       |         + END LOOP
;       |      }
;       |   }
;       + END LOOP
; END REGION

; Debug messages:

; CHECK: Opportunity: <7>          if (%p1 != 0) --> Level 0, Candidate: Yes
; CHECK:   - Code size will NOT increase, thresholds NOT needed
; CHECK: Opportunity: <11>         if (%p2 == 3) --> Level 0, Candidate: Yes
; CHECK:   - Code size will increase, thresholds needed

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%p1 != 0)
; CHECK:       {
; CHECK:          if (%p2 == 3)
; CHECK:          {
; CHECK:             + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:             |   %0 = (%b)[i1];
; CHECK:             |   %1 = trunc.i64.i32(i1);
; CHECK:             |   
; CHECK:             |   + DO i2 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:             |   |   + DO i3 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:             |   |   |   (%a)[zext.i32.i64(%1) * i2 + i3] = i1 + %0;
; CHECK:             |   |   + END LOOP
; CHECK:             |   + END LOOP
; CHECK:             + END LOOP
; CHECK:          }
; CHECK:          else
; CHECK:          {
; CHECK:             + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:             |   %0 = (%b)[i1];
; CHECK:             |   %1 = trunc.i64.i32(i1);
; CHECK:             |   
; CHECK:             |   + DO i2 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:             |   |   (%b)[i1 + i2] = ((%0 + %1) * %m);
; CHECK:             |   |   (%a)[zext.i32.i64(%1) * i2] = ((%0 + %1) * %m);
; CHECK:             |   + END LOOP
; CHECK:             + END LOOP
; CHECK:          }
; CHECK:       }
; CHECK: END REGION

; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local void @_Z3fooPiS_iiii(ptr nocapture noundef writeonly %a, ptr nocapture noundef %b, i32 noundef %n, i32 noundef %m, i32 noundef %p1, i32 noundef %p2) {
entry:
  %cmp63 = icmp sgt i32 %n, 0
  br i1 %cmp63, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp1.not = icmp eq i32 %p1, 0
  %cmp2 = icmp eq i32 %p2, 3
  %cmp2157 = icmp sgt i32 %m, 0
  %wide.trip.count81 = zext i32 %n to i64
  %wide.trip.count = zext i32 %m to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %if.end33
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %if.end33
  %indvars.iv79 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next80, %if.end33 ]
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %indvars.iv79
  %0 = load i32, ptr %arrayidx
  %1 = trunc i64 %indvars.iv79 to i32
  %add = add nsw i32 %0, %1
  %mul = mul nsw i32 %add, %m
  br i1 %cmp1.not, label %if.end33, label %if.then

if.then:                                          ; preds = %for.body
  br i1 %cmp2, label %for.cond4.preheader, label %for.cond20.preheader

for.cond20.preheader:                             ; preds = %if.then
  br i1 %cmp2157, label %for.body23.preheader, label %if.end33

for.body23.preheader:                             ; preds = %for.cond20.preheader
  br label %for.body23

for.cond4.preheader:                              ; preds = %if.then
  br i1 %cmp2157, label %for.body11.lr.ph.preheader, label %if.end33

for.body11.lr.ph.preheader:                       ; preds = %for.cond4.preheader
  br label %for.body11.lr.ph

for.body11.lr.ph:                                 ; preds = %for.body11.lr.ph.preheader, %for.cond.cleanup10
  %indvars.iv74 = phi i64 [ %indvars.iv.next75, %for.cond.cleanup10 ], [ 0, %for.body11.lr.ph.preheader ]
  %2 = mul nuw nsw i64 %indvars.iv74, %indvars.iv79
  br label %for.body11

for.cond.cleanup10:                               ; preds = %for.body11
  %indvars.iv.next75 = add nuw nsw i64 %indvars.iv74, 1
  %exitcond78.not = icmp eq i64 %indvars.iv.next75, %wide.trip.count
  br i1 %exitcond78.not, label %if.end33.loopexit, label %for.body11.lr.ph

for.body11:                                       ; preds = %for.body11.lr.ph, %for.body11
  %indvars.iv69 = phi i64 [ 0, %for.body11.lr.ph ], [ %indvars.iv.next70, %for.body11 ]
  %3 = add nuw nsw i64 %indvars.iv69, %2
  %arrayidx15 = getelementptr inbounds i32, ptr %a, i64 %3
  store i32 %add, ptr %arrayidx15
  %indvars.iv.next70 = add nuw nsw i64 %indvars.iv69, 1
  %exitcond73.not = icmp eq i64 %indvars.iv.next70, %wide.trip.count81
  br i1 %exitcond73.not, label %for.cond.cleanup10, label %for.body11

for.body23:                                       ; preds = %for.body23.preheader, %for.body23
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body23 ], [ 0, %for.body23.preheader ]
  %4 = add nuw nsw i64 %indvars.iv, %indvars.iv79
  %arrayidx26 = getelementptr inbounds i32, ptr %b, i64 %4
  store i32 %mul, ptr %arrayidx26
  %5 = mul nuw nsw i64 %indvars.iv, %indvars.iv79
  %arrayidx29 = getelementptr inbounds i32, ptr %a, i64 %5
  store i32 %mul, ptr %arrayidx29
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %if.end33.loopexit84, label %for.body23

if.end33.loopexit:                                ; preds = %for.cond.cleanup10
  br label %if.end33

if.end33.loopexit84:                              ; preds = %for.body23
  br label %if.end33

if.end33:                                         ; preds = %if.end33.loopexit84, %if.end33.loopexit, %for.cond20.preheader, %for.cond4.preheader, %for.body
  %indvars.iv.next80 = add nuw nsw i64 %indvars.iv79, 1
  %exitcond82.not = icmp eq i64 %indvars.iv.next80, %wide.trip.count81
  br i1 %exitcond82.not, label %for.cond.cleanup.loopexit, label %for.body
}
