; RUN: opt -passes=instcombine -S < %s | FileCheck %s
; RUN: opt -debugify-each -passes=instcombine -S < %s | FileCheck %s

; These are both direct calls, but make sure instcombine leaves the casts
; alone.

define i32 @call_thunk(i32 %x, i32 %y) {
  %r = call i32 @inc_first_arg_thunk(i32 %x, i32 %y)
  ret i32 %r
}

; CHECK-LABEL: inc_first_arg_thunk
; CHECK: musttail call void (i32, ...) @plus(i32 {{.*}}, ...)
define internal void @inc_first_arg_thunk(i32 %arg1, ...) #0 {
entry:
  %inc = add i32 %arg1, 1
  musttail call void (i32, ...) @plus(i32 %inc, ...)
  ret void
}

define internal i32 @plus(i32 %x, i32 %y) {
  %r = add i32 %x, %y
  ret i32 %r
}

attributes #0 = { "thunk" }
