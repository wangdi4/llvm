; RUN: %oclopt -internalize-nonkernel-functions -globaldce -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -internalize-nonkernel-functions -globaldce -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; only function nonKernel3 should be removed

define void @thisIsKernel() nounwind !kernel_wrapper !0 {
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

!opencl.kernels = !{!0}

!0 = !{void ()* @thisIsKernel}

; CHECK:        define void @thisIsKernel()
; CHECK:        %x = call i32 @nonKernel1()
; CHECK:        define internal i32 @nonKernel1()
; CHECK:        %y = call i32 @nonKernel2()
; CHECK:        define internal i32 @nonKernel2()
; CHECK:        %z = add i32 1, 1
; CHECK-NOT:    define{{.*}}i32 @nonKernel3()

; DEBUGIFY: WARNING: Missing line 7
; DEBUGIFY: WARNING: Missing line 8
; DEBUGIFY: WARNING: Missing variable 4
; DEBUGIFY-NOT: WARNING
