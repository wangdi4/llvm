; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes=dtrans-deletefield -S 2>&1 | FileCheck %s

; This test checks that the DTrans delete field pass handles updating
; global variable initializers when a field is being deleted from a type
; that contains a nested structure when there is a non-zero initializer.

; We expect the 'i16' field to be deleted from the outer structure.
%struct.test01t = type { i32, i32 }
%struct.test01 = type { i32, i16, %struct.test01t }

@glob = internal global %struct.test01 { i32 0, i16 1, %struct.test01t { i32 2, i32 3 } }
; CHECK: @glob = internal global %__DFT_struct.test01 { i32 0, %struct.test01t { i32 2, i32 3 } }

define i32 @test01(%struct.test01* %s1) {
  %pA = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 0
  %pC0 = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 2, i32 0
  %pC1 = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 2, i32 1

  %vA = load i32, i32* %pA
  %vC0 = load i32, i32* %pC0
  %vC1 = load i32, i32* %pC1

  %tmp = add i32 %vC0, %vC1
  %res = add i32 %tmp, %vA
  ret i32 %res
}
