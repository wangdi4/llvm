; Vectorizer does not support vectorization of functions with struct return type,
; so disable vectorization for kernel that contains both struct return type
; callees and sub groups

; RUN: opt -passes="sycl-kernel-add-function-attrs,print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

; CHECK: warning: kernel "kernel": Kernel can't be vectorized due to unsupported struct type return in callee
; CHECK: Kernel --> VF:
; CHECK:   <kernel> : 1
; CHECK: Kernel --> SGEmuSize:
; CHECK:   <kernel> : 16


define void @f1(float %x, ptr addrspace(1) noalias %a) #0 !kernel_arg_base_type !3 !arg_type_null_val !4 {
entry:
  %conv = fpext float %x to double
  %call = call {double, double} @direct(double %conv)
  call void @_Z18work_group_barrierj12memory_scope(i32 1, i32 1)
  ret void
}

define void @kernel(float %x, ptr addrspace(1) noalias %a) #0 !kernel_has_sub_groups !2 !intel_reqd_sub_group_size !1 !kernel_arg_base_type !3 !arg_type_null_val !4 {
entry:
  %conv1 = fpext float %x to double
  %call = call {double, double} @direct(double %conv1)
  call void @f1(float %x, ptr addrspace(1) noalias %a)
  ret void
}

declare void @_Z18work_group_barrierj12memory_scope(i32, i32)

define {double, double} @direct(double %val) #0 {
entry:
  %call.ii = tail call { double, double } @clog(double %val, double 0.000000e+00)
  ret { double, double } %call.ii
}

declare dso_local { double, double } @clog(double, double) local_unnamed_addr #0

attributes #0 = { "has-sub-groups" }

!sycl.kernels = !{!0}

!0 = !{ptr @kernel}
!1 = !{i32 16}
!2 = !{i1 true}
!3 = !{!"float", !"int*"}
!4 = !{float zeroinitializer, ptr addrspace(1) null}
