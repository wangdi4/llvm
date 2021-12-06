; RUN: opt -dpcpp-kernel-local-buffers -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-local-buffers' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-local-buffers -S %s | FileCheck %s
; RUN: opt -passes='dpcpp-kernel-local-buffers' -S %s | FileCheck %s

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



; CHECK:        define void @foo(i32 addrspace(1)* %pInt, i8 addrspace(1)* %pChar, float addrspace(1)* %pFloat,
; CHECK-NEXT:   entry:
; CHECK-NEXT:   %dummyInt = load i32, i32 addrspace(1)* @foo.localInt, align 4
; CHECK-NEXT:   store i32 %dummyInt, i32 addrspace(1)* %pInt
; CHECK-NEXT:   ret void

; CHECK:        define void @bar(<4 x i32> addrspace(1)* %pInt4, <16 x i64> addrspace(1)* %pLong16,
; CHECK-NEXT:   entry:
; CHECK-NEXT:   %dummyLong16 = load <16 x i64>, <16 x i64> addrspace(1)* @bar.localLong16, align 128
; CHECK-NEXT:   store <16 x i64> %dummyLong16, <16 x i64> addrspace(1)* %pLong16
; CHECK-NEXT:   ret void

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
