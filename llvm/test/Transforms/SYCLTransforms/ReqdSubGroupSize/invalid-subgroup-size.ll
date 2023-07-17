; Check that invalid required sub group size is set for corresponding functions
; RUN: opt -passes=sycl-kernel-reqd-sub-group-size -sycl-reqd-sub-group-size="foo(-1),bar(4)" %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-reqd-sub-group-size -sycl-reqd-sub-group-size="foo(2),bar(-1)" %s -S -o - | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"


; CHECK-NOT: !intel_reqd_sub_group_size ![[subgroup_size_foo:[0-9]+]] void @foo
; CHECK-NOT: !intel_reqd_sub_group_size ![[subgroup_size_bar:[0-9]+]] void @bar

declare void @"foo"(ptr addrspace(1))

declare void @"bar"(ptr addrspace(1))

!sycl.kernels = !{!0}


!0 = !{ptr @"foo", ptr @"bar"}

; DEBUGIFY-NOT: WARNING

