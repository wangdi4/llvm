; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -whole-program-assume -dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -passes=dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is inhibited in the presence of
; an indirect call to an aligned memory allocation routine.

; CHECK: DTRANS Weak Align: inhibited -- May allocate alignment memory

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Address taken of new(unsigned int, align_val_t) routine.
@malloc_func = dso_local local_unnamed_addr global i8* (i64, i64)* @_ZnwmSt11align_val_t, align 8

define internal void @test01() {
  %fn_addr = load i8* (i64, i64)*, i8* (i64, i64)** @malloc_func, align 8
  %call = tail call i8* %fn_addr(i64 64, i64 16)
  ret void
}

define i32 @main() {
  call void @test01()
  ret i32 0
}

declare i8* @_ZnwmSt11align_val_t(i64, i64)
