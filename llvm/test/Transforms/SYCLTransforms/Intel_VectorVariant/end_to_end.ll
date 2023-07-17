; RUN: opt %s -sycl-enable-direct-function-call-vectorization -passes="sycl-kernel-vector-variant-lowering,sycl-kernel-sg-size-collector,sycl-kernel-sg-size-collector-indirect,sycl-kernel-vec-clone,sycl-kernel-vector-variant-fillin,sycl-kernel-update-call-attrs" -S | FileCheck %s
; RUN: opt %s -enable-debugify -disable-output -sycl-enable-direct-function-call-vectorization -passes="sycl-kernel-vector-variant-lowering,sycl-kernel-sg-size-collector,sycl-kernel-sg-size-collector-indirect,sycl-kernel-vec-clone,sycl-kernel-vector-variant-fillin,sycl-kernel-update-call-attrs" -S 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

define void @test(ptr addrspace(1) noalias %a) !kernel_has_sub_groups !12 !recommended_vector_length !13 !kernel_arg_base_type !14 !arg_type_null_val !15 {
entry:
  call void() @direct() #0

  ret void
}

define void @direct() {
entry:
  %call.i = call ptr @_ZNKSt5arrayIPFiifELm2EE4dataEv()
  call i32 (ptr, ...) @__intel_indirect_call_i32_p0p0f_i32i32f32f(ptr %call.i, i32 5, float 2.000000e+00)
; CHECK: call i32 (ptr, ...) @__intel_indirect_call_i32_p0p0f_i32i32f32f(ptr %call.i, i32 5, float 2.000000e+00) #[[ATTR0:.*]]

  ret void
}

; CHECK-DAG: define void @_ZGVbN4u_test
; CHECK-DAG: define void @_ZGVbM4u_test
; CHECK-DAG: define void @_ZGVbM4_direct
; CHECK-DAG: define void @_ZGVbN4_direct

declare dso_local ptr @_ZNKSt5arrayIPFiifELm2EE4dataEv()

declare i32 @__intel_indirect_call_i32_p0p0f_i32i32f32f(ptr, ...)

attributes #0 = { "vector-variants"="_ZGV_unknown_N4u_test,_ZGV_unknown_M4u_test" }

; CHECK: attributes #[[ATTR0]] = { "vector-variants"="_ZGVbM4vv___intel_indirect_call_XXX,_ZGVbN4vv___intel_indirect_call_XXX" }

!sycl.kernels = !{!4}

!4 = !{ptr @test}
!12 = !{i1 true}
!13 = !{i32 4}
!14 = !{!"int*"}
!15 = !{ptr addrspace(1) null}

; Intructions of SIMD loop
; DEBUGIFY-COUNT-44: WARNING: Instruction with empty DebugLoc
; DEBUGIFY-NOT: WARNING
