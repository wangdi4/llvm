; This test verifies that DynClone is not triggered. It is unable to
; track uses of memory allocation because pointers (%tp1 and %tp4) stored
; in 2nd array field of @n are coming from two different memory allocations.

; REQUIRES: asserts
;  RUN: opt < %s -dtransop-allow-typed-pointers -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dyncloneop -debug-only=dtrans-dynclone 2>&1 | FileCheck %s
;  RUN: opt < %s -dtransop-allow-typed-pointers -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dyncloneop -debug-only=dtrans-dynclone 2>&1 | FileCheck %s

; CHECK: Track uses of AllocCalls Failed
; CHECK-NOT: store i8 1, i8* @__Shrink__Happened__

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

; Type of AOSTOSOA variable.
%struct.ns = type { i64*, %struct.test.01**, %struct.test.01**, i64* }

; Memory allocation pointers, which are returned by calloc in "init" routine,
; are stored in 2nd and 3rd array fields of @n.
@n = internal global %struct.ns zeroinitializer, align 8

; AOSTOSOA allocation call is marked with this annotation.
@__intel_dtrans_aostosoa_alloc = private constant [38 x i8] c"{dtrans} AOS-to-SOA allocation {id:0}\00"

; This routine is selected as InitRoutine.
; Uses of %call1 memory allocation pointer (%tp1 and %tp2) are stored in
; 2nd and 3rd array fields of @n.
; Uses of %call2 memory allocation pointer (%tp4) is stored in
; 2nd array fields of @n. We are unable track these pointers due it this.
define "intel_dtrans_func_index"="1" %struct.test.01* @init() !intel.dtrans.func.type !7 {
  %call0 = call i8* @calloc(i64 1000, i64 32)
  %call.ptr = call i8* @llvm.ptr.annotation.p0i8(i8* %call0, i8* getelementptr inbounds ([38 x i8], [38 x i8]* @__intel_dtrans_aostosoa_alloc, i32 0, i32 0), i8* null, i32 0, i8* null)
  %C01 = getelementptr i8, i8* %call0, i64 0
  %C02 = bitcast i8* %C01 to i64*
  store i64* %C02, i64** getelementptr (%struct.ns, %struct.ns* @n, i64 0, i32 0)
  %call1 = tail call i8* @calloc(i64 10, i64 48)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %tp2 = getelementptr %struct.test.01, %struct.test.01* %tp1, i64 2
  %tp3 = getelementptr %struct.test.01, %struct.test.01* %tp1, i64 4
  %call2 = tail call i8* @calloc(i64 20, i64 48)
  %tp4 = bitcast i8* %call2 to %struct.test.01*

; Store %tp1 in 2nd array field of @n
  %N7 = getelementptr %struct.ns, %struct.ns* @n, i64 0, i32 1
  %LN7 = load %struct.test.01**, %struct.test.01*** %N7
  %PN7 = getelementptr %struct.test.01*, %struct.test.01** %LN7, i64 1
  %BC1 = bitcast %struct.test.01** %PN7 to i64*
  %NLD1 = load i64, i64* %BC1
  store %struct.test.01* %tp1, %struct.test.01** %PN7

; Store %tp2 in 3rd array field of @n
  %N8 = getelementptr %struct.ns, %struct.ns* @n, i64 0, i32 2
  %LN8 = load %struct.test.01**, %struct.test.01*** %N8
  %PN8 = getelementptr %struct.test.01*, %struct.test.01** %LN8, i64 1
  %BC2 = bitcast %struct.test.01** %PN8 to i64*
  %NLD2 = load i64, i64* %BC2
  store %struct.test.01* %tp2, %struct.test.01** %PN8

; Store %tp4 in 2nd array field of @n
  %N9 = getelementptr %struct.ns, %struct.ns* @n, i64 0, i32 1
  %LN9 = load %struct.test.01**, %struct.test.01*** %N9
  %PN9 = getelementptr %struct.test.01*, %struct.test.01** %LN9, i64 1
  %BC3 = bitcast %struct.test.01** %PN9 to i64*
  %NLD3 = load i64, i64* %BC3
  store %struct.test.01* %tp4, %struct.test.01** %PN9

; Below instructions are needed to select this routine as InitRoutine and
; increase field frequency of struct.test.01.
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
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

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call i8* @calloc(i64 10, i64 48)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  ret void
}

declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #0
declare i8* @llvm.ptr.annotation.p0i8(i8*, i8*, i8*, i32, i8*)

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test.01 zeroinitializer, i32 2}  ; %struct.test.01**
!6 = !{%struct.test.01 zeroinitializer, i32 1}  ; %struct.test.01*
!7 = distinct !{!6}
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!8}
!10 = !{!"S", %struct.test.01 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !4, !2} ; { i32, i64, i32, i32, i16, i64*, i64 }
!11 = !{!"S", %struct.ns zeroinitializer, i32 4, !4, !5, !5, !4} ; { i64*, %struct.test.01**, %struct.test.01**, i64* }

!intel.dtrans.types = !{!10, !11}
