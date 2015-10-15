; RUN: opt -module-cleanup -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; only function nonKernel3 should be removed

define void @thisIsKernel() nounwind {
entry:
  %x = call i32 @nonKernel1()
  ret void
}

define i32 @nonKernel1() nounwind {
entry:
  %y = call i32 @nonKernel2()
  ret i32 %y
}

define i32 @nonKernel2() nounwind {
entry:
  %z = add i32 1, 1
  ret i32 %z
}

define i32 @nonKernel3() nounwind {
entry:
  %w = call i32 @nonKernel1()
  ret i32 %w
}

!opencl.kernel_info = !{!0}

!0 = !{void ()* @thisIsKernel, !1}
!1 = !{!2}
!2 = !{!"kernel_wrapper", void ()* @thisIsKernel}

; CHECK:        define void @thisIsKernel()
; CHECK:        %x = call i32 @nonKernel1()
; CHECK:        define i32 @nonKernel1()
; CHECK:        %y = call i32 @nonKernel2()
; CHECK:        define i32 @nonKernel2()
; CHECK:        %z = add i32 1, 1
; CHECK-NOT:    define i32 @nonKernel3()

