; REQUIRES: asserts
; This test verifies no candidate is qualified for DynClone transformation
; due to multiple reasons like large constant assignment, non-candidate
; field assignment etc.

;  RUN: opt < %s -dtransop-allow-typed-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -dtrans-dyncloneop -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s
;  RUN: opt < %s -dtransop-allow-typed-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

; CHECK-LABEL:  Possible Candidate fields:
; CHECK:    struct: struct.test.01 Index: 1
; CHECK:    struct: struct.test.01 Index: 6
; CHECK:    struct: struct.test.01 Index: 7

; CHECK:  No Candidate is qualified

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;  Field 1: Assigned invalid field (Field 7) in proc2
;
;  Field 6: Assigned large constant in proc1
;
;  Field 7: Assigned non-candidate field (of %struct.test.02) in proc1
;
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }

; Fields in this struct are not selected as candidates as it
; violates safety issues.
%struct.test.02 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }

; In this routine, fields 1, 6 and 7 are assigned with unknown values.
define void @init(%struct.test.01* "intel_dtrans_func_index"="1" %tp1) !intel.dtrans.func.type !6 {
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  store i64 %g1, i64* %F6, align 8
  %F7 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 7
  store i64 %g1, i64* %F7, align 8
  ret void
}

; field_6 = -1;
; field_7 = field_1 of struct.test02;
define void @proc1(%struct.test.01* "intel_dtrans_func_index"="1" %tp2, %struct.test.02* "intel_dtrans_func_index"="2" %tp4) !intel.dtrans.func.type !8 {
  %F7 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 7
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  store i64 -1, i64* %F6, align 8
  %F1_2 = getelementptr %struct.test.02, %struct.test.02* %tp4, i32 0, i32 1
  %ld1 = load i64, i64* %F1_2
  store i64 %ld1, i64* %F7, align 8
  ret void
}

; field_1 = field_7;
define void @proc2(%struct.test.01* "intel_dtrans_func_index"="1" %tp3) !intel.dtrans.func.type !9 {
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp3, i32 0, i32 1
  %F7 = getelementptr %struct.test.01, %struct.test.01* %tp3, i32 0, i32 7
  %ld3 = load i64, i64* %F7
  store i64 %ld3, i64* %F1, align 8
  ret void
}

define i32 @main() {
entry:
  %call1 = tail call i8* @calloc(i64 10, i64 56)
  %j = bitcast i8* %call1 to %struct.test.01*
  %i = bitcast i8* %call1 to i32*
  %call2 = tail call i8* @calloc(i64 10, i64 50)
  %j2 = bitcast i8* %call2 to %struct.test.02*
  call void @init(%struct.test.01* %j);
  call void @proc1(%struct.test.01* %j,  %struct.test.02* %j2);
  call void @proc2(%struct.test.01* %j);
  ret i32 0
}

declare !intel.dtrans.func.type !11 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test.01 zeroinitializer, i32 1}  ; %struct.test.01*
!6 = distinct !{!5}
!7 = !{%struct.test.02 zeroinitializer, i32 1}  ; %struct.test.02*
!8 = distinct !{!5, !7}
!9 = distinct !{!5}
!10 = !{i8 0, i32 1}  ; i8*
!11 = distinct !{!10}
!12 = !{!"S", %struct.test.01 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !2} ; { i32, i64, i32, i32, i16, i64*, i64, i64 }
!13 = !{!"S", %struct.test.02 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !2} ; { i32, i64, i32, i32, i16, i64*, i64, i64 }

!intel.dtrans.types = !{!12, !13}
