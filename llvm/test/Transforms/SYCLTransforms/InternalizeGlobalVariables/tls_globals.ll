; This test checks that the TLS globals are not internalized by the pass

; RUN: opt -passes=sycl-kernel-add-tls-globals,sycl-kernel-internalize-global-variables %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-tls-globals,sycl-kernel-internalize-global-variables %s -S | FileCheck %s

; CHECK: @pWorkDim = linkonce_odr thread_local global ptr undef
; CHECK: @pWGId = linkonce_odr thread_local global ptr undef
; CHECK: @BaseGlbId = linkonce_odr thread_local global [4 x i64] undef
; CHECK-NOT: @pSpecialBuf
; CHECK: @RuntimeHandle = linkonce_odr thread_local global ptr undef

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; DEBUGIFY-NOT: WARNING
