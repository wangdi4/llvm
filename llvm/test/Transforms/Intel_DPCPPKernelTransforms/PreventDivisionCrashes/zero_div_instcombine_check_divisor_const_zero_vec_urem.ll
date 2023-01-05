; RUN: opt -passes=dpcpp-kernel-prevent-div-crashes,instcombine -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-prevent-div-crashes,instcombine -S %s -o - | FileCheck %s

; CHECK: @sample_test
define void @sample_test(<16 x i64> %x, <16 x i64> addrspace(1)* nocapture %res) nounwind {
entry:
  %rem = urem <16 x i64> %x, zeroinitializer
  store <16 x i64> %rem, <16 x i64> addrspace(1)* %res
  ret void
}

; %rem = urem %x, <1, 1, 1, ..., 1> = %x --> udiv instruction should disappear
; CHECK-NOT: 	urem <16 x i64> %x

; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY-NOT: WARNING
