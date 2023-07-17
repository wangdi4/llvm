; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-vec-dim-analysis>' %s -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #0

define void @test_0(ptr addrspace(1) nocapture %out, ptr addrspace(1) nocapture %in) !no_barrier_path !1 !kernel_has_sub_groups !2 !recommended_vector_length !3 !kernel_arg_base_type !4 !arg_type_null_val !5 {
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

; CHECK: VectorizationDimensionAnalysis for function test_0
; CHECK-NEXT: VectorizeDim 1

define void @test_1(ptr addrspace(1) nocapture %out, ptr addrspace(1) nocapture %in) !no_barrier_path !1 !kernel_has_sub_groups !2 !recommended_vector_length !3 !kernel_arg_base_type !4 !arg_type_null_val !5 {
  %gid.0 = call i64 @_Z13get_global_idj(i32 0)
  %gid.1 = call i64 @_Z13get_global_idj(i32 1)
  %gid.2 = call i64 @_Z13get_global_idj(i32 2)
  %use.0 = add i64 %gid.0, %gid.1
  %use.1 = add i64 %use.0, %gid.2
  %load.addr = getelementptr i32, ptr addrspace(1) %in, i64 %gid.0
  %load = load i32, ptr addrspace(1) %load.addr  ; good for both dim 0 (consecutive) and 1 (uniform)
  %store.addr = getelementptr i32, ptr addrspace(1) %out, i64 %gid.1
  store i32 %load, ptr addrspace(1) %store.addr  ; good for both dim 0 (uniform) and 1 (consecutive)
  ret void
}

; CHECK: VectorizationDimensionAnalysis for function test_1
; CHECK-NEXT: VectorizeDim 0

; Same as test_0, but with no_barrier_path being false, choose dim 0.
define void @test_2(ptr addrspace(1) nocapture %out, ptr addrspace(1) nocapture %in) !no_barrier_path !2 !kernel_has_sub_groups !2 !recommended_vector_length !3 !kernel_arg_base_type !4 !arg_type_null_val !5 {
  %gid.0 = call i64 @_Z13get_global_idj(i32 0)
  %gid.1 = call i64 @_Z13get_global_idj(i32 1)
  %gid.2 = call i64 @_Z13get_global_idj(i32 2)
  %use.0 = add i64 %gid.0, %gid.1
  %use.1 = add i64 %use.0, %gid.2
  %mul = mul i64 %gid.0, 2
  %load.addr = getelementptr i32, ptr addrspace(1) %in, i64 %mul
  %load = load i32, ptr addrspace(1) %load.addr
  %store.addr = getelementptr i32, ptr addrspace(1) %out, i64 %gid.0
  store i32 %load, ptr addrspace(1) %store.addr
  ret void
}

; CHECK: VectorizationDimensionAnalysis for function test_2
; CHECK-NEXT: VectorizeDim 0

; Same as test_0, but with kernel_has_sub_groups being true, choose dim 0.
define void @test_3(ptr addrspace(1) nocapture %out, ptr addrspace(1) nocapture %in) !no_barrier_path !1 !kernel_has_sub_groups !1 !recommended_vector_length !3 !kernel_arg_base_type !4 !arg_type_null_val !5 {
  %gid.0 = call i64 @_Z13get_global_idj(i32 0)
  %gid.1 = call i64 @_Z13get_global_idj(i32 1)
  %gid.2 = call i64 @_Z13get_global_idj(i32 2)
  %use.0 = add i64 %gid.0, %gid.1
  %use.1 = add i64 %use.0, %gid.2
  %mul = mul i64 %gid.0, 2
  %load.addr = getelementptr i32, ptr addrspace(1) %in, i64 %mul
  %load = load i32, ptr addrspace(1) %load.addr
  %store.addr = getelementptr i32, ptr addrspace(1) %out, i64 %gid.0
  store i32 %load, ptr addrspace(1) %store.addr
  ret void
}

; CHECK: VectorizationDimensionAnalysis for function test_3
; CHECK-NEXT: VectorizeDim 0

define void @test_4(ptr addrspace(1) nocapture %out, ptr addrspace(1) nocapture %in) !no_barrier_path !1 !kernel_has_sub_groups !2 !recommended_vector_length !3 !kernel_arg_base_type !6 !arg_type_null_val !7 {
  %gid.0 = call i64 @_Z13get_global_idj(i32 0)
  %gid.1 = call i64 @_Z13get_global_idj(i32 1)
  %gid.2 = call i64 @_Z13get_global_idj(i32 2)
  %use.0 = add i64 %gid.0, %gid.1
  %use.1 = add i64 %use.0, %gid.2
  %load.addr = getelementptr <4 x i32>, ptr addrspace(1) %in, i64 %gid.0
  %load = load <4 x i32>, ptr addrspace(1) %load.addr  ; good for dim 1 (uniform), bad for dim 0 (consecutive on vector type)
  %store.addr = getelementptr <4 x i32>, ptr addrspace(1) %out, i64 %gid.0
  store <4 x i32> %load, ptr addrspace(1) %store.addr  ; good for dim 1 (uniform), bad for dim 0 (consecutive on vector type)
  ret void
}

; CHECK: VectorizationDimensionAnalysis for function test_4
; CHECK-NEXT: VectorizeDim 1

define void @test_5(ptr addrspace(1) nocapture %out, ptr addrspace(1) nocapture %in) !no_barrier_path !1 !kernel_has_sub_groups !2 !recommended_vector_length !3 !kernel_arg_base_type !4 !arg_type_null_val !5 {
  %gid.0 = call i64 @_Z13get_global_idj(i32 0)
  %gid.1 = call i64 @_Z13get_global_idj(i32 1)
  %gid.2 = call i64 @_Z13get_global_idj(i32 2)
  %use.0 = add i64 %gid.0, %gid.1
  %use.1 = add i64 %use.0, %gid.2
  %load.addr = getelementptr i32, ptr addrspace(1) %in, i64 %gid.0
  %load = load i32, ptr addrspace(1) %load.addr  ; good for both dim 0 (consecutive) and 1, 2 (uniform)
  %cond = icmp ne i64 %gid.0, %gid.1
  br i1 %cond, label %t, label %return

t: ; non-divergent block for dim 0 and dim 1, thus choose dim 2
  %store.addr = getelementptr i32, ptr addrspace(1) %out, i64 %gid.1
  store i32 %load, ptr addrspace(1) %store.addr  ; good for both dim 0, 2 (uniform) and 1 (consecutive)
  br label %return

return:
  ret void
}

; CHECK: VectorizationDimensionAnalysis for function test_5
; CHECK-NEXT: VectorizeDim 2

attributes #0 = { nounwind readnone }
attributes #0 = { nounwind }

!sycl.kernels = !{!0}

!0 = !{ptr @test_0, ptr @test_1, ptr @test_2, ptr @test_3, ptr @test_4, ptr @test_5}
!1 = !{i1 true}
!2 = !{i1 false}
!3 = !{i32 16}
!4 = !{!"int*", !"int*"}
!5 = !{ptr addrspace(1) null, ptr addrspace(1) null}
!6 = !{!"int4*", !"int4*"}
!7 = !{ptr addrspace(1) null, ptr addrspace(1) null}
