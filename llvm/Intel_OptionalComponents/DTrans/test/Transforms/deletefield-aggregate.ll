; RUN: opt < %s -whole-program-assume -passes="dtrans-deletefield" -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -S 2>&1 | FileCheck %s

; Check that all unused fields are deleted and GEPs are updated.
; Fields (A:1:1) and (B:1) should stay.

; CHECK-DAG: %__DFT_struct.A = type { %__DFT_struct.B }
; CHECK-DAG: %__DFT_struct.B = type { i16 }

; CHECK-DAG: getelementptr inbounds %__DFT_struct.A, %__DFT_struct.A* %a, i64 0, i32 0, i32 0
; CHECK-DAG: getelementptr inbounds %__DFT_struct.B, %__DFT_struct.B* %b, i64 0, i32 0

%struct.A = type { i16, %struct.B, i32 }
%struct.B = type { i8, i16, i32 }

define i16 @foo(%struct.A* %a) {
entry:
  %y = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 1, i32 1
  %0 = load i16, i16* %y, align 4
  ret i16 %0
}

define i16 @bar(%struct.B* %b) {
entry:
  %y = getelementptr inbounds %struct.B, %struct.B* %b, i64 0, i32 1
  %0 = load i16, i16* %y, align 4
  ret i16 %0
}

