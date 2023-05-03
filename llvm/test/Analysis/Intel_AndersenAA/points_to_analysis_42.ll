; This test verifies that CallBacks are handled conservatively for
; VarArg functions by checking "%i45" in main is pointing to "<universal>"
; in foo.
; RUN: opt < %s -passes='require<anders-aa>' -disable-output -print-anders-points-to 2>&1 | FileCheck %s
; CHECK: [1] main:i45    --> ({{[0-9]+}}): <universal>
; REQUIRES: asserts
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal void @foo(ptr %wx, ...) {
entry:
  %call = call ptr @malloc(i64 4)
  %i7 = load ptr, ptr %wx, align 8
  %i9 = bitcast ptr %i7 to ptr
  store ptr %call, ptr %i9, align 8
  ret void
}

define i32 @main() personality ptr undef {
if.end72:
  %xyzw = alloca ptr, align 8
  %call74 = tail call ptr @malloc(i64 0)
  %i42 = bitcast ptr %call74 to ptr
  %i41 = bitcast ptr %xyzw to ptr
  store ptr %call74, ptr %i41, align 8
  call void (i32, ptr, ...) @broker(i32 3, ptr @foo, ptr %xyzw)
  br label %for.cond80.preheader

for.cond80.preheader:                             ; preds = %if.end72
  %arrayidx85 = getelementptr inbounds ptr, ptr %i42, i64 undef
  br label %for.body82

for.body82:                                       ; preds = %for.body82, %for.cond80.preheader
  %i45 = load ptr, ptr %i42, align 8
  %nonnull = getelementptr inbounds i32, ptr %i45, i64 undef
  store i32 0, ptr %nonnull, align 4
  br label %for.body82
}

declare ptr @malloc(i64)

declare !callback !0 void @broker(i32, ptr, ...)

!0 = !{!1}
!1 = !{i64 1, i64 2, i1 false}
