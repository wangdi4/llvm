; This test case checks that the interprocedural sparse conditional constant
; propagation passes when the beginning of an array is casted to a pointer
; and it is used inside a callback.

; RUN: opt -ipsccp -S %s | FileCheck %s
; RUN: opt -passes=ipsccp -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%TestStruct = type { [16 x i8], [16 x i8], i32 }
@globArray = internal global [1000 x %TestStruct] zeroinitializer

define internal void @callback(i8* %ID, [1000 x %TestStruct]* %Arr) {
entry:
  %dummy = getelementptr [1000 x %TestStruct], [1000 x %TestStruct]* %Arr, i64 0, i64 0
  ret void
}

define internal void @foo(%TestStruct* %Arr) {
entry:
  call void (i32, void (i8*, ...)*, ...) @broker(i32 3, void (i8*, ...)* bitcast (void (i8*, [1000 x %TestStruct]*)* @callback to void (i8*, ...)*), %TestStruct* %Arr)
  ret void
}

define void @bar() {
entry:
  call void @foo(%TestStruct* getelementptr inbounds ([1000 x %TestStruct], [1000 x %TestStruct]* bitcast ([1000 x %TestStruct]* @globArray to [1000 x %TestStruct]*), i64 0, i64 0))
  ret void
}

declare !callback !0 void @broker(i32, void (i8*, ...)*, ...)

!0 = !{!1}
!1 = !{i64 1, i64 -1, i1 true}

; Check that @globArray was casted correctly and %Arr was replaced with
; %bc_const in the GEP %dummy
; CHECK: define internal void @callback(i8* %ID, [1000 x %TestStruct]* %Arr) {
; CHECK: %bc_const = bitcast %TestStruct* getelementptr inbounds ([1000 x %TestStruct], [1000 x %TestStruct]* @globArray, i64 0, i64 0) to [1000 x %TestStruct]*
; CHECK: %dummy = getelementptr [1000 x %TestStruct], [1000 x %TestStruct]* %bc_const, i64 0, i64 0

; Check that the parameter in the call site for @callback was updated with the
; correct type
; CHECK: call void (i32, void (i8*, ...)*, ...) @broker(i32 3, void (i8*, ...)* bitcast (void (i8*, [1000 x %TestStruct]*)* @callback to void (i8*, ...)*), %TestStruct* getelementptr inbounds ([1000 x %TestStruct], [1000 x %TestStruct]* @globArray, i64 0, i64 0))
