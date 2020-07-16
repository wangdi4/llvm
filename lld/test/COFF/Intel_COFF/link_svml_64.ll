; UNSUPPORTED: intel_opencl
; REQUIRES: x86, system-windows

; RUN: llvm-as -o %t.bc %s
; RUN: lld-link /out:%t.exe /entry:main %t.bc /subsystem:console > %t_out 2>&1
; RUN: cat %t_out | FileCheck -allow-empty %s
; RUN: rm %t_out
; TODO: enable with CMPLRLLVM-21197 fix.
; XFAIL: *

; This test case checks that lld-link links correctly the svml libraries. It
; has declared several svml functions and the linker should ignore them before
; LTO. The LTO process should be able to call CodeGen, emit the definitions and
; select the preferred sqrt version. Later, the linker will do a second symbols
; resolution after LTO and will be able to find the definition for the svml
; function.

; This test case handles the 64 bit version of lld-link. The linker won't add
; an extra leading underscore in front of the symbols. Therefore, the symbol
; name will be the same as the function name. This test case looks for __svml_
; with 2 leading underscores.

; Check that there is no undefined svml symbols
; CHECK-NOT: lld-link: error: undefined symbol: __svml_

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

@llvm.compiler.used = appending global [6 x i8*] [i8* bitcast (<2 x double> (<2 x double>)* @__svml_sqrt2 to i8*), i8*
 bitcast (<4 x double> (<4 x double>)* @__svml_sqrt4 to i8*), i8* bitcast (<8 x double> (<8 x double>)* @__svml_sqrt8
to i8*), i8* bitcast (<16 x double> (<16 x double>)* @__svml_sqrt16 to i8*), i8* bitcast (<32 x double> (<32 x double>
)* @__svml_sqrt32 to i8*), i8* bitcast (<64 x double> (<64 x double>)* @__svml_sqrt64 to i8*)], section "llvm.metadata"

@_fltused = dllexport local_unnamed_addr global i32 0, align 4

define i32 @main(double %x) {
entry:
  %tmp = call double @sqrt(double %x)
  %tmp2 = fptosi double %tmp to i32
  ret i32 %tmp2
}

define double @sqrt(double %x) {
  %call = call double @llvm.sqrt(double %x)
  ret double %call
}

declare double @llvm.sqrt(double)

declare dso_local <2 x double> @__svml_sqrt2(<2 x double>) local_unnamed_addr #0
declare dso_local <4 x double> @__svml_sqrt4(<4 x double>) local_unnamed_addr #0
declare dso_local <8 x double> @__svml_sqrt8(<8 x double>) local_unnamed_addr #0
declare dso_local <16 x double> @__svml_sqrt16(<16 x double>) local_unnamed_addr #0
declare dso_local <32 x double> @__svml_sqrt32(<32 x double>) local_unnamed_addr #0
declare dso_local <64 x double> @__svml_sqrt64(<64 x double>) local_unnamed_addr #0

attributes #0 = { nofree nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-
pointer"="none" "imf-arch-consistency"="truewq" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-mat
h"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"
="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "imf-arch-consistency"="truewq" "vector-function-abi-variant"="_ZGV_LLVM_N2v_sqrt(__svml_sq
rt2),_ZGV_LLVM_N4v_sqrt(__svml_sqrt4),_ZGV_LLVM_N8v_sqrt(__svml_sqrt8),_ZGV_LLVM_N16v_sqrt(__svml_sqrt16),_ZGV_LLVM_N3
2v_sqrt(__svml_sqrt32),_ZGV_LLVM_N64v_sqrt(__svml_sqrt64)" }
