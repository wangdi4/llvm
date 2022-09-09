; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -whole-program-assume -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test verifies that a type that passes all the qualification
; conditions is accepted for transformation.
%struct.test01 = type { i32, i64, i32 }

; Pointer to the type being transformed.
@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer


define void @test01() {
  %p1 = call i8* @calloc(i64 4, i64 16)
  %p1_test = bitcast i8* %p1 to %struct.test01*

  store %struct.test01* %p1_test, %struct.test01** @g_test01ptr

  ; Set all elements of the structure for the first array element to 1.
  %f1 = getelementptr %struct.test01, %struct.test01* %p1_test, i64 0, i32 0
  store i32 1, i32* %f1
  %f2 = getelementptr %struct.test01, %struct.test01* %p1_test, i64 0, i32 1
  store i64 1, i64* %f2
  %f3 = getelementptr %struct.test01, %struct.test01* %p1_test, i64 0, i32 2
  store i32 1, i32* %f3

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
