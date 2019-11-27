; REQUIRES: asserts
; RUN: opt < %s -qsortrecognizer -debug-only=qsortrecognizer -qsort-unit-test -qsort-test-min -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(qsortrecognizer)' -debug-only=qsortrecognizer -qsort-unit-test -qsort-test-min -disable-output 2>&1 | FileCheck %s

; Check that the computation of MIN is recognized, but didn't pass the test
; because there is only one MIN.

; CHECK: QsortRec: Checking computation of MIN in qsort_min
; CHECK: QsortRec: Computation of MIN in qsort_min PASSED Test.
; CHECK-NOT: FOUND QSORT

; ModuleID = 'qsort-min02.ll'
source_filename = "qsort-min02.ll"

define internal fastcc void @qsort_min(i8* %arg, i64 %arg1) unnamed_addr {
bb:
  %tmp = ptrtoint i8* %arg to i64
  %tmp1 = icmp ult i64 %arg1, 7
  br i1 %tmp1, label %bb2, label %bb_exit

bb2:                                              ; preds = %bb4, %bb
  %tmp2 = phi i64 [ %tmp8, %bb4 ], [ %tmp, %bb ]
  %tmp3 = phi i8* [ %tmp7, %bb4 ], [ %arg, %bb ]
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
  br i1 %tmp14, label %bb2, label %bb_exit

bb_exit:                                          ; preds = %bb4, %bb
  ret void
}
