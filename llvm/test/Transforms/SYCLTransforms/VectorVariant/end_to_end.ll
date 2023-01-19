; RUN: opt %s -dpcpp-enable-direct-function-call-vectorization -passes="dpcpp-kernel-vector-variant-lowering,dpcpp-kernel-sg-size-collector,dpcpp-kernel-sg-size-collector-indirect,dpcpp-kernel-vec-clone,dpcpp-kernel-vector-variant-fillin,dpcpp-kernel-update-call-attrs" -S | FileCheck %s
; RUN: opt %s -enable-debugify -disable-output -dpcpp-enable-direct-function-call-vectorization -passes="dpcpp-kernel-vector-variant-lowering,dpcpp-kernel-sg-size-collector,dpcpp-kernel-sg-size-collector-indirect,dpcpp-kernel-vec-clone,dpcpp-kernel-vector-variant-fillin,dpcpp-kernel-update-call-attrs" -S 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

define void @test(i32 addrspace(1)* noalias %a) !kernel_has_sub_groups !12 !recommended_vector_length !13 {
entry:
  call void() @direct() #0

  ret void
}

define void @direct() {
entry:
  %call.i = call i32 (i32, float)** @_ZNKSt5arrayIPFiifELm2EE4dataEv()
  call i32 (i32 (i32, float)**, ...) @__intel_indirect_call_i32_p0p0f_i32i32f32f(i32 (i32, float)** %call.i, i32 5, float 2.000000e+00)
; CHECK: call i32 (i32 (i32, float)**, ...) @__intel_indirect_call_i32_p0p0f_i32i32f32f(i32 (i32, float)** %call.i, i32 5, float 2.000000e+00) #[[ATTR0:.*]]

  ret void
}

; CHECK-DAG: define void @_ZGVbN4u_test
; CHECK-DAG: define void @_ZGVbM4u_test
; CHECK-DAG: define void @_ZGVbM4_direct
; CHECK-DAG: define void @_ZGVbN4_direct

declare dso_local i32 (i32, float)** @_ZNKSt5arrayIPFiifELm2EE4dataEv()

declare i32 @__intel_indirect_call_i32_p0p0f_i32i32f32f(i32 (i32, float)**, ...)

attributes #0 = { "vector-variants"="_ZGVxN4u_test,_ZGVxM4u_test" }

; CHECK: attributes #[[ATTR0]] = { "vector-variants"="_ZGVbM4vv___intel_indirect_call_XXX,_ZGVbN4vv___intel_indirect_call_XXX" }

!sycl.kernels = !{!4}

!4 = !{void (i32 addrspace(1)*)* @test}
!12 = !{i1 true}
!13 = !{i32 4}

; Intructions of SIMD loop
; DEBUGIFY-COUNT-46: WARNING: Instruction with empty DebugLoc
; DEBUGIFY-NOT: WARNING
