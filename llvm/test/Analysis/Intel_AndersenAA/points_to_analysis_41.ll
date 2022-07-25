; This test verifies that CallBacks are handled correctly by checking
; "%i45" in main is pointing to "%call" in foo.

; RUN: opt < %s -passes='require<anders-aa>' -disable-output -print-anders-points-to 2>&1 | FileCheck %s

; CHECK: [1] main:i45    --> ({{[0-9]+}}): foo:call

; REQUIRES: asserts

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal void @foo(i32*** %wx) {
entry:
  %call = call i8* @malloc(i64 4)
  %i7 = load i32**, i32*** %wx, align 8
  %i9 = bitcast i32** %i7 to i8**
  store i8* %call, i8** %i9, align 8
  ret void
}

define i32 @main() personality i8* undef {
if.end72:
  %xyzw = alloca i32**, align 8
  %call74 = tail call i8* @malloc(i64 0)
  %i42 = bitcast i8* %call74 to i32**
  %i41 = bitcast i32*** %xyzw to i8**
  store i8* %call74, i8** %i41, align 8
  call void (i32, void (i32, ...)*, ...) @broker(i32 3, void (i32, ...)* bitcast (void (i32***)* @foo to void (i32, ...)*), i32*** %xyzw)
  br label %for.cond80.preheader

for.cond80.preheader:                             ; preds = %if.end72
  %arrayidx85 = getelementptr inbounds i32*, i32** %i42, i64 undef
  br label %for.body82

for.body82:                                       ; preds = %for.body82, %for.cond80.preheader
  %i45 = load i32*, i32** %i42, align 8
  %nonnull = getelementptr inbounds i32, i32* %i45, i64 undef
  store i32 0, i32* %nonnull, align 4
  br label %for.body82
}

declare i8* @malloc(i64)

declare !callback !0 void @broker(i32, void (i32, ...)*, ...)

!0 = !{!1}
!1 = !{i64 1, i64 2, i1 false}
