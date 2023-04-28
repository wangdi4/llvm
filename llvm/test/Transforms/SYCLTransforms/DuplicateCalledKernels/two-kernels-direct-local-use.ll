; RUN: opt -passes=sycl-kernel-duplicate-called-kernels %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-duplicate-called-kernels %s -S | FileCheck %s

; Check local variable @foo.v_local2 is cloned since it is directly used in two
; kernels.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@foo.v_local1 = internal addrspace(3) global i32 undef, align 4
@foo.v_local2 = internal addrspace(3) global i32 undef, align 4

; CHECK: @foo.v_local1 = internal addrspace(3) global i32 undef, align 4
; CHECK: @foo.v_local2 = internal addrspace(3) global i32 undef, align 4
; CHECK: @foo.v_local2.clone = internal addrspace(3) global i32 undef, align 4

define dso_local void @foo(ptr addrspace(1) %result) {
entry:
; CHECK-LABEL: define dso_local void @foo(
; CHECK: store i32 33, ptr addrspace(3) @foo.v_local1,
; CHECK: load i32, ptr addrspace(3) @foo.v_local2,
;
  store i32 33, ptr addrspace(3) @foo.v_local1, align 4
  %0 = load i32, ptr addrspace(3) @foo.v_local2, align 4
  ret void
}

define dso_local void @bar(ptr addrspace(1) %result) {
entry:
; CHECK-LABEL: define dso_local void @bar(
; CHECK: store i32 66, ptr addrspace(3) @foo.v_local2.clone,
; CHECK: store i32 0, ptr addrspacecast (ptr addrspace(3) @foo.v_local2.clone to ptr), align 4
;
  store i32 66, ptr addrspace(3) @foo.v_local2, align 4
  store i32 0, ptr addrspacecast (ptr addrspace(3) @foo.v_local2 to ptr), align 4
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @foo, ptr @bar}

; DEBUGIFY-NOT: WARNING
