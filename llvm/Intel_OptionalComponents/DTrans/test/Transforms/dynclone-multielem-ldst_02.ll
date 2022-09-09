; UNSUPPORTED: enable-opaque-pointers

; This test verifies that DynClone is not triggered when analysis
; is not able to detect all fields involved in MultiElem load in
; proc1.

; REQUIRES: asserts
;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dynclone -debug-only=dtrans-dynclone 2>&1 | FileCheck %s
;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dynclone -debug-only=dtrans-dynclone 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: Verification of Multi field Load/Stores Failed
; CHECK-NOT: store i8 1, i8* @__Shrink__Happened__

; 1 and 6 fields are shrunk to i16. 2nd and 3rd fields are
; marked as aostosoa index fields.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

; Type of AOSTOSOA variable.
%struct.ns = type { i64*, %struct.test.01**, %struct.test.01**, i64* }

; Memory allocation pointers, which are returned by calloc in "init" routine,
; are stored in 2nd and 3rd array fields of @n.
@n = internal global %struct.ns zeroinitializer, align 8

; AOSTOSOA allocation call is marked with this annotation.
@__intel_dtrans_aostosoa_alloc = private constant [38 x i8] c"{dtrans} AOS-to-SOA allocation {id:0}\00"

; AOSTOSOA allocation call is marked with this annotation.
@__intel_dtrans_aostosoa_index = private constant [41 x i8] c"{dtrans} AOS-to-SOA peeling index {id:0}\00"

; %L involves with multiple fields of %struct.test.01 ( 2nd and 3rd).
; Both 2nd  and 3rd fields (%A2 and %A3) are marked as aostosoa index fields.
; But, DynClone not be triggered since it can't detect all fields involved
; in %L currently due to unexpected bitcast (%F3).
define void @proc1() {
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F2 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 2
  %A2 = call i32* @llvm.ptr.annotation.p0i32(i32* %F2, i8* getelementptr inbounds ([41 x i8], [41 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* null, i32 0, i8* null)
  %B3 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 3
  %F3 = bitcast i32* %B3 to i32*
  %A3 = call i32* @llvm.ptr.annotation.p0i32(i32* %F3, i8* getelementptr inbounds ([41 x i8], [41 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* null, i32 0, i8* null)
  %M = select i1 undef, i32* %F2, i32* %F3
  %L = load i32, i32* %M
  %I6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  ret void
}

; This routine is selected as InitRoutine.
define %struct.test.01* @init() {
  %call0 = call noalias i8* @calloc(i64 1000, i64 32)
  %call.ptr = call i8* @llvm.ptr.annotation.p0i8(i8* %call0, i8* getelementptr inbounds ([38 x i8], [38 x i8]* @__intel_dtrans_aostosoa_alloc, i32 0, i32 0), i8* null, i32 0, i8* null)
  %C01 = getelementptr i8, i8* %call0, i64 0
  %C02 = bitcast i8* %C01 to i64*
  store i64* %C02, i64** getelementptr (%struct.ns, %struct.ns* @n, i64 0, i32 0)
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %tp2 = getelementptr %struct.test.01, %struct.test.01* %tp1, i64 2

; Below instructions are needed to select this routine as InitRoutine and
; increase field frequency of struct.test.01.
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8

  ret %struct.test.01* null
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  %tp2 = call %struct.test.01* @init();
  call void @proc1();
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
declare dso_local noalias i8* @llvm.ptr.annotation.p0i8(i8*, i8*, i8*, i32, i8*)
declare dso_local noalias i32* @llvm.ptr.annotation.p0i32(i32*, i8*, i8*, i32, i8*)
