; RUN: opt -prevent-div-crash -instcombine -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(i32 %x, i32 addrspace(1)* nocapture %res) nounwind {
entry:
  %div = udiv i32 %x, 0
  store i32 %div, i32 addrspace(1)* %res
  ret void
}

; %div = udiv %x, 1 = %x --> udiv instruction should disappear
; CHECK-NOT: 	udiv i32 %x

