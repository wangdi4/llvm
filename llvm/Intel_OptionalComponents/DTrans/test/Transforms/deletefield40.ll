; This test checks that the delete fields transformation happens with enclosed
; structures. For this case, the fields 0 and 2 in %struct.B will be deleted
; even if it is enclosed in %struct.A.

; RUN: opt < %s -whole-program-assume -passes="internalize,dtrans-deletefield" -debug-only=dtrans-deletefield -dtrans-outofboundsok=false -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -internalize -dtrans-deletefield -debug-only=dtrans-deletefield -dtrans-outofboundsok=false -S 2>&1 | FileCheck %s

; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; CHECK-NOT: Rejecting %struct.B based on safety data of enclosing type %struct.A

; Check that the fields 0 and 2 in %struct.B were identified
; CHECK-DAG: LLVM Type: %struct.B
; CHECK-DAG: Can delete field: %struct.B @ 0
; CHECK-DAG: Can delete field: %struct.B @ 2
; CHECK-DAG: Selected for deletion: %struct.B

; Confirm that the fields were deleted and field 1 was updated
; CHECK: struct.B[0] = DELETED
; CHECK-NEXT: struct.B[1] = 0
; CHECK-NEXT: struct.B[2] = DELETED

; Find the calls related to %struct.B
; CHECK: Found call involving type with deleted fields:
; CHECK-NEXT: call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 16, i1 false)
; CHECK-NEXT: %struct.A = type { i32, %struct.B, i32 }

; Confirm that the instruction related to %struct.B was replaced
; CHECK: Delete field: Replacing instruction
; CHECK-NEXT: %y = getelementptr inbounds %__DFT_struct.B, %__DFT_struct.B* %b, i64 0, i32 1
; CHECK-NEXT: with
; CHECK-NEXT: %y = getelementptr inbounds %__DFT_struct.B, %__DFT_struct.B* %b, i64 0, i32 0

; Make sure that %struct.A and %struct.B were updated
; CHECK-NOT: %struct.A = type { i32, %struct.B, i32 }
; CHECK-NOT: %struct.B = type { i8, i16, i32 }
; CHECK-DAG: %__DFDT_struct.A = type { i32, %__DFT_struct.B, i32 }
; CHECK-DAG: %__DFT_struct.B = type { i16 }

; Confirm that the instructions related to %struct.A were updated
; CHECK: define internal i32 @foo.1(%__DFDT_struct.A* %a, i32** %q) {
; CHECK: %x = getelementptr inbounds %__DFDT_struct.A, %__DFDT_struct.A* %a, i64 0, i32 0
; CHECK: %y = getelementptr inbounds %__DFDT_struct.A, %__DFDT_struct.A* %a, i64 0, i32 2

%struct.A = type { i32, %struct.B, i32 }
%struct.B = type { i8, i16, i32 }

define i32 @foo(%struct.A* %a, i32** %q) {
entry:
  %p = bitcast %struct.A* %a to i8*
  call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 16, i1 false)

  %x = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 0
  %y = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 2
  %0 = load i32, i32* %x, align 4
  %1 = load i32, i32* %y, align 4
  %2 = add i32 %0, %1

  store i32* %x, i32** %q

  ret i32 %2
}

define i16 @bar(%struct.B* %b) {
entry:
  %y = getelementptr inbounds %struct.B, %struct.B* %b, i64 0, i32 1
  %0 = load i16, i16* %y, align 4
  ret i16 %0
}

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
