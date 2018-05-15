; REQUIRES: asserts
; RUN: opt < %s -disable-output -dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test verifies that a type that passes all the qualification
; conditions is accepted for transformation.
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

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01wrapper()
  ret i32 0
}

; CHECK: DTRANS-AOSTOSOA: Passed qualification tests: struct.test01


declare i8* @calloc(i64, i64)

