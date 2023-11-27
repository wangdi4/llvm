; This test verifies that Transpose doesn't crash when dope vector
; argument is used by FreezeInst. (CMPLRLLVM-47170)

; RUN: opt < %s -S -passes=dtrans-transpose  2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: pop_reductionsmod_mp_pop_globalmaxval2dr8_

%"QNCA_a0$i32*$rank3$.418" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

@grid_mp_calcu_ = internal global [58 x [8 x [54 x i32]]] zeroinitializer

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #0

define internal fastcc double @pop_reductionsmod_mp_pop_globalmaxval2dr8_(ptr noalias readonly dereferenceable_or_null(120) "assumed_shape" "ptrnoalias" %0) {
  %i9 = freeze ptr %0
  %i14 = icmp eq ptr %i9, null
  %i15 = getelementptr inbounds %"QNCA_a0$i32*$rank3$.418", ptr %i9, i64 0, i32 0
  %i16 = getelementptr inbounds %"QNCA_a0$i32*$rank3$.418", ptr %i9, i64 0, i32 6, i64 0, i32 1
  ret double 0.000000e+00
}

define internal fastcc void @hmix_aniso_mp_init_aniso_() {
  br label %1

1:                                                ; preds = %1, %0
  br label %1

2:                                                ; No predecessors!
  store i64 0, ptr null, align 8
  %3 = call fastcc double @pop_reductionsmod_mp_pop_globalmaxval2dr8_(ptr null)
  ret void
}

define internal fastcc void @hmix_del2_mp_init_del2u_() {
  %1 = alloca %"QNCA_a0$i32*$rank3$.418", i32 0, align 8
  br label %2

2:                                                ; preds = %2, %0
  br label %2

3:                                                ; No predecessors!
  %4 = getelementptr %"QNCA_a0$i32*$rank3$.418", ptr %1, i64 0, i32 3
  %5 = getelementptr %"QNCA_a0$i32*$rank3$.418", ptr %1, i64 0, i32 1
  %6 = getelementptr %"QNCA_a0$i32*$rank3$.418", ptr %1, i64 0, i32 4
  %7 = getelementptr %"QNCA_a0$i32*$rank3$.418", ptr %1, i64 0, i32 2
  %8 = getelementptr %"QNCA_a0$i32*$rank3$.418", ptr %1, i64 0, i32 6, i64 0, i32 1
  %9 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 0, ptr elementtype(i64) %8, i32 0)
  store i64 4, ptr %9, align 1
  %10 = getelementptr %"QNCA_a0$i32*$rank3$.418", ptr %1, i64 0, i32 6, i64 0, i32 2
  %11 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 0, ptr elementtype(i64) %10, i32 0)
  store i64 1, ptr %11, align 1
  %12 = getelementptr %"QNCA_a0$i32*$rank3$.418", ptr %1, i64 0, i32 6, i64 0, i32 0
  %13 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 0, ptr elementtype(i64) %12, i32 0)
  store i64 54, ptr %13, align 1
  %14 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 0, ptr elementtype(i64) %8, i32 1)
  store i64 216, ptr %14, align 1
  %15 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 0, ptr elementtype(i64) %10, i32 1)
  store i64 1, ptr %15, align 1
  %16 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 0, ptr elementtype(i64) %12, i32 1)
  store i64 8, ptr %16, align 1
  %17 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 0, ptr elementtype(i64) %8, i32 2)
  store i64 1728, ptr %17, align 1
  %18 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 0, ptr elementtype(i64) %10, i32 2)
  store i64 1, ptr %18, align 1
  %19 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 0, ptr elementtype(i64) %12, i32 2)
  store i64 58, ptr %19, align 1
  %20 = getelementptr %"QNCA_a0$i32*$rank3$.418", ptr %1, i64 0, i32 0
  store ptr @grid_mp_calcu_, ptr %20, align 8
  %21 = call fastcc double @pop_reductionsmod_mp_pop_globalmaxval2dr8_(ptr %1)
  ret void
}

attributes #0 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }

!ifx.types.dv = !{!0}
!0 = !{%"QNCA_a0$i32*$rank3$.418" zeroinitializer, i32 0}
