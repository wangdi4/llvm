; RUN: %oclopt -cleanup-wrapped-kernels -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -cleanup-wrapped-kernels -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; clean up functions preserving Metadata

; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32-msvc-elf"

define void @__test_if_separated_args() !kernel_wrapper !15 {
  ret void
}

; CHECK: declare !kernel_wrapper{{.*}}__test_if_separated_args

define void @test_if() {
  ret void
}

!opencl.kernels = !{!4}

!4 = !{void ()* @__test_if_separated_args}
!15 = !{void ()* @test_if}

; DEBUGIFY-NOT: WARNING
; __test_if_separated_args is wrapped, and will be removed in the pass, we ignore related warnings
; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing variable 1
; DEBUGIFY-NOT: WARNING
