; Checks diagnostic message is emitted if WG/SG builtin with given VF is not implemented.

; RUN: not opt -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

; intel_reqd_sub_group_size doesn't allow fallback
; CHECK: error: kernel "test": Unimplemented function(s): _Z14work_group_alli with vectorization width 128
; CHECK-SAME: _Z13sub_group_alli with vectorization width 128
define void @test(i32 %a) "has-sub-groups" !kernel_has_sub_groups !{i1 true} !intel_reqd_sub_group_size !{i32 128} {
  %call = call i32 @_Z13sub_group_alli(i32 %a)
  %call1 = call i32 @_Z14work_group_alli(i32 %a)
  ret void
}

declare spir_func i32 @_Z13sub_group_alli(i32)
declare spir_func i32 @_Z14work_group_alli(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test}
