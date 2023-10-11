; RUN: opt -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-local-buffers,check-debugify' -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-local-buffers' -S < %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"


; This test is to check no crash during calculating local sizes for recursive
; function.

; Used local variables
@foo.localInt = internal addrspace(3) global i32 0, align 4
@foo.localChar = internal addrspace(3) global i8 0, align 1
@foo.localFloat = internal addrspace(3) global float 0.000000e+00, align 4

define void @foo(ptr addrspace(1) %ApInt, ptr addrspace(1) %BpInt, ptr addrspace(1) %pChar, ptr addrspace(1) %pFloat) !recursive_call !0 !kernel_arg_base_type !2 !arg_type_null_val !3 {
entry:
  %ApIntValue = load i32, ptr addrspace(1) %ApInt
  %check = icmp eq i32 %ApIntValue, 1000
  br i1 %check, label %body, label %exit

body:
  %dummyInt = load i32, ptr addrspace(3) @foo.localInt, align 4
  %AddOne = add i32 %dummyInt, 1
  store i32 %AddOne, ptr addrspace(1) %ApInt
  store i32 %dummyInt, ptr addrspace(1) %BpInt

  %dummyChar = load i8, ptr addrspace(3) @foo.localChar, align 1
  store i8 %dummyChar, ptr addrspace(1) %pChar

  %dummyFloat = load float, ptr addrspace(3) @foo.localFloat, align 4
  store float %dummyFloat, ptr addrspace(1) %pFloat

  call void @foo(ptr addrspace(1) %ApInt, ptr addrspace(1) %BpInt, ptr addrspace(1) %pChar, ptr addrspace(1) %pFloat)
  br label %exit

exit:
  ret void
}


!sycl.kernels = !{!1}

!0 = !{i1 true}
!1 = !{ptr @foo}
!2 = !{!"int*", !"int*", !"char*", !"float*"}
!3 = !{ptr addrspace(1) null, ptr addrspace(1) null, ptr addrspace(1) null, ptr addrspace(1) null}

; CHECK:           define void @foo(ptr addrspace(1) %ApInt, ptr addrspace(1) %BpInt, ptr addrspace(1) %pChar, ptr addrspace(1) %pFloat,
; CHECK-NEXT:   entry:

; CHECK-NEXT:   [[VAR0:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; CHECK-NEXT:   [[VAR1:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 4
; CHECK-NEXT:   [[VAR2:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 8

; CHECK:   body:
; CHECK:   %dummyInt = load i32, ptr addrspace(3) [[VAR0]], align 4
; CHECK:   %dummyChar = load i8, ptr addrspace(3) [[VAR1]], align 1
; CHECK:   %dummyFloat = load float, ptr addrspace(3) [[VAR2]], align 4

; CHECK:   call void @foo(ptr addrspace(1) %ApInt, ptr addrspace(1) %BpInt, ptr addrspace(1) %pChar, ptr addrspace(1) %pFloat, ptr addrspace(3) noalias undef, ptr noalias undef, ptr noalias undef, [4 x i32] undef, ptr noalias undef, ptr noalias undef)


; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
