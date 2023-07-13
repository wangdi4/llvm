; REQUIRES: asserts
; This test verifies that "init" routine is not qualified as InitRoutine
; for DynClone transformation since memory is allocated for %struct.test.01
; before "init" is called in main.

;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1, 6 and 7 fields are selected for possible candidates.
%struct.test.01 = type { i32, i64, i32, i32, i16, ptr, i64, i64 }

; CHECK-LABEL:   Init Routine: init
; CHECK:    InitRoutine failed...Struct accessed before

; "init" routine is not qualified as InitRoutine since memory
; is allocated for %struct.test.01 before "init" is called.
define i32 @main() {
entry:
  %call1 = tail call ptr @calloc(i64 10, i64 56)
  call void @init(ptr %call1)
  ret i32 0
}

; This routine is selected as InitRoutine but not qualified.
define void @init(ptr "intel_dtrans_func_index"="1" %tp1) !intel.dtrans.func.type !6 {
  %F1 = getelementptr %struct.test.01, ptr %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, ptr %F1, align 8
  %F6 = getelementptr %struct.test.01, ptr %tp1, i32 0, i32 6
  store i64 %g1, ptr %F6, align 8
  %F7 = getelementptr %struct.test.01, ptr %tp1, i32 0, i32 7
  store i64 %g1, ptr %F7, align 8
  ret void
}

; Function Attrs: nounwind
declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

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
