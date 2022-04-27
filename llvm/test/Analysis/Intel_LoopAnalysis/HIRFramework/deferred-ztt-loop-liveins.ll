; RUN: opt < %s -analyze -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework -hir-details 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -hir-details 2>&1 | FileCheck %s

; Verify that %t1's symbase is marked as livein to i2 loop when (%t1 > 0) is set as the ztt.

; CHECK: + DO i64 i1 = 0, zext.i32.i64(%t2) + -1, 1   <DO_LOOP>
; CHECK: |
; CHECK: |   + Ztt: if (%t1 > 0)
; CHECK: |   + LiveIn symbases: [[T1SB:[0-9]+]]
; CHECK: |   + DO i64 i2 = 0, zext.i32.i64(%t7), 1   <DO_LOOP>  <MAX_TC_EST = 1073741824>
; CHECK: |   | <ZTT-REG> LINEAR i32 %t1 {sb:[[T1SB]]}
; CHECK: |   |
; CHECK: |   |   (%ptr)[0] = 4 * i2;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


define void @foo(i1 %cmp, i32 %t1, i32 %t2, i64* %ptr) {
entry:
  %cmp8123 = icmp sgt i32 %t1, 0
  %t5 = add i32 %t1, -1
  %.v159 = select i1 %cmp, i32 2, i32 3
  %t7 = lshr i32 %t5, %.v159
  %t8 = add nuw nsw i32 %t7, 1
  %wide.trip.count154 = zext i32 %t8 to i64
  %.v = select i1 %cmp, i32 2, i32 3
  %wide.trip.count157 = zext i32 %t2 to i64
  br label %for.outer

for.outer:                              ; preds = %for.inc29, %entry
  %indvars.iv150 = phi i64 [ 0, %entry ], [ %indvars.iv.next151.pre-phi, %for.inc29 ]
  %indvar = phi i64 [ 0, %entry ], [ %indvar.next, %for.inc29 ]
  br i1 %cmp8123, label %for.inner.lr.ph, label %for.outer.for.inc29_crit_edge

for.outer.for.inc29_crit_edge:          ; preds = %for.outer
  %.pre = add nuw nsw i64 %indvars.iv150, 4
  br label %for.inc29

for.inner.lr.ph:                       ; preds = %for.outer
  %t12 = add nuw nsw i64 %indvars.iv150, 4
  br label %for.inner

for.inner:                             ; preds = %for.inner, %for.inner.lr.ph
  %indvar146 = phi i64 [ 0, %for.inner.lr.ph ], [ %indvar.next147, %for.inner ]
  %indvars.iv131 = phi i64 [ 0, %for.inner.lr.ph ], [ %indvars.iv.next132, %for.inner ]
  %t25 = mul nuw nsw i64 %indvar146, 4
  store i64 %t25, i64* %ptr, align 8
  %indvars.iv.next132 = add nuw nsw i64 %indvars.iv131, 4
  %indvar.next147 = add nuw nsw i64 %indvar146, 1
  %exitcond155 = icmp eq i64 %indvar.next147, %wide.trip.count154
  br i1 %exitcond155, label %for.inc29.loopexit, label %for.inner

for.inc29.loopexit:                               ; preds = %for.inner
  br label %for.inc29

for.inc29:                                        ; preds = %for.inc29.loopexit, %for.outer.for.inc29_crit_edge
  %indvars.iv.next151.pre-phi = phi i64 [ %.pre, %for.outer.for.inc29_crit_edge ], [ %t12, %for.inc29.loopexit ]
  %indvar.next = add nuw nsw i64 %indvar, 1
  %exitcond158 = icmp eq i64 %indvar.next, %wide.trip.count157
  br i1 %exitcond158, label %loopexit, label %for.outer

loopexit:
  ret void
}
