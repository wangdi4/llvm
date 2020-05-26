; RUN: opt -ipsccp -S %s | FileCheck %s
; RUN: opt -passes=ipsccp -S %s | FileCheck %s
;
; Check that thread dependent constant is not propagated to the callback from
; the broker function caller.
;
; __thread int Var;
;
; static void callback(void *ID, int *A, int *B) {
;   *A = *B;
; }
;
; __attribute__((callback(CB, -1, A, B)))
; void broker(void (*CB)(void*, int*, int*), int *A, int *B);
;
; void foo(int *A) {
;   broker(callback, A, &Var);
; }
;
; CHECK-LABEL: define internal void @callback
; CHECK-NOT:  %{{.+}} = load i32, i32* @Var

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@Var = thread_local global i32 0, align 4

define void @foo(i32* %A) {
entry:
  call void @broker(void (i8*, i32*, i32*)* @callback, i32* %A, i32* @Var)
  ret void
}

declare !callback !0 void @broker(void (i8*, i32*, i32*)*, i32*, i32*)

define internal void @callback(i8* %ID, i32* %A, i32* %B) {
entry:
  %0 = load i32, i32* %B, align 4
  store i32 %0, i32* %A, align 4
  ret void
}

!0 = !{!1}
!1 = !{i64 0, i64 -1, i64 1, i64 2, i1 false}
