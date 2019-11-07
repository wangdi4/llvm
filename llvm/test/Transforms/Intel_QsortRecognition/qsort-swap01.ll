; REQUIRES: asserts
; RUN: opt < %s -qsortrecognizer -debug-only=qsortrecognizer -qsort-test-insert=false -qsort-test-pivot=false -qsort-test-pivot-movers=false -qsort-test-min=false -qsort-test-swap=true -qsort-test-recursion=false -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(qsortrecognizer)' -debug-only=qsortrecognizer -qsort-test-insert=false -qsort-test-pivot=false -qsort-test-pivot-movers=false -qsort-test-min=false -qsort-test-swap=true -qsort-test-recursion=false -disable-output 2>&1 | FileCheck %s

; Check that the computation of swap is recognized

; CHECK: QsortRec: Checking computation of swap in qsort_swap
; CHECK: QsortRec: Computation of swap in qsort_swap PASSED Test.
; CHECK: QsortRec: Checking computation of swap in qsort_swap
; CHECK: QsortRec: Computation of swap in qsort_swap PASSED Test.
; CHECK: FOUND QSORT

define internal fastcc void @qsort_swap(i8* %arg, i64 %arg1) unnamed_addr {
bb:
  %temp = ptrtoint i8* %arg to i64
  %temp1 = icmp ult i64 %arg1, 7
  br i1 %temp1, label %bb2, label %bb_exit

bb2:                                              ; preds = %bb5, %bb
  %temp2 = phi i64 [ %temp9, %bb10 ], [ %temp, %bb ]
  %temp3 = phi i8* [ %temp7, %bb10 ], [ %arg, %bb ]
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
  %temp31 = sub i64 0, %temp13
  %temp32 = getelementptr inbounds i8, i8* %temp3, i64 %temp31
  %temp33 = shl i64 %temp13, 32
  %temp34 = ashr exact i64 %temp33, 32
  %temp35 = lshr i64 %temp34, 3
  %temp36 = bitcast i8* %arg to i64*
  %temp37 = bitcast i8* %temp32 to i64*
  br label %bb9

bb9:
  %temp38 = phi i64* [ %temp37, %bb8 ], [ %temp50, %bb9 ]
  %temp39 = phi i64* [ %temp36, %bb8 ], [ %temp43, %bb9 ]
  %temp40 = phi i64 [ %temp35, %bb8 ], [ %temp51, %bb9 ]
  %temp41 = load i64, i64* %temp39, align 8
  %temp42 = load i64, i64* %temp38, align 8
  %temp43 = getelementptr inbounds i64, i64* %temp39, i64 1
  store i64 %temp42, i64* %temp39, align 8
  %temp50 = getelementptr inbounds i64, i64* %temp38, i64 1
  store i64 %temp41, i64* %temp38, align 8
  %temp51 = add nsw i64 %temp40, -1
  %temp52 = icmp sgt i64 %temp51, 0
  br i1 %temp52, label %bb9, label %bb10

bb10:
  %temp53 = icmp ugt i64 %temp51, 8
  br i1 %temp53, label %bb_exit, label %bb2

bb_exit:
  ret void
}
