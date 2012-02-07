; RUN: opt -prevent-div-crash  -instcombine -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(<16 x i64> %x, <16 x i64> addrspace(1)* nocapture %res) nounwind {
entry:
  %rem = urem <16 x i64> %x, zeroinitializer
  store <16 x i64> %rem, <16 x i64> addrspace(1)* %res
  ret void
}

; %rem = urem %x, <1, 1, 1, ..., 1> = %x --> udiv instruction should disappear
; CHECK-NOT: 	urem <16 x i64> %x

