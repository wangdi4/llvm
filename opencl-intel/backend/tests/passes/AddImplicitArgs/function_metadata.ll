; RUN: %oclopt -add-implicit-args -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -add-implicit-args -S -verify < %s | FileCheck %s
; check the metadata is preserved correctly during transformations

; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: noduplicate nounwind
define void @UniformAddKernel() !vectorized_kernel !14 {
entry:
  ret void
}

; CHECK: @UniformAddKernel{{.*}} !vectorized_kernel ![[VEC:[0-9]+]]

; Function Attrs: noduplicate nounwind
define void @__Vectorized_.UniformAddKernel() !scalarized_kernel !4 {
entry:
  ret void
}

; CHECK: @__Vectorized_.UniformAddKernel{{.*}} !scalarized_kernel ![[SCAL:[0-9]+]]

!4 = !{void ()* @UniformAddKernel}
!14 = !{void ()* @__Vectorized_.UniformAddKernel}

; CHECK-DAG: ![[SCAL]] = !{void {{.*}} @UniformAddKernel}
; CHECK-DAG: ![[VEC]] = !{void {{.*}} @__Vectorized_.UniformAddKernel}

; DEBUGIFY-NOT: WARNING
