; RUN: %oclopt -module-cleanup -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; all functions here are not kernels and they should be all removed

define void @thisIsNotKernel() nounwind {
entry:
  %x = add i32 1,1
  ret void
}

define void @alsoNotKernel(i32 %input) nounwind {
entry:
  %x = add i32 1, %input
  ret void
}

define i32 @notKernel() nounwind {
entry:
  %res = add i32 1, 1
  ret i32 %res
}

define float @notKernel2(float %input) nounwind {
entry:
  %result = fmul float %input, 2.0
  ret float %result
}

; CHECK-NOT:    define void @thisIsNotKernel
; CHECK-NOT:    define void @alsoNotKernel
; CHECK-NOT:    define i32 @notKernel
; CHECK-NOT:    define float @notKernel2

