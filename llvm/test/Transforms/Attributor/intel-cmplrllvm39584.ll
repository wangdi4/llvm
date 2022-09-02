; RUN: opt -passes='default<O2>' -disable-output < %s 2>&1

; Verify that the compiler does not crash due to the Attributor replacing
; part of a ConstantExpr. A ConstantExpr can contain another ConstantExpr,
; and changing part of it would invalidate the hash key used by the original
; ConstantExpr. This could lead to an assertion being triggered if the dead
; code elimination tries to delete last live instance of the ConstantExpr
; because the lookup in the hash table will fail.

; This test does not need to be converted to opaque pointers when switching to
; use opaque pointers by default because the GEP is elided from the IR in that
; case, so would not show the original issue.

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"

%"struct.std::complex" = type { { double, double } }

@counter_N0.ascast.red.__local = external addrspace(3) global %"struct.std::complex"

define weak void @__omp_offloading_10309_8e10c0d__Z34test_target_teams__distribute_simdv_l15() {
  store double 0.000000e+00, double addrspace(3)* getelementptr inbounds (%"struct.std::complex", %"struct.std::complex" addrspace(3)* @counter_N0.ascast.red.__local, i64 0, i32 0, i32 0), align 8
  %_M_value.real.i.i.i = load double, double addrspace(4)* getelementptr inbounds (%"struct.std::complex", %"struct.std::complex" addrspace(4)* addrspacecast (%"struct.std::complex" addrspace(3)* @counter_N0.ascast.red.__local to %"struct.std::complex" addrspace(4)*), i64 0, i32 0, i32 0), align 8
  %add.r.i.i22 = fadd fast double 0.000000e+00, %_M_value.real.i.i.i
  ret void
}

!llvm.module.flags = !{!0, !1, !2, !3, !4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"openmp-device", i32 50}
!3 = !{i32 7, !"PIC Level", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
