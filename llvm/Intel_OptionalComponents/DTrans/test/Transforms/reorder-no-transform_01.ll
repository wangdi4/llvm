; This test verifies that Field-reordering transformation is not applied
; to instructions related to %struct.test2. BadPtrManipulation safety
; check is the reason why Field-reordering transformation is not triggered
; for %struct.test2. %struct.test1 doesn't violate any safety checks
; to trigger Field-reordering.

; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -S -dtrans-reorderfields | FileCheck %s
; RUN: opt  < %s -whole-program-assume -S -passes=dtrans-reorderfields | FileCheck %s

; CHECK-DAG: %__DFR_struct.test1 = type { i64, i64, i64, i32, i32, i32, i16 }
; CHECK-DAG: %struct.test2 = type { i32, i64, i32, i32, i16, i64, i64 }
; CHECK: %i = getelementptr inbounds %struct.test2, %struct.test2* %tp, i64 0, i32 0
; CHECK: %c = getelementptr inbounds %struct.test2, %struct.test2* %tp, i64 0, i32 1
; CHECK: %t = getelementptr inbounds %struct.test2, %struct.test2* %tp, i64 0, i32 2
; CHECK: %num = sdiv i64 %diff, 48
; CHECK: %call = tail call noalias i8* @calloc(i64 10, i64 48)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test1 = type { i32, i64, i32, i32, i16, i64, i64 }
%struct.test2 = type { i32, i64, i32, i32, i16, i64, i64 }

define void @foo(%struct.test2* %tp, %struct.test2* %tp2) {
entry:
  %i = getelementptr inbounds %struct.test2, %struct.test2* %tp, i64 0, i32 0
  %0 = load i32, i32* %i, align 8
  %add = add nsw i32 %0, 20
  %conv = sext i32 %add to i64
  %c = getelementptr inbounds %struct.test2, %struct.test2* %tp, i64 0, i32 1
  store i64 %conv, i64* %c, align 8
  %add2 = add i32 %0, 40
  %t = getelementptr inbounds %struct.test2, %struct.test2* %tp, i64 0, i32 2
  %p1 = ptrtoint %struct.test2* %tp2 to i64
  %p2 = ptrtoint %struct.test2* %tp to i64
  %diff = sub i64  %p2, %p1
  %num = sdiv i64 %diff, 48
  store i32 %add2, i32* %t, align 8
  ret void
}

define i32 @main() {
entry:
  %call = tail call noalias i8* @calloc(i64 10, i64 48)
  %0 = bitcast i8* %call to %struct.test2*
  tail call void @foo(%struct.test2* %0, %struct.test2* %0)
  %call2 = tail call i8* @malloc(i64 480)
  %1 = bitcast i8* %call2 to %struct.test2*
  %h = getelementptr inbounds i8, i8* %call, i64 20
  %h1 = bitcast i8* %h to %struct.test2*
  tail call void @foo1(%struct.test2* %h1)
  tail call void @foo(%struct.test2* %1, %struct.test2* %0)
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %2 = bitcast i8* %call1 to %struct.test1*
  %i = getelementptr %struct.test1, %struct.test1* %2, i64 0, i32 0
  store i32 10, i32* %i, align 8
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
declare dso_local noalias i8* @malloc(i64)
declare void @foo1(%struct.test2*)
