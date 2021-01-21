; RUN: %oclopt %s -enable-direct-function-call-vectorization -vector-variant-lowering -sg-size-collector -sg-size-collector-indirect -vector-variant-isa-override=MIC -ocl-vecclone -ocl-vec-clone-isa-encoding-override=AVX512Core -vector-variant-fillin -update-call-attrs -S | FileCheck %s

define void @test(i32 addrspace(1)* noalias %a) !kernel_has_sub_groups !12 !ocl_recommended_vector_length !13 {
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

; CHECK-DAG: define void @_ZGVeN4u_test
; CHECK-DAG: define void @_ZGVeM4u_test
; CHECK-DAG: define void @_ZGV{{[bcde]}}M4_direct
; CHECK-DAG: define void @_ZGV{{[bcde]}}N4_direct

declare dso_local i32 (i32, float)** @_ZNKSt5arrayIPFiifELm2EE4dataEv()

declare i32 @__intel_indirect_call_i32_p0p0f_i32i32f32f(i32 (i32, float)**, ...)

attributes #0 = { "vector-variants"="_ZGVxN4u_test,_ZGVxM4u_test" }

; CHECK: attributes #[[ATTR0]] = { "vector-variants"="_ZGV{{[bcde]}}M4vv___intel_indirect_call_XXX,_ZGV{{[bcde]}}N4vv___intel_indirect_call_XXX" }

!opencl.kernels = !{!4}

!4 = !{void (i32 addrspace(1)*)* @test}
!12 = !{i1 true}
!13 = !{i32 4}
