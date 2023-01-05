; RUN: opt -passes=dpcpp-kernel-prevent-div-crashes,instcombine -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-prevent-div-crashes,instcombine -S %s -o - | FileCheck %s

; CHECK: @sample_test
define void @sample_test(i32 %x, i32 addrspace(1)* nocapture %res) nounwind {
entry:
  %div = udiv i32 %x, 0
  store i32 %div, i32 addrspace(1)* %res
  ret void
}

; %div = udiv %x, 1 = %x --> udiv instruction should disappear
; CHECK-NOT: 	udiv i32 %x


; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY-NOT: WARNING
