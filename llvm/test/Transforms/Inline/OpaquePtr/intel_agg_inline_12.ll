; REQUIRES: asserts
; RUN: opt -opaque-pointers < %s -whole-program-assume -intel-libirc-allowed -agginliner -debug-only=agginliner -mtriple=i686-- -mattr=+avx2 -disable-output 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers < %s -whole-program-assume -intel-libirc-allowed -passes='module(agginliner)' -debug-only=agginliner -mtriple=i686-- -mattr=+avx2 -disable-output 2>&1 | FileCheck %s

; Verifies that LBM_allocateGrid is recognized as malloc routine.

; CHECK: AggInl: HugeMallocGlobalPointersHeuristic
; CHECK: AggInl: LBM_allocateGrid malloc routine found

@srcGrid = internal global ptr null, align 8
@dstGrid = internal global ptr null, align 8

; Function Attrs: norecurse
define internal fastcc void @LBM_allocateGrid(ptr nocapture %arg) unnamed_addr #0 {
bb:
  %i = tail call noalias dereferenceable_or_null(214400000) ptr @malloc(i64 214400000)
  store ptr %i, ptr %arg, align 8
  %i2 = icmp eq ptr %i, null
  br i1 %i2, label %bb3, label %bb4

bb3:                                              ; preds = %bb
  tail call void @exit(i32 1)
  unreachable

bb4:                                              ; preds = %bb
  %i6 = getelementptr inbounds double, ptr %i, i64 400000
  store ptr %i6, ptr %arg, align 8
  ret void
}

; Function Attrs: norecurse
define dso_local i32 @main(i32 %arg, ptr nocapture readonly %arg1) local_unnamed_addr #0 {
bb:
  call fastcc void @MAIN_initialize()
  ret i32 0
}

; Function Attrs: norecurse
define internal fastcc void @MAIN_initialize() unnamed_addr #0 {
bb:
  tail call fastcc void @LBM_allocateGrid(ptr @srcGrid)
  tail call fastcc void @LBM_allocateGrid(ptr @dstGrid)
  ret void
}

declare dso_local noalias ptr @malloc(i64) local_unnamed_addr

declare dso_local void @exit(i32) local_unnamed_addr

attributes #0 = { norecurse }
