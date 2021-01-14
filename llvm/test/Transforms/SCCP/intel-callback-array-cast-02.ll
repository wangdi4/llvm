; This test case checks that the interprocedural sparse conditional constant
; propagation won't be applied because the array was casted to a different
; type.

; RUN: opt -ipsccp -S %s | FileCheck %s
; RUN: opt -passes=ipsccp -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%DummyStruct = type { [2 x i8] }
%TestStruct = type { [16 x i8], [16 x i8], i32, i8* }
%TestStruct.1 = type { [16 x i8], [16 x i8], i32, %DummyStruct* }
@globArray = internal global [1000 x %TestStruct] zeroinitializer

define internal void @callback(i8* %ID, [1000 x %TestStruct.1]* %Arr) {
entry:
  %dummy = getelementptr [1000 x %TestStruct.1], [1000 x %TestStruct.1]* %Arr, i64 0, i64 0
  ret void
}

define internal void @foo(%TestStruct.1* %Arr) {
entry:
  call void (i32, void (i8*, ...)*, ...) @broker(i32 3, void (i8*, ...)* bitcast (void (i8*, [1000 x %TestStruct.1]*)* @callback to void (i8*, ...)*), %TestStruct.1* %Arr)
  ret void
}

define void @bar() {
entry:
  call void @foo(%TestStruct.1* getelementptr inbounds ([1000 x %TestStruct.1], [1000 x %TestStruct.1]* bitcast ([1000 x %TestStruct]* @globArray to [1000 x %TestStruct.1]*), i64 0, i64 0))
  ret void
}

declare !callback !0 void @broker(i32, void (i8*, ...)*, ...)

!0 = !{!1}
!1 = !{i64 1, i64 -1, i1 true}

; Check that the GEP in @callback wasn't changed
; CHECK: %dummy = getelementptr [1000 x %TestStruct.1], [1000 x %TestStruct.1]* %Arr, i64 0, i64 0

; Check that the parameter in the call site for @callback wasn't updated
; CHECK: call void (i32, void (i8*, ...)*, ...) @broker(i32 3, void (i8*, ...)* bitcast (void (i8*, [1000 x %TestStruct.1]*)* @callback to void (i8*, ...)*), %TestStruct.1* %Arr)
