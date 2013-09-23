; RUN: opt -add-implicit-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; adding implicit arguments to a call of a function


define void @functionWithoutArgs() nounwind {
entry:
  %x = add i32 100, 10
  ret void
}

define i32 @functionWithArgs(i32 %x, i32 %y) nounwind {
entry:
  %temp = add i32 %x, 10
  %res = mul i32 %temp, %y
  ret i32 %res
}

define i32 @caller(i32 %x, i32 %y) {
entry:
    call void @functionWithoutArgs()
    %call = call i32 @functionWithArgs(i32 %x, i32 %y)
    ret i32 %call
}


; CHECK:        declare void @__functionWithoutArgs_original() nounwind
; CHECK:        declare i32 @__functionWithArgs_original(i32, i32) nounwind

; CHECK:        define void @functionWithoutArgs(i8 addrspace(3)* noalias [[P_LOCAL_MEM:%[a-zA-Z0-9]+]], 
; CHECK:            { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* noalias [[P_WORK_DIM:%[a-zA-Z0-9]+]], 
; CHECK:            i64* noalias [[P_WORKGROUP_ID:%[a-zA-Z0-9]+]], 
; CHECK:            <{ [4 x i64] }>* noalias [[P_BASE_GLOBAL_ID:%[a-zA-Z0-9]+]], 
; CHECK:            i64* noalias [[CONTEXT_POINTER:%[a-zA-Z0-9]+]], 
; CHECK:            <{ [4 x i64] }>* noalias [[P_LOCAL_IDS:%[a-zA-Z0-9]+]], 
; CHECK:            i64 [[ITER_COUNT:%[a-zA-Z0-9]+]], 
; CHECK:            i8* noalias [[P_SPECIAL_BUFFER:%[a-zA-Z0-9]+]], 
; CHECK:            i64* noalias [[P_CURRECT_WI:%[a-zA-Z0-9]+]],
; CHECK:            %struct.ExtendedExecutionContext* noalias [[EXTCONTEXT_POINTER:%[a-zA-Z0-9]+]]) nounwind {
; CHECK-NEXT:   entry:
; CHECK-NEXT:   %x = add i32 100, 10
; CHECK-NEXT:   ret void

; CHECK:        define i32 @functionWithArgs(i32 %x, i32 %y, i8 addrspace(3)* noalias [[P_LOCAL_MEM]], 
; CHECK:            { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* noalias [[P_WORK_DIM]], 
; CHECK:            i64* noalias [[P_WORKGROUP_ID]], 
; CHECK:            <{ [4 x i64] }>* noalias [[P_BASE_GLOBAL_ID]], 
; CHECK:            i64* noalias [[CONTEXT_POINTER]], 
; CHECK:            <{ [4 x i64] }>* noalias [[P_LOCAL_IDS]], 
; CHECK:            i64 [[ITER_COUNT]], 
; CHECK:            i8* noalias [[P_SPECIAL_BUFFER]], 
; CHECK:            i64* noalias [[P_CURRECT_WI]],
; CHECK:            %struct.ExtendedExecutionContext* noalias [[EXTCONTEXT_POINTER]]) nounwind {
; CHECK-NEXT:   entry:
; CHECK-NEXT:   %temp = add i32 %x, 10
; CHECK-NEXT:   %res = mul i32 %temp, %y
; CHECK-NEXT:   ret i32 %res

; CHECK:        define i32 @caller(i32 %x, i32 %y, i8 addrspace(3)* noalias [[P_LOCAL_MEM]], 
; CHECK:            { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* noalias [[P_WORK_DIM]], 
; CHECK:            i64* noalias [[P_WORKGROUP_ID]], 
; CHECK:            <{ [4 x i64] }>* noalias [[P_BASE_GLOBAL_ID]], 
; CHECK:            i64* noalias [[CONTEXT_POINTER]], 
; CHECK:            <{ [4 x i64] }>* noalias [[P_LOCAL_IDS]], 
; CHECK:            i64 [[ITER_COUNT]], 
; CHECK:            i8* noalias [[P_SPECIAL_BUFFER]], 
; CHECK:            i64* noalias [[P_CURRECT_WI]],
; CHECK:            %struct.ExtendedExecutionContext* noalias [[EXTCONTEXT_POINTER]]) {
; CHECK-NEXT:   entry:
; CHECK-NEXT:   [[VAR0:%[a-zA-Z0-9]+]] = getelementptr i8 addrspace(3)* [[P_LOCAL_MEM]], i32 0
; CHECK-NEXT:   call void @functionWithoutArgs(i8 addrspace(3)* [[VAR0]], 
; CHECK:            { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* [[P_WORK_DIM:%[a-zA-Z0-9]+]], 
; CHECK:            i64* [[P_WORKGROUP_ID:%[a-zA-Z0-9]+]], 
; CHECK:            <{ [4 x i64] }>* [[P_BASE_GLOBAL_ID:%[a-zA-Z0-9]+]], 
; CHECK:            i64* [[CONTEXT_POINTER:%[a-zA-Z0-9]+]], 
; CHECK:            <{ [4 x i64] }>* [[P_LOCAL_IDS:%[a-zA-Z0-9]+]], 
; CHECK:            i64 [[ITER_COUNT:%[a-zA-Z0-9]+]], 
; CHECK:            i8* [[P_SPECIAL_BUFFER:%[a-zA-Z0-9]+]], 
; CHECK:            i64* [[P_CURRECT_WI:%[a-zA-Z0-9]+]],
; CHECK:            %struct.ExtendedExecutionContext* [[EXTCONTEXT_POINTER:%[a-zA-Z0-9]+]])
; CHECK-NEXT:   [[VAR1:%[a-zA-Z0-9]+]] = getelementptr i8 addrspace(3)* [[P_LOCAL_MEM]], i32 0
; CHECK-NEXT:   [[VAR2:%[a-zA-Z0-9]+]] = call i32 @functionWithArgs(i32 %x, i32 %y, i8 addrspace(3)* [[VAR1]], 
; CHECK:            { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* [[P_WORK_DIM:%[a-zA-Z0-9]+]], 
; CHECK:            i64* [[P_WORKGROUP_ID:%[a-zA-Z0-9]+]], 
; CHECK:            <{ [4 x i64] }>* [[P_BASE_GLOBAL_ID:%[a-zA-Z0-9]+]], 
; CHECK:            i64* [[CONTEXT_POINTER:%[a-zA-Z0-9]+]], 
; CHECK:            <{ [4 x i64] }>* [[P_LOCAL_IDS:%[a-zA-Z0-9]+]], 
; CHECK:            i64 [[ITER_COUNT:%[a-zA-Z0-9]+]], 
; CHECK:            i8* [[P_SPECIAL_BUFFER:%[a-zA-Z0-9]+]], 
; CHECK:            i64* [[P_CURRECT_WI:%[a-zA-Z0-9]+]],
; CHECK:            %struct.ExtendedExecutionContext* [[EXTCONTEXT_POINTER:%[a-zA-Z0-9]+]])
; CHECK-NEXT:   ret i32 [[VAR2]]

