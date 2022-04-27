; RUN: opt -dpcpp-enable-native-subgroups -passes=dpcpp-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-enable-native-subgroups -dpcpp-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-enable-native-subgroups -passes=dpcpp-kernel-analysis %s -S | FileCheck %s
; RUN: opt -dpcpp-enable-native-subgroups -dpcpp-kernel-analysis %s -S | FileCheck %s

; RUN: opt -dpcpp-enable-native-subgroups -passes=dpcpp-kernel-analysis %s -S -debug -disable-output 2>&1| FileCheck %s -check-prefix=CHECK-DEBUG
; RUN: opt -dpcpp-enable-native-subgroups -dpcpp-kernel-analysis %s -S -debug -disable-output 2>&1| FileCheck %s -check-prefix=CHECK-DEBUG

; CHECK-DEBUG: DPCPPKernelAnalysisPass
; CHECK-DEBUG-DAG: Kernel <testKernel1>: {{.*}} KernelHasSubgroups=1
; CHECK-DEBUG-DAG: Kernel <testKernel2>: {{.*}} KernelHasSubgroups=0
; CHECK-DEBUG-DAG: Kernel <testKernel3>: {{.*}} KernelHasSubgroups=1

; CHECK-DEBUG: Functions that call subgroup builtins:
; CHECK-DEBUG-DAG: testKernel1
; CHECK-DEBUG-DAG: testKernel3
; CHECK-DEBUG-DAG: callee

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-LABEL: define void @testKernel1
; CHECK-SAME: #[[#ATTR:]]
; CHECK-SAME: !kernel_has_sub_groups ![[#SGMD_TRUE:]]
define void @testKernel1(i32 addrspace(1)* noalias %sub_groups_sizes) {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0)
  %call1 = tail call i32 @_Z22get_max_sub_group_sizev()
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %sub_groups_sizes, i64 %call
  store i32 %call1, i32 addrspace(1)* %arrayidx, align 4
  ret void
}

; CHECK-LABEL: define void @testKernel2
; CHECK-NOT: #
; CHECK-SAME: !kernel_has_sub_groups ![[#SGMD_FALSE:]]
define void @testKernel2() {
entry:
  ret void
}

; CHECK-LABEL: define void @testKernel3
; CHECK-SAME: #[[#ATTR]]
; CHECK-SAME: !kernel_has_sub_groups ![[#SGMD_TRUE]]
define void @testKernel3(i32 addrspace(1)* noalias %sub_groups_sizes) {
entry:
  tail call void @callee(i32 addrspace(1)* noalias %sub_groups_sizes)
  ret void
}

; CHECK-LABEL: define void @callee
; CHECK-SAME: #[[#ATTR]] {
define void @callee(i32 addrspace(1)* noalias %sub_groups_sizes) {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0)
  %call1 = tail call i32 @_Z22get_max_sub_group_sizev()
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %sub_groups_sizes, i64 %call
  store i32 %call1, i32 addrspace(1)* %arrayidx, align 4
  ret void
}

declare i64 @_Z13get_global_idj(i32)

declare i32 @_Z22get_max_sub_group_sizev()

declare i64 @_Z14get_local_sizej(i32)

; CHECK: attributes #[[#ATTR]] = { "has-sub-groups" }

!sycl.kernels = !{!0}
!0 = !{void (i32 addrspace(1)*)* @testKernel1, void ()* @testKernel2, void (i32 addrspace(1)*)* @testKernel3}

; CHECK: ![[#SGMD_TRUE]] = !{i1 true}
; CHECK: ![[#SGMD_FALSE]] = !{i1 false}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: PASS
