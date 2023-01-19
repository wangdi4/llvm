; RUN: opt -passes=dpcpp-kernel-builtin-call-to-inst -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-builtin-call-to-inst -S %s | FileCheck %s
; RUN: opt -opaque-pointers -passes=dpcpp-kernel-builtin-call-to-inst -S %s | FileCheck %s

define void @foo() {
  ret void
}

; Check the call with function pointer is ignored by BuiltinCallToInst pass
; CHECK: call i32 %func(i32
define void @sample_test(i8* %funcptr, i32 addrspace(1)* %p1, i32 %a) nounwind {
entry:
  %func = bitcast i8* %funcptr to i32 (i32) *
  %call = call i32 %func(i32 %a);
  store i32 %call, i32 addrspace(1)* %p1
  ret void
}

define void @main() nounwind {
entry:
  %0 = alloca i32, align 4
  %1 = alloca i32, align 4
  store i32 1, i32 * %1
  %2 = load i32, i32 * %1
  %3 = addrspacecast i32* %0 to i32 addrspace(1)*
  %4 = bitcast void()* @foo to i8*
  call void @sample_test(i8* %4, i32 addrspace(1)* %3, i32 %2)
  ret void
}
; DEBUGIFY-NOT: WARNING
