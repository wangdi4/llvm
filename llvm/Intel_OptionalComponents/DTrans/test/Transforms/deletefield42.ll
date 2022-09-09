; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -passes="dtrans-deletefield" -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -S 2>&1 | FileCheck %s

; This test verifies that the DTrans delete-fields optimization removes the entry 0 in
; the global @glob_A since it isn't used, and the GEP for %x was updated correctly.
; The structure %struct.B won't be affected since it doesn't qualify for the optimization.

; Check that %struct.B wasn't modified but the field 0 in %struct.A was deleted
; CHECK-DAG: %__DFT_struct.A = type { %struct.B, i32 }
; CHECK-DAG: %struct.B = type { i32 }

; Check that the GEP for %struct.A:1 was replaced correctly
; CHECK: %x = getelementptr inbounds %__DFT_struct.A, %__DFT_struct.A* @glob_A, i64 0, i32 0, i32 0

; Check that the GEP for %struct.A:2 was replaced correctly
; CHECK: %y = getelementptr inbounds %__DFT_struct.A, %__DFT_struct.A* @glob_A, i64 0, i32 1

%struct.A = type { i32, %struct.B, i32 }
%struct.B = type { i32 }

@glob_A = private global %struct.A zeroinitializer

define i32 @foo(%struct.B %test_B) {
entry:
  call void @init(%struct.B %test_B)

  %x = getelementptr inbounds %struct.A, %struct.A* @glob_A, i64 0, i32 1, i32 0
  %y = getelementptr inbounds %struct.A, %struct.A* @glob_A, i64 0, i32 2
  %0 = load i32, i32* %x, align 4
  %1 = load i32, i32* %y, align 4
  %2 = add i32 %0, %1

  ret i32 %2
}

define void @init(%struct.B %test_B) {
  store %struct.B %test_B, %struct.B* getelementptr (%struct.A,
                                                     %struct.A* @glob_A,
                                                     i64 0, i32 1)
  store i32 100, i32* getelementptr (%struct.A, %struct.A* @glob_A, i64 0, i32 0)
  store i32 1000, i32* getelementptr (%struct.A, %struct.A* @glob_A, i64 0, i32 2)

  ret void
}