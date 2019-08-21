; This test verifies that Field-reordering transformation applied
; correctly to ByteFlattened GEP instructions related to %struct.test.

;  RUN: opt  -whole-program-assume < %s -S -dtrans-reorderfields | FileCheck %s
;  RUN: opt  -whole-program-assume < %s -S -passes=dtrans-reorderfields | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.01 = type { i32, i64, i32, i32, i16, i64, i64 }

; This struct is used to verify that no GEPs related to it are modified
; by reordering-fields.
%struct.test.02 = type { i32, i64 }

define i32 @main() {
entry:
  %call = tail call noalias i8* @calloc(i64 10, i64 48)
  %0 = bitcast i8* %call to %struct.test.01*
  %bp1 = bitcast %struct.test.01* %0 to i8*
; CHECK: %bp1 = bitcast %__DFR_struct.test.01* %0 to i8*

  %F2 = getelementptr i8, i8* %bp1, i64 8
; CHECK: %F2 = getelementptr i8, i8* %bp1, i64 0

  %F3 = getelementptr i8, i8* %bp1, i64 16
; CHECK: %F3 = getelementptr i8, i8* %bp1, i64 28

  %F4 = getelementptr i8, i8* %bp1, i64 20
; CHECK: %F4 = getelementptr i8, i8* %bp1, i64 32

  %F5 = getelementptr i8, i8* %bp1, i64 24
; CHECK: %F5 = getelementptr i8, i8* %bp1, i64 36

  %call1 = tail call noalias i8* @calloc(i64 20, i64 16)
  %tp5 = bitcast i8* %call1 to %struct.test.02*
  tail call void @foo(%struct.test.01* %0, %struct.test.02* %tp5)

; GEP shouldn't be modified
  %F9 = getelementptr i8, i8* %call1, i64 8
; CHECK: %F9 = getelementptr i8, i8* %call1, i64 8

  ret i32 0
}

define void @foo(%struct.test.01* %tp, %struct.test.02* %tp3) {
entry:
  %bp = bitcast %struct.test.01* %tp to i8*
; CHECK: %bp = bitcast %__DFR_struct.test.01* %tp to i8*

  %F1 = getelementptr i8, i8* %bp, i64 0
; CHECK: %F1 = getelementptr i8, i8* %bp, i64 24

  %F6 = getelementptr i8, i8* %bp, i64 32
; CHECK: %F6 = getelementptr i8, i8* %bp, i64 8

  %F7 = getelementptr i8, i8* %bp, i64 40
; CHECK: %F7 = getelementptr i8, i8* %bp, i64 16

  %bp3 = bitcast %struct.test.02* %tp3 to i8*

; GEP shouldn't be modified
  %F8 = getelementptr i8, i8* %bp3, i64 0
; CHECK: %F8 = getelementptr i8, i8* %bp3, i64 0

  ret void
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
