; RUN: %oclopt -prevent-div-crash -eyeq-div-crash-behavior -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -prevent-div-crash -eyeq-div-crash-behavior -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

target triple = "spir64-unknown-unknown-inteleyeq"

; CHECK: @sample_test
define void @sample_test(i32 %x, i32 %y, i32 addrspace(1)* nocapture %res) nounwind {
entry:
  %rem = urem i32 %x, %y
  store i32 %rem, i32 addrspace(1)* %res
  ret void
}

; CHECK:      [[IS_DIVISOR_ZERO:%[a-zA-Z0-9]+]] = icmp eq i32 %y, 0
; CHECK-NEXT: [[IS_DIVISOR_BAD:%[a-zA-Z0-9]+]] = or i1 false, [[IS_DIVISOR_ZERO]]
; CHECK-NEXT: [[NEW_DIVISOR:%[a-zA-Z0-9]+]] = select i1 [[IS_DIVISOR_BAD]], i32 1, i32 %y
; CHECK-NEXT: [[NEW_DIVIDEND:%[a-zA-Z0-9.]+]] = select i1 [[IS_DIVISOR_ZERO]], i32 0, i32 %x
; CHECK-NEXT: urem i32 [[NEW_DIVIDEND]], [[NEW_DIVISOR]]


; DEBUGIFY-NOT: WARNING
