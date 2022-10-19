; REQUIRES: asserts
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -agginliner -debug-only=agginliner -mtriple=i686-- -mattr=+avx2 -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -passes='module(agginliner)' -debug-only=agginliner -mtriple=i686-- -mattr=+avx2 -disable-output 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Verifies that LBM_allocateGrid is recognized as malloc routine.

; CHECK: AggInl: HugeMallocGlobalPointersHeuristic
; CHECK: AggInl: LBM_allocateGrid malloc routine found

@srcGrid = internal global [26000000 x double]* null, align 8
@dstGrid = internal global [26000000 x double]* null, align 8

define internal fastcc void @LBM_allocateGrid(double** nocapture %0) unnamed_addr #0 {
  %2 = tail call noalias dereferenceable_or_null(214400000) i8* @malloc(i64 214400000)
  %3 = bitcast double** %0 to i8**
  store i8* %2, i8** %3, align 8
  %4 = icmp eq i8* %2, null
  br i1 %4, label %5, label %6

5:                                                ; preds = %1
  tail call void @exit(i32 1)
  unreachable

6:                                                ; preds = %1
  %7 = bitcast i8* %2 to double*
  %8 = getelementptr inbounds double, double* %7, i64 400000
  store double* %8, double** %0, align 8
  ret void
}


define dso_local i32 @main(i32 %0, i8** nocapture readonly %1) local_unnamed_addr #0 {
  call fastcc void @MAIN_initialize()
  ret i32 0
}

define internal fastcc void @MAIN_initialize() unnamed_addr #0 {
  tail call fastcc void @LBM_allocateGrid(double** bitcast ([26000000 x double]** @srcGrid to double**))
  tail call fastcc void @LBM_allocateGrid(double** bitcast ([26000000 x double]** @dstGrid to double**))
  ret void
}

declare dso_local noalias i8* @malloc(i64 %0) local_unnamed_addr
declare dso_local void @exit(i32 %0) local_unnamed_addr

attributes #0 = { norecurse }
