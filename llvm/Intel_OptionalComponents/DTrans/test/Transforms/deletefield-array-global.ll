; RUN: opt < %s -whole-program-assume -passes="dtrans-deletefield" -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -S 2>&1 | FileCheck %s

; Check that all unused fields are deleted and GEPs are updated.
; Fields (A:0) and (A:2) should stay.

; CHECK-DAG: %__DFT_struct.A = type { i32, i32 }
; CHECK-DAG: getelementptr inbounds ([4 x %__DFT_struct.A], [4 x %__DFT_struct.A]* @a, i64 0, i64 0, i32 0)
; CHECK-DAG: getelementptr inbounds ([4 x %__DFT_struct.A], [4 x %__DFT_struct.A]* @a, i64 0, i64 3, i32 1)

%struct.A = type { i32, i32, i32 }

@a = global [4 x %struct.A] zeroinitializer

define i32 @bar() {
entry:
  %0 = load i32, i32* getelementptr inbounds ([4 x %struct.A], [4 x %struct.A]* @a, i64 0, i64 0, i32 0)
  %1 = load i32, i32* getelementptr inbounds ([4 x %struct.A], [4 x %struct.A]* @a, i64 0, i64 3, i32 2)
  %add = add nsw i32 %1, %0
  ret i32 %add
}

