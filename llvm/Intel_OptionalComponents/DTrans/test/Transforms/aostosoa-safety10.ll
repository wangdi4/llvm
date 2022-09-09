; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -disable-output -dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt < %s -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -disable-output -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

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


declare i8* @calloc(i64, i64)

