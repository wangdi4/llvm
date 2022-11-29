; RUN: opt -passes='dpcpp-kernel-preprocess-spv-ir' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-preprocess-spv-ir' -S %s | FileCheck %s

define void @foo() {
  ret void
}

; SYCL program
!spirv.Source = !{!0}
!0 = !{i32 4, i32 100000}

; CHECK: !opencl.ocl.version = !{![[#ID:]]}
; CHECK: ![[#ID]] = !{i32 2, i32 0}

; DEBUGIFY-NOT: WARNING
