; This test checks that the pass adds the expected thread-local storage
; global variables to the module.

; RUN: opt -dpcpp-kernel-add-tls-globals %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-tls-globals %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-add-tls-globals %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-add-tls-globals %s -S | FileCheck %s

; CHECK: @pLocalMemBase = linkonce_odr thread_local global i8 addrspace(3)* undef
; CHECK: @pWorkDim = linkonce_odr thread_local global { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* undef
; CHECK: @pWGId = linkonce_odr thread_local global i64* undef
; CHECK: @BaseGlbId = linkonce_odr thread_local global [4 x i64] undef
; CHECK: @pSpecialBuf = linkonce_odr thread_local global i8* undef
; CHECK: @RuntimeHandle = linkonce_odr thread_local global {}* undef

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; DEBUGIFY-NOT: WARNING
