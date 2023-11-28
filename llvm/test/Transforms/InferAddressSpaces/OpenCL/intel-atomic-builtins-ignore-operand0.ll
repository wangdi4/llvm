; RUN: opt -passes="infer-address-spaces" -S -mtriple=x86_64-unknown-unknown -override-flat-addr-space=4 -infer-as-rewrite-opencl-bis %s | FileCheck %s

; This test is to check the atomic builtins with generic address space are
; ignored by InferAddressSpace pass, since generic address space is supported in
; underlying OpenCL CPU runtime. And it's not correct to infer generic address
; space to flat.

; Function Attrs: nounwind
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #0

; Function Attrs: nounwind
define void @__omp_offloading_811_6e0774__Z53test_target_teams_distribute_parallel_for_map_default_l37(ptr addrspace(1) %scalar2.ascast) local_unnamed_addr #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !9 !kernel_arg_type !10 !kernel_arg_type_qual !11 !kernel_arg_base_type !10 {
newFuncRoot:
  %scalar2.ascast.fpriv = alloca i32, align 4
  %0 = load i32, ptr addrspace(1) %scalar2.ascast, align 4
  store i32 %0, ptr %scalar2.ascast.fpriv, align 4

  %1 = tail call i64 @_Z13get_global_idj(i32 0) #0
  %2 = trunc i64 %1 to i32
  %3 = addrspacecast ptr %scalar2.ascast.fpriv to ptr addrspace(4)
  call void @_Z21atomic_store_explicitPU3AS4VU7_Atomicii12memory_order12memory_scope(ptr addrspace(4) %3, i32 %2, i32 0, i32 2) #0

; CHECK: call void @_Z21atomic_store_explicitPU3AS4VU7_Atomicii12memory_order12memory_scope(ptr addrspace(4) %{{.*}}, i32 %{{.*}}, i32 0, i32 2) #0
; CHECK-NOT: _Z21atomic_store_explicitPVU7_Atomicii12memory_order12memory_scope

  ret void
}

; Function Attrs: nounwind
; declare void @__kmpc_init_program(ptr addrspace(1)) local_unnamed_addr #0

; Function Attrs: nounwind
declare void @_Z21atomic_store_explicitPU3AS4VU7_Atomicii12memory_order12memory_scope(ptr addrspace(4), i32, i32, i32) local_unnamed_addr #0


attributes #0 = { nounwind }
attributes #1 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" }
attributes #2 = { argmemonly nounwind willreturn }

!spirv.MemoryModel = !{!0, !0, !0, !0, !0, !0}
!spirv.Source = !{!1, !1, !1, !1, !1, !1}
!opencl.spir.version = !{!2, !2, !2, !2, !2, !2}
!opencl.ocl.version = !{!2, !2, !2, !2, !2, !2}
!opencl.used.extensions = !{!3, !4, !4, !4, !4, !4}
!opencl.used.optional.core.features = !{!5, !4, !4, !5, !4, !5}
!spirv.Generator = !{!6, !6, !6, !6, !6, !6}
!opencl.enable.FP_CONTRACT = !{}
!opencl.kernels = !{!7}

!0 = !{i32 2, i32 2}
!1 = !{i32 4, i32 200000}
!2 = !{i32 2, i32 0}
!3 = !{!"cl_khr_int64_extended_atomics", !"cl_khr_subgroups"}
!4 = !{}
!5 = !{!"cl_doubles"}
!6 = !{i16 6, i16 14}
!7 = !{void (ptr addrspace(1))* @__omp_offloading_811_6e0774__Z53test_target_teams_distribute_parallel_for_map_default_l37}
!8 = !{i32 1}
!9 = !{!"none"}
!10 = !{!"int*"}
!11 = !{!""}
!12 = !{i32 1}
!13 = !{!"none"}
!14 = !{!"int*"}
!15 = !{!""}
!16 = !{i32 1, i32 1}
!17 = !{!"none", !"none"}
!18 = !{!"int*", !"int*"}
!19 = !{!"", !""}
!20 = !{!"void*"}
!21 = !{!"char*"}
