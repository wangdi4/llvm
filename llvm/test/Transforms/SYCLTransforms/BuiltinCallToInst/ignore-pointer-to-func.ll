; RUN: opt -passes=sycl-kernel-builtin-call-to-inst -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-builtin-call-to-inst -S %s | FileCheck %s

define void @foo() {
  ret void
}

; Check the call with function pointer is ignored by BuiltinCallToInst pass
; CHECK: call i32 %funcptr(i32
define void @sample_test(ptr %funcptr, ptr addrspace(1) %p1, i32 %a) nounwind {
entry:
  %call = call i32 %funcptr(i32 %a);
  store i32 %call, ptr addrspace(1) %p1
  ret void
}

define void @main() nounwind {
entry:
  %0 = alloca i32, align 4
  %1 = alloca i32, align 4
  store i32 1, ptr %1
  %2 = load i32, ptr %1
  %3 = addrspacecast ptr %0 to ptr addrspace(1)
  call void @sample_test(ptr @foo, ptr addrspace(1) %3, i32 %2)
  ret void
}
; DEBUGIFY-NOT: WARNING
