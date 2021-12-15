; REQUIRES: asserts
; RUN: opt  -whole-program-assume < %s -disable-output -dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt  -whole-program-assume < %s -disable-output -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test verifies that a type within an array is disqualified
; from being transformed.
%struct.test01 = type { i32, i64, i32 }
%struct.test01a = type [4 x %struct.test01]
define void @test01() {
  %p1 = call i8* @calloc(i64 1, i64 64)
  %p1_test = bitcast i8* %p1 to %struct.test01a*
  ret void
}

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01()
  ret i32 0
}
; CHECK: DTRANS-AOSTOSOA: Rejecting -- Array of type seen: struct.test01

declare i8* @calloc(i64, i64)

