; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes=dtrans-deletefield -S 2>&1 | FileCheck %s

; This test checks that the DTrans delete field pass handles updating
; global variable initializers when fields are deleted from both the
; nested structure and the type that contains it when there is a non-zero
; initializer.

; The 'i16' field should be deleted from both structures.
%struct.test01t = type { i32, i16 }
%struct.test01 = type { i32, i16, %struct.test01t }

@glob = internal global %struct.test01 { i32 0, i16 1, %struct.test01t { i32 2, i16 3 } }
; CHECK: @glob = internal global %__DFT_struct.test01 { i32 0, %__DFT_struct.test01t { i32 2 } }

define i32 @test01(%struct.test01* %s1) {
  %pA = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 0
  %pC0 = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 2, i32 0

  %vA = load i32, i32* %pA
  %vC0 = load i32, i32* %pC0

  %res = add i32 %vC0, %vA
  ret i32 %res
}
