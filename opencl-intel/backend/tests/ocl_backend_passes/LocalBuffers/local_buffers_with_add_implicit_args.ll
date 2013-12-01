; RUN: opt -add-implicit-args -local-buffers -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll
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
  %dummyInt = load i32 addrspace(3)* @foo.localInt, align 4
  store i32 %dummyInt, i32 addrspace(1)* %ApInt
  store i32 %dummyInt, i32 addrspace(1)* %BpInt
  
  %dummyChar = load i8 addrspace(3)* @foo.localChar, align 1
  store i8 %dummyChar, i8 addrspace(1)* %pChar
  
  %dummyFloat = load float addrspace(3)* @foo.localFloat, align 4
  store float %dummyFloat, float addrspace(1)* %pFloat
  
  ret void
}

define void @bar(<4 x i32> addrspace(1)* %pInt4, <16 x i64> addrspace(1)* %pLong16) {
entry:
  %dummyInt4 = load <4 x i32> addrspace(3)* @bar.localInt4, align 16
  store <4 x i32> %dummyInt4, <4 x i32> addrspace(1)* %pInt4
  
  %dummyLong16 = load <16 x i64> addrspace(3)* @bar.localLong16, align 128
  store <16 x i64> %dummyLong16, <16 x i64> addrspace(1)* %pLong16
  
  ret void
}



; CHECK:        define void @foo(i32 addrspace(1)* %ApInt, i32 addrspace(1)* %BpInt, i8 addrspace(1)* %pChar, float addrspace(1)* %pFloat, 
; CHECK:            i8 addrspace(3)* noalias [[P_LOCAL_MEM:%[a-zA-Z0-9]+]], 
; CHECK:            { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }* noalias [[P_WORK_DIM:%[a-zA-Z0-9]+]], 
; CHECK:            i32* noalias [[P_WORKGROUP_ID:%[a-zA-Z0-9]+]], 
; CHECK:            <{ [4 x i32] }>* noalias [[P_BASE_GLOBAL_ID:%[a-zA-Z0-9]+]], 
; CHECK:            i32* noalias [[CONTEXT_POINTER:%[a-zA-Z0-9]+]], 
; CHECK:            <{ [4 x i32] }>* noalias [[P_LOCAL_IDS:%[a-zA-Z0-9]+]], 
; CHECK:            i32 [[ITER_COUNT:%[a-zA-Z0-9]+]], 
; CHECK:            i8* noalias [[P_SPECIAL_BUFFER:%[a-zA-Z0-9]+]], 
; CHECK:            i32* noalias [[P_CURRECT_WI:%[a-zA-Z0-9]+]],
; CHECK:            %struct.ExtendedExecutionContext* noalias [[EXTCONTEXT_POINTER:%[a-zA-Z0-9]+]]) {
; CHECK-NEXT:   entry:
; CHECK-NEXT:   [[VAR0:%[a-zA-Z0-9]+]] = getelementptr i8 addrspace(3)* [[P_LOCAL_MEM]], i32 0
; CHECK-NEXT:   [[VAR1:%[a-zA-Z0-9]+]] = bitcast i8 addrspace(3)* [[VAR0]] to i32 addrspace(3)*

; CHECK-NEXT:   [[VAR2:%[a-zA-Z0-9]+]] = getelementptr i8 addrspace(3)* [[P_LOCAL_MEM]], i32 128
; CHECK-NEXT:   [[VAR3:%[a-zA-Z0-9]+]] = bitcast i8 addrspace(3)* [[VAR2]] to i8 addrspace(3)*

; CHECK-NEXT:   [[VAR4:%[a-zA-Z0-9]+]] = getelementptr i8 addrspace(3)* [[P_LOCAL_MEM]], i32 256
; CHECK-NEXT:   [[VAR5:%[a-zA-Z0-9]+]] = bitcast i8 addrspace(3)* [[VAR4]] to float addrspace(3)*

; CHECK-NEXT:   %dummyInt = load i32 addrspace(3)* [[VAR1]], align 4
; CHECK-NEXT:   store i32 %dummyInt, i32 addrspace(1)* %ApInt
; CHECK-NEXT:   store i32 %dummyInt, i32 addrspace(1)* %BpInt
; CHECK-NEXT:   %dummyChar = load i8 addrspace(3)* [[VAR3]], align 1
; CHECK-NEXT:   store i8 %dummyChar, i8 addrspace(1)* %pChar
; CHECK-NEXT:   %dummyFloat = load float addrspace(3)* [[VAR5]], align 4
; CHECK-NEXT:   store float %dummyFloat, float addrspace(1)* %pFloat
; CHECK-NEXT:   ret void


; CHECK:        define void @bar(<4 x i32> addrspace(1)* %pInt4, <16 x i64> addrspace(1)* %pLong16, 
; CHECK:            i8 addrspace(3)* noalias [[P_LOCAL_MEM]], 
; CHECK:            { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }* noalias [[P_WORK_DIM]], 
; CHECK:            i32* noalias [[P_WORKGROUP_ID]], 
; CHECK:            <{ [4 x i32] }>* noalias [[P_BASE_GLOBAL_ID]], 
; CHECK:            i32* noalias [[CONTEXT_POINTER]], 
; CHECK:            <{ [4 x i32] }>* noalias [[P_LOCAL_IDS]], 
; CHECK:            i32 [[ITER_COUNT]], 
; CHECK:            i8* noalias [[P_SPECIAL_BUFFER]], 
; CHECK:            i32* noalias [[P_CURRECT_WI]], 
; CHECK:            struct.ExtendedExecutionContext* noalias [[EXTCONTEXT_POINTER]]) {
; CHECK-NEXT:   entry:
; CHECK-NEXT:   [[VAR10:%[a-zA-Z0-9]+]] = getelementptr i8 addrspace(3)* [[P_LOCAL_MEM]], i32 0
; CHECK-NEXT:   [[VAR11:%[a-zA-Z0-9]+]] = bitcast i8 addrspace(3)* [[VAR10]] to <4 x i32> addrspace(3)*

; CHECK-NEXT:   [[VAR12:%[a-zA-Z0-9]+]] = getelementptr i8 addrspace(3)* [[P_LOCAL_MEM]], i32 128
; CHECK-NEXT:   [[VAR13:%[a-zA-Z0-9]+]] = bitcast i8 addrspace(3)* [[VAR12]] to <16 x i64> addrspace(3)*

; CHECK-NEXT:   %dummyInt4 = load <4 x i32> addrspace(3)* [[VAR11]], align 16
; CHECK-NEXT:   store <4 x i32> %dummyInt4, <4 x i32> addrspace(1)* %pInt4
; CHECK-NEXT:   %dummyLong16 = load <16 x i64> addrspace(3)* [[VAR13]], align 128
; CHECK-NEXT:   store <16 x i64> %dummyLong16, <16 x i64> addrspace(1)* %pLong16
; CHECK-NEXT:   ret void
