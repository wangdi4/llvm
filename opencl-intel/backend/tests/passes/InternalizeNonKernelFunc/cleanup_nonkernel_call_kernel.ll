; RUN: %oclopt -internalize-nonkernel-functions -globaldce -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -internalize-nonkernel-functions -globaldce -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; All non-kernels should be removed

define i32 @thisIsNotKernel() nounwind {
entry:
  call void @anotherNotKernel()
  %w = add i32 1, 1
  ret i32 %w
}

define void @anotherNotKernel() nounwind {
entry:
  call void @thisIsKernel()
  ret void
}

define void @thisIsKernel() nounwind !kernel_wrapper !0 {
entry:
  %z = add i32 1,1
  ret void
}

!opencl.kernels = !{!0}

!0 = !{void ()* @thisIsKernel}

; CHECK-NOT:    define{{.*}}void @thisIsNotKernel()
; CHECK-NOT:    define{{.*}}void @anotherNotKernel()
; CHECK:        define void @thisIsKernel()

; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY: WARNING: Missing line 3
; DEBUGIFY: WARNING: Missing line 4
; DEBUGIFY: WARNING: Missing line 5
; DEBUGIFY: WARNING: Missing variable 1
; DEBUGIFY: WARNING: Missing variable 2
; DEBUGIFY-NOT: WARNING
