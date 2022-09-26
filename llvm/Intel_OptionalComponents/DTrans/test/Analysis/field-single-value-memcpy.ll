; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes="require<dtransanalysis>" -dtrans-print-types -disable-output < %s 2>&1 | FileCheck %s

; Verify that Single Value info is present even if there is a bad memfunc size
; or bad memfunc manipulation.

; In this test case multiple memcpy calls are made, each of them end up either
; in a bad memfunc size issue or bad memfunc manipulation issue. The goal it
; to check that Single Value analysis results for a nested %struct.Funcs will
; not be impacted.

; CHECK-LABEL: LLVMType: %struct.A = type
; CHECK: Safety data:
; CHECK-SAME: Bad memfunc manipulation

; CHECK-LABEL: LLVMType: %struct.B = type
; CHECK: Safety data:
; CHECK-SAME: Bad memfunc size
; CHECK-SAME: Bad memfunc manipulation

; CHECK-LABEL: LLVMType: %struct.Funcs = type
; CHECK: Single Value:
; CHECK-SAME: @foo
; CHECK: Single Value:
; CHECK-SAME: @bar

source_filename = "field-single-value-memcpy01.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.A = type { %struct.B, i32, %struct.B, [100 x i32], [100 x i32], %struct.Funcs }
%struct.B = type { i32, i32, i32 }
%struct.Funcs = type { i32 (i32)*, i32 (i32)* }
%struct.Another = type { i32, %struct.B }

define dso_local i32 @foo(i32 %x) {
entry:
  ret i32 0
}

define dso_local i32 @bar(i32 %x) {
entry:
  %add = add nsw i32 %x, 1
  ret i32 %add
}

define dso_local void @m(%struct.A* %a, %struct.A* %b, %struct.Another* %aa, i32* %arr) {
entry:
  ; Copy an array from one field to another within the same type
  %y = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 3
  %0 = bitcast [100 x i32]* %y to i8*
  %z = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 4
  %1 = bitcast [100 x i32]* %z to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 %0, i8* nonnull align 8 %1, i64 400, i1 false)

  ; Copy an array field to the memory pointed by %arr
  %2 = bitcast i32* %arr to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 %0, i8* align 4 %2, i64 200, i1 false)

  ; Copy a nested structure from one type to another
  %b2 = getelementptr inbounds %struct.Another, %struct.Another* %aa, i64 0, i32 1
  %3 = bitcast %struct.B* %b2 to i8*
  %4 = bitcast %struct.A* %a to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 4 %3, i8* align 8 %4, i64 12, i1 false)

  ; Copy a tuple of two %struct.B fields and one %struct.A field from one instance of struct.A to another
  %b5 = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 0, i32 1
  %5 = bitcast i32* %b5 to i8*
  %b7 = getelementptr inbounds %struct.A, %struct.A* %b, i64 0, i32 0, i32 1
  %6 = bitcast i32* %b7 to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 4 %5, i8* nonnull align 4 %6, i64 12, i1 false)

  ; Copy a tuple of one %struct.A fields and two %struct.B field from one instance of struct.A to another
  %b8 = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 1
  %7 = bitcast i32* %b8 to i8*
  %b9 = getelementptr inbounds %struct.A, %struct.A* %b, i64 0, i32 1
  %8 = bitcast i32* %b9 to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 4 %7, i8* nonnull align 4 %8, i64 12, i1 false)

  %f1 = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 5, i32 0
  store i32 (i32)* @foo, i32 (i32)** %f1, align 8
  %f2 = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 5, i32 1
  store i32 (i32)* @bar, i32 (i32)** %f2, align 8
  ret void
}

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1)

