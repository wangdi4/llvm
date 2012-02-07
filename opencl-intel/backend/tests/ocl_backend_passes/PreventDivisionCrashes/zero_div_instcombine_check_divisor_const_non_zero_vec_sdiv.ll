; RUN: opt -prevent-div-crash -instcombine -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(<2 x i32> %x, <2 x i32> addrspace(1)* nocapture %res) nounwind {
entry:
  %div = sdiv <2 x i32> %x, <i32 -1, i32 6>
  store <2 x i32> %div, <2 x i32> addrspace(1)* %res
  ret void
}

; CHECK: 		[[IS_DIVIDEND_MIN_INT:%[a-zA-Z0-9]+]] = icmp eq <2 x i32> %x, <i32 -2147483648, i32 -2147483648>
; CHECK-NEXT: 	[[IS_INTEGER_OVERFLOW:%[a-zA-Z0-9]+]] = and <2 x i1> [[IS_DIVIDEND_MIN_INT]], <i1 true, i1 false>
; CHECK-NEXT: 	[[NEW_DIVISOR:%[a-zA-Z0-9]+]] = select <2 x i1> [[IS_INTEGER_OVERFLOW]], <2 x i32> <i32 1, i32 1>, <2 x i32> <i32 -1, i32 6>
; CHECK-NEXT: 	sdiv <2 x i32> %x, [[NEW_DIVISOR]]

