; RUN: opt < %s -whole-program-assume -passes="dtrans-deletefield" -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -S 2>&1 | FileCheck %s

; Check that the first field will not be deleted as the value escapes via store to %p.
; The second (i8) field will not be deleted.

; CHECK: %__DFT_struct.A = type { i32 }

%struct.A = type { i32, i8 }

define void @test(%struct.A* %a, i32* %p) {
entry:
  %x = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 0
  %val = load i32, i32* %x, align 4
  store i32 %val, i32* %p, align 4
  ret void
}

