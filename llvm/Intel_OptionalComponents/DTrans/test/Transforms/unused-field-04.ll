; RUN: opt < %s -whole-program-assume -passes="dtrans-deletefield" -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -S 2>&1 | FileCheck %s

; Check that the first field will be deleted as the value is not escaping.
; The second (i8) field will not be deleted.

; CHECK: %__DFT_struct.A = type { i8 }

%struct.A = type { i32, i8 }

define i8 @test(%struct.A* %a) {
entry:
  %x = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 0
  %val = load i32, i32* %x, align 4
  %inc = add nsw i32 %val, 1
  br i1 undef, label %store_val, label %exit

store_val:
  store i32 %inc, i32* %x, align 4
  br label %exit

exit:
  %s = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 1
  %sval = load i8, i8* %s
  ret i8 %sval
}
