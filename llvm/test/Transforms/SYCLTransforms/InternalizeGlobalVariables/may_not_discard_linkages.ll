; This test checks that a global with common, external, weak or weak_odr linkage
; in global address space is not internalized by the pass.

; RUN: opt -passes=sycl-kernel-internalize-global-variables,globaldce %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-internalize-global-variables,globaldce %s -S | FileCheck %s

; CHECK: @a
; CHECK: @b
; CHECK: @c
; CHECK: @d

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@a = addrspace(1) global i32 0, align 4
@b = common addrspace(1) global i32 0, align 4
@c = weak addrspace(1) global i32 0, align 4
@d = weak_odr addrspace(1) global i32 0, align 4

; DEBUGIFY-NOT: WARNING
