; RUN: opt %s -dpcpp-kernel-vector-variant-lowering -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -dpcpp-kernel-vector-variant-lowering -S | FileCheck %s
; RUN: opt %s -passes=dpcpp-kernel-vector-variant-lowering -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -passes=dpcpp-kernel-vector-variant-lowering -S | FileCheck %s

define void @bar(i32, float) {
entry:
  ret void
}

define void @foo(i32 %i, float %f) {
entry:
  call void @bar(i32 %i, float %f) #0
; CHECK: call void @bar(i32 %i, float %f) #0
; CHECK: attributes #0 = { "vector-variants"="_ZGVbN0lu_XXX,_ZGVbM0vv_XXX" }
  ret void
}

attributes #0 = { "vector-variants"="_ZGVxN0lu_XXX,_ZGVxM0vv_XXX" }

; DEBUGIFY-NOT: WARNING
