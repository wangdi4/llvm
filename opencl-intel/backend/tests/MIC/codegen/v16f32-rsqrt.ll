; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.madd233.ps(<16 x float>, <16 x float>)
declare i32 @llvm.x86.mic.mask16.to.int(<16 x i1>)
declare <16 x float> @llvm.x86.mic.fixup.ps(<16 x float>, i32)
declare <16 x float> @llvm.x86.mic.rcpres.ps(<16 x float>)
declare <16 x float> @llvm.x86.mic.rsqrtlut.ps(<16 x float>)

define internal <16 x float> @knf_rsqrt(<16 x float> %x) nounwind alwaysinline {
; KNF: @knf_rsqrt
; KNF: vrcpresps	[[R0:%v[0-9]+]], [[R1:%v[0-9]+]]
; KNF: vmadd233ps	.LCPI0_0(%rip), [[R1]], [[R2:%v[0-9]+]]
; KNF: vmulps	%v{{[0-9]+}}, %v{{[0-9]+}}, [[R3:%v[0-9]+]]
; KNF: vrsqrtlutps	[[R0]], [[R4:%v[0-9]+]]
; KNF: vmadd{{[123]+}}ps	%v{{[0-9]+}}, %v{{[0-9]+}}, [[R5:%v[0-9]+]]
; KNF: vfixupps	$2112, [[R5]], %v{{[0-9]+}}
  %e0 = call <16 x float> @llvm.x86.mic.rcpres.ps(<16 x float> %x)
  %p1p2 = bitcast <16 x i32> <i32 1056965147, i32 1052771564, i32 0, i32 0, i32 1056965147, i32 1052771564, i32 0, i32 0, i32 1056965147, i32 1052771564, i32 0, i32 0, i32 1056965147, i32 1052771564, i32 0, i32 0> to <16 x float>
  %pa = call <16 x float> @llvm.x86.mic.madd233.ps(<16 x float> %e0, <16 x float> %p1p2)
  %pb = fmul <16 x float> %e0, %pa
  %t = call <16 x float> @llvm.x86.mic.rsqrtlut.ps(<16 x float> %x)
  %ymul = fmul <16 x float> %t, %pb
  %y = fadd <16 x float> %t, %ymul
  %yfixed = call <16 x float> @llvm.x86.mic.fixup.ps(<16 x float> %y, i32 2112)
  ret <16 x float> %yfixed
}
