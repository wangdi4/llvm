; RUN: opt -add-implicit-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; not adding implicit arguments to the declaration of a function


declare void @functionWithoutArgs() nounwind

declare i32 @functionWithArgs(i32 %x, i32 %y) nounwind

define i32 @caller(i32 %x, i32 %y) {
entry:
    call void @functionWithoutArgs()
    %res = call i32 @functionWithArgs(i32 %x, i32 %y)
    ret i32 %res
}


; CHECK:        declare void @functionWithoutArgs() nounwind
; CHECK:        declare i32 @functionWithArgs(i32, i32) nounwind

; CHECK:        define i32 @caller(i32 %x, i32 %y, i8 addrspace(3)* noalias [[P_LOCAL_MEM:%[a-zA-Z0-9]+]], 
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
; CHECK-NEXT:   call void @functionWithoutArgs()
; CHECK-NEXT:   %res = call i32 @functionWithArgs(i32 %x, i32 %y)
; CHECK-NEXT:   ret i32 %res
