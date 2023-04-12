; This test verifies that BasicAA is invalidated and recomputed for func5.
; Also makes sure that BasicAA is invalidated and recomputed for func5 only once.

; RUN: opt -aa-pipeline=basic-aa -passes=arg-noalias-prop -disable-output  -debug-pass-manager %s  2>&1 | FileCheck %s

; REQUIRES: asserts

; CHECK: Running analysis: BasicAA on func5

; Makes sure BasicAA is invalidated for func5
; CHECK: Invalidating analysis: BasicAA on func5
; CHECK: Running analysis: BasicAA on func5


; Makes sure BasicAA is not invalidated again for func5
; CHECK-NOT: Invalidating analysis: BasicAA on func5
; CHECK-NOT: Running analysis: BasicAA on func5

define internal void @callback(ptr nocapture %ID, ptr nocapture %A, ptr nocapture %B) {
bb:
  %X = load i32, ptr %B, align 4
  store i32 %X, ptr %A, align 4
  ret void
}

define void @func1(ptr noalias %A, ptr noalias %B) {
bb:
  tail call void (i32, ptr, ...) @broker(i32 2, ptr @callback, ptr %A, ptr %B)
  ret void
}

define void @func2(ptr noalias %A, ptr noalias %B) {
bb:
  tail call void @callback(ptr null, ptr %A, ptr %B)
  ret void
}

define i32 @func3() {
bb:
  %A = alloca i32, align 4
  %B = alloca i32, align 4
  store i32 2, ptr %B, align 4
  call void @callback(ptr null, ptr %A, ptr %B)
  %X = load i32, ptr %A, align 4
  ret i32 %X
}

define i32 @func4() {
bb:
  %A = alloca i32, align 4
  %B = tail call noalias ptr @malloc(i64 4)
  store i32 2, ptr %B, align 4
  call void @callback(ptr null, ptr %A, ptr %B)
  tail call void @free(ptr %B)
  %X = load i32, ptr %A, align 4
  ret i32 %X
}

define void @func5(ptr noalias %A, ptr noalias %B, i1 %cond) {
entry:
  br i1 %cond, label %if, label %else

if:                                               ; preds = %entry
  tail call void (i32, ptr, ...) @broker(i32 2, ptr @callback, ptr %A, ptr %B)
  br label %done

else:                                             ; preds = %entry
  tail call void (i32, ptr, ...) @broker(i32 2, ptr @callback, ptr %B, ptr %A)
  br label %done

done:                                             ; preds = %else, %if
  ret void
}

define i32 @func6() {
entry:
  %A = tail call noalias ptr @malloc(i64 4)
  %B = tail call noalias ptr @malloc(i64 4)
  store i32 2, ptr %B, align 4
  store i32 3, ptr %B, align 4
  br label %body

body:                                             ; preds = %body, %entry
  %X = phi ptr [ %A, %entry ], [ %Y, %body ]
  %Y = phi ptr [ %B, %entry ], [ %X, %body ]
  call void @callback(ptr null, ptr %X, ptr %Y)
  %val = load i32, ptr %X, align 4
  %cond = icmp ne i32 %val, 0
  br i1 %cond, label %exit, label %body

exit:                                             ; preds = %body
  tail call void @free(ptr %A)
  tail call void @free(ptr %B)
  %fval = load i32, ptr %A, align 4
  ret i32 %fval
}

declare noalias ptr @malloc(i64)

declare void @free(ptr nocapture)

declare !callback !0 void @broker(i32, ptr, ...)

!0 = !{!1}
!1 = !{i64 1, i64 -1, i1 true}
