; RUN: opt -add-implicit-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

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


; CHECK:      declare void @__functionWithoutArgs_before.AddImplicitArgs() #0
; CHECK:      declare i32 @__functionWithArgs_before.AddImplicitArgs(i32, i32) #0

; CHECK:      define void @functionWithoutArgs(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK:          { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* noalias %pWorkDim,
; CHECK:          i32* noalias %pWGId,
; CHECK:          [4 x i32] %BaseGlbId,
; CHECK:          i8* noalias %pSpecialBuf,
; CHECK:          {}* noalias %RuntimeHandle) #0 {
; CHECK-NEXT:   entry:
; CHECK-NEXT:   %x = add i32 100, 10
; CHECK-NEXT:   ret void

; CHECK:      define i32 @functionWithArgs(i32 %x, i32 %y,
; CHECK:          i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK:          { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* noalias %pWorkDim,
; CHECK:          i32* noalias %pWGId,
; CHECK:          [4 x i32] %BaseGlbId,
; CHECK:          i8* noalias %pSpecialBuf,
; CHECK:          {}* noalias %RuntimeHandle) #0 {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %temp = add i32 %x, 10
; CHECK-NEXT:   %res = mul i32 %temp, %y
; CHECK-NEXT:   ret i32 %res

; CHECK:      define i32 @caller(i32 %x, i32 %y,
; CHECK:          i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK:          { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* noalias %pWorkDim,
; CHECK:          i32* noalias %pWGId,
; CHECK:          [4 x i32] %BaseGlbId,
; CHECK:          i8* noalias %pSpecialBuf,
; CHECK:          {}* noalias %RuntimeHandle) {
; CHECK-NEXT:   entry:
; CHECK-NEXT:   %pLocalMem_functionWithoutArgs = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK-NEXT:   call void @functionWithoutArgs(i8 addrspace(3)* noalias %pLocalMem_functionWithoutArgs,
; CHECK:          { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* noalias %pWorkDim,
; CHECK:          i32* noalias %pWGId,
; CHECK:          [4 x i32] %BaseGlbId,
; CHECK:          i8* noalias %pSpecialBuf,
; CHECK:          {}* noalias %RuntimeHandle)
; CHECK-NEXT:   %pLocalMem_functionWithArgs = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK-NEXT:   [[VAR2:%[a-zA-Z0-9]+]] = call i32 @functionWithArgs(i32 %x, i32 %y,
; CHECK:          i8 addrspace(3)* noalias %pLocalMem_functionWithArgs,
; CHECK:          { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* noalias %pWorkDim,
; CHECK:          i32* noalias %pWGId,
; CHECK:          [4 x i32] %BaseGlbId,
; CHECK:          i8* noalias %pSpecialBuf,
; CHECK:          {}* noalias %RuntimeHandle)
; CHECK-NEXT:   ret i32 [[VAR2]]

; CHECK: attributes #0 = { nounwind }

