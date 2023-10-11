; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S | FileCheck %s

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

; CHECK:        define i32 @caller(i32 %x, i32 %y,
; CHECK:     ptr addrspace(3) noalias %pLocalMemBase,
; CHECK:     ptr noalias %pWorkDim,
; CHECK:     ptr noalias %pWGId,
; CHECK:            [4 x i32] %BaseGlbId,
; CHECK:     ptr noalias %pSpecialBuf,
; CHECK:     ptr noalias %RuntimeHandle) {
; CHECK-NEXT:     entry:
; CHECK-NEXT:     call void @functionWithoutArgs()
; CHECK-NEXT:     %res = call i32 @functionWithArgs(i32 %x, i32 %y)
; CHECK-NEXT:     ret i32 %res

; DEBUGIFY-NOT: WARNING
