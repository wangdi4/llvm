; Checks unimplemented builtin can fallback to heuristic VF in the context of "intel_vec_len_hint".

; RUN: opt -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

; intel_vec_len_hint allow fallback
; CHECK: warning: kernel "test": Fall back vectorization width to 4 due to unsupported vec_len_hint value for workgroup/subgroup builtins

; CHECK-NOT: error
; CHECK-LABEL: Kernel --> VF:
; CHECK: <test> : 4
; CHECK-LABEL: Kernel --> SGEmuSize:
define void @test(i32 %a) "has-sub-groups" !kernel_has_sub_groups !{i1 true} !intel_vec_len_hint !{i32 128} {
  %call = call i32 @_Z13sub_group_alli(i32 %a)
  ret void
}

declare spir_func i32 @_Z13sub_group_alli(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test}
