; RUN: opt -add-implicit-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

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

; CHECK:        declare void @__functionWithoutArgs_original() nounwind
; CHECK:        declare i32 @__functionWithArgs_original(i32, i32) nounwind

; CHECK:      define void @functionWithoutArgs(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK:          { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], i64, {}*, [4 x i64]* }* noalias %pWorkDim
; CHECK:          i64* noalias %pWGId,
; CHECK:          [4 x i64] %BaseGlbId,
; CHECK:          i8* noalias %pSpecialBuf,
; CHECK:          i64* noalias %pCurrWI,
; CHECK:          {}* noalias %RuntimeHandle) nounwind {
; CHECK-NEXT: entry:
; CHECK-NEXT:     %x = add i32 100, 10
; CHECK-NEXT:     ret void

; CHECK:      define i32 @functionWithArgs(i32 %x, i32 %y,
; CHECK:          i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK:          { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], i64, {}*, [4 x i64]* }* noalias %pWorkDim
; CHECK:          i64* noalias %pWGId,
; CHECK:          [4 x i64] %BaseGlbId,
; CHECK:          i8* noalias %pSpecialBuf,
; CHECK:          i64* noalias %pCurrWI,
; CHECK:          {}* noalias %RuntimeHandle) nounwind {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %temp = add i32 %x, 10
; CHECK-NEXT:   %res = mul i32 %temp, %y
; CHECK-NEXT:   ret i32 %res
