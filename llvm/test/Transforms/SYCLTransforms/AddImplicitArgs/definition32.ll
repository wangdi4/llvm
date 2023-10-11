; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S | FileCheck %s

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

; CHECK:             declare void @__functionWithoutArgs_before.AddImplicitArgs() #0
; CHECK:             declare i32 @__functionWithArgs_before.AddImplicitArgs(i32, i32) #0

; CHECK:      define void @functionWithoutArgs(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK:          ptr noalias %pWorkDim,
; CHECK:          ptr noalias %pWGId,
; CHECK:                 [4 x i32] %BaseGlbId,
; CHECK:          ptr noalias %pSpecialBuf,
; CHECK:          ptr noalias %RuntimeHandle) #0 {
; CHECK-NEXT:          entry:
; CHECK-NEXT:          %x = add i32 100, 10
; CHECK-NEXT:          ret void

; CHECK:           define i32 @functionWithArgs(i32 %x, i32 %y,
; CHECK:        ptr addrspace(3) noalias %pLocalMemBase,
; CHECK:        ptr noalias %pWorkDim,
; CHECK:        ptr noalias %pWGId,
; CHECK:               [4 x i32] %BaseGlbId,
; CHECK:        ptr noalias %pSpecialBuf,
; CHECK:        ptr noalias %RuntimeHandle) #0 {
; CHECK-NEXT:        entry:
; CHECK-NEXT:        %temp = add i32 %x, 10
; CHECK-NEXT:        %res = mul i32 %temp, %y
; CHECK-NEXT:        ret i32 %res

; CHECK: attributes #0 = { nounwind }

; DEBUGIFY-NOT: WARNING
