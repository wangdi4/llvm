; RUN: opt < %s -whole-program-assume -passes="dtrans-deletefield" -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -S 2>&1 | FileCheck %s

; Check that the first field will be deleted as the value does not escape.
; The second (i8) field will not be deleted.

; CHECK: %__DFT_struct.A = type { i8 }

%struct.A = type { i32, i8 }

define i8 @test(%struct.A* %a, i32* %p) {
entry:
  %x = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 0
  %val = load i32, i32* %x, align 4
  %val64 = zext i32 %val to i64
  %y = getelementptr inbounds i32, i32* %p, i64 %val64
  %val2 = load i32, i32* %y, align 4
  store i32 %val2, i32* %x, align 4

  %s = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 1
  %sval = load i8, i8* %s
  ret i8 %sval
}
