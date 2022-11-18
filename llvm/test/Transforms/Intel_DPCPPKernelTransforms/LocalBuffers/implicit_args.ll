; RUN: opt -dpcpp-kernel-add-implicit-args -debugify -dpcpp-kernel-local-buffers -check-debugify -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers -dpcpp-kernel-add-implicit-args -debugify -dpcpp-kernel-local-buffers -check-debugify -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-local-buffers,check-debugify' -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-local-buffers,check-debugify' -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-local-buffers -S < %s | FileCheck %s -check-prefixes=CHECK,NONOPAQUE
; RUN: opt -opaque-pointers -dpcpp-kernel-add-implicit-args -dpcpp-kernel-local-buffers -S < %s | FileCheck %s -check-prefixes=CHECK,OPAQUE
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-local-buffers' -S < %s | FileCheck %s -check-prefixes=CHECK,NONOPAQUE
; RUN: opt -opaque-pointers -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-local-buffers' -S < %s | FileCheck %s -check-prefixes=CHECK,OPAQUE

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; map used local variables to local buffer with correct alignment

; Used local variables
@foo.localInt = internal addrspace(3) global i32 0, align 4
@foo.localChar = internal addrspace(3) global i8 0, align 1
@foo.localFloat = internal addrspace(3) global float 0.000000e+00, align 4
@bar.localInt4 = internal addrspace(3) global <4 x i32> zeroinitializer, align 16
@bar.localLong16 = internal addrspace(3) global <16 x i64> zeroinitializer, align 128

define void @foo(i32 addrspace(1)* %ApInt, i32 addrspace(1)* %BpInt, i8 addrspace(1)* %pChar, float addrspace(1)* %pFloat) {
entry:
  %dummyInt = load i32, i32 addrspace(3)* @foo.localInt, align 4
  store i32 %dummyInt, i32 addrspace(1)* %ApInt
  store i32 %dummyInt, i32 addrspace(1)* %BpInt

  %dummyChar = load i8, i8 addrspace(3)* @foo.localChar, align 1
  store i8 %dummyChar, i8 addrspace(1)* %pChar

  %dummyFloat = load float, float addrspace(3)* @foo.localFloat, align 4
  store float %dummyFloat, float addrspace(1)* %pFloat

  ret void
}

define void @bar(<4 x i32> addrspace(1)* %pInt4, <16 x i64> addrspace(1)* %pLong16) {
entry:
  %dummyInt4 = load <4 x i32>, <4 x i32> addrspace(3)* @bar.localInt4, align 16
  store <4 x i32> %dummyInt4, <4 x i32> addrspace(1)* %pInt4

  %dummyLong16 = load <16 x i64>, <16 x i64> addrspace(3)* @bar.localLong16, align 128
  store <16 x i64> %dummyLong16, <16 x i64> addrspace(1)* %pLong16

  ret void
}



; NONOPAQUE:        define void @foo(i32 addrspace(1)* %ApInt, i32 addrspace(1)* %BpInt, i8 addrspace(1)* %pChar, float addrspace(1)* %pFloat,
; OPAQUE:           define void @foo(ptr addrspace(1) %ApInt, ptr addrspace(1) %BpInt, ptr addrspace(1) %pChar, ptr addrspace(1) %pFloat,
; NONOPAQUE-SAME:     i8 addrspace(3)* noalias %pLocalMemBase,
; OPAQUE-SAME:        ptr addrspace(3) noalias %pLocalMemBase,
; NONOPAQUE-SAME:     { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }* noalias %pWorkDim,
; OPAQUE-SAME:        ptr noalias %pWorkDim,
; NONOPAQUE-SAME:     i32* noalias %pWGId,
; OPAQUE-SAME:        ptr noalias %pWGId,
; CHECK-SAME:         [4 x i32] %BaseGlbId,
; NONOPAQUE-SAME:     i8* noalias %pSpecialBuf,
; OPAQUE-SAME:        ptr noalias %pSpecialBuf,
; NONOPAQUE-SAME:     {}* noalias %RuntimeHandle) {
; OPAQUE-SAME:        ptr noalias %RuntimeHandle) {
; CHECK-NEXT:   entry:
; NONOPAQUE-NEXT:   [[VAR0:%[a-zA-Z0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; NONOPAQUE-NEXT:   [[VAR1:%[a-zA-Z0-9]+]] = addrspacecast i8 addrspace(3)* [[VAR0]] to i32 addrspace(3)**
; NONOPAQUE-NEXT:   [[VAR2:%[a-zA-Z0-9]+]] = load i32 addrspace(3)*, i32 addrspace(3)** [[VAR1]]
; OPAQUE-NEXT:      [[VAR0:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; OPAQUE-NEXT:      [[VAR1:%[a-zA-Z0-9]+]] = addrspacecast ptr addrspace(3) [[VAR0]] to ptr
; OPAQUE-NEXT:      [[VAR2:%[a-zA-Z0-9]+]] = load ptr addrspace(3), ptr [[VAR1]]

; NONOPAQUE-NEXT:   [[VAR3:%[a-zA-Z0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 128
; NONOPAQUE-NEXT:   [[VAR4:%[a-zA-Z0-9]+]] = addrspacecast i8 addrspace(3)* [[VAR3]] to i8 addrspace(3)**
; NONOPAQUE-NEXT:   [[VAR5:%[a-zA-Z0-9]+]] = load i8 addrspace(3)*, i8 addrspace(3)** [[VAR4]]
; OPAQUE-NEXT:      [[VAR3:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 128
; OPAQUE-NEXT:      [[VAR4:%[a-zA-Z0-9]+]] = addrspacecast ptr addrspace(3) [[VAR3]] to ptr
; OPAQUE-NEXT:      [[VAR5:%[a-zA-Z0-9]+]] = load ptr addrspace(3), ptr [[VAR4]]

; NONOPAQUE-NEXT:   [[VAR6:%[a-zA-Z0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 256
; NONOPAQUE-NEXT:   [[VAR7:%[a-zA-Z0-9]+]] = addrspacecast i8 addrspace(3)* [[VAR6]] to float addrspace(3)**
; NONOPAQUE-NEXT:   [[VAR8:%[a-zA-Z0-9]+]] = load float addrspace(3)*, float addrspace(3)** [[VAR7]]
; OPAQUE-NEXT:      [[VAR6:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 256
; OPAQUE-NEXT:      [[VAR7:%[a-zA-Z0-9]+]] = addrspacecast ptr addrspace(3) [[VAR6]] to ptr
; OPAQUE-NEXT:      [[VAR8:%[a-zA-Z0-9]+]] = load ptr addrspace(3), ptr [[VAR7]]

; NONOPAQUE-NEXT:   %dummyInt = load i32, i32 addrspace(3)* [[VAR2]], align 4
; NONOPAQUE-NEXT:   store i32 %dummyInt, i32 addrspace(1)* %ApInt
; NONOPAQUE-NEXT:   store i32 %dummyInt, i32 addrspace(1)* %BpInt
; NONOPAQUE-NEXT:   %dummyChar = load i8, i8 addrspace(3)* [[VAR5]], align 1
; NONOPAQUE-NEXT:   store i8 %dummyChar, i8 addrspace(1)* %pChar
; NONOPAQUE-NEXT:   %dummyFloat = load float, float addrspace(3)* [[VAR8]], align 4
; NONOPAQUE-NEXT:   store float %dummyFloat, float addrspace(1)* %pFloat
; OPAQUE-NEXT:      %dummyInt = load i32, ptr addrspace(3) [[VAR2]], align 4
; OPAQUE-NEXT:      store i32 %dummyInt, ptr addrspace(1) %ApInt
; OPAQUE-NEXT:      store i32 %dummyInt, ptr addrspace(1) %BpInt
; OPAQUE-NEXT:      %dummyChar = load i8, ptr addrspace(3) [[VAR5]], align 1
; OPAQUE-NEXT:      store i8 %dummyChar, ptr addrspace(1) %pChar
; OPAQUE-NEXT:      %dummyFloat = load float, ptr addrspace(3) [[VAR8]], align 4
; OPAQUE-NEXT:      store float %dummyFloat, ptr addrspace(1) %pFloat
; CHECK-NEXT:       ret void


; NONOPAQUE:        define void @bar(<4 x i32> addrspace(1)* %pInt4, <16 x i64> addrspace(1)* %pLong16,
; OPAQUE:           define void @bar(ptr addrspace(1) %pInt4, ptr addrspace(1) %pLong16,
; NONOPAQUE-SAME:     i8 addrspace(3)* noalias %pLocalMemBase,
; OPAQUE-SAME:        ptr addrspace(3) noalias %pLocalMemBase,
; NONOPAQUE-SAME:     { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }* noalias %pWorkDim,
; OPAQUE-SAME:        ptr noalias %pWorkDim,
; NONOPAQUE-SAME:     i32* noalias %pWGId,
; OPAQUE-SAME:        ptr noalias %pWGId,
; CHECK:              [4 x i32] %BaseGlbId,
; NONOPAQUE-SAME:     i8* noalias %pSpecialBuf,
; OPAQUE-SAME:        ptr noalias %pSpecialBuf,
; NONOPAQUE-SAME:     {}* noalias %RuntimeHandle) {
; OPAQUE-SAME:        ptr noalias %RuntimeHandle) {
; CHECK-NEXT:   entry:
; NONOPAQUE-NEXT:   [[VAR10:%[a-zA-Z0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; NONOPAQUE-NEXT:   [[VAR11:%[a-zA-Z0-9]+]] = addrspacecast i8 addrspace(3)* [[VAR10]] to <4 x i32> addrspace(3)**
; NONOPAQUE-NEXT:   [[VAR12:%[a-zA-Z0-9]+]] = load <4 x i32> addrspace(3)*, <4 x i32> addrspace(3)** [[VAR11]]
; OPAQUE-NEXT:      [[VAR10:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; OPAQUE-NEXT:      [[VAR11:%[a-zA-Z0-9]+]] = addrspacecast ptr addrspace(3) [[VAR10]] to ptr
; OPAQUE-NEXT:      [[VAR12:%[a-zA-Z0-9]+]] = load ptr addrspace(3), ptr [[VAR11]]

; NONOPAQUE-NEXT:   [[VAR13:%[a-zA-Z0-9]+]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 128
; NONOPAQUE-NEXT:   [[VAR14:%[a-zA-Z0-9]+]] = addrspacecast i8 addrspace(3)* [[VAR13]] to <16 x i64> addrspace(3)**
; NONOPAQUE-NEXT:   [[VAR15:%[a-zA-Z0-9]+]] = load <16 x i64> addrspace(3)*, <16 x i64> addrspace(3)** [[VAR14]]
; OPAQUE-NEXT:      [[VAR13:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 128
; OPAQUE-NEXT:      [[VAR14:%[a-zA-Z0-9]+]] = addrspacecast ptr addrspace(3) [[VAR13]] to ptr
; OPAQUE-NEXT:      [[VAR15:%[a-zA-Z0-9]+]] = load ptr addrspace(3), ptr [[VAR14]]

; NONOPAQUE-NEXT:   %dummyInt4 = load <4 x i32>, <4 x i32> addrspace(3)* [[VAR12]], align 16
; NONOPAQUE-NEXT:   store <4 x i32> %dummyInt4, <4 x i32> addrspace(1)* %pInt4
; NONOPAQUE-NEXT:   %dummyLong16 = load <16 x i64>, <16 x i64> addrspace(3)* [[VAR15]], align 128
; NONOPAQUE-NEXT:   store <16 x i64> %dummyLong16, <16 x i64> addrspace(1)* %pLong16
; OPAQUE-NEXT:      %dummyInt4 = load <4 x i32>, ptr addrspace(3) [[VAR12]], align 16
; OPAQUE-NEXT:      store <4 x i32> %dummyInt4, ptr addrspace(1) %pInt4
; OPAQUE-NEXT:      %dummyLong16 = load <16 x i64>, ptr addrspace(3) [[VAR15]], align 128
; OPAQUE-NEXT:      store <16 x i64> %dummyLong16, ptr addrspace(1) %pLong16
; CHECK-NEXT:       ret void

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
