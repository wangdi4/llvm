; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -whole-program-assume -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-aostosoa -dtrans-ignore-bfi=true -dtrans-aostosoa-frequency-threshold=20 -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=dtrans-aostosoa -dtrans-ignore-bfi=true -dtrans-aostosoa-frequency-threshold=20 -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test verifies that a type that passes all the safety conditions will
; not be selected for being transformed when it does not pass the hotness
; threshold. And verifies that a type that passes the safety conditions and
; meets the threshold will be selected.

; This type will pass the safety checks, but have a frequency that is too low
; to qualify.
%struct.test01 = type { i32, i64, i32 }

; This type will pass the safety checks, and have a high enough threshold to
; qualify.
%struct.test02 = type { i64, i64, i64 }

@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer
@g_test02ptr = internal unnamed_addr global %struct.test02* zeroinitializer

;Create struct.test01, and generate a low frequency count for it.
define void @test01() {
  %p1 = call i8* @calloc(i64 4, i64 16)
  %p1_test = bitcast i8* %p1 to %struct.test01*
  store %struct.test01* %p1_test, %struct.test01** @g_test01ptr

  ; Only have one access to this structure to keep the frequency low.
  %f1 = getelementptr %struct.test01, %struct.test01* %p1_test, i64 0, i32 0
  store i32 1, i32* %f1
  ret void
}

; Create and use struct.test02 to generate a high frequency count on it.
define void @test02() {
  %p1 = call i8* @calloc(i64 4, i64 24)
  %p1_test = bitcast i8* %p1 to %struct.test02*
  store %struct.test02* %p1_test, %struct.test02** @g_test02ptr

  %f0_0 = getelementptr %struct.test02, %struct.test02* %p1_test, i64 0, i32 0
  store i64 1, i64* %f0_0
  %f0_1 = getelementptr %struct.test02, %struct.test02* %p1_test, i64 0, i32 1
  store i64 1, i64* %f0_1
  %f0_2 = getelementptr %struct.test02, %struct.test02* %p1_test, i64 0, i32 2
  store i64 1, i64* %f0_2

  %f1_0 = getelementptr %struct.test02, %struct.test02* %p1_test, i64 1, i32 0
  %val0 = load i64, i64* %f0_0
  store i64 %val0, i64* %f0_1
  %f1_1 = getelementptr %struct.test02, %struct.test02* %p1_test, i64 1, i32 1
  %val1 = load i64, i64* %f0_1
  store i64 %val1, i64* %f0_1
  %f1_2 = getelementptr %struct.test02, %struct.test02* %p1_test, i64 1, i32 2
  %val2 = load i64, i64* %f0_2
  store i64 %val2, i64* %f0_2

  ret void
}

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01()
  call void @test02()
  ret i32 0
}

; CHECK-DAG: DTRANS-AOSTOSOA: Rejecting -- Does not meet hotness threshold: struct.test01
; CHECK-DAG: DTRANS-AOSTOSOA: Passed qualification tests: struct.test02

declare i8* @calloc(i64, i64)
