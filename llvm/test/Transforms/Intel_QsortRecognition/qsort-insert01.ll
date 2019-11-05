; REQUIRES: asserts
; RUN: opt < %s -qsortrecognizer -debug-only=qsortrecognizer -qsort-test-insert=true -qsort-test-pivot=false -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(qsortrecognizer)' -debug-only=qsortrecognizer -qsort-test-insert=true -qsort-test-pivot=false -disable-output 2>&1 | FileCheck %s

; Check that the insertion sort is recognized.

; CHECK: QsortRec: Checking Insertion Sort Candidate in qsort_insert
; CHECK: QsortRec: Insertion Sort Candidate in qsort_insert PASSED Test.
; ModuleID = 'qsort-insert01.ll'

%struct.basket = type { %struct.arc*, i64, i64, i64 }
%struct.arc = type { i32, i64, %struct.node*, %struct.node*, i16, %struct.arc*, %struct.arc*, i64, i64 }
%struct.node = type { i64, i32, %struct.node*, %struct.node*, %struct.node*, %struct.node*, %struct.arc*, %struct.arc*, %struct.arc*, %struct.arc*, i64, i64, i32, i32 }

@myglobal = dso_local global i64 32, align 4
@buffer = dso_local global i8* null, align 8

declare i32 @cost_compare(%struct.basket** nocapture readonly, %struct.basket** nocapture readonly)

define internal fastcc void @qsort_insert(i8* %arg, i64 %arg1) unnamed_addr {
bb:
  %tmp = icmp ult i64 %arg1, 7
  br i1 %tmp, label %bb2, label %bb28

bb2:                                              ; preds = %bb29, %bb
  %tmp3 = phi i64 [ %arg1, %bb ], [ %tmp30, %bb29 ]
  %tmp4 = phi i8* [ %arg, %bb ], [ %tmp31, %bb29 ]
  %tmp5 = getelementptr inbounds i8, i8* %tmp4, i64 8
  %tmp6 = shl i64 %tmp3, 3
  %tmp7 = getelementptr inbounds i8, i8* %tmp4, i64 %tmp6
  %tmp8 = icmp sgt i64 %tmp6, 8
  br i1 %tmp8, label %bb9, label %bb28

bb9:                                              ; preds = %bb12, %bb2
  %tmp10 = phi i8* [ %tmp13, %bb12 ], [ %tmp5, %bb2 ]
  %tmp11 = icmp ugt i8* %tmp10, %tmp4
  br i1 %tmp11, label %bb15, label %bb12

bb12:                                             ; preds = %bb22, %bb15, %bb9
  %tmp13 = getelementptr inbounds i8, i8* %tmp10, i64 8
  %tmp14 = icmp ult i8* %tmp13, %tmp7
  br i1 %tmp14, label %bb9, label %bb28

bb15:                                             ; preds = %bb22, %bb9
  %tmp16 = phi i8* [ %tmp17, %bb22 ], [ %tmp10, %bb9 ]
  %tmp17 = getelementptr inbounds i8, i8* %tmp16, i64 -8
  %tmp18 = bitcast i8* %tmp17 to %struct.basket**
  %tmp19 = bitcast i8* %tmp16 to %struct.basket**
  %tmp20 = tail call i32 @cost_compare(%struct.basket** nonnull %tmp18, %struct.basket** %tmp19)
  %tmp21 = icmp sgt i32 %tmp20, 0
  br i1 %tmp21, label %bb22, label %bb12

bb22:                                             ; preds = %bb15
  %tmp23 = bitcast i8* %tmp16 to i64*
  %tmp24 = load i64, i64* %tmp23, align 8
  %tmp25 = bitcast i8* %tmp17 to i64*
  %tmp26 = load i64, i64* %tmp25, align 8
  store i64 %tmp26, i64* %tmp23, align 8
  store i64 %tmp24, i64* %tmp25, align 8
  %tmp27 = icmp ugt i8* %tmp17, %tmp4
  br i1 %tmp27, label %bb15, label %bb12

bb28:                                             ; preds = %bb12, %bb2, %bb
  ret void

bb29:                                             ; No predecessors!
  %tmp30 = load i64, i64* @myglobal, align 8
  %tmp31 = load i8*, i8** @buffer, align 8
  br label %bb2
}
