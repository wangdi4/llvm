; RUN: opt -module-cleanup -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; no functions should be removed

define void @thisIsKernel() nounwind {
entry:
  %x = call i32 @thisIsNotKernel()
  ret void
}

define i32 @thisIsNotKernel() nounwind {
entry:
  %x = add i32 1,1
  ret i32 %x
}

!opencl.kernel_info = !{!0}

!0 = !{void ()* @thisIsKernel, !1}
!1 = !{!2}
!2 = !{!"kernel_wrapper", void ()* @thisIsKernel}

; CHECK:        define void @thisIsKernel()
; CHECK:        %x = call i32 @thisIsNotKernel()
; CHECK:        define i32 @thisIsNotKernel()
; CHECK:        %x = add i32 1, 1


