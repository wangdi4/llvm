; RUN: opt -add-implicit-args -S %s | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; adding implicit arguments to the definition of a function

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

; CHECK:      declare void @__functionWithoutArgs_before.AddImplicitArgs() #0
; CHECK:      declare i32 @__functionWithArgs_before.AddImplicitArgs(i32, i32) #0

; CHECK:      define void @functionWithoutArgs(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK:          { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* noalias %pWorkDim,
; CHECK:          i32* noalias %pWGId,
; CHECK:          [4 x i32] %BaseGlbId,
; CHECK:          i8* noalias %pSpecialBuf,
; CHECK:          {}* noalias %RuntimeHandle) #0 {
; CHECK-NEXT: entry:
; CHECK-NEXT:     %x = add i32 100, 10
; CHECK-NEXT:     ret void

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

; CHECK: attributes #0 = { nounwind }

