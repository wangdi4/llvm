; This test verifies that DynClone is triggered because it is able to
; track uses of memory allocation, which are stored in array fields of
; AOSTOSOA global variable.

;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop 2>&1 | FileCheck %s

; CHECK: store i8 1, ptr @__Shrink__Happened__

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to i16.
%struct.test.01 = type { i32, i64, i32, i32, i16, ptr, i64 }

; Type of AOSTOSOA variable.
%struct.ns = type { ptr, ptr, ptr, ptr }

; Memory allocation pointers, which are returned by calloc in "init" routine,
; are stored in 2nd and 3rd array fields of @n.
@n = internal global %struct.ns zeroinitializer, align 8

; AOSTOSOA allocation call is marked with this annotation.
@__intel_dtrans_aostosoa_alloc = private constant [38 x i8] c"{dtrans} AOS-to-SOA allocation {id:0}\00"

; This routine is selected as InitRoutine.
; Uses of Memory allocation pointer (%tp1, %tp2 and %tp3) are stored in
; 2nd and 3rd array fields of @n.
; We can track all these uses and able to find locations where these
; pointers are stored.
define "intel_dtrans_func_index"="1" ptr @init() !intel.dtrans.func.type !7 {
  %call0 = call ptr @calloc(i64 1000, i64 32)
  %call.ptr = call ptr @llvm.ptr.annotation.p0i8(ptr %call0, ptr getelementptr inbounds ([38 x i8], ptr @__intel_dtrans_aostosoa_alloc, i32 0, i32 0), ptr null, i32 0, ptr null)
  %C01 = getelementptr i8, ptr %call0, i64 0
  %C02 = bitcast ptr %C01 to ptr

  ; DynClone uses pattern match to find the GEPOperator as the pointer parameter
  ; of the store. Use index 1 to avoid the GEP removal.
  store ptr %C02, ptr getelementptr (%struct.ns, ptr @n, i64 0, i32 1)
  %call1 = tail call ptr @calloc(i64 10, i64 48)
  %tp1 = bitcast ptr %call1 to ptr
  %tp2 = getelementptr %struct.test.01, ptr %tp1, i64 4
  %call3 = tail call ptr @calloc(i64 20, i64 48)
  %tp3 = bitcast ptr %call3 to ptr

; Store %tp1 in 2nd array field of @n
  %N7 = getelementptr %struct.ns, ptr @n, i64 0, i32 1
  %LN7 = load ptr, ptr %N7
  %PN7 = getelementptr ptr, ptr %LN7, i64 1
  %BC1 = bitcast ptr %PN7 to ptr
  %NLD1 = load i64, ptr %BC1
  store ptr %tp1, ptr %PN7

; Store %tp3 in 3rd array field of @n
  %N8 = getelementptr %struct.ns, ptr @n, i64 0, i32 2
  %LN8 = load ptr, ptr %N8
  %PN8 = getelementptr ptr, ptr %LN8, i64 1
  %BC2 = bitcast ptr %PN8 to ptr
  %NLD2 = load i64, ptr %BC2
  store ptr %tp3, ptr %PN8

; Store %tp2 in 2nd array field of @n
  %N9 = getelementptr %struct.ns, ptr @n, i64 0, i32 1
  %LN9 = load ptr, ptr %N9
  %PN9 = getelementptr ptr, ptr %LN9, i64 1
  %BC3 = bitcast ptr %PN9 to ptr
  %NLD3 = load i64, ptr %BC3
  store ptr %tp2, ptr %PN9

; Below instructions are needed to select this routine as InitRoutine and
; increase field frequency of struct.test.01.
  %F1 = getelementptr %struct.test.01, ptr %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, ptr %F1, align 8
  store i64 %g1, ptr %F1, align 8
  %F6 = getelementptr %struct.test.01, ptr %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, ptr %F6, align 8
  store i64 %g2, ptr %F6, align 8

  ret ptr null
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  %tp2 = call ptr @init();
  call void @proc1();
  ret i32 0
}

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call ptr @calloc(i64 10, i64 48)
  %tp2 = bitcast ptr %call1 to ptr
  %F6 = getelementptr %struct.test.01, ptr %tp2, i32 0, i32 6
  ret void
}

declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0
declare ptr @llvm.ptr.annotation.p0i8(ptr, ptr, ptr, i32, ptr)

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
