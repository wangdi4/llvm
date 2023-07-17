; Check that required sub group size is correctly set in metadata for two
; functions
; RUN: opt -passes=sycl-kernel-reqd-sub-group-size -sycl-reqd-sub-group-size="foo(8),bar(4)" %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-reqd-sub-group-size -sycl-reqd-sub-group-size="foo(8),bar(4)" %s -S -o - | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"


; CHECK-DAG: !intel_reqd_sub_group_size ![[subgroup_size_foo:[0-9]+]] void @foo
; CHECK-DAG: !intel_reqd_sub_group_size ![[subgroup_size_bar:[0-9]+]] void @bar

; CHECK-DAG: ![[subgroup_size_foo]] = !{i32 8}
; CHECK-DAG: ![[subgroup_size_bar]] = !{i32 4}

declare void @"foo"(ptr addrspace(1))

declare void @"bar"(ptr addrspace(1))

!sycl.kernels = !{!0}


!0 = !{ptr @"foo", ptr @"bar"}

; DEBUGIFY-NOT: WARNING

