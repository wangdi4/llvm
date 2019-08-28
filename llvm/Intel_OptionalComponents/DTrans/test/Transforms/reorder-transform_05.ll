; This test verifies that Field-reordering transformation applied
; correctly to struct that has a field of another type that is being
; transformed by field-reorder.

;  RUN: opt  -whole-program-assume < %s -S -dtrans-reorderfields | FileCheck %s
;  RUN: opt  -whole-program-assume < %s -S -passes=dtrans-reorderfields | FileCheck %s

; CHECK-DAG: %__DFR_struct.test01 = type { i64, i64, %__DFR_struct.test02*, i32, i32, i32, i16 }
; CHECK-DAG: %__DFR_struct.test02 = type { i64, %__DFR_struct.test01*, i64, i32, i32, i32, i16 }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i32, i64, i32, i32, i16, i64, %struct.test02* }
%struct.test02 = type { i32, i64, i32, i32, i16, %struct.test01*, i64 }

define i32 @main() {
entry:
  %call = tail call noalias i8* @calloc(i64 10, i64 48)
  %0 = bitcast i8* %call to %struct.test01*
  %i = bitcast i8* %call to i32*
  store i32 10, i32* %i, align 8
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %1 = bitcast i8* %call1 to %struct.test02*
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
