; RUN: opt < %s -whole-program-assume -passes="dtrans-deletefield" -debug-only=dtrans-deletefield -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -debug-only=dtrans-deletefield -S 2>&1 | FileCheck %s

; REQUIRES: asserts

; Check that no fields will be deleted from %struct.B because of a
; safety violation (FieldAddressTaken) in the dependent type %struct.A.

; CHECK: Rejecting %struct.B based on safety data of enclosing type %struct.A

; CHECK-DAG: %struct.A = type { i32, %struct.B, i32 }
; CHECK-DAG: %struct.B = type { i8, i16, i32 }

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
