; RUN: opt -dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers -dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers -passes=dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-implicit-args %s -S | FileCheck -check-prefix=CHECK -check-prefix=CHECK-NONOPAQUE %s
; RUN: opt -passes=dpcpp-kernel-add-implicit-args %s -S | FileCheck -check-prefix=CHECK -check-prefix=CHECK-NONOPAQUE %s
; RUN: opt -opaque-pointers -dpcpp-kernel-add-implicit-args %s -S | FileCheck -check-prefix=CHECK -check-prefix=CHECK-OPAQUE %s
; RUN: opt -opaque-pointers -passes=dpcpp-kernel-add-implicit-args %s -S | FileCheck -check-prefix=CHECK -check-prefix=CHECK-OPAQUE %s

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


; CHECK:                declare void @__functionWithoutArgs_before.AddImplicitArgs() #0
; CHECK:                declare i32 @__functionWithArgs_before.AddImplicitArgs(i32, i32) #0

; CHECK-NONOPAQUE:      define void @functionWithoutArgs(i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK-OPAQUE:         define void @functionWithoutArgs(ptr addrspace(3) noalias %pLocalMemBase,
; CHECK-NONOPAQUE:          { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }* noalias %pWorkDim,
; CHECK-OPAQUE:             ptr noalias %pWorkDim,
; CHECK-NONOPAQUE:          i32* noalias %pWGId,
; CHECK-OPAQUE:             ptr noalias %pWGId,
; CHECK:                    [4 x i32] %BaseGlbId,
; CHECK-NONOPAQUE:          i8* noalias %pSpecialBuf,
; CHECK-OPAQUE:             ptr noalias %pSpecialBuf,
; CHECK-NONOPAQUE:          {}* noalias %RuntimeHandle) #0 {
; CHECK-OPAQUE:             ptr noalias %RuntimeHandle) #0 {
; CHECK-NEXT:             entry:
; CHECK-NEXT:             %x = add i32 100, 10
; CHECK-NEXT:             ret void

; CHECK:                define i32 @functionWithArgs(i32 %x, i32 %y,
; CHECK-NONOPAQUE:          i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK-OPAQUE:             ptr addrspace(3) noalias %pLocalMemBase,
; CHECK-NONOPAQUE:          { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }* noalias %pWorkDim,
; CHECK-OPAQUE:             ptr noalias %pWorkDim,
; CHECK-NONOPAQUE:          i32* noalias %pWGId,
; CHECK-OPAQUE:             ptr noalias %pWGId,
; CHECK:                    [4 x i32] %BaseGlbId,
; CHECK-NONOPAQUE:          i8* noalias %pSpecialBuf,
; CHECK-OPAQUE:             ptr noalias %pSpecialBuf,
; CHECK-NONOPAQUE:          {}* noalias %RuntimeHandle) #0 {
; CHECK-OPAQUE:             ptr noalias %RuntimeHandle) #0 {
; CHECK-NEXT:             entry:
; CHECK-NEXT:             %temp = add i32 %x, 10
; CHECK-NEXT:             %res = mul i32 %temp, %y
; CHECK-NEXT:             ret i32 %res

; CHECK:                define i32 @caller(i32 %x, i32 %y,
; CHECK-NONOPAQUE:          i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK-OPAQUE:             ptr addrspace(3) noalias %pLocalMemBase,
; CHECK-NONOPAQUE:          { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }* noalias %pWorkDim,
; CHECK-OPAQUE:             ptr noalias %pWorkDim,
; CHECK-NONOPAQUE:          i32* noalias %pWGId,
; CHECK-OPAQUE:             ptr noalias %pWGId,
; CHECK:                    [4 x i32] %BaseGlbId,
; CHECK-NONOPAQUE:          i8* noalias %pSpecialBuf,
; CHECK-OPAQUE:             ptr noalias %pSpecialBuf,
; CHECK-NONOPAQUE:          {}* noalias %RuntimeHandle) {
; CHECK-OPAQUE:             ptr noalias %RuntimeHandle) {
; CHECK-NEXT:             entry:
; CHECK-NONOPAQUE-NEXT:   %LocalMem_functionWithoutArgs = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK-OPAQUE-NEXT:      %LocalMem_functionWithoutArgs = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; CHECK-NONOPAQUE-NEXT:   call void @functionWithoutArgs(i8 addrspace(3)* noalias %LocalMem_functionWithoutArgs,
; CHECK-OPAQUE-NEXT:      call void @functionWithoutArgs(ptr addrspace(3) noalias %LocalMem_functionWithoutArgs,
; CHECK-NONOPAQUE:          { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }* noalias %pWorkDim,
; CHECK-OPAQUE:             ptr noalias %pWorkDim,
; CHECK-NONOPAQUE:          i32* noalias %pWGId,
; CHECK-OPAQUE:             ptr noalias %pWGId,
; CHECK:                    [4 x i32] %BaseGlbId,
; CHECK-NONOPAQUE:          i8* noalias %pSpecialBuf,
; CHECK-OPAQUE:             ptr noalias %pSpecialBuf,
; CHECK-NONOPAQUE:          {}* noalias %RuntimeHandle)
; CHECK-OPAQUE:             ptr noalias %RuntimeHandle)
; CHECK-NONOPAQUE-NEXT:   %LocalMem_functionWithArgs = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK-OPAQUE-NEXT:      %LocalMem_functionWithArgs = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; CHECK:                  [[VAR2:%[a-zA-Z0-9]+]] = call i32 @functionWithArgs(i32 %x, i32 %y,
; CHECK-NONOPAQUE:          i8 addrspace(3)* noalias %LocalMem_functionWithArgs,
; CHECK-OPAQUE:             ptr addrspace(3) noalias %LocalMem_functionWithArgs,
; CHECK-NONOPAQUE:          { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }* noalias %pWorkDim,
; CHECK-OPAQUE:             ptr noalias %pWorkDim,
; CHECK-NONOPAQUE:          i32* noalias %pWGId,
; CHECK-OPAQUE:             ptr noalias %pWGId,
; CHECK:                    [4 x i32] %BaseGlbId,
; CHECK-NONOPAQUE:          i8* noalias %pSpecialBuf,
; CHECK-OPAQUE:             ptr noalias %pSpecialBuf,
; CHECK-NONOPAQUE:          {}* noalias %RuntimeHandle)
; CHECK-OPAQUE:             ptr noalias %RuntimeHandle)
; CHECK-NEXT:             ret i32 [[VAR2]]

; CHECK: attributes #0 = { nounwind }

; DEBUGIFY: CheckModuleDebugify: PASS
