; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -disable-output -padded-pointer-prop-op -padded-pointer-info < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -disable-output -padded-pointer-info -passes="padded-pointer-prop-op" < %s 2>&1 | FileCheck %s

; Checks padding propagation for simple recursion case
; C code
; int* foo(int* P) {
;   return __builtin_intel_padded(foo(P), 32);
; }

; CHECK: ==== INITIAL FUNCTION SET ====
; CHECK: Function info(foo):
; CHECK-NEXT:  HasUnknownCallSites: 0
; CHECK-NEXT:  Return Padding: -1
; CHECK-NEXT:  Arguments' Padding:
; CHECK-NEXT:    ptr %p : -1
; CHECK-NEXT:  Value paddings:
; CHECK-NEXT:      %t0 = tail call ptr @llvm.ptr.annotation.p0(ptr %call, ptr @0, ptr @.str, i32 2, ptr null) :: 32
; CHECK: ==== END OF INITIAL FUNCTION SET ====

; CHECK: ==== TRANSFORMED FUNCTION SET ====
; CHECK: Function info(foo):
; CHECK-NEXT:  HasUnknownCallSites: 0
; CHECK-NEXT:  Return Padding: 32
; CHECK-NEXT:  Arguments' Padding:
; CHECK-NEXT:    ptr %p : -1
; CHECK-NEXT:  Value paddings:
; CHECK-NEXT:      %t0 = tail call ptr @llvm.ptr.annotation.p0(ptr %call, ptr @0, ptr @.str, i32 2, ptr null) :: 32
; CHECK-NEXT:      %call = call ptr @foo(ptr %p) :: 32
; CHECK: ==== END OF TRANSFORMED FUNCTION SET ====

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@0 = private constant [16 x i8] c"padded 32 bytes\00"
@.str = private constant [14 x i8] c"recursion_1.c\00", section "llvm.metadata"

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare ptr @llvm.ptr.annotation.p0(ptr, ptr, ptr, i32, ptr) #0

; Function Attrs: nounwind uwtable
define internal "intel_dtrans_func_index"="1" ptr @foo(ptr "intel_dtrans_func_index"="2" %p) #1 !intel.dtrans.func.type !6 {
entry:
  %call = call ptr @foo(ptr %p)
  %t0 = tail call ptr @llvm.ptr.annotation.p0(ptr %call, ptr getelementptr inbounds ([16 x i8], ptr @0, i64 0, i64 0), ptr getelementptr inbounds ([14 x i8], ptr @.str, i64 0, i64 0), i32 2, ptr null)
  ret ptr %t0
}

attributes #0 = { inaccessiblememonly nofree nosync nounwind willreturn }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!6 = distinct !{!7, !7}
!7 = !{i32 0, i32 1}
