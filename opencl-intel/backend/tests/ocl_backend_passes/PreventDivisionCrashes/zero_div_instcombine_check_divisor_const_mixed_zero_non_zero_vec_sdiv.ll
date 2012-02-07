; RUN: opt -prevent-div-crash  -instcombine -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(<4 x i32> %x, <4 x i32> addrspace(1)* nocapture %res) nounwind {
entry:
  %div = sdiv <4 x i32> %x, <i32 -2, i32 0, i32 632, i32 0>
  store <4 x i32> %div, <4 x i32> addrspace(1)* %res
  ret void
}

; replacing 0 with 1 in the divisor vector
; CHECK: 	sdiv <4 x i32> %x, <i32 -2, i32 1, i32 632, i32 1>

