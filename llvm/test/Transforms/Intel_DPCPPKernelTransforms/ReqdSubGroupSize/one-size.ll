; Check that required sub group size is correctly set in metadata
; RUN: opt -dpcpp-kernel-reqd-sub-group-size -dpcpp-reqd-sub-group-size="foo(8)" %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-reqd-sub-group-size -dpcpp-reqd-sub-group-size="foo(8)" %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-reqd-sub-group-size -dpcpp-reqd-sub-group-size="foo(8)" %s -S -o - | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-reqd-sub-group-size -dpcpp-reqd-sub-group-size="foo(8)" %s -S -o - | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"


; CHECK: !intel_reqd_sub_group_size ![[subgroup_size:[0-9]+]]
; CHECK: ![[subgroup_size]] = !{i32 8}

declare void @"foo"(i32 addrspace(1)*)

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*)* @"foo"}

; DEBUGIFY-NOT: WARNING
