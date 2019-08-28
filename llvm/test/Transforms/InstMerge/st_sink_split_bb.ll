;INTEL_CUSTOMIZATION
; Test to make sure that a new block is inserted if we
; have more than 2 predecessors for the block we're going to sink to.
; RUN: opt -basicaa -memdep -mldst-motion -S < %s | FileCheck %s
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"

; Function Attrs: nounwind uwtable
define dso_local void @st_sink_split_bb(i32* nocapture %arg, i32* nocapture %arg1, i1 zeroext %arg2, i1 zeroext %arg3) local_unnamed_addr {
bb:
  br i1 %arg2, label %bb4, label %bb5

bb4:                                              ; preds = %bb
  store i32 1, i32* %arg, align 4
  br label %bb9

bb5:                                              ; preds = %bb
  br i1 %arg3, label %bb6, label %bb7

; CHECK-LABEL: bb6:
bb6:                                              ; preds = %bb5
  ; CHECK:   store i32 2, i32* %arg
  store i32 2, i32* %arg, align 4
  %tmp1 = getelementptr inbounds i32, i32* %arg1, i64 1
  ; CHECK-NOT: store i32
  store i32 3, i32* %tmp1, align 4
  %tmp = getelementptr inbounds i32, i32* %arg1, i64 2
  ; CHECK-NOT: store i32
  store i32 3, i32* %tmp, align 4
  br label %bb9

; CHECK-LABEL: bb7:
bb7:                                              ; preds = %bb5
  ; CHECK:   store i32 3, i32* %arg
  store i32 3, i32* %arg, align 4
  %tmp2 = getelementptr inbounds i32, i32* %arg1, i64 1
  ; CHECK-NOT: store i32
  store i32 3, i32* %tmp2, align 4
  %tmp8 = getelementptr inbounds i32, i32* %arg1, i64 2
  ; CHECK-NOT: store i32
  store i32 3, i32* %tmp8, align 4
  br label %bb9

; CHECK-LABEL: .sink.split:
; CHECK: store
; CHECK: store

; CHECK-LABEL: bb9:
bb9:                                              ; preds = %bb7, %bb6, %bb4
  ; CHECK-NOT: store i32
  ret void
}
;end INTEL_CUSTOMIZATION
