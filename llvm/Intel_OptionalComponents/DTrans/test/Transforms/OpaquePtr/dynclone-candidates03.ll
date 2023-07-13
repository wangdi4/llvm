; REQUIRES: asserts
; This test verifies that possible candidate fields are pruned with
; compile-time analysis for DynClone transformation.

;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

; CHECK-LABEL:  Possible Candidate fields:
; CHECK:    struct: struct.test.01 Index: 1
; CHECK:    struct: struct.test.01 Index: 6
; CHECK:    struct: struct.test.01 Index: 7

; CHECK-LABEL:  Final Candidate fields:
; CHECK:    struct: struct.test.01 Index: 1
; CHECK:    struct: struct.test.01 Index: 6
; CHECK:    struct: struct.test.01 Index: 7

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1, 6 and 7 fields are selected for possible candidates. After pruning,
; all 1, 6 and 7 fields are qualified as final candidates for DynClone.
; Fields 1, 6 and 7: Unknown values are assigned to these fields only in
; one routine "init". In other places, fields are written as
;  1. small values are assigned or
;  2. assigned with one of the final candidate fields or
;  3. reset using memset
%struct.test.01 = type { i32, i64, i32, i32, i16, ptr, i64, i64 }

; In this routine, fields 1, 6 and 7 are assigned with unknown values.
define void @init() {
  %call1 = tail call ptr @calloc(i64 10, i64 56)
  %F1 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, ptr %F1, align 8
  %F6 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 6
  store i64 %g1, ptr %F6, align 8
  %F7 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 7
  store i64 %g1, ptr %F7, align 8
  ret void
}

; field_6 = 100;
; field_6 = field_1;
define void @proc1(ptr "intel_dtrans_func_index"="1" %tp2) !intel.dtrans.func.type !6 {
  %F6 = getelementptr %struct.test.01, ptr %tp2, i32 0, i32 6
  store i64 100, ptr %F6, align 8
  %F1 = getelementptr %struct.test.01, ptr %tp2, i32 0, i32 1
  %ld1 = load i64, ptr %F1
  store i64 %ld1, ptr %F6, align 8
  ret void
}

; field_1 = 99999;
; field_1 = field_6;
; field_1 = field_7;
define void @proc2(ptr "intel_dtrans_func_index"="1" %tp3) !intel.dtrans.func.type !7 {
  %F1 = getelementptr %struct.test.01, ptr %tp3, i32 0, i32 1
  %ld3 = load i64, ptr %F1
  store i64 99999, ptr %F1, align 8
  %F6 = getelementptr %struct.test.01, ptr %tp3, i32 0, i32 6
  %ld2 = load i64, ptr %F6
  store i64 %ld2, ptr %F1, align 8
  %F7 = getelementptr %struct.test.01, ptr %tp3, i32 0, i32 7
  %ld4 = load i64, ptr %F7
  store i64 %ld4, ptr %F1, align 8
  ret void
}

; Reset struct.test.01 using memset in this routine.
define i32 @main() {
entry:
  call void @init();
  %call1 = tail call ptr @calloc(i64 10, i64 56)
  call void @proc1(ptr %call1);
  call void @proc2(ptr %call1);
  call void @llvm.memset.p0i8.i64(ptr %call1, i8 0, i64 56, i1 false)
  ret i32 0
}

declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0
declare !intel.dtrans.func.type !10 void @llvm.memset.p0i8.i64(ptr "intel_dtrans_func_index"="1", i8, i64, i1)

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test.01 zeroinitializer, i32 1}  ; %struct.test.01*
!6 = distinct !{!5}
!7 = distinct !{!5}
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!8}
!10 = distinct !{!8}
!11 = !{!"S", %struct.test.01 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !2} ; { i32, i64, i32, i32, i16, i64*, i64, i64 }

!intel.dtrans.types = !{!11}
