; RUN: opt -passes=sycl-kernel-equalizer -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-equalizer -S %s | FileCheck %s

; Check that kernel linkage is changed to external.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: define dso_local void @test()

define internal spir_kernel void @test() {
  ret void
}

; DEBUGIFY-NOT: WARNING
