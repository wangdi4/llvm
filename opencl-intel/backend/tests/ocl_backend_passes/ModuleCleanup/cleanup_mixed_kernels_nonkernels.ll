; RUN: opt -module-cleanup -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; only non-kernel functions should be removed

define void @thisIsKernel() nounwind !kernel_wrapper !0 {
entry:
  %x = add i32 1,1
  ret void
}

define void @thisIsNotKernel() nounwind {
entry:
  %x = add i32 1,2
  ret void
}

!0 = !{void ()* @thisIsKernel}

; CHECK:        define void @thisIsKernel()
; CHECK:        %x = add i32 1, 1
; CHECK-NOT:    define void @thisIsNotKernel()
