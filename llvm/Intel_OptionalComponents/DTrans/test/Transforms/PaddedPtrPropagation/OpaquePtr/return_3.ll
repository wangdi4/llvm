; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output -padded-pointer-prop-op -padded-pointer-info < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output -padded-pointer-info -passes="padded-pointer-prop-op" < %s 2>&1 | FileCheck %s

; Checks propagation of the return padding to the caller
; C code
; extern int* IP;
;
; int* callee() {
;   return __builtin_intel_padded(IP, 32);
; }
;
; int* caller(void) {
;   int* p = callee();
;   return p;
; }

; CHECK: ==== INITIAL FUNCTION SET ====
; CHECK: Function info(callee):
; CHECK-NEXT:  HasUnknownCallSites: 0
; CHECK-NEXT:  Return Padding: -1
; CHECK-NEXT:  Value paddings:
; CHECK-NEXT:  %i1 = tail call ptr @llvm.ptr.annotation.p0(ptr %i, ptr @0, ptr @.str, i32 4, ptr null) :: 32
; CHECK: ==== END OF INITIAL FUNCTION SET ====
; CHECK: ==== TRANSFORMED FUNCTION SET ====
; CHECK: Function info(callee):
; CHECK-NEXT:  HasUnknownCallSites: 0
; CHECK-NEXT:  Return Padding: 32
; CHECK-NEXT:  Value paddings:
; CHECK-NEXT:  %i1 = tail call ptr @llvm.ptr.annotation.p0(ptr %i, ptr @0, ptr @.str, i32 4, ptr null) :: 32
; CHECK: Function info(caller):
; CHECK-NEXT:  HasUnknownCallSites: 0
; CHECK-NEXT:  Return Padding: 32
; CHECK-NEXT:  Value paddings:
; CHECK-NEXT:  %call = call ptr @callee() :: 32
; CHECK: ==== END OF TRANSFORMED FUNCTION SET ====

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@IP = internal global ptr null, align 8, !intel_dtrans_type !0
@0 = private constant [16 x i8] c"padded 32 bytes\00"
@.str = private constant [11 x i8] c"return_3.c\00", section "llvm.metadata"

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare ptr @llvm.ptr.annotation.p0(ptr, ptr, ptr, i32, ptr) #0

; Function Attrs: nounwind uwtable
define internal "intel_dtrans_func_index"="1" ptr @callee() #1 !intel.dtrans.func.type !7 {
bb:
  %i = load ptr, ptr @IP, align 8
  %i1 = tail call ptr @llvm.ptr.annotation.p0(ptr %i, ptr getelementptr inbounds ([16 x i8], ptr @0, i64 0, i64 0), ptr getelementptr inbounds ([11 x i8], ptr @.str, i64 0, i64 0), i32 4, ptr null)
  ret ptr %i1
}

; Function Attrs: nounwind uwtable
define internal "intel_dtrans_func_index"="1" ptr @caller() #1 !intel.dtrans.func.type !8 {
entry:
  %call = call ptr @callee()
  ret ptr %call
}

attributes #0 = { inaccessiblememonly nofree nosync nounwind willreturn }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!1, !2, !3, !4, !5}
!intel.dtrans.types = !{}
!llvm.ident = !{!6}

!0 = !{i32 0, i32 1}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"Virtual Function Elim", i32 0}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 1, !"ThinLTO", i32 0}
!5 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!6 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!7 = distinct !{!0}
!8 = distinct !{!0}
