; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: @a = global { [1 x ptr], ptr } { [1 x ptr] [ptr @foo], ptr @bar }

@a = global { [1 x ptr], ptr } { [1 x ptr] [ptr @foo], ptr @bar }
@b = global ptr @foo

; CHECK: define i32 @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle)

define i32 @foo() {
  ret i32 0
}

; CHECK: define void @bar(ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle)

define void @bar() {
  ret void
}

; DEBUGIFY-NOT: WARNING
