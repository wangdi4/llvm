; REQUIRES: x86, system-windows
; RUN: llvm-as -o %T/link_svml.bc %s
; RUN: lld-link /out:%T/link_svml.exe /entry:main %T/link_svml.bc /subsystem:console > %t_out 2>&1
; RUN: cat %t_out | FileCheck -allow-empty %s
; RUN: rm %t_out

; This test case checks that lld-link links correctly the svml libraries. It
; has declared several svml functions and the linker should ignore them before
; LTO. The LTO process should be able to call CodeGen, emit the definitions and
; select the preferred sqrt version. Later, the linker will do a second symbols
; resolution after LTO and will be able to find the definition for the svml
; function. This is the same test case as link_svml_01.ll but for 32bits
; compiler.

; This test case handles the 32 bit version of lld-link. The linker will add
; an extra leading underscore in front of the symbols name since it uses the
; same naming rules as MS when handling 32 bit objects. This test case looks
; for the symbol ___svml_ with 3 leading underscores.

; Check that there is no undefined svml symbols
; CHECK-NOT: lld-link: error: undefined symbol: ___svml_

target datalayout = "e-m:x-p:32:32-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:32-n8:16:32-a:0:32-S32"
target triple = "i386-pc-windows-msvc"

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
