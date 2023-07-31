; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" < %s -disable-output -xmain-opt-level=3 < %s 2>&1 | FileCheck %s

; This test case checks that loop unswitch was applied for the condition
; "if (m > 1024)", but the loop wasn't marked as modified. The reason is
; that the HIR code gen won't transform a loop, unless the unswitching
; benefits another transformation.

; Original C/C++ loopnest:

; void foo(int *a, int n, int m) {
;   for(int i = 0; i < n; i++) {
;     if (m > 1024) {
;       for (int j = 0; j < 10; j++) {
;         int x = a[j];
;         a[j] = x + 1;
;         a[j] += x + 3;
;       }
;     }
;
;     int y = a[i];
;     a[i] = y + 1;
;   }
; }

; Original HIR
;Function: _Z3fooPiii
;         BEGIN REGION { }
;               + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;               |   if (%m > 1024)
;               |   {
;               |      + DO i2 = 0, 9, 1   <DO_LOOP>
;               |      |   %0 = (%a)[i2];
;               |      |   (%a)[i2] = 2 * %0 + 4;
;               |      + END LOOP
;               |   }
;               |   %1 = (%a)[i1];
;               |   (%a)[i1] = %1 + 1;
;               + END LOOP
;         END REGION

; HIR must change, but shouldn't be marked as modified.
; CHECK:   BEGIN REGION { }
; CHECK:         if (%m > 1024)
; CHECK:         {
; CHECK:            + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:            |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:            |   |   %0 = (%a)[i2];
; CHECK:            |   |   (%a)[i2] = 2 * %0 + 4;
; CHECK:            |   + END LOOP
; CHECK:            |
; CHECK:            |   %1 = (%a)[i1];
; CHECK:            |   (%a)[i1] = %1 + 1;
; CHECK:            + END LOOP
; CHECK:         }
; CHECK:         else
; CHECK:         {
; CHECK:            + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:            |   %1 = (%a)[i1];
; CHECK:            |   (%a)[i1] = %1 + 1;
; CHECK:            + END LOOP
; CHECK:         }
; CHECK:   END REGION

define dso_local void @_Z3fooPiii(ptr nocapture noundef %a, i32 noundef %n, i32 noundef %m) {
entry:
  %cmp37 = icmp sgt i32 %n, 0
  br i1 %cmp37, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp1 = icmp sgt i32 %m, 1024
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %if.end
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %if.end
  %indvars.iv39 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next40, %if.end ]
  br i1 %cmp1, label %for.body5.preheader, label %if.end

for.body5.preheader:                              ; preds = %for.body
  br label %for.body5

for.body5:                                        ; preds = %for.body5.preheader, %for.body5
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body5 ], [ 0, %for.body5.preheader ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx
  %add8 = shl i32 %0, 1
  %add11 = add i32 %add8, 4
  store i32 %add11, ptr %arrayidx
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond.not, label %if.end.loopexit, label %for.body5

if.end.loopexit:                                  ; preds = %for.body5
  br label %if.end

if.end:                                           ; preds = %if.end.loopexit, %for.body
  %arrayidx13 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv39
  %1 = load i32, ptr %arrayidx13
  %add14 = add nsw i32 %1, 1
  store i32 %add14, ptr %arrayidx13
  %indvars.iv.next40 = add nuw nsw i64 %indvars.iv39, 1
  %exitcond41.not = icmp eq i64 %indvars.iv.next40, %wide.trip.count
  br i1 %exitcond41.not, label %for.cond.cleanup.loopexit, label %for.body
}