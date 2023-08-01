; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-locality-analysis>" -hir-spatial-locality -disable-output 2>&1 | FileCheck %s

source_filename = "test.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; The HIR for this code is
; BEGIN REGION { }
;       + DO i1 = 0, 384307168202282324, 1   <DO_LOOP>
;       |   %tmp16 = &((null)[i1].0);
;       |   %tmp20 = (%struct.spam**)(%tmp16)[16];
;       |   (%tmp20)[0].1 = null;
;       + END LOOP
; END REGION
;
; The code may seem quite nonsensical, but it arises from optimizations
; causing incomplete simplification of code, and is guarded by an if statement
; that never fires. The purpose of this test is to confirm that we do not crash
; instead of a particular result.


%struct.ham = type { i8, %struct.spam, i64 }
%struct.spam = type { i32, ptr, ptr, ptr }

define void @bar() {
; CHECK: Locality Info for Loop level: 1
; CHECK-SAME: NumCacheLines: {{[0-9]+}}
bb10:                                             ; preds = %bb
  br label %bb11

bb11:                                             ; preds = %bb56, %bb10
  %tmp13 = phi ptr [ %tmp59, %bb11 ], [ null, %bb10 ]
  %tmp16 = getelementptr inbounds %struct.ham, ptr %tmp13, i64 0, i32 0
  %tmp18 = getelementptr inbounds i8, ptr %tmp16, i64 16
  %tmp19 = bitcast ptr %tmp18 to ptr
  %tmp20 = load ptr, ptr %tmp19, align 8
  %tmp38 = getelementptr inbounds %struct.spam, ptr %tmp20, i64 0, i32 1
  store ptr null, ptr %tmp38, align 8
  %tmp59 = getelementptr inbounds %struct.ham, ptr %tmp13, i64 1
  %tmp61 = icmp eq ptr %tmp59, null
  br i1 %tmp61, label %bb135, label %bb11

bb135:                                            ; preds = %bb135, %bb117
  ret void
}
