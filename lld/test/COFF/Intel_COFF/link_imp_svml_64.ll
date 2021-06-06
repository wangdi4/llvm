; UNSUPPORTED: intel_opencl
; REQUIRES: x86, system-windows

; RUN: llvm-as -o %t.bc %s
; RUN: not lld-link /out:%t.exe /entry:main %t.bc /subsystem:console > %t_out 2>&1
; RUN: cat %t_out | FileCheck -allow-empty %s
; RUN: rm %t_out

; This test case checks that lld-link links correctly the svml libraries that
; are marked as dllimport.

; This test case handles the 64 bit version of lld-link. The linker won't add
; an extra leading underscore in front of the symbols. Therefore, the symbol
; name will be the same as the function name. This test case looks for
; __imp___svml_ with 2 leading underscores before "svml_".

; NOTE: This test case will produce undefined symbol for
; __declspec(dllimport) _atanf. The reason is because we aren't supplying
; the math libraries to the linker. The main goal of this test case is to
; make sure that there aren't undefined symbols for __svml_atanf.

; Check that there is no undefined svml symbols
; CHECK-NOT: lld-link: error: undefined symbol: __declspec(dllimport) __svml_atanf

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

@llvm.compiler.used = appending global [6 x i8*] [i8* bitcast (<2 x float> (<2 x float>)* @__svml_atanf2 to i8*), i8*
 bitcast (<4 x float> (<4 x float>)* @__svml_atanf4 to i8*), i8* bitcast (<8 x float> (<8 x float>)* @__svml_atanf8
to i8*), i8* bitcast (<16 x float> (<16 x float>)* @__svml_atanf16 to i8*), i8* bitcast (<32 x float> (<32 x float>
)* @__svml_atanf32 to i8*), i8* bitcast (<64 x float> (<64 x float>)* @__svml_atanf64 to i8*)], section "llvm.metadata"

@_fltused = dllexport local_unnamed_addr global i32 0, align 4

define i32 @main(float %x) {
entry:
  %tmp = call float @atanf(float %x) #1
  %tmp2 = fptosi float %tmp to i32
  ret i32 %tmp2
}

declare dllimport float @atanf(float %x) #0

declare dllimport  <2 x float> @__svml_atanf2(<2 x float>) local_unnamed_addr #0
declare dllimport  <4 x float> @__svml_atanf4(<4 x float>) local_unnamed_addr #0
declare dllimport  <8 x float> @__svml_atanf8(<8 x float>) local_unnamed_addr #0
declare dllimport  <16 x float> @__svml_atanf16(<16 x float>) local_unnamed_addr #0
declare dllimport  <32 x float> @__svml_atanf32(<32 x float>) local_unnamed_addr #0
declare dllimport  <64 x float> @__svml_atanf64(<64 x float>) local_unnamed_addr #0

attributes #0 = { nofree nosync nounwind readnone willreturn mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone willreturn "vector-function-abi-variant"="_ZGV_LLVM_N2v_atanf(__svml_atanf2),_ZGV_LLVM_N4v_atanf(__svml_atanf4),_ZGV_LLVM_N8v_atanf(__svml_atanf8),_ZGV_LLVM_N16v_atanf(__svml_atanf16),_ZGV_LLVM_N32v_atanf(__svml_atanf32),_ZGV_LLVM_N64v_atanf(__svml_atanf64)" }