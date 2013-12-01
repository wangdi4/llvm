; RUN: opt -local-buffers -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; not mapping unused local variables to local buffer

; Used local variables
@foo.localInt = internal addrspace(3) global i32 0, align 4
@bar.localLong16 = internal addrspace(3) global <16 x i64> zeroinitializer, align 128

; Unused local variables
@foo.localChar = internal addrspace(3) global i8 0, align 1
@foo.localFloat = internal addrspace(3) global float 0.000000e+00, align 4
@bar.localInt4 = internal addrspace(3) global <4 x i32> zeroinitializer, align 16


define void @foo(i32 addrspace(1)* %pInt, i8 addrspace(1)* %pChar, float addrspace(1)* %pFloat, i8 addrspace(3)* %pLocalMem, { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }* %pWorkDim, i32* %pWGId, <{ [4 x i32] }>* %pBaseGlbId, <{ [4 x i32] }>* %pLocalIds, i32* %contextpointer, i32 %iterCount, i8* %pSpecialBuf, i32* %pCurrWI, i32* %extExecContextPointer) {
entry:
  %dummyInt = load i32 addrspace(3)* @foo.localInt, align 4
  store i32 %dummyInt, i32 addrspace(1)* %pInt
  
  ret void
}

define void @bar(<4 x i32> addrspace(1)* %pInt4, <16 x i64> addrspace(1)* %pLong16, i8 addrspace(3)* %pLocalMem, { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }* %pWorkDim, i32* %pWGId, <{ [4 x i32] }>* %pBaseGlbId, <{ [4 x i32] }>* %pLocalIds, i32* %contextpointer, i32 %iterCount, i8* %pSpecialBuf, i32* %pCurrWI, i32* %extExecContextPointer) {
entry:
  %dummyLong16 = load <16 x i64> addrspace(3)* @bar.localLong16, align 128
  store <16 x i64> %dummyLong16, <16 x i64> addrspace(1)* %pLong16
  
  ret void
}



; CHECK:        define void @foo(i32 addrspace(1)* %pInt, i8 addrspace(1)* %pChar, float addrspace(1)* %pFloat,
; CHECK-NEXT:   entry:
; CHECK-NEXT:   [[VAR0:%[a-zA-Z0-9]+]] = getelementptr i8 addrspace(3)* %pLocalMem, i32 0
; CHECK-NEXT:   [[VAR1:%[a-zA-Z0-9]+]] = bitcast i8 addrspace(3)* [[VAR0]] to i32 addrspace(3)*

; CHECK-NEXT:   %dummyInt = load i32 addrspace(3)* [[VAR1]], align 4
; CHECK-NEXT:   store i32 %dummyInt, i32 addrspace(1)* %pInt
; CHECK-NEXT:   ret void


; CHECK:        define void @bar(<4 x i32> addrspace(1)* %pInt4, <16 x i64> addrspace(1)* %pLong16,
; CHECK-NEXT:   entry:
; CHECK-NEXT:   [[VAR10:%[a-zA-Z0-9]+]] = getelementptr i8 addrspace(3)* %pLocalMem, i32 0
; CHECK-NEXT:   [[VAR11:%[a-zA-Z0-9]+]] = bitcast i8 addrspace(3)* [[VAR10]] to <16 x i64> addrspace(3)*

; CHECK-NEXT:   %dummyLong16 = load <16 x i64> addrspace(3)* [[VAR11]], align 128
; CHECK-NEXT:   store <16 x i64> %dummyLong16, <16 x i64> addrspace(1)* %pLong16
; CHECK-NEXT:   ret void
