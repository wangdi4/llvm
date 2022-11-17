; RUN: opt -dpcpp-kernel-add-implicit-args -debugify -dpcpp-kernel-local-buffers -check-debugify -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers -dpcpp-kernel-add-implicit-args -debugify -dpcpp-kernel-local-buffers -check-debugify -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-local-buffers,check-debugify' -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-local-buffers,check-debugify' -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-local-buffers -S < %s | FileCheck %s -check-prefixes=CHECK,NONOPAQUE
; RUN: opt -opaque-pointers -dpcpp-kernel-add-implicit-args -dpcpp-kernel-local-buffers -S < %s | FileCheck %s -check-prefixes=CHECK,OPAQUE
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-local-buffers' -S < %s | FileCheck %s -check-prefixes=CHECK,NONOPAQUE
; RUN: opt -opaque-pointers -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-local-buffers' -S < %s | FileCheck %s -check-prefixes=CHECK,OPAQUE

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"


; This test is to check no crash during calculating local sizes for recursive
; function.

; Used local variables
@foo.localInt = internal addrspace(3) global i32 0, align 4
@foo.localChar = internal addrspace(3) global i8 0, align 1
@foo.localFloat = internal addrspace(3) global float 0.000000e+00, align 4

define void @foo(i32 addrspace(1)* %ApInt, i32 addrspace(1)* %BpInt, i8 addrspace(1)* %pChar, float addrspace(1)* %pFloat) !recursive_call !0 {
entry:
  %ApIntValue = load i32, i32 addrspace(1)* %ApInt
  %check = icmp eq i32 %ApIntValue, 1000
  br i1 %check, label %body, label %exit

body:
  %dummyInt = load i32, i32 addrspace(3)* @foo.localInt, align 4
  %AddOne = add i32 %dummyInt, 1
  store i32 %AddOne, i32 addrspace(1)* %ApInt
  store i32 %dummyInt, i32 addrspace(1)* %BpInt

  %dummyChar = load i8, i8 addrspace(3)* @foo.localChar, align 1
  store i8 %dummyChar, i8 addrspace(1)* %pChar

  %dummyFloat = load float, float addrspace(3)* @foo.localFloat, align 4
  store float %dummyFloat, float addrspace(1)* %pFloat

  call void @foo(i32 addrspace(1)* %ApInt, i32 addrspace(1)* %BpInt, i8 addrspace(1)* %pChar, float addrspace(1)* %pFloat)
  br label %exit

exit:
  ret void
}


!0 = !{i1 true}


; NONOPAQUE:        define void @foo(i32 addrspace(1)* %ApInt, i32 addrspace(1)* %BpInt, i8 addrspace(1)* %pChar, float addrspace(1)* %pFloat,
; OPAQUE:           define void @foo(ptr addrspace(1) %ApInt, ptr addrspace(1) %BpInt, ptr addrspace(1) %pChar, ptr addrspace(1) %pFloat,
; CHECK-NEXT:   entry:
; NONOPAQUE-NEXT:   [[VAR0:%[a-zA-Z0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; NONOPAQUE-NEXT:   [[VAR1:%[a-zA-Z0-9]+]] = addrspacecast i8 addrspace(3)* [[VAR0]] to i32 addrspace(3)**
; NONOPAQUE-NEXT:   [[VAR2:%[a-zA-Z0-9]+]] = load i32 addrspace(3)*, i32 addrspace(3)** [[VAR1]]
; OPAQUE-NEXT:   [[VAR0:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; OPAQUE-NEXT:   [[VAR1:%[a-zA-Z0-9]+]] = addrspacecast ptr addrspace(3) [[VAR0]] to ptr
; OPAQUE-NEXT:   [[VAR2:%[a-zA-Z0-9]+]] = load ptr addrspace(3), ptr [[VAR1]]

; NONOPAQUE-NEXT:   [[VAR3:%[a-zA-Z0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 128
; NONOPAQUE-NEXT:   [[VAR4:%[a-zA-Z0-9]+]] = addrspacecast i8 addrspace(3)* [[VAR3]] to i8 addrspace(3)**
; NONOPAQUE-NEXT:   [[VAR5:%[a-zA-Z0-9]+]] = load i8 addrspace(3)*, i8 addrspace(3)** [[VAR4]]
; OPAQUE-NEXT:   [[VAR3:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 128
; OPAQUE-NEXT:   [[VAR4:%[a-zA-Z0-9]+]] = addrspacecast ptr addrspace(3) [[VAR3]] to ptr
; OPAQUE-NEXT:   [[VAR5:%[a-zA-Z0-9]+]] = load ptr addrspace(3), ptr [[VAR4]]

; NONOPAQUE-NEXT:   [[VAR6:%[a-zA-Z0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 256
; NONOPAQUE-NEXT:   [[VAR7:%[a-zA-Z0-9]+]] = addrspacecast i8 addrspace(3)* [[VAR6]] to float addrspace(3)**
; NONOPAQUE-NEXT:   [[VAR8:%[a-zA-Z0-9]+]] = load float addrspace(3)*, float addrspace(3)** [[VAR7]]
; OPAQUE-NEXT:   [[VAR6:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 256
; OPAQUE-NEXT:   [[VAR7:%[a-zA-Z0-9]+]] = addrspacecast ptr addrspace(3) [[VAR6]] to ptr
; OPAQUE-NEXT:   [[VAR8:%[a-zA-Z0-9]+]] = load ptr addrspace(3), ptr [[VAR7]]

; NONOPAQUE-NEXT:   %ApIntValue = load i32, i32 addrspace(1)* %ApInt, align 4
; OPAQUE-NEXT:   %ApIntValue = load i32, ptr addrspace(1) %ApInt, align 4
; CHECK-NEXT:   %check = icmp eq i32 %ApIntValue, 1000
; CHECK-NEXT:   br i1 %check, label %body, label %exit

; CHECK:   body:
; NONOPAQUE-NEXT:   %dummyInt = load i32, i32 addrspace(3)* [[VAR2]], align 4
; OPAQUE-NEXT:   %dummyInt = load i32, ptr addrspace(3) [[VAR2]], align 4
; CHECK-NEXT:   %AddOne = add i32 %dummyInt, 1
; NONOPAQUE-NEXT:   store i32 %AddOne, i32 addrspace(1)* %ApInt
; NONOPAQUE-NEXT:   store i32 %dummyInt, i32 addrspace(1)* %BpInt
; NONOPAQUE-NEXT:   %dummyChar = load i8, i8 addrspace(3)* [[VAR5]], align 1
; NONOPAQUE-NEXT:   store i8 %dummyChar, i8 addrspace(1)* %pChar
; NONOPAQUE-NEXT:   %dummyFloat = load float, float addrspace(3)* [[VAR8]], align 4
; NONOPAQUE-NEXT:   store float %dummyFloat, float addrspace(1)* %pFloat
; OPAQUE-NEXT:   store i32 %AddOne, ptr addrspace(1) %ApInt
; OPAQUE-NEXT:   store i32 %dummyInt, ptr addrspace(1) %BpInt
; OPAQUE-NEXT:   %dummyChar = load i8, ptr addrspace(3) [[VAR5]], align 1
; OPAQUE-NEXT:   store i8 %dummyChar, ptr addrspace(1) %pChar
; OPAQUE-NEXT:   %dummyFloat = load float, ptr addrspace(3) [[VAR8]], align 4
; OPAQUE-NEXT:   store float %dummyFloat, ptr addrspace(1) %pFloat

; NONOPAQUE-NEXT:   call void @foo(i32 addrspace(1)* %ApInt, i32 addrspace(1)* %BpInt, i8 addrspace(1)* %pChar, float addrspace(1)* %pFloat, i8 addrspace(3)* noalias undef, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }* noalias undef, i32* noalias undef, [4 x i32] undef, i8* noalias undef, {}* noalias undef)
; OPAQUE-NEXT:   call void @foo(ptr addrspace(1) %ApInt, ptr addrspace(1) %BpInt, ptr addrspace(1) %pChar, ptr addrspace(1) %pFloat, ptr addrspace(3) noalias undef, ptr noalias undef, ptr noalias undef, [4 x i32] undef, ptr noalias undef, ptr noalias undef)
; CHECK-NEXT:   br label %exit

; CHECK:   exit:
; CHECK-NEXT:   ret void


; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
