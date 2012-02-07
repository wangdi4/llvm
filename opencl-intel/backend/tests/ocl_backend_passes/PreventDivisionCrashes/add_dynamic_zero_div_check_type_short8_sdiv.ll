; RUN: opt -prevent-div-crash -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(<8 x i16> %x, <8 x i16> %y, <8 x i16> addrspace(1)* nocapture %res) nounwind {
entry:
  %div = sdiv <8 x i16> %x, %y
  store <8 x i16> %div, <8 x i16> addrspace(1)* %res
  ret void
}

; CHECK: 		[[IS_DIVISOR_NEG_ONE:%[a-zA-Z0-9]+]] = icmp eq <8 x i16> %y, <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>
; CHECK-NEXT: 	[[IS_DIVIDEND_MIN_INT:%[a-zA-Z0-9]+]] = icmp eq <8 x i16> %x, <i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768>
; CHECK-NEXT: 	[[IS_INTEGER_OVERFLOW:%[a-zA-Z0-9]+]] = and <8 x i1> [[IS_DIVISOR_NEG_ONE]], [[IS_DIVIDEND_MIN_INT]]
; CHECK-NEXT: 	[[IS_DIVISOR_ZERO:%[a-zA-Z0-9]+]] = icmp eq <8 x i16> %y, zeroinitializer
; CHECK-NEXT: 	[[IS_DIVISOR_BAD:%[a-zA-Z0-9]+]] = or <8 x i1> [[IS_INTEGER_OVERFLOW]], [[IS_DIVISOR_ZERO]]
; CHECK-NEXT: 	[[NEW_DIVISOR:%[a-zA-Z0-9]+]] = select <8 x i1> [[IS_DIVISOR_BAD]], <8 x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>, <8 x i16> %y
; CHECK-NEXT: 	sdiv <8 x i16> %x, [[NEW_DIVISOR]]

