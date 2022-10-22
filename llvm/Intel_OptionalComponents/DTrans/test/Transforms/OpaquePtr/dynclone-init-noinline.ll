; This test verifies that noinline is set for "init" call when DynClone
; transformation is triggered.

;  RUN: opt < %s -dtransop-allow-typed-pointers -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; DynClone is triggered for this struct.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

; Call to "init" routine is qualified as InitRoutine for DynClone.
; Call to init routine will be marked as "noinline".
define i32 @main() {
entry:
; CHECK: call void @init() #[[ATTR:[0-9]+]]
  call void @init();
  ret i32 0
}
; CHECK: store i8 1, i8* @__Shrink__Happened__
; CHECK: attributes #[[ATTR]] = { noinline }


; This routine is selected as InitRoutine.
define void @init() {
  %call1 = tail call i8* @calloc(i64 10, i64 48)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
  ret void
}

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call i8* @calloc(i64 10, i64 48)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  ret void
}

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test.01 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !4, !2} ; { i32, i64, i32, i32, i16, i64*, i64 }

!intel.dtrans.types = !{!7}
