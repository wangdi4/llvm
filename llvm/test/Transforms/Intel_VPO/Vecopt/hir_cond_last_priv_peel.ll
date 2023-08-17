; RUN: opt %s -disable-output -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-vec-scenario="s1;v2;n0" 2>&1 | FileCheck %s

; CHECK:      + DO i2 = 0, %peel.ub, 1   <DO_LOOP>  <MAX_TC_EST = 1>  <LEGAL_MAX_TC = 1> <vector-peel> <nounroll> <novectorize> <max_trip_count = 1>
; CHECK-NEXT: |   if (%cond != 0)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      %i = 0;
; CHECK-NEXT: |   }
; CHECK-NEXT: + END LOOP
;
; CHECK: %phi.temp = %i;

define void @foo(i1 %cond) {
bb:
  br label %bb1

bb1:                                              ; preds = %bb8, %bb
  %i = phi i32 [ %i6, %bb8 ], [ 0, %bb ]
  br label %bb2

bb2:                                              ; preds = %bb5, %bb1
  %i3 = phi i32 [ %i, %bb1 ], [ %i3, %bb5 ]
  br i1 %cond, label %bb4, label %bb5

bb4:                                              ; preds = %bb2
  br label %bb5

bb5:                                              ; preds = %bb4, %bb2
  %i6 = phi i32 [ 0, %bb4 ], [ %i3, %bb2 ]
  %i7 = icmp eq i64 0, 0
  br i1 %i7, label %bb8, label %bb2

bb8:                                              ; preds = %bb5
  store i32 %i, ptr null, align 8
  %i9 = icmp eq i64 0, 0
  br i1 %i9, label %bb10, label %bb1

bb10:                                             ; preds = %bb8
  ret void
}
