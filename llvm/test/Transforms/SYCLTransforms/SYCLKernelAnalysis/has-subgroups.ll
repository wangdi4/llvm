; RUN: opt -passes=sycl-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-analysis %s -S | FileCheck %s

; RUN: opt -passes=sycl-kernel-analysis %s -S -debug -disable-output 2>&1| FileCheck %s -check-prefix=CHECK-DEBUG

; CHECK-DEBUG: SYCLKernelAnalysisPass
; CHECK-DEBUG: Kernel <test1>:
; CHECK-DEBUG:   KernelHasSubgroups=1
; CHECK-DEBUG: Kernel <test2>:
; CHECK-DEBUG:   KernelHasSubgroups=0
; CHECK-DEBUG: Kernel <test3>:
; CHECK-DEBUG:   KernelHasSubgroups=1
; CHECK-DEBUG: Kernel <test4>:
; CHECK-DEBUG:   KernelHasSubgroups=1
; CHECK-DEBUG: Kernel <test5>:
; CHECK-DEBUG:   KernelHasSubgroups=1
; CHECK-DEBUG: Kernel <test6>:
; CHECK-DEBUG:   KernelHasSubgroups=1
; CHECK-DEBUG: Kernel <test7>:
; CHECK-DEBUG:   KernelHasSubgroups=1
; CHECK-DEBUG: Kernel <test8>:
; CHECK-DEBUG:   KernelHasSubgroups=1
; CHECK-DEBUG: Kernel <test9>:
; CHECK-DEBUG:   KernelHasSubgroups=1
; CHECK-DEBUG: Kernel <test10>:
; CHECK-DEBUG:   KernelHasSubgroups=1
; CHECK-DEBUG: Kernel <test11>:
; CHECK-DEBUG:   KernelHasSubgroups=1
; CHECK-DEBUG: Kernel <test12>:
; CHECK-DEBUG:   KernelHasSubgroups=1
; CHECK-DEBUG: Kernel <test13>:
; CHECK-DEBUG:   KernelHasSubgroups=1
; CHECK-DEBUG: Kernel <test14>:
; CHECK-DEBUG:   KernelHasSubgroups=1
; CHECK-DEBUG: Kernel <test15>:
; CHECK-DEBUG:   KernelHasSubgroups=1
; CHECK-DEBUG: Kernel <test16>:
; CHECK-DEBUG:   KernelHasSubgroups=1

; CHECK-DEBUG: Functions that call subgroup builtins:
; CHECK-DEBUG-DAG: test1
; CHECK-DEBUG-DAG: test3
; CHECK-DEBUG-DAG: callee
; CHECK-DEBUG-DAG: test4
; CHECK-DEBUG-DAG: test5
; CHECK-DEBUG-DAG: test6
; CHECK-DEBUG-DAG: test7
; CHECK-DEBUG-DAG: test8
; CHECK-DEBUG-DAG: test9
; CHECK-DEBUG-DAG: test10
; CHECK-DEBUG-DAG: test11
; CHECK-DEBUG-DAG: test12
; CHECK-DEBUG-DAG: test13
; CHECK-DEBUG-DAG: test14
; CHECK-DEBUG-DAG: test15
; CHECK-DEBUG-DAG: test16

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-LABEL: define void @test1(
; CHECK-SAME: [[ATTR_HAS_SG:#[0-9]+]]
; CHECK-SAME: !kernel_has_sub_groups [[SGMD_TRUE:![0-9]+]]
define void @test1(ptr addrspace(1) noalias %sub_groups_sizes) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0)
  %call1 = tail call i32 @_Z22get_max_sub_group_sizev()
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %sub_groups_sizes, i64 %call
  store i32 %call1, ptr addrspace(1) %arrayidx, align 4
  ret void
}

; CHECK-LABEL: define void @test2(
; CHECK-NOT: #
; CHECK-SAME: !kernel_has_sub_groups [[SGMD_FALSE:![0-9]+]]
define void @test2() {
entry:
  ret void
}

; CHECK: define void @test3({{.*}} [[ATTR_HAS_SG]] {{.*}} !kernel_has_sub_groups [[SGMD_TRUE]]
define void @test3(ptr addrspace(1) noalias %sub_groups_sizes) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  tail call void @callee(ptr addrspace(1) noalias %sub_groups_sizes)
  ret void
}

; CHECK: define void @callee({{.*}} [[ATTR_HAS_SG]]
define void @callee(ptr addrspace(1) noalias %sub_groups_sizes) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0)
  %call1 = tail call i32 @_Z22get_max_sub_group_sizev()
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %sub_groups_sizes, i64 %call
  store i32 %call1, ptr addrspace(1) %arrayidx, align 4
  ret void
}

; CHECK: define void @test4({{.*}} [[ATTR_HAS_SG]] {{.*}} !kernel_has_sub_groups [[SGMD_TRUE]]
define void @test4(ptr addrspace(1) noundef align 4 %dst) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %call = tail call <4 x i32> @_Z16sub_group_balloti(i32 noundef 0) #0
  ret void
}

; CHECK: define void @test5({{.*}} [[ATTR_HAS_SG]] {{.*}} !kernel_has_sub_groups [[SGMD_TRUE]]
define void @test5(ptr addrspace(1) noundef align 4 %src) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %call = tail call i32 @_Z26intel_sub_group_block_readPU3AS1Kj(ptr addrspace(1) noundef %src) #0
  ret void
}

; CHECK: define void @test6({{.*}} [[ATTR_HAS_SG]] {{.*}} !kernel_has_sub_groups [[SGMD_TRUE]]
define void @test6(ptr addrspace(1) noundef align 4 %src) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %call = tail call <4 x i32> @_Z27intel_sub_group_block_read4PU3AS1Kj(ptr addrspace(1) noundef %src) #0
  ret void
}

; CHECK: define void @test7({{.*}} [[ATTR_HAS_SG]] {{.*}} !kernel_has_sub_groups [[SGMD_TRUE]]
define void @test7(ptr addrspace(1) noundef align 4 %dst) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  tail call void @_Z27intel_sub_group_block_writePU3AS1jj(ptr addrspace(1) noundef %dst, i32 noundef 0) #0
  ret void
}

; CHECK: define void @test8({{.*}} [[ATTR_HAS_SG]] {{.*}} !kernel_has_sub_groups [[SGMD_TRUE]]
define void @test8(ptr addrspace(1) noundef align 4 %dst) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  tail call void @_Z28intel_sub_group_block_write4PU3AS1jDv4_j(ptr addrspace(1) noundef %dst, <4 x i32> noundef zeroinitializer) #0
  ret void
}

; CHECK: define void @test9({{.*}} [[ATTR_HAS_SG]] {{.*}} !kernel_has_sub_groups [[SGMD_TRUE]]
define void @test9(ptr addrspace(1) noundef align 4 %dst) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %0 = tail call i32 @_Z17sub_group_shuffleij(i32 noundef 0, i32 noundef 0) #0
  ret void
}

; CHECK: define void @test10({{.*}} [[ATTR_HAS_SG]] {{.*}} !kernel_has_sub_groups [[SGMD_TRUE]]
define void @test10(ptr addrspace(1) noundef align 4 %dst) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %0 = tail call i32 @_Z21sub_group_shuffle_xorij(i32 noundef 0, i32 noundef 0) #0
  ret void
}

; CHECK: define void @test11({{.*}} [[ATTR_HAS_SG]] {{.*}} !kernel_has_sub_groups [[SGMD_TRUE]]
define void @test11(ptr addrspace(1) noundef align 4 %dst) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %0 = tail call i32 @_Z20sub_group_shuffle_upij(i32 noundef 0, i32 noundef 0) #0
  ret void
}

; CHECK: define void @test12({{.*}} [[ATTR_HAS_SG]] {{.*}} !kernel_has_sub_groups [[SGMD_TRUE]]
define void @test12(ptr addrspace(1) noundef align 4 %dst) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %0 = tail call i32 @_Z22sub_group_shuffle_downij(i32 noundef 0, i32 noundef 0) #0
  ret void
}

; CHECK: define void @test13({{.*}} [[ATTR_HAS_SG]] {{.*}} !kernel_has_sub_groups [[SGMD_TRUE]]
define void @test13(ptr addrspace(1) noundef align 4 %dst) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %0 = tail call i32 @_Z23intel_sub_group_shuffleij(i32 noundef 0, i32 noundef 0) #0
  ret void
}

; CHECK: define void @test14({{.*}} [[ATTR_HAS_SG]] {{.*}} !kernel_has_sub_groups [[SGMD_TRUE]]
define void @test14(ptr addrspace(1) noundef align 4 %dst) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %0 = tail call i32 @_Z27intel_sub_group_shuffle_xorij(i32 noundef 0, i32 noundef 0) #0
  ret void
}

; CHECK: define void @test15({{.*}} [[ATTR_HAS_SG]] {{.*}} !kernel_has_sub_groups [[SGMD_TRUE]]
define void @test15(ptr addrspace(1) noundef align 4 %dst) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %0 = tail call i32 @_Z26intel_sub_group_shuffle_upiij(i32 noundef 0, i32 noundef 0, i32 noundef 0) #0
  ret void
}

; CHECK: define void @test16({{.*}} [[ATTR_HAS_SG]] {{.*}} !kernel_has_sub_groups [[SGMD_TRUE]]
define void @test16(ptr addrspace(1) noundef align 4 %dst) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %0 = tail call i32 @_Z28intel_sub_group_shuffle_downiij(i32 noundef 0, i32 noundef 0, i32 noundef 0) #0
  ret void
}

declare i64 @_Z13get_global_idj(i32)

declare i32 @_Z22get_max_sub_group_sizev()

declare i64 @_Z14get_local_sizej(i32)

; Function Attrs: convergent
declare <4 x i32> @_Z16sub_group_balloti(i32 noundef) #0

; Function Attrs: convergent
declare i32 @_Z26intel_sub_group_block_readPU3AS1Kj(ptr addrspace(1) noundef) #0

; Function Attrs: convergent
declare <4 x i32> @_Z27intel_sub_group_block_read4PU3AS1Kj(ptr addrspace(1) noundef) #0

; Function Attrs: convergent
declare void @_Z27intel_sub_group_block_writePU3AS1jj(ptr addrspace(1) noundef, i32 noundef) #0

; Function Attrs: convergent
declare void @_Z28intel_sub_group_block_write4PU3AS1jDv4_j(ptr addrspace(1) noundef, <4 x i32> noundef) #0

; Function Attrs: convergent
declare i32 @_Z17sub_group_shuffleij(i32 noundef, i32 noundef) #0

; Function Attrs: convergent
declare i32 @_Z21sub_group_shuffle_xorij(i32 noundef, i32 noundef) #0

; Function Attrs: convergent
declare i32 @_Z20sub_group_shuffle_upij(i32 noundef, i32 noundef) #0

; Function Attrs: convergent
declare i32 @_Z22sub_group_shuffle_downij(i32 noundef, i32 noundef) #0

; Function Attrs: convergent
declare i32 @_Z23intel_sub_group_shuffleij(i32 noundef, i32 noundef) #0

; Function Attrs: convergent
declare i32 @_Z27intel_sub_group_shuffle_xorij(i32 noundef, i32 noundef) #0

; Function Attrs: convergent
declare i32 @_Z26intel_sub_group_shuffle_upiij(i32 noundef, i32 noundef, i32 noundef) #0

; Function Attrs: convergent
declare i32 @_Z28intel_sub_group_shuffle_downiij(i32 noundef, i32 noundef, i32 noundef) #0

; CHECK: attributes [[ATTR_HAS_SG]] = { "has-sub-groups" }

attributes #0 = { convergent }

!sycl.kernels = !{!0}
!0 = !{ptr @test1, ptr @test2, ptr @test3, ptr @test4, ptr @test5, ptr @test6, ptr @test7, ptr @test8, ptr @test9, ptr @test10, ptr @test11, ptr @test12, ptr @test13, ptr @test14, ptr @test15, ptr @test16}
!1 = !{!"int*"}
!2 = !{ptr addrspace(1) null}

; CHECK-DAG: [[SGMD_FALSE]] = !{i1 false}
; CHECK-DAG: [[SGMD_TRUE]] = !{i1 true}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: PASS
