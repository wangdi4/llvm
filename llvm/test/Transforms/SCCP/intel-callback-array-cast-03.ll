; This test case checks that the interprocedural sparse conditional constant
; propagation passes when a pointer is casted to an array and it is used inside
; a callback.

; RUN: opt -passes=ipsccp -S %s | FileCheck %s --check-prefix OPAQUE

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%TestStruct = type { [16 x double], [16 x double], i32 }
@globVar = internal global %TestStruct zeroinitializer

define internal void @callback(ptr %ID, ptr %Arr) {
entry:
  %dummy = getelementptr [16 x double], ptr %Arr, i64 0, i64 0
  ret void
}

define internal void @foo(ptr %Arr) {
entry:
  call void (i32, void (ptr, ...)*, ...) @broker(i32 3, void (ptr, ...)* bitcast (void (ptr, ptr)* @callback to void (ptr, ...)*), ptr %Arr)
  ret void
}

define void @bar() {
entry:
  %tmp = getelementptr inbounds %TestStruct, ptr @globVar, i64 0, i32 1, i64 0
  call void @foo(ptr %tmp)
  ret void
}

declare !callback !0 void @broker(i32, void (ptr, ...)*, ...)

!0 = !{!1}
!1 = !{i64 1, i64 -1, i1 true}

; Check that casting was done correctly
; OPAQUE: define internal void @callback(ptr %ID, ptr %Arr) {
; OPAQUE-NOT: bitcast

; Check that the parameter in the call site for @callback was updated with the
; correct type
; OPAQUE: call void (i32, ptr, ...) @broker(i32 3, ptr @callback, ptr getelementptr inbounds (%TestStruct, ptr @globVar, i64 0, i32 1, i64 0))
