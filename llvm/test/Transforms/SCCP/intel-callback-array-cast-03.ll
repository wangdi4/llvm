; This test case checks that the interprocedural sparse conditional constant
; propagation passes when a pointer is casted to an array and it is used inside
; a callback.

; RUN: opt -ipsccp -S %s | FileCheck %s
; RUN: opt -passes=ipsccp -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%TestStruct = type { [16 x double], [16 x double], i32 }
@globVar = internal global %TestStruct zeroinitializer

define internal void @callback(i8* %ID, [16 x double]* %Arr) {
entry:
  %dummy = getelementptr [16 x double], [16 x double]* %Arr, i64 0, i64 0
  ret void
}

define internal void @foo(double* %Arr) {
entry:
  call void (i32, void (i8*, ...)*, ...) @broker(i32 3, void (i8*, ...)* bitcast (void (i8*, [16 x double]*)* @callback to void (i8*, ...)*), double* %Arr)
  ret void
}

define void @bar() {
entry:
  %tmp = getelementptr inbounds %TestStruct, %TestStruct* @globVar, i64 0, i32 1, i64 0
  call void @foo(double* %tmp)
  ret void
}

declare !callback !0 void @broker(i32, void (i8*, ...)*, ...)

!0 = !{!1}
!1 = !{i64 1, i64 -1, i1 true}

; Check that casting was done correctly
; CHECK: define internal void @callback(i8* %ID, [16 x double]* %Arr) {
; CHECK: %bc_const = bitcast double* getelementptr inbounds (%TestStruct, %TestStruct* @globVar, i64 0, i32 1, i64 0) to [16 x double]*
; CHECK: %dummy = getelementptr [16 x double], [16 x double]* %bc_const, i64 0, i64 0

; Check that the parameter in the call site for @callback was updated with the
; correct type
; CHECK: call void (i32, void (i8*, ...)*, ...) @broker(i32 3, void (i8*, ...)* bitcast (void (i8*, [16 x double]*)* @callback to void (i8*, ...)*), double* getelementptr inbounds (%TestStruct, %TestStruct* @globVar, i64 0, i32 1, i64 0))
