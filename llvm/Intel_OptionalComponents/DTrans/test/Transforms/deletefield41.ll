; This test checks that the delete fields transformation isn't applied since
; the field 2 (type %struct.C) in %struct.A is address taken and it produces
; a safety issue.

; RUN: opt < %s -whole-program-assume -passes="dtrans-deletefield" -debug-only=dtrans-deletefield -dtrans-outofboundsok=false -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -debug-only=dtrans-deletefield -dtrans-outofboundsok=false -S 2>&1 | FileCheck %s

; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; Check that the optimization was rejected in all structures and
; that the type %struct.B won't be modified since the enclosing
; type didn't pass the safety checks.
; CHECK-DAG: LLVM Type: %struct.C
; CHECK-DAG:  Can delete field: %struct.C @ 0
; CHECK-DAG:  Rejecting %struct.C based on safety data.
; CHECK-DAG: LLVM Type: %struct.B
; CHECK-DAG:  Can delete field: %struct.B @ 0
; CHECK-DAG:  Can delete field: %struct.B @ 2
; CHECK-DAG: LLVM Type: %struct.A
; CHECK-DAG:  Can delete field: %struct.A @ 0
; CHECK-DAG:  Rejecting %struct.A based on safety data.
; CHECK-DAG: Rejecting %struct.B based on safety data of enclosing type %struct.A
; CHECK-DAG:  No candidates found.

; Confirm that the structures weren't modified:
; CHECK: %struct.A = type { i32, %struct.B, %struct.C }
; CHECK: %struct.B = type { i8, i16, i32 }
; CHECK: %struct.C = type { i32 }

; Check that the structure %struct.B wasn't modified
; CHECK-NOT: %__DFT_struct.B = type { i16 }

%struct.A = type { i32, %struct.B, %struct.C }
%struct.B = type { i8, i16, i32 }
%struct.C = type { i32 }

define void @foo(%struct.A* %a) {
entry:
  %0 = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 2
  call void @bas(%struct.C* %0)
  ret void
}

define i16 @bar(%struct.B* %b) {
entry:
  %y = getelementptr inbounds %struct.B, %struct.B* %b, i64 0, i32 1
  %0 = load i16, i16* %y, align 4
  ret i16 %0
}

declare void @bas(%struct.C* %c)

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
