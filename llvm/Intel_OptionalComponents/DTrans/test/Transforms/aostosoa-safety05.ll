; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that a type that is allocated with a user-alloc function is
; disqualifed from being transformed.
%struct.test01 = type { i32, i64, i32 }

; Test with user alloc-like wrapper
define i8* @test01alloc(i64 %size) {
  %call = tail call noalias i8* @malloc(i64 %size)
  ret i8* %call
}

define void @test01() {
  %p1 = call i8* @test01alloc(i64 16)
  %p1_test = bitcast i8* %p1 to %struct.test01*
  ret void
}

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01()
  ret i32 0
}

; CHECK: DTRANS-AOSTOSOA: Rejecting -- Unsupported allocation function: struct.test01

declare i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
