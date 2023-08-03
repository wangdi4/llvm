; RUN: opt -hir-details -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that the case does not execute an assert.
; Note: test runs hir-opt-predicate twice, this is to simulate regular compiler pipeline and to reproduce the issue.

; BEGIN REGION { }
;       + DO i1 = 0, 0, 1   <DO_MULTI_EXIT_LOOP>
;       |   %tmp4 = 0;
;       |   if (undef != 0)
;       |   {
;       |      %tmp4 = (undef)[0];
;       |   }
;       |   %tmp9.in2 = 0.000000e+00;
;       |   if (undef != 0 && %tmp4 > 0)
;       |   {
;       |      %tmp9.in2 = undef;
;       |   }
;       |   %tmp12.in3 = 0.000000e+00;
;       |   if (%tmp4 > 0)
;       |   {
;       |      %tmp12.in3 = undef;
;       |   }
;       |   if (undef true undef && %tmp4 > 0)
;       |   {
;       |   }
;       |   else
;       |   {
;       |      goto bb16;
;       |   }
;       + END LOOP
; END REGION

; CHECK: modified

define void @blam() {
bb:
  br label %bb1

bb1:                                              ; preds = %bb14, %bb
  br i1 undef, label %bb2, label %bb3

bb2:                                              ; preds = %bb1
  %tmp = load i32, ptr undef, align 4
  br label %bb3

bb3:                                              ; preds = %bb2, %bb1
  %tmp4 = phi i32 [ %tmp, %bb2 ], [ 0, %bb1 ]
  %tmp5 = icmp sgt i32 %tmp4, 0
  %tmp6 = and i1 undef, %tmp5
  br i1 %tmp6, label %bb7, label %bb8

bb7:                                              ; preds = %bb3
  br label %bb8

bb8:                                              ; preds = %bb7, %bb3
  %tmp9 = phi double [ undef, %bb7 ], [ 0.000000e+00, %bb3 ]
  br i1 %tmp5, label %bb10, label %bb11

bb10:                                             ; preds = %bb8
  br label %bb11

bb11:                                             ; preds = %bb10, %bb8
  %tmp12 = phi double [ undef, %bb10 ], [ 0.000000e+00, %bb8 ]
  %tmp13 = and i1 true, %tmp5
  br i1 %tmp13, label %bb14, label %bb16

bb14:                                             ; preds = %bb11
  %tmp15 = icmp eq i64 undef, undef
  br i1 %tmp15, label %bb17, label %bb1

bb16:                                             ; preds = %bb11
  unreachable

bb17:                                             ; preds = %bb14
  unreachable
}
