; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -passes="dtrans-deletefield" -S 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that all unused fields are deleted and GEPs are updated.
; Fields (A:0) and (A:2) should stay.

; CHECK-DAG: %__DFT_struct.A = type { i32, i32 }
; CHECK-DAG: getelementptr inbounds [4 x %__DFT_struct.A], [4 x %__DFT_struct.A]* %a, i64 0, i64 0, i32 0
; CHECK-DAG: getelementptr inbounds [4 x %__DFT_struct.A], [4 x %__DFT_struct.A]* %a, i64 0, i64 3, i32 1

%struct.A = type { i32, i32, i32 }

define i32 @bar([4 x %struct.A]* %a) {
entry:
  %x = getelementptr inbounds [4 x %struct.A], [4 x %struct.A]* %a, i64 0, i64 0, i32 0
  %0 = load i32, i32* %x, align 4
  %z = getelementptr inbounds [4 x %struct.A], [4 x %struct.A]* %a, i64 0, i64 3, i32 2
  %1 = load i32, i32* %z, align 4
  %add = add nsw i32 %1, %0
  ret i32 %add
}

