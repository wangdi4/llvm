; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Check that the struct offsets will be added to the lowest dimension.

; CHECK: BEGIN REGION { }
; CHECK:      + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:      |   (%arg)[0][i1].0.0.0 = 5;
; CHECK:      + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%struct.blam.42 = type { %struct.quux }
%struct.quux = type { %struct.hoge }
%struct.hoge = type { i32, i32, ptr }

define dso_local void @barney(ptr %arg) {
bb:
  br label %bb3

bb2:                                              ; preds = %bb3
  ret void

bb3:                                              ; preds = %bb3, %bb
  %tmp = phi i32 [ 0, %bb ], [ %tmp33, %bb3 ]
  %tmp4 = getelementptr inbounds [4 x %struct.blam.42], ptr %arg, i32 0, i32 %tmp, i32 0
  %tmp7 = getelementptr inbounds %struct.quux, ptr %tmp4, i32 0, i32 0, i32 0
  store i32 5, ptr %tmp7, align 4
  %tmp33 = add nuw nsw i32 %tmp, 1
  %tmp34 = icmp eq i32 %tmp33, 4
  %tmp35 = bitcast i32 %tmp33 to i32
  br i1 %tmp34, label %bb2, label %bb3
}

