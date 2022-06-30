; Check that required sub group size is NOT set since the
; -dpcpp-reqd-sub-group-size option doesn't specify any function name and size.
; RUN: opt -passes=dpcpp-kernel-reqd-sub-group-size -dpcpp-reqd-sub-group-size="" %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-reqd-sub-group-size -dpcpp-reqd-sub-group-size="" %s -S -o - | FileCheck %s
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-reqd-sub-group-size -dpcpp-reqd-sub-group-size="" %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-reqd-sub-group-size -dpcpp-reqd-sub-group-size="" %s -S -o - | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"


; CHECK-NOT: !intel_reqd_sub_group_size

declare void @"foo"(i32 addrspace(1)*)

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*)* @"foo"}

; DEBUGIFY-NOT: WARNING
