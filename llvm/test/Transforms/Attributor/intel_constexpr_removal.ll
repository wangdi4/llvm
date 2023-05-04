; RUN: opt -passes='default<O2>' -disable-output < %s 2>&1

; Verify that the compiler does not crash due to the Attributor replacing
; ConstantExprs which are contained in other constant objects,
; e.g. a ConstantExpr or a ConstantVector. Changing the contained ConstExpr
; would invalidate the hash key used by the original object.
; This could lead to an assertion being triggered if the dead code elimination
; tries to delete last live instance of the original object
; because the lookup in the hash table will fail.

; This test does not need to be converted to opaque pointers when switching to
; use opaque pointers by default because the GEP is elided from the IR in that
; case, so would not show the original issue.

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"

%"struct.std::complex" = type { { double, double } }

@counter_N0.ascast.red.__local = external addrspace(3) global %"struct.std::complex"

; CMPLRLLVM-39584
define weak void @test_constexprs() {
  store double 0.000000e+00, ptr addrspace(3) @counter_N0.ascast.red.__local, align 8
  %_M_value.real.i.i.i = load double, ptr addrspace(4) addrspacecast (ptr addrspace(3) @counter_N0.ascast.red.__local to ptr addrspace(4)), align 8
  %add.r.i.i22 = fadd fast double 0.000000e+00, %_M_value.real.i.i.i
  ret void
}

; CMPLRLLVM-45318
define weak void @test_constvec() {
  call void @llvm.masked.scatter.v2f64.v2p3(<2 x double> <double 1.000000e+00, double 2.000000e+00>, <2 x ptr addrspace(3)> <ptr addrspace(3) @counter_N0.ascast.red.__local, ptr addrspace(3) @counter_N0.ascast.red.__local>, i32 8, <2 x i1> <i1 true, i1 true>)
  ret void
}

declare void @llvm.masked.scatter.v2f64.v2p3(<2 x double>, <2 x ptr addrspace(3)>, i32 immarg, <2 x i1>) #0

!llvm.module.flags = !{!0, !1}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{i32 7, !"openmp-device", i32 50}
