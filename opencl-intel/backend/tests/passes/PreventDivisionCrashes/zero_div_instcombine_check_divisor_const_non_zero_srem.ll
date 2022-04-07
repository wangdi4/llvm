; RUN: %oclopt -prevent-div-crash -instcombine -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -prevent-div-crash -instcombine -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(i32 %x, i32 addrspace(1)* nocapture %res) nounwind {
entry:
  %rem = srem i32 %x, 2
  store i32 %rem, i32 addrspace(1)* %res
  ret void
}

; CHECK: 	srem i32 %x, 2


; DEBUGIFY-NOT: WARNING
