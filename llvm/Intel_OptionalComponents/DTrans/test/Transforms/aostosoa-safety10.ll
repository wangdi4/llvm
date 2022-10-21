; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -disable-output -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that if the allocation path cannot be
; uniquely traced back to 'main', the type is disqualified
; from being transformed.
%struct.test01 = type { i32, i64, i32 }
define void @test01() {
  %p1 = call i8* @calloc(i64 4, i64 16)
  %p1_test = bitcast i8* %p1 to %struct.test01*
  ret void
}

define void @test01wrapper() {
  call void @test01()
  ret void
}

define i32 @notmain(i32 %argc, i8** %argv) {
  call void @test01wrapper()
  ret i32 0
}

; CHECK: DTRANS-AOSTOSOA: Rejecting -- Multiple call paths: struct.test01

declare i8* @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }
