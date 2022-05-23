; RUN: opt -passes=dpcpp-kernel-prevent-div-crashes,instcombine -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-prevent-div-crashes,instcombine -S %s -o - | FileCheck %s
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-prevent-div-crashes -instcombine -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-prevent-div-crashes -instcombine -S %s -o - | FileCheck %s

; CHECK: @sample_test
define void @sample_test(<4 x i32> %x, <4 x i32> addrspace(1)* nocapture %res) nounwind {
entry:
  %div = sdiv <4 x i32> %x, <i32 -2, i32 0, i32 632, i32 0>
  store <4 x i32> %div, <4 x i32> addrspace(1)* %res
  ret void
}

; replacing 0 with 1 in the divisor vector
; CHECK: 	sdiv <4 x i32> %x, <i32 -2, i32 1, i32 632, i32 1>


; DEBUGIFY-NOT: WARNING
