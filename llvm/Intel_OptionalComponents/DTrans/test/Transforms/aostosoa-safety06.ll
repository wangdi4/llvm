; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test verifies that a type allocated in a function called from
; multiple locations is disqualified from being transformed
%struct.test01 = type { i32, i64, i32 }
define void @test01() {
  %p1 = call i8* @calloc(i64 4, i64 16)
  %p1_test = bitcast i8* %p1 to %struct.test01*
  ret void
}

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01()
  call void @test01()
  ret i32 0
}

; CHECK: DTRANS-AOSTOSOA: Rejecting -- Multiple call paths: struct.test01


declare i8* @calloc(i64, i64)
declare void @free(i8*)
