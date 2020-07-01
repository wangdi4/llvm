; RUN: opt < %s -analyze -hir-ssa-deconstruction -hir-framework -hir-framework-debug=parser | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; Check for successful parsing.

; CHECK: + DO i1 = 0, %arg1 + -1, 1   <DO_LOOP>
; CHECK: |   (%tmp4)[undef] = undef;
; CHECK: |   %tmp9 = bitcast.i32*.i8*(&((%tmp4)[0]));
; CHECK: |   %tmp4 = &((i32*)(%tmp9)[undef]);
; CHECK: + END LOOP

source_filename = "test.ll"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @quux(i32* %arg, i32 %arg1) {
bb:
  %tmp = getelementptr inbounds i32, i32* %arg, i64 undef
  br i1 undef, label %bb2, label %bb15

bb2:                                              ; preds = %bb
  br label %bb3

bb3:                                              ; preds = %bb8, %bb2
  %tmp4 = phi i32* [ %tmp, %bb2 ], [ %tmp11, %bb8 ]
  %tmp5 = phi i32 [ 0, %bb2 ], [ %tmp12, %bb8 ]
  %tmp6 = zext i32 undef to i64
  %tmp7 = getelementptr inbounds i32, i32* %tmp4, i64 %tmp6
  store i32 undef, i32* %tmp7, align 4
  br label %bb8

bb8:                                              ; preds = %bb3
  %tmp9 = bitcast i32* %tmp4 to i8*
  %tmp10 = getelementptr inbounds i8, i8* %tmp9, i64 undef
  %tmp11 = bitcast i8* %tmp10 to i32*
  %tmp12 = add nuw nsw i32 %tmp5, 1
  %tmp13 = icmp eq i32 %tmp12, %arg1
  br i1 %tmp13, label %bb14, label %bb3

bb14:                                             ; preds = %bb8
  unreachable

bb15:                                             ; preds = %bb
  unreachable
}

