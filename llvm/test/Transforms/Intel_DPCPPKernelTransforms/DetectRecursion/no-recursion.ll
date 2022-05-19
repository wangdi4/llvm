; RUN: opt -enable-new-pm=0 -dpcpp-kernel-detect-recursion -S %s | FileCheck %s
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-detect-recursion -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-detect-recursion %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-detect-recursion %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

define void @func1() nounwind{
entry:
  ret void
}

; CHECK-NOT: recursive_call

; DEBUGIFY-NOT: WARNING
