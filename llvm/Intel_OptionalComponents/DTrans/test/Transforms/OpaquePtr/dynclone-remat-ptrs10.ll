; This test verifies that DynClone is not triggered because it is unable to
; find AOSTOSOA allocation call due to missing aostosoa_alloc annotation.

; REQUIRES: asserts
;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop -debug-only=dtrans-dynclone 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: Track uses of AllocCalls Failed
; CHECK-NOT: store i8 1, ptr @__Shrink__Happened__

%struct.test.01 = type { i32, i64, i32, i32, i16, ptr, i64 }

; Type of AOSTOSOA variable.
%struct.ns = type { ptr, ptr, ptr, ptr }

; Memory allocation pointers, which are returned by calloc in "init" routine,
; are stored in 2nd and 3rd array fields of @n.
@n = internal global %struct.ns zeroinitializer, align 8

; This routine is selected as InitRoutine.
; Uses of Memory allocation pointer (%tp1, %tp2 and %tp3) are stored in
; 2nd and 3rd array fields of @n.
; We can track all these uses but unable to find AOSTOSOA memory allocation.
define "intel_dtrans_func_index"="1" ptr @init() !intel.dtrans.func.type !7 {
  %call0 = call ptr @calloc(i64 1000, i64 32)
  %C01 = getelementptr i8, ptr %call0, i64 0
  store ptr %C01, ptr getelementptr (%struct.ns, ptr @n, i64 0, i32 0)
  %call1 = tail call ptr @calloc(i64 10, i64 48)
  %tp2 = getelementptr %struct.test.01, ptr %call1, i64 2
  %tp3 = getelementptr %struct.test.01, ptr %call1, i64 4

; Store %tp1 in 2nd array field of @n
  %N7 = getelementptr %struct.ns, ptr @n, i64 0, i32 1
  %LN7 = load ptr, ptr %N7
  %PN7 = getelementptr ptr, ptr %LN7, i64 1
  %NLD1 = load i64, ptr %PN7
  store ptr %call1, ptr %PN7

; Store %tp2 in 3rd array field of @n
  %N8 = getelementptr %struct.ns, ptr @n, i64 0, i32 2
  %LN8 = load ptr, ptr %N8
  %PN8 = getelementptr ptr, ptr %LN8, i64 1
  %NLD2 = load i64, ptr %PN8
  store ptr %tp2, ptr %PN8

; Store %tp3 in 2nd array field of @n
  %N9 = getelementptr %struct.ns, ptr @n, i64 0, i32 1
  %LN9 = load ptr, ptr %N9
  %PN9 = getelementptr ptr, ptr %LN9, i64 1
  %NLD3 = load i64, ptr %PN9
  store ptr %tp3, ptr %PN9

; Below instructions are needed to select this routine as InitRoutine and
; increase field frequency of struct.test.01.
  %F1 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, ptr %F1, align 8
  store i64 %g1, ptr %F1, align 8
  store i64 %g1, ptr %F1, align 8
  %F6 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, ptr %F6, align 8
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
  %F6 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 6
  ret void
}

declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0

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
