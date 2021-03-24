; RUN: %oclopt -add-implicit-args -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -add-implicit-args -S -verify < %s | FileCheck %s
; check the metadata is preserved correctly during transformations

; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: noduplicate nounwind
define void @UniformAddKernel() {
entry:
  ret void
}

!opencl.kernels = !{!4}

; CHECK: !opencl.kernels = !{![[KERNELS:[0-9]+]]}

!4 = !{void ()* @UniformAddKernel}

; CHECK: ![[KERNELS]] = !{void {{.*}}* @UniformAddKernel}

; DEBUGIFY-NOT: WARNING
