; REQUIRES: asserts
; RUN: opt  -whole-program-assume < %s -disable-output -dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt  -whole-program-assume < %s -disable-output -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test verifies that a type that is allocated multiple times is a
; disqualifed from being transformed.
%struct.test01 = type { i32, i64, i32 }
define void @test01() {
  %p1 = call i8* @calloc(i64 4, i64 16)
  %p1_test = bitcast i8* %p1 to %struct.test01*

  ; Perform a second allocation of the type
  %p2 = call i8* @calloc(i64 2, i64 16)
  %p2_test = bitcast i8* %p2 to %struct.test01*
  ret void
}

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01()
  ret i32 0
}

; CHECK: DTRANS-AOSTOSOA: Rejecting -- Too many allocations: struct.test01

declare i8* @calloc(i64, i64)

