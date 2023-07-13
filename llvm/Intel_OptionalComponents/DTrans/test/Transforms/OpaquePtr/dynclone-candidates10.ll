; REQUIRES: asserts
; This test is same as dynclone-candidates09.ll except one additional call
; "call void @proc3(i64 %bc1, %struct.test.01* %tp3)", which passes value of
; field_0 as first argument, in "proc2" routine. Since it is not safe to
; assign value of field_0 (non-candidate field) to field_1 of the struct,
; field_1 is not qualified as candidate field.

;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1, 6 and 7 fields are selected for possible candidates. After pruning and
; dependency analysis, none of the fields is qualified as final candidate
; for DynClone.
%struct.test.01 = type { i32, i64, i32, i32, i16, ptr, i64, i64 }

; CHECK-LABEL:  Possible Candidate fields:
; CHECK:    struct: struct.test.01 Index: 1
; CHECK:    struct: struct.test.01 Index: 6
; CHECK:    struct: struct.test.01 Index: 7

; CHECK: Pruning in routine: proc3
; CHECK: Checking i64 %cost argument assignment to struct: struct.test.01 Index: 1
; CHECK: Checking callsite:   call void @proc3(i64 %bc1, ptr %tp3)
; CHECK: Argument:   %bc1 = zext i32 %ld5 to i64
; CHECK: Invalid...More than one init routine: struct: struct.test.01 Index: 1

; CHECK: No Candidate is qualified after Dep...Skip DynClone

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

; field_1 = cost;
define void @proc3(i64 %cost, ptr "intel_dtrans_func_index"="1" %tp4) !intel.dtrans.func.type !6 {
  %F1 = getelementptr %struct.test.01, ptr %tp4, i32 0, i32 1
  store i64 %cost, ptr %F1, align 8
  ret void
}

; field_6 = 100;
; field_6 = field_1;
; call proc3(field_1, tp2);
; call proc3(30, tp2);
define void @proc1(ptr "intel_dtrans_func_index"="1" %tp2) !intel.dtrans.func.type !7 {
  %F6 = getelementptr %struct.test.01, ptr %tp2, i32 0, i32 6
  store i64 100, ptr %F6, align 8
  %F1 = getelementptr %struct.test.01, ptr %tp2, i32 0, i32 1
  %ld1 = load i64, ptr %F1
  store i64 %ld1, ptr %F6, align 8
  call void @proc3(i64 %ld1, ptr %tp2);
  call void @proc3(i64 30, ptr %tp2);
  ret void
}

; field_1 = 99999;
; field_1 = field_6;
; call proc3(field_6, tp3);
; field_1 = field_7;
; call proc3(field_7, tp3);
; call proc3(field_0, tp3);
define void @proc2(ptr "intel_dtrans_func_index"="1" %tp3) !intel.dtrans.func.type !8 {
  %F1 = getelementptr %struct.test.01, ptr %tp3, i32 0, i32 1
  %ld3 = load i64, ptr %F1
  store i64 99999, ptr %F1, align 8
  %F6 = getelementptr %struct.test.01, ptr %tp3, i32 0, i32 6
  %ld2 = load i64, ptr %F6
  store i64 %ld2, ptr %F1, align 8
  call void @proc3(i64 %ld2, ptr %tp3);
  %F7 = getelementptr %struct.test.01, ptr %tp3, i32 0, i32 7
  %ld4 = load i64, ptr %F7
  store i64 %ld4, ptr %F1, align 8
  %F0 = getelementptr %struct.test.01, ptr %tp3, i32 0, i32 0
  %ld5 = load i32, ptr %F0
  %bc1 = zext i32 %ld5 to i64
  call void @proc3(i64 %bc1, ptr %tp3);
  ret void
}

; Reset struct.test.01 using memset in this routine.
define i32 @main() {
entry:
  call void @init();
  %call1 = tail call ptr @calloc(i64 10, i64 56)
  call void @proc1(ptr %call1);
  call void @proc2(ptr %call1);
  call void @llvm.memset.p0.i64(ptr %call1, i8 0, i64 56, i1 false)
  ret i32 0
}

declare !intel.dtrans.func.type !10 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0
declare !intel.dtrans.func.type !11 void @llvm.memset.p0.i64(ptr "intel_dtrans_func_index"="1", i8, i64, i1)

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
!11 = distinct !{!9}
!12 = !{!"S", %struct.test.01 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !2} ; { i32, i64, i32, i32, i16, i64*, i64, i64 }

!intel.dtrans.types = !{!12}
