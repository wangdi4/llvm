; RUN: opt -module-cleanup -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; all functions here are kernels and they should not be removed

define void @thisIsKernel() nounwind {
entry:
  %x = add i32 1,1
  ret void
}

define void @alsoKernel(i32 %input) nounwind {
entry:
  %x = add i32 1, %input
  ret void
}

!opencl.kernel_info = !{!0, !3}

!0 = metadata !{void ()* @thisIsKernel, metadata !1}
!1 = metadata !{metadata !2}
!2 = metadata !{metadata !"kernel_wrapper", void ()* @thisIsKernel}

!3 = metadata !{void (i32)* @alsoKernel, metadata !4}
!4 = metadata !{metadata !5}
!5 = metadata !{metadata !"kernel_wrapper", void (i32)* @alsoKernel}

; CHECK:        define void @thisIsKernel()
; CHECK:        %x = add i32 1, 1
; CHECK:        define void @alsoKernel(i32 %input)
; CHECK:        %x = add i32 1, %input


