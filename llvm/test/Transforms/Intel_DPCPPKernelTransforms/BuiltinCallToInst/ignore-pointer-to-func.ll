; RUN: opt -dpcpp-kernel-builtin-call-to-inst -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-builtin-call-to-inst -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-builtin-call-to-inst -S %s | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-builtin-call-to-inst -S %s | FileCheck %s

; checks the test doesn't crash
; CHECK: @sample_test
define void @sample_test(i8* %funcptr, i32 addrspace(1)* %p1, i32 %a) nounwind {
entry:
  %func = bitcast i8* %funcptr to i32 (i32) *
  %call = call i32 %func(i32 %a);
  store i32 %call, i32 addrspace(1)* %p1
  ret void
}

; DEBUGIFY-NOT: WARNING
