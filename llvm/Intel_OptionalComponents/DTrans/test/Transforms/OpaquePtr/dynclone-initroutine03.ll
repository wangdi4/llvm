; REQUIRES: asserts
; This test verifies that "init" routine is not qualified as InitRoutine
; for DynClone transformation due to indirect call.

;  RUN: opt < %s -dtransop-allow-typed-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -dtrans-dyncloneop -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s
;  RUN: opt < %s -dtransop-allow-typed-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1, 6 and 7 fields are selected for possible candidates.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }

; CHECK:    Indirect Call ... Skip DynClone
; CHECK-NOT:    Init Routine: init

; Indirect call %t0() and direct call "proc2()" are two legality issues
; here to qualify  "init" routine as InitRoutine. These two calls are called
; before "init()". It will report "Indirect call" is the reason for not
; qualified because analysis starts from entry block.
define i32 @main() {
entry:
  %i = alloca i64, align 8
  %t0 = bitcast i64* %i to i32 ()*
  %call = call i32 %t0(), !intel_dtrans_type !5
  br label %D

D:
  %call1 = tail call i8* @calloc(i64 10, i64 56)
  call void @proc2()
  %j = bitcast i8* %call1 to %struct.test.01*
  call void @init(%struct.test.01* %j)
  ret i32 0
}

; This routine is selected as InitRoutine.
define void @init(%struct.test.01* "intel_dtrans_func_index"="1" %tp1) !intel.dtrans.func.type !7 {
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  store i64 %g1, i64* %F6, align 8
  %F7 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 7
  store i64 %g1, i64* %F7, align 8
  ret void
}

define void @proc2() {
  ret void
}

declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{!"F", i1 false, i32 0, !1}  ; i32 ()
!6 = !{%struct.test.01 zeroinitializer, i32 1}  ; %struct.test.01*
!7 = distinct !{!6}
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!8}
!10 = !{!"S", %struct.test.01 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !2} ; { i32, i64, i32, i32, i16, i64*, i64, i64 }

!intel.dtrans.types = !{!10}
