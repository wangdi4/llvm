; RUN: opt -prevent-div-crash -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(<16 x i8> %x, <16 x i8> %y, <16 x i8> addrspace(1)* nocapture %res) nounwind {
entry:
  %rem = urem <16 x i8> %x, %y
  store <16 x i8> %rem, <16 x i8> addrspace(1)* %res
  ret void
}

; CHECK: 		[[IS_DIVISOR_ZERO:%[a-zA-Z0-9]+]] = icmp eq <16 x i8> %y, zeroinitializer
; CHECK: 		[[IS_DIVISOR_BAD:%[a-zA-Z0-9]+]] = or <16 x i1> zeroinitializer, [[IS_DIVISOR_ZERO]]
; CHECK-NEXT: 	[[NEW_DIVISOR:%[a-zA-Z0-9]+]] = select <16 x i1> [[IS_DIVISOR_BAD]], <16 x i8> <i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1>, <16 x i8> %y
; CHECK-NEXT: 	urem <16 x i8> %x, [[NEW_DIVISOR]]

