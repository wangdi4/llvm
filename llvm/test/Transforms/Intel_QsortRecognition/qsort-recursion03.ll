; REQUIRES: asserts
; RUN: opt < %s -qsortrecognizer -debug-only=qsortrecognizer -qsort-test-insert=false -qsort-test-pivot=false -qsort-test-pivot-movers=false -qsort-test-min=true -qsort-test-swap=false -qsort-test-recursion=true -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(qsortrecognizer)' -debug-only=qsortrecognizer -qsort-test-insert=false -qsort-test-pivot=false -qsort-test-pivot-movers=false -qsort-test-min=true -qsort-test-swap=false -qsort-test-recursion=true -disable-output 2>&1 | FileCheck %s

; Check that the direct recursion is recognized, but the tail recursion
; is not in the IR. The computation of MIN will pass for the direct recursive
; call, but will fail for the tail recursion.
; the computation of MIN.

; CHECK: QsortRec: Computation of MIN in qsort_rec PASSED Test.
; CHECK: QsortRec: Computation of MIN in qsort_rec FAILED Test.
; CHECK-NOT: FOUND QSORT

define internal fastcc void @qsort_rec(i8* %arg, i64 %arg1) unnamed_addr {
bb:
  %tmp1 = icmp ult i64 %arg1, 7
  br i1 %tmp1, label %bb2, label %bb3

bb2:
  %tmp2 = phi i8* [ %tmp12, %bb10 ], [ %arg, %bb ]
  %tmp3 = phi i64 [ %tmp11, %bb10 ], [ %arg1, %bb ]
  br label %bb4

bb3:
  %tmp4 = phi i8* [ %tmp12, %bb10 ], [ %arg, %bb ]
  %tmp5 = phi i64 [ %tmp11, %bb10 ], [ %arg1, %bb ]
  br label %bb4

bb4:
  %tmp6 = phi i8* [ %tmp4, %bb3 ], [ %tmp2, %bb2 ]
  %tmp7 = phi i64 [ %tmp5, %bb3 ], [ %tmp3, %bb2 ]
  br label %bb5

bb5:
  %tmp8 = ptrtoint i8* %tmp6 to i64
  %tmp9 = getelementptr inbounds i8, i8* %tmp6, i64 8
  %tmp10 = shl i64 %tmp7, 3
  %tmp11 = add i64 %tmp10, -8
  %tmp12 = getelementptr inbounds i8, i8* %tmp6, i64 %tmp11
  br label %bb6

bb6:
  %tmp13 = ptrtoint i8* %tmp9 to i64
  %tmp14 = sub i64 %tmp13, %tmp8
  %tmp15 = ptrtoint i8* %tmp12 to i64
  %tmp16 = sub i64 %tmp15, %tmp13
  %tmp17 = icmp slt i64 %tmp14, %tmp16
  %tmp18 = select i1 %tmp17, i64 %tmp14, i64 %tmp16
  %tmp19 = icmp eq i64 %tmp18, 0
  br i1 %tmp19, label %bb7, label %bb_exit

bb7:
  %tmp20 = ptrtoint i8* %tmp9 to i64
  %tmp21 = ptrtoint i8* %tmp12 to i64
  %tmp22 = sub i64 %tmp20, %tmp21
  %tmp23 = ptrtoint i8* %tmp12 to i64
  %tmp24 = sub i64 %tmp23, %tmp20
  %tmp25 = add i64 %tmp24, -8
  %tmp26 = icmp slt i64 %tmp22, %tmp25
  %tmp27 = select i1 %tmp26, i64 %tmp22, i64 %tmp25
  %tmp28 = icmp eq i64 %tmp27, 0
  br i1 %tmp28, label %bb8, label %bb_exit

bb8:
  %tmp29 = icmp ugt i64 %tmp16, 8
  br i1 %tmp29, label %bb9, label %bb_exit

bb9:
  %tmp30 = lshr i64 %tmp16, 3
  tail call fastcc void @qsort_rec(i8* %tmp6, i64 %tmp30)
  br label %bb10

bb10:
  %tmp31 = icmp ugt i64 %tmp22, 8
  br i1 %tmp31, label %bb2, label %bb3

bb_exit:
  ret void
}
