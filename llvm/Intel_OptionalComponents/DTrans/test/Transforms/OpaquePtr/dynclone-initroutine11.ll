; REQUIRES: asserts
; This test verifies that "init" routine is not qualified as InitRoutine
; for DynClone transformation since memory allocation for %struct.test.01
; is not found in "init".

;  RUN: opt < %s -dtransop-allow-typed-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1, 6 and 7 fields are selected for possible candidates.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }

; CHECK-LABEL:   Init Routine: init
; CHECK:    InitRoutine failed...no allocation seen

; This routine is selected as InitRoutine but not qualified since
; it doesn't have any memory allocation for %struct.test.01.
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

define i32 @main() {
entry:
  call void @init(%struct.test.01* null)
  %call1 = tail call i8* @calloc(i64 10, i64 56)
  %j = bitcast i8* %call1 to %struct.test.01*
  ret i32 0
}

declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test.01 zeroinitializer, i32 1}  ; %struct.test.01*
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = !{!"S", %struct.test.01 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !2} ; { i32, i64, i32, i32, i16, i64*, i64, i64 }

!intel.dtrans.types = !{!9}
