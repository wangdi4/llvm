; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-vec-dim-analysis>' %s -S 2>&1 | FileCheck %s

declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #0

define void @test_optnone(ptr addrspace(1) nocapture %out, ptr addrspace(1) nocapture %in) noinline optnone !no_barrier_path !1 !kernel_has_sub_groups !2 !recommended_vector_length !3 !kernel_arg_base_type !4 !arg_type_null_val !5 {
  %gid.0 = call i64 @_Z13get_global_idj(i32 0)
  %gid.1 = call i64 @_Z13get_global_idj(i32 1)
  %gid.2 = call i64 @_Z13get_global_idj(i32 2)
  %use.0 = add i64 %gid.0, %gid.1
  %use.1 = add i64 %use.0, %gid.2
  %mul = mul i64 %gid.0, 2
  %load.addr = getelementptr i32, ptr addrspace(1) %in, i64 %mul
  %load = load i32, ptr addrspace(1) %load.addr  ; good for dim 1 (uniform), bad for dim 0 (strided)
  %store.addr = getelementptr i32, ptr addrspace(1) %out, i64 %gid.0
  store i32 %load, ptr addrspace(1) %store.addr  ; good for dim 1 (uniform), good for dim 0 (consecutive)
  ret void
}

; CHECK: VectorizationDimensionAnalysis for function test_optnone
; CHECK-NEXT: VectorizeDim 1

attributes #0 = { nounwind readnone }
attributes #0 = { nounwind }

!sycl.kernels = !{!0}

!0 = !{ptr @test_optnone}
!1 = !{i1 true}
!2 = !{i1 false}
!3 = !{i32 16}
!4 = !{!"int*", !"int*"}
!5 = !{ptr addrspace(1) null, ptr addrspace(1) null}
