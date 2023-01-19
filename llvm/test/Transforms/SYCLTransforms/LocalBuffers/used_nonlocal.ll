; RUN: opt -passes='dpcpp-kernel-local-buffers' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers -passes='dpcpp-kernel-local-buffers' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-local-buffers' -S %s | FileCheck %s -check-prefixes=CHECK,NONOPAQUE
; RUN: opt -opaque-pointers -passes='dpcpp-kernel-local-buffers' -S %s | FileCheck %s -check-prefixes=CHECK,OPAQUE

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; not mapping used non-local variables to local buffer

; Used non-local variables
@foo.localInt = internal addrspace(1) global i32 0, align 4
@bar.localLong16 = internal addrspace(1) global <16 x i64> zeroinitializer, align 128

define void @foo(i32 addrspace(1)* %pInt, i8 addrspace(1)* %pChar, float addrspace(1)* %pFloat, i8 addrspace(3)* %pLocalMem, { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }* %pWorkDim, i32* %pWGId, <{ [4 x i32] }>* %pBaseGlbId, <{ [4 x i32] }>* %pLocalIds, i32* %contextpointer, i32 %iterCount, i8* %pSpecialBuf, i32* %pCurrWI, i32* %extExecContextPointer) {
entry:
  %dummyInt = load i32, i32 addrspace(1)* @foo.localInt, align 4
  store i32 %dummyInt, i32 addrspace(1)* %pInt

  ret void
}

define void @bar(<4 x i32> addrspace(1)* %pInt4, <16 x i64> addrspace(1)* %pLong16, i8 addrspace(3)* %pLocalMem, { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }* %pWorkDim, i32* %pWGId, <{ [4 x i32] }>* %pBaseGlbId, <{ [4 x i32] }>* %pLocalIds, i32* %contextpointer, i32 %iterCount, i8* %pSpecialBuf, i32* %pCurrWI, i32* %extExecContextPointer) {
entry:
  %dummyLong16 = load <16 x i64>, <16 x i64> addrspace(1)* @bar.localLong16, align 128
  store <16 x i64> %dummyLong16, <16 x i64> addrspace(1)* %pLong16

  ret void
}



; NONOPAQUE:        define void @foo(i32 addrspace(1)* %pInt, i8 addrspace(1)* %pChar, float addrspace(1)* %pFloat,
; OPAQUE:           define void @foo(ptr addrspace(1) %pInt, ptr addrspace(1) %pChar, ptr addrspace(1) %pFloat,
; CEHCK:   entry:
; NONOPAQUE:   %dummyInt = load i32, i32 addrspace(1)* @foo.localInt, align 4
; NONOPAQUE-NEXT:   store i32 %dummyInt, i32 addrspace(1)* %pInt
; OPAQUE:   %dummyInt = load i32, ptr addrspace(1) @foo.localInt, align 4
; OPAQUE-NEXT:   store i32 %dummyInt, ptr addrspace(1) %pInt
; CHECK-NEXT:   ret void

; NONOPAQUE:        define void @bar(<4 x i32> addrspace(1)* %pInt4, <16 x i64> addrspace(1)* %pLong16,
; OPAQUE:           define void @bar(ptr addrspace(1) %pInt4, ptr addrspace(1) %pLong16,
; CHECK-NEXT:   entry:
; NONOPAQUE-NEXT:   %dummyLong16 = load <16 x i64>, <16 x i64> addrspace(1)* @bar.localLong16, align 128
; NONOPAQUE-NEXT:   store <16 x i64> %dummyLong16, <16 x i64> addrspace(1)* %pLong16
; OPAQUE-NEXT:   %dummyLong16 = load <16 x i64>, ptr addrspace(1) @bar.localLong16, align 128
; OPAQUE-NEXT:   store <16 x i64> %dummyLong16, ptr addrspace(1) %pLong16
; CHECK-NEXT:   ret void

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
