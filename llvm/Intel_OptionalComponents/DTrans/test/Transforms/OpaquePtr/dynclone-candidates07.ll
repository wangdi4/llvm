; REQUIRES: asserts
; This test verifies init routine is not identified for DynClone
; transformation.

;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

; CHECK-LABEL:  Possible Candidate fields:
; CHECK:    struct: struct.test.01 Index: 1
; CHECK:    struct: struct.test.01 Index: 6
; CHECK:    struct: struct.test.01 Index: 7

; CHECK-LABEL:  Candidate fields after Pruning:
; CHECK-NEXT:  struct: struct.test.01 Index: 6
; CHECK-NEXT:  struct: struct.test.01 Index: 7

; CHECK-LABEL: Checking init routine for
; CHECK:  No Init Routine identified

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;  Field 1: Not qualified since this field is assigned with two unknown
;  values in init and proc1.
;
;  Field 6: Assigned with unknown value only in proc1
;
;  Field 7: Assigned with unknown value only in proc2
;
; DynClone will not be triggered since candidate fields Field 6 and 7
; don't have same init routine.

%struct.test.01 = type { i32, i64, i32, i32, i16, ptr, i64, i64 }

; field_1 = unknown value.
define void @init(ptr "intel_dtrans_func_index"="1" %tp1) !intel.dtrans.func.type !6 {
  %F1 = getelementptr %struct.test.01, ptr %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, ptr %F1, align 8
  ret void
}

; field_1 = unknown value;
; field_6 = unknown value;
define void @proc1(ptr "intel_dtrans_func_index"="1" %tp2) !intel.dtrans.func.type !7 {
  %F1 = getelementptr %struct.test.01, ptr %tp2, i32 0, i32 1
  %g1 = select i1 undef, i64 600, i64 2000
  store i64 %g1, ptr %F1, align 8
  %F6 = getelementptr %struct.test.01, ptr %tp2, i32 0, i32 6
  store i64 %g1, ptr %F6, align 8
  ret void
}

; field_7 = unknown value;
; A dependancy for candidate fields:
; field_7 = field_6;
define void @proc2(ptr "intel_dtrans_func_index"="1" %tp3) !intel.dtrans.func.type !8 {
  %g1 = select i1 undef, i64 700, i64 3000
  %F7 = getelementptr %struct.test.01, ptr %tp3, i32 0, i32 7
  store i64 %g1, ptr %F7, align 8
  %F6 = getelementptr %struct.test.01, ptr %tp3, i32 0, i32 6
  %L = load i64, ptr %F6
  store i64 %L, ptr %F7
  ret void
}

define i32 @main() {
entry:
  %call1 = tail call ptr @calloc(i64 10, i64 56)
  call void @init(ptr %call1);
  call void @proc1(ptr %call1);
  call void @proc2(ptr %call1);
  ret i32 0
}

; Function Attrs: nounwind
declare !intel.dtrans.func.type !10 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test.01 zeroinitializer, i32 1}  ; %struct.test.01*
!6 = distinct !{!5}
!7 = distinct !{!5}
!8 = distinct !{!5}
!9 = !{i8 0, i32 1}  ; i8*
!10 = distinct !{!9}
!11 = !{!"S", %struct.test.01 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !2} ; { i32, i64, i32, i32, i16, i64*, i64, i64 }

!intel.dtrans.types = !{!11}
