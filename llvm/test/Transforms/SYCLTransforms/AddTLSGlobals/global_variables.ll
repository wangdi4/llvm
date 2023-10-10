; This test checks that the pass adds the expected thread-local storage
; global variables to the module.

; RUN: opt -passes=sycl-kernel-add-tls-globals %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-tls-globals %s -S | FileCheck %s

; CHECK-NOT: @__pLocalMemBase

; CHECK: @__pWorkDim = internal thread_local global ptr undef
; CHECK: @__pWGId = internal thread_local global ptr undef
; CHECK: @__BaseGlbId = internal thread_local global [4 x i64] undef
; CHECK-NOT: @__pSpecialBuf
; CHECK: @__RuntimeHandle = internal thread_local global ptr undef
; CHECK: @__pBufferRanges = internal thread_local global ptr undef

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; DEBUGIFY-NOT: WARNING
