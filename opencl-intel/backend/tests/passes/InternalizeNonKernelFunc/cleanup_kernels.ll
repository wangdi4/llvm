; RUN: %oclopt -internalize-nonkernel-functions -globaldce -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -internalize-nonkernel-functions -globaldce -S %s -o %t.ll
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

!opencl.kernels = !{!0}
!0 = !{void ()* @thisIsKernel, void (i32)* @alsoKernel}

; CHECK:        define void @thisIsKernel()
; CHECK:        %x = add i32 1, 1
; CHECK:        define void @alsoKernel(i32 %input)
; CHECK:        %x = add i32 1, %input



; DEBUGIFY-NOT: WARNING
