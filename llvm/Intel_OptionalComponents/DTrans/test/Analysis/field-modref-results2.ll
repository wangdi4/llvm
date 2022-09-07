; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-usecrulecompat -dtrans-fieldmodref-analysis -dtrans-fieldmodref-eval -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-usecrulecompat -passes='require<dtrans-fieldmodref-analysis>' -dtrans-fieldmodref-eval -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Test mod/ref query results for a function call that makes an indirect
; function call based on a function address passed in a parameter.

%struct.test01 = type { i32, i32*, i64 }

define internal void @testbase(%struct.test01* %in) {
  %fieldaddr = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
  %ld1 = load i32, i32* %fieldaddr

  ; ModRef queries of this call site should identify the result as 'ModRef' because
  ; 'test01' will directly modify the field, and it will reach a function that is
  ; indirectly called that will reference the field.
  call void @test01()
  ret void
}

define internal void @test01() {
  %st_mem = call i8* @malloc(i64 24)
  %st = bitcast i8* %st_mem to %struct.test01*

  %f0 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  store i32 8, i32* %f0

  %ar1_mem = call i8* @malloc(i64 64)
  %ar1_mem2 = bitcast i8* %ar1_mem to i32*
  %f1 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 1
  store i32* %ar1_mem2, i32** %f1

  %f2 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 2
  store i64 1, i64* %f2

  ; Calling this function with the function address should result in mod/ref
  ; queries that include the behavior of the indirect function call. Note, the
  ; analysis is not context sensitive to the call site for pass-through
  ; functions, so the behavior of all functions that may be passed into use01a
  ; will be merged together to produce the result.
  call void @use01a(%struct.test01* %st, void (%struct.test01*)* @filter01a)
  ret void
}

define void @use01a(%struct.test01* %st, void (%struct.test01*)* nocapture %filter) {
  call void %filter(%struct.test01* %st)
  ret void
}

; Function that will be address taken because it is a pass-through to the
; callback function.
define void @filter01a(%struct.test01* %st) {
  %fieldaddr = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  %ld1 = load i32, i32* %fieldaddr
  ret void
}

declare i8* @malloc(i64)

; CHECK: FieldModRefQuery: - ModRef     : [testbase]   %ld1 = load i32, i32* %fieldaddr, align 4 --   call void @test01()
; CHECK: FieldModRefQuery: - Ref        : [test01]   store i32 8, i32* %f0, align 4 --   call void @use01a(%struct.test01* %st, void (%struct.test01*)* @filter01a)
