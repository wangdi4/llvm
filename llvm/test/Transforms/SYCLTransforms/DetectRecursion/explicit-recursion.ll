; RUN: opt -passes=sycl-kernel-detect-recursion %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-detect-recursion %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

define void @rec1() nounwind{
entry:
  call void @rec1()
  ret void
}

; CHECK: define void @rec1() #{{[0-9]*}} !recursive_call ![[#REC:]]
; CHECK: ![[#REC]] = !{i1 true}

!spirv.Source = !{!0}
!0 = !{i32 4, i32 100000}

; DEBUGIFY-NOT: WARNING
