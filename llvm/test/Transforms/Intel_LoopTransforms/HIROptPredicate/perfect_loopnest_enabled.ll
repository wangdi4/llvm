; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" < %s -disable-output -xmain-opt-level=3 < %s 2>&1 | FileCheck %s

; This test case checks that loop unswitch was applied to create a perfect
; loopnest with a small count of nested loops when the condition is loop
; invariant.

; Original C/C++ loopnest:

; void foo(int *a, int n, int m, int q, int p) {
;   for(int i = 0; i < n; i++) {
;     for (int j= 0; j < m; j++) {
;       for (int k = 0; k < q; k++) {
;         if (q < 50)
;           a[k]+= j;
;         else
;           a[k]+= i;
;       }
;     }
;   }
; }


; Original HIR

; Function: _Z3fooPiiiii
;    BEGIN REGION { }
;          + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;          |   + DO i2 = 0, %m + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;          |   |   + DO i3 = 0, zext.i32.i64(%q) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;          |   |   |   if (%q < 50)
;          |   |   |   {
;          |   |   |      %0 = (%a)[i3];
;          |   |   |      (%a)[i3] = i2 + %0;
;          |   |   |   }
;          |   |   |   else
;          |   |   |   {
;          |   |   |      %1 = (%a)[i3];
;          |   |   |      (%a)[i3] = i1 + %1;
;          |   |   |   }
;          |   |   + END LOOP
;          |   + END LOOP
;          + END LOOP
;    END REGION

; HIR after loop unswitch

; CHECK:  BEGIN REGION { modified }
; CHECK:        if (%q < 50)
; CHECK:        {
; CHECK:           + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |   + DO i2 = 0, %m + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |   |   + DO i3 = 0, zext.i32.i64(%q) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |   |   |   %0 = (%a)[i3];
; CHECK:           |   |   |   (%a)[i3] = i2 + %0;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:        }
; CHECK:        else
; CHECK:        {
; CHECK:           + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |   + DO i2 = 0, %m + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |   |   + DO i3 = 0, zext.i32.i64(%q) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |   |   |   %1 = (%a)[i3];
; CHECK:           |   |   |   (%a)[i3] = i1 + %1;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:        }
; CHECK:  END REGION


define dso_local void @_Z3fooPiiiii(i32* nocapture noundef %a, i32 noundef %n, i32 noundef %m, i32 noundef %q, i32 noundef %p) {
entry:
  %cmp35 = icmp sgt i32 %n, 0
  br i1 %cmp35, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp233 = icmp sgt i32 %m, 0
  %cmp631 = icmp sgt i32 %q, 0
  %cmp9 = icmp slt i32 %q, 50
  %wide.trip.count = zext i32 %q to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup3
  %i.036 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc17, %for.cond.cleanup3 ]
  br i1 %cmp233, label %for.cond5.preheader.preheader, label %for.cond.cleanup3

for.cond5.preheader.preheader:                    ; preds = %for.cond1.preheader
  br label %for.cond5.preheader

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.cond5.preheader:                              ; preds = %for.cond5.preheader.preheader, %for.cond.cleanup7
  %j.034 = phi i32 [ %inc14, %for.cond.cleanup7 ], [ 0, %for.cond5.preheader.preheader ]
  br i1 %cmp631, label %for.body8.preheader, label %for.cond.cleanup7

for.body8.preheader:                              ; preds = %for.cond5.preheader
  br label %for.body8

for.cond.cleanup3.loopexit:                       ; preds = %for.cond.cleanup7
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.cond1.preheader
  %inc17 = add nuw nsw i32 %i.036, 1
  %exitcond38.not = icmp eq i32 %inc17, %n
  br i1 %exitcond38.not, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.cond.cleanup7.loopexit:                       ; preds = %for.inc
  br label %for.cond.cleanup7

for.cond.cleanup7:                                ; preds = %for.cond.cleanup7.loopexit, %for.cond5.preheader
  %inc14 = add nuw nsw i32 %j.034, 1
  %exitcond37.not = icmp eq i32 %inc14, %m
  br i1 %exitcond37.not, label %for.cond.cleanup3.loopexit, label %for.cond5.preheader

for.body8:                                        ; preds = %for.body8.preheader, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body8.preheader ]
  br i1 %cmp9, label %if.then, label %if.else

if.then:                                          ; preds = %for.body8
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %0, %j.034
  store i32 %add, i32* %arrayidx, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body8
  %arrayidx11 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx11, align 4
  %add12 = add nsw i32 %1, %i.036
  store i32 %add12, i32* %arrayidx11, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup7.loopexit, label %for.body8
}
