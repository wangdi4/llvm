; RUN: %oclopt %s -vector-variant-lowering -vector-variant-isa-override=AVX2 -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt %s -vector-variant-lowering -vector-variant-isa-override=AVX2 -S | FileCheck %s

define void @bar(i32, float) {
entry:
  ret void
}

define void @foo(i32 %i, float %f) {
entry:
  call void @bar(i32 %i, float %f) #0
; CHECK: call void @bar(i32 %i, float %f) #0
; CHECK: attributes #0 = { "vector-variants"="_ZGVdN0lu_XXX,_ZGVdM0vv_XXX" }
  ret void
}

attributes #0 = { "vector-variants"="_ZGVxN0lu_XXX,_ZGVxM0vv_XXX" }

; DEBUGIFY-NOT: WARNING
