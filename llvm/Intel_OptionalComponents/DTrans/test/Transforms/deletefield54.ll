; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes=dtrans-deletefield -S 2>&1 | FileCheck %s

; This test checks that the DTrans delete field pass handles updating
; global variable initializers when a field is being deleted from a type
; contained within another type when there is a non-zero initializer.

; The 'i16' field should be deleted from the nested structure.
%struct.test01t = type { i32, i16 }
%struct.test01 = type { i32, i32, %struct.test01t }

@glob = internal global %struct.test01 { i32 0, i32 1, %struct.test01t { i32 2, i16 3 } }
; CHECK: @glob = internal global %__DFDT_struct.test01 { i32 0, i32 1, %__DFT_struct.test01t { i32 2 } }

define i32 @test01(%struct.test01* %s1) {
  %pA = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 0
  %pB = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 1
  %pC0 = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 2, i32 0

  %vA = load i32, i32* %pA
  %vB = load i32, i32* %pB
  %vC0 = load i32, i32* %pC0

  %tmp = add i32 %vC0, %vB
  %res = add i32 %tmp, %vA
  ret i32 %res
}
