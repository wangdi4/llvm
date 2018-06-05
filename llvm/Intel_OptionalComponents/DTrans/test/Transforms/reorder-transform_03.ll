; This test verifies that Field-reordering transformation applied
; correctly to sdiv and malloc instructions related to %struct.test.

;  RUN: opt < %s -S -dtrans-reorderfields | FileCheck %s
;  RUN: opt < %s -S -passes=dtrans-reorderfields | FileCheck %s

; CHECK-DAG: malloc(i64 400)
; CHECK-NOT: malloc(i64 480)
; CHECK-DAG: sdiv i64 %diff, 40
; CHECK-NOT: sdiv i64 %diff, 48

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define void @foo(%struct.test* %tp, %struct.test* %tp2) {
entry:
  %p1 = ptrtoint %struct.test* %tp2 to i64
  %p2 = ptrtoint %struct.test* %tp to i64
  %diff = sub i64  %p2, %p1
  %num = sdiv i64 %diff, 48
  ret void
}

define i32 @main() {
entry:
  %call = tail call noalias i8* @calloc(i64 10, i64 48)
  %0 = bitcast i8* %call to %struct.test*
  %i = bitcast i8* %call to i32*
  store i32 10, i32* %i, align 8
  tail call void @foo(%struct.test* %0, %struct.test* %0)
  %call2 = tail call i8* @malloc(i64 480)
  %1 = bitcast i8* %call2 to %struct.test*
  tail call void @foo(%struct.test* %1, %struct.test* %0)
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
declare dso_local noalias i8* @malloc(i64)
