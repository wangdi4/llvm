; RUN: opt -module-cleanup -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; all functions here are kernels and they should not be removed

define void @thisIsKernel() nounwind !kernel_wrapper !0 {
entry:
  %x = add i32 1,1
  ret void
}

define void @alsoKernel(i32 %input) nounwind !kernel_wrapper !1 {
entry:
  %x = add i32 1, %input
  ret void
}

!0 = !{void ()* @thisIsKernel}
!1 = !{void (i32)* @alsoKernel}

; CHECK:        define void @thisIsKernel()
; CHECK:        %x = add i32 1, 1
; CHECK:        define void @alsoKernel(i32 %input)
; CHECK:        %x = add i32 1, %input


