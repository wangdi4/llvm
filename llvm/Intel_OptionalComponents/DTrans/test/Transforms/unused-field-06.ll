; RUN: opt < %s -whole-program-assume -passes="dtrans-deletefield" -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -S 2>&1 | FileCheck %s

; Check that the first field will not be deleted as the value is escaping via @f call.
; The second (i8) field will be deleted.

; CHECK: %__DFT_struct.A = type { i32 }

%struct.A = type { i32, i8 }

declare i32 @f(i32)

define void @test(%struct.A* %a) {
entry:
  %x = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 0
  %val = load i32, i32* %x, align 4
  %inc = call i32 @f(i32 %val)
  store i32 %inc, i32* %x, align 4
  ret void
}

