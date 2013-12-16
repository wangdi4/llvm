; RUN: opt -add-implicit-args -S -verify < %s | FileCheck %s
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


; CHECK:        declare void @functionWithoutArgs()
; CHECK:        declare i32 @functionWithArgs(i32, i32)

; CHECK:      define i32 @caller(i32 %x, i32 %y,
; CHECK:          i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK:          { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32], i32, {}*, [4 x i32]* }* noalias %pWorkDim,
; CHECK:          i32* noalias %pWGId,
; CHECK:          [4 x i32] %BaseGlbId,
; CHECK:          i8* noalias %pSpecialBuf,
; CHECK:          i32* noalias %pCurrWI,
; CHECK:          {}* noalias %RuntimeContext) {
; CHECK-NEXT:   entry:
; CHECK-NEXT:   call void @functionWithoutArgs()
; CHECK-NEXT:   %res = call i32 @functionWithArgs(i32 %x, i32 %y)
; CHECK-NEXT:   ret i32 %res
