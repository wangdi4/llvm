; REQUIRES: asserts
; RUN: opt < %s -qsortrecognizer -debug-only=qsortrecognizer -qsort-unit-test -qsort-test-min -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(qsortrecognizer)' -debug-only=qsortrecognizer -qsort-unit-test -qsort-test-min -disable-output 2>&1 | FileCheck %s

; Check that the computation of MIN is recognized.

; CHECK: QsortRec: Checking computation of MIN in qsort_min
; CHECK: QsortRec: Computation of MIN in qsort_min PASSED Test.
; CHECK: QsortRec: Checking computation of MIN in qsort_min
; CHECK: QsortRec: Computation of MIN in qsort_min PASSED Test.
; CHECK: FOUND QSORT

; ModuleID = 'qsort-min01.ll'
source_filename = "qsort-min01.ll"

define internal fastcc void @qsort_min(i8* %arg, i64 %arg1) unnamed_addr {
bb:
  %tmp = ptrtoint i8* %arg to i64
  %tmp1 = icmp ult i64 %arg1, 7
  br i1 %tmp1, label %bb2, label %bb_exit

bb2:                                              ; preds = %bb5, %bb
  %tmp2 = phi i64 [ %tmp8, %bb5 ], [ %tmp, %bb ]
  %tmp3 = phi i8* [ %tmp7, %bb5 ], [ %arg, %bb ]
  br label %bb3

bb3:                                              ; preds = %bb2
  %tmp4 = getelementptr inbounds i8, i8* %tmp3, i64 8
  %tmp5 = shl i64 %arg1, 3
  %tmp6 = add i64 %tmp5, -8
  %tmp7 = getelementptr inbounds i8, i8* %tmp3, i64 %tmp6
  br label %bb4

bb4:                                              ; preds = %bb3
  %tmp8 = ptrtoint i8* %tmp4 to i64
  %tmp9 = sub i64 %tmp8, %tmp2
  %tmp10 = ptrtoint i8* %tmp7 to i64
  %tmp11 = sub i64 %tmp10, %tmp8
  %tmp12 = icmp slt i64 %tmp9, %tmp11
  %tmp13 = select i1 %tmp12, i64 %tmp9, i64 %tmp11
  %tmp14 = icmp eq i64 %tmp13, 0
  br i1 %tmp14, label %bb5, label %bb_exit

bb5:                                              ; preds = %bb4
  %tmp15 = ptrtoint i8* %tmp4 to i64
  %tmp16 = ptrtoint i8* %tmp7 to i64
  %tmp17 = sub i64 %tmp15, %tmp16
  %tmp18 = ptrtoint i8* %tmp7 to i64
  %tmp19 = sub i64 %tmp18, %tmp15
  %tmp20 = add i64 %tmp19, -8
  %tmp21 = icmp slt i64 %tmp17, %tmp20
  %tmp22 = select i1 %tmp21, i64 %tmp17, i64 %tmp20
  %tmp23 = icmp eq i64 %tmp22, 0
  br i1 %tmp23, label %bb2, label %bb_exit

bb_exit:                                          ; preds = %bb5, %bb4, %bb
  ret void
}
