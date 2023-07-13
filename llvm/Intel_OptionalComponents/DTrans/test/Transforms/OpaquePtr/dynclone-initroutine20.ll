; It verifies that code is generated correctly to initialize/set/
; reset/test allocation of unsafe flag correctly for DynClone
; transformation even though init routine has two user calls
; (@safecall and @unsafecall). Only unsafecall is considered as
; unsafe because 2nd field of @glob, where pointer of %struct.test.01
; is saved in init routine, is accessed.

;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes='internalize,dtrans-dyncloneop' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to i32.
%struct.test.01 = type { i32, i64, i32, i32, i16, ptr, i64 }

%struct.netw = type { ptr, ptr }
@glob = internal global %struct.netw zeroinitializer, align 8

; CHECK-LABEL: define internal void @init()

; Initialize safe flag
; CHECK: [[ALLOC_SAFE:%dyn.safe[0-9]*]] = alloca i8
; CHECK: store i8 0, ptr [[ALLOC_SAFE]]

; Set safe flag
; CHECK: store i8 1, ptr %dyn.safe
; CHECK: %call1 = tail call ptr @calloc(i64 10, i64 48)

; Reset safe flag
; CHECK: store i8 0, ptr %dyn.safe
; CHECK: tail call void @unsafecall()

; Test safe flag
; CHECK: [[OR_1:%d.or[0-9]*]] = or i1
; CHECK: [[LD:%d.ld[0-9]*]] = load i8, ptr [[ALLOC_SAFE]]
; CHECK: [[CMP:%d.cmp[0-9]*]] = icmp eq i8 [[LD]], 0
; CHECK:  [[OR_2:%d.or[0-9]*]] = or i1 [[OR_1]], [[CMP]]
; CHECK: br i1 [[OR_2]],

; CHECK: store i8 1, ptr @__Shrink__Happened__

; This is selected as InitRoutine. Pointer of %struct.test.01 is saved
; in 2nd field of @glob.
define void @init() {
  %call1 = tail call ptr @calloc(i64 10, i64 48)
  tail call void @safecall()
  %F1 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, ptr %F1, align 8
  %F6 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, ptr %F6, align 8
  tail call void @unsafecall()
  store ptr %call1, ptr getelementptr (%struct.netw, ptr @glob, i64 0, i32 1)
  ret void
}

; 1st field of @glob is accessed.
define void @safecall() {
  store ptr null, ptr getelementptr (%struct.netw, ptr @glob, i64 0, i32 0)
  ret void
}

; 2nd field of @glob is accessed.
define void @unsafecall() {
  store ptr null, ptr getelementptr (%struct.netw, ptr @glob, i64 0, i32 1)
  ret void
}

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call ptr @calloc(i64 10, i64 48)
  %F6 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 6
  store i64 0, ptr %F6, align 8
  store i64 1, ptr %F6, align 8
  ret void
}

define i32 @main() {
  call void @init()
  call void @proc1()
  ret i32 0
}

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test.01 zeroinitializer, i32 1}  ; %struct.test.01*
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test.01 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !4, !2} ; { i32, i64, i32, i32, i16, i64*, i64 }
!9 = !{!"S", %struct.netw zeroinitializer, i32 2, !5, !5} ; { %struct.test.01*, %struct.test.01* }

!intel.dtrans.types = !{!8, !9}
