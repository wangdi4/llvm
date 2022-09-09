; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the DTrans delete field pass does not incorrectly
; transform IR that directly accesses the first structure field using a bitcast
; of the structure pointer.

; This structure can be transformed by the delete field transformation, however
; the first field should not be removed because delete field does not currently
; support rewriting the instruction that stores directly to that field using a
; pointer to the structure. The last field can be safety removed.
%struct.test01 = type { i32*, i32, i32, i64 }
; CHECK: %__DFT_struct.test01 = type { i32*, i32, i32 }

define void @test01(%struct.test01* %p)
{
  %p1 = getelementptr %struct.test01, %struct.test01* %p, i64 0, i32 1
  %p2 = getelementptr %struct.test01, %struct.test01* %p, i64 0, i32 2
  %z = load i32, i32* %p2
  %y = load i32, i32* %p1
  store i32 %y, i32* %p2
  store i32 %z, i32* %p1

  ; The following access to the structure field is not supported by the delete
  ; field transformation, and should prevent removal of the first field even
  ; though there are no load instructions for the field.
  %p0 = bitcast %struct.test01* %p to i32**
  store i32* null, i32** %p0
  ret void
}
; CHECK: %p1 = getelementptr %__DFT_struct.test01, %__DFT_struct.test01* %p, i64 0, i32 1
; CHECK: %p2 = getelementptr %__DFT_struct.test01, %__DFT_struct.test01* %p, i64 0, i32 2
