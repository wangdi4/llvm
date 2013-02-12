; RUN: opt -module-cleanup -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; only non-kernel functions should be removed

define void @thisIsKernel() nounwind {
entry:
  %x = add i32 1,1
  ret void
}

define void @thisIsNotKernel() nounwind {
entry:
  %x = add i32 1,1
  ret void
}

!opencl.wrappers = !{!0}

!0 = metadata !{void ()* @thisIsKernel}

; CHECK:        define void @thisIsKernel()
; CHECK:        %x = add i32 1, 1
; CHECK-NOT:    define void @thisIsNotKernel()


