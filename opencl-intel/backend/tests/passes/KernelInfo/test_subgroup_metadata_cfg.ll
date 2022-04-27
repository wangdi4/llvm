; RUN: %oclopt -kernel-sub-group-info -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -kernel-sub-group-info -S < %s | FileCheck %s
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-LABEL: @testKernel
; CHECK: [[ATTR:#[0-9]+]]
; CHECK: !kernel_has_sub_groups [[SGMD:![0-9]+]]
define void @testKernel(i32 addrspace(1)* noalias %sub_groups_sizes) {
entry:
  tail call void @callee(i32 addrspace(1)* noalias %sub_groups_sizes)
  ret void
}

; CHECK-LABEL: @callee
; CHECK: [[ATTR]] {
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

; CHECK: attributes [[ATTR]] = { "has-sub-groups" }

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*)* @testKernel}
; CHECK: [[SGMD]] = !{i1 true}

; DEBUGIFY-NOT: WARNING
