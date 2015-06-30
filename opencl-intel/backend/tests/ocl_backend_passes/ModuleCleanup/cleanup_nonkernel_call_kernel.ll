; RUN: opt -module-cleanup -S %s -o %t.ll
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

define void @thisIsKernel() nounwind {
entry:
  %z = add i32 1,1
  ret void
}

!opencl.kernel_info = !{!0}

!0 = !{void ()* @thisIsKernel, !1}
!1 = !{!2}
!2 = !{!"kernel_wrapper", void ()* @thisIsKernel}

; CHECK-NOT:    define void @thisIsNotKernel()
; CHECK-NOT:    define void @anotherNotKernel()
; CHECK:        define void @thisIsKernel()
