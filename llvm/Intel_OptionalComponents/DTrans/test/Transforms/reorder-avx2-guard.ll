;  RUN: opt < %s -S -whole-program-assume -dtrans-reorderfields | FileCheck %s
;  RUN: opt < %s -S -whole-program-assume -passes=dtrans-reorderfields | FileCheck %s

; Verify that the DTrans reorder fields transformation does not occur
; when not using Intel AVX2.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }
; CHECK: %struct.test = type { i32, i64, i32, i32, i16, i64, i64 }
; CHECK-NOT: %__DFR_struct.test = type { i64, i64, i64, i32, i32, i32, i16 }

define i32 @main() {
entry:
  %call = tail call noalias i8* @calloc(i64 10, i64 48)
  %0 = bitcast i8* %call to %struct.test*
  %i = getelementptr %struct.test, %struct.test* %0, i64 0, i32 0
  store i32 10, i32* %i, align 8
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
