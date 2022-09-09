; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt < %s -whole-program-assume -passes="dtrans-deletefield" -debug-only=dtrans-deletefield -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -debug-only=dtrans-deletefield -S 2>&1 | FileCheck %s

; This test verifies that the DTrans delete-fields optimization removes the entry 0 of
; @glob_A since it isn't used, and the GEP for %x was updated correctly. The structure
; %struct.B won't be affected since it doesn't qualify for the optimization.

; This is the same test as deletefield42.ll but it checks the prints from debug-only.

; Check that %struct.B wasn't modified but the field 0 in %struct.A was deleted
; CHECK-DAG: LLVM Type: %struct.B
; CHECK-DAG: LLVM Type: %struct.A
; CHECK-DAG:  Can delete field: %struct.A @ 0
; CHECK-DAG:  Selected for deletion: %struct.A
; CHECK-DAG: struct.A[0] = DELETED
; CHECK-DAG: struct.A[1] = 0
; CHECK-DAG: struct.A[2] = 1
; CHECK-DAG: Delete field: New structure body: %__DFT_struct.A = type { %struct.B, i32 }

; Check that the GEPs were replaced correctly
; CHECK: Delete field: Replacing instruction
; CHECK-NEXT:  %x = getelementptr inbounds %__DFT_struct.A, %__DFT_struct.A* @glob_A, i64 0, i32 1, i32 0
; CHECK-NEXT:    with
; CHECK-NEXT:  %x = getelementptr inbounds %__DFT_struct.A, %__DFT_struct.A* @glob_A, i64 0, i32 0, i32 0

; CHECK: Delete field: Replacing instruction
; CHECK:  %y = getelementptr inbounds %__DFT_struct.A, %__DFT_struct.A* @glob_A, i64 0, i32 2
; CHECK-NEXT:    with
; CHECK-NEXT:  %y = getelementptr inbounds %__DFT_struct.A, %__DFT_struct.A* @glob_A, i64 0, i32 1

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