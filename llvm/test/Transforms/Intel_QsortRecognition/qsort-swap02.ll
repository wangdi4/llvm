; REQUIRES: asserts
; RUN: opt < %s -qsortrecognizer -debug-only=qsortrecognizer -qsort-unit-test -qsort-test-swap -qsort-test-recursion -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(qsortrecognizer)' -debug-only=qsortrecognizer -qsort-unit-test -qsort-test-swap -disable-output 2>&1 | FileCheck %s

; Check that the computation of swap is recognized, but the test didn't
; pass because there aren't enough swaps.

; CHECK: QsortRec: Checking computation of swap in qsort_swap
; CHECK: QsortRec: Computation of swap in qsort_swap PASSED Test.
; CHECK-NOT: FOUND QSORT

define internal fastcc void @qsort_swap(i8* %arg, i64 %arg1) unnamed_addr {
bb:
  %temp = ptrtoint i8* %arg to i64
  %temp1 = icmp ult i64 %arg1, 7
  br i1 %temp1, label %bb2, label %bb_exit

bb2:                                              ; preds = %bb5, %bb
  %temp2 = phi i64 [ %temp9, %bb8 ], [ %temp, %bb ]
  %temp3 = phi i8* [ %temp7, %bb8 ], [ %arg, %bb ]
  br label %bb3

bb3:                                              ; preds = %bb2
  %temp4 = getelementptr inbounds i8, i8* %temp3, i64 8
  %temp5 = shl i64 %temp, 3
  %temp6 = add i64 %temp5, -8
  %temp7 = getelementptr inbounds i8, i8* %temp3, i64 %temp6
  br label %bb4

bb4:                                              ; preds = %bb3
  %temp8 = ptrtoint i8* %temp4 to i64
  %temp9 = sub i64 %temp8, %temp2
  %temp10 = ptrtoint i8* %temp7 to i64
  %temp11 = sub i64 %temp10, %temp8
  %temp12 = icmp slt i64 %temp9, %temp11
  %temp13 = select i1 %temp12, i64 %temp9, i64 %temp11
  %temp14 = icmp eq i64 %temp13, 0
  br i1 %temp14, label %bb6, label %bb_exit

bb6:
  %temp15 = sub i64 0, %temp13
  %temp16 = getelementptr inbounds i8, i8* %temp3, i64 %temp15
  %temp17 = shl i64 %temp13, 32
  %temp18 = ashr exact i64 %temp17, 32
  %temp19 = lshr i64 %temp18, 3
  %temp20 = bitcast i8* %arg to i64*
  %temp21 = bitcast i8* %temp16 to i64*
  br label %bb7

bb7:
  %temp22 = phi i64* [ %temp21, %bb6 ], [ %temp28, %bb7 ]
  %temp23 = phi i64* [ %temp20, %bb6 ], [ %temp27, %bb7 ]
  %temp24 = phi i64 [ %temp19, %bb6 ], [ %temp29, %bb7 ]
  %temp25 = load i64, i64* %temp23, align 8
  %temp26 = load i64, i64* %temp22, align 8
  %temp27 = getelementptr inbounds i64, i64* %temp23, i64 1
  store i64 %temp26, i64* %temp23, align 8
  %temp28 = getelementptr inbounds i64, i64* %temp22, i64 1
  store i64 %temp25, i64* %temp22, align 8
  %temp29 = add nsw i64 %temp24, -1
  %temp30 = icmp sgt i64 %temp29, 0
  br i1 %temp30, label %bb7, label %bb8

bb8:
  %temp31 = icmp ugt i64 %temp29, 8
  br i1 %temp31, label %bb_exit, label %bb2

bb_exit:
  ret void
}
