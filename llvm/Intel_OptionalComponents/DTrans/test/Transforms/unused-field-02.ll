; RUN: opt < %s -whole-program-assume -passes="dtrans-deletefield" -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -S 2>&1 | FileCheck %s

; Check that the first field will not be deleted as the value is escaping via %cmp
; and return instruction. The second (i8) field will be deleted.

; CHECK: %__DFT_struct.A = type { i32 }

%struct.A = type { i32, i8 }

define i1 @test(%struct.A* %a) {
entry:
  %x = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 0
  %val = load i32, i32* %x, align 4
  %inc = add nsw i32 %val, 1
  store i32 %inc, i32* %x, align 4
  %cmp = icmp uge i32 %val, 10
  ret i1 %cmp
}

