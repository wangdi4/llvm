; This test verifies that DynClone is not triggered because it is unable to
; track all uses of memory allocation pointer in init routine. Current
; analysis is able track if a pointer is first saved to some location and
; loaded later from the location by tracking uses of the load. But, it is
; unable to track if the loaded pointer is again saved.

; REQUIRES: asserts
;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dyncloneop -debug-only=dtrans-dynclone 2>&1 | FileCheck %s
;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dyncloneop -debug-only=dtrans-dynclone 2>&1 | FileCheck %s

; CHECK: Track uses of AllocCalls Failed
; CHECK-NOT: store i8 1, i8* @__Shrink__Happened__

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

; Memory allocation pointer, which is returned by calloc in "init" routine,
; is saved in @glob.
%struct.netw = type { %struct.test.01*, %struct.test.01* }
@glob = internal global %struct.netw zeroinitializer, align 8

; This routine is selected as InitRoutine.
; Memory allocation pointer (i.e return value of calloc) is first saved
; (%tp1) to @glob and then loaded (%tp2) from the location. One of
; the uses of the loaded pointer (%tp4) is saved again. Analysis
; is unable to track 2nd store. DynClone will be disabled since it is
; unable to track all uses of calloc.
define "intel_dtrans_func_index"="1" %struct.test.01* @init() !intel.dtrans.func.type !6 {
blk0:
  %call1 = tail call i8* @calloc(i64 10, i64 48)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  store %struct.test.01* %tp1, %struct.test.01** getelementptr (%struct.netw, %struct.netw* @glob, i64 0, i32 1)
  br i1 undef, label %blk1, label %blk2

blk1:
  %tp2 = load %struct.test.01*, %struct.test.01** getelementptr (%struct.netw, %struct.netw* @glob, i64 0, i32 1)
  br label %blk2

blk2:
  %tp3 = phi %struct.test.01* [%tp1, %blk0], [%tp2, %blk1]
  %tp4 = getelementptr %struct.test.01, %struct.test.01* %tp3, i64 2
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp4, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp4, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
  store %struct.test.01* %tp4, %struct.test.01** getelementptr (%struct.netw, %struct.netw* @glob, i64 0, i32 0)
  ret %struct.test.01* null
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  %tp2 = call %struct.test.01* @init();
  call void @proc1();
  ret i32 0
}

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call i8* @calloc(i64 10, i64 48)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  store i64 10, i64* %F6, align 8
  store i64 20, i64* %F6, align 8
  ret void
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
!9 = !{!"S", %struct.test.01 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !4, !2} ; { i32, i64, i32, i32, i16, i64*, i64 }
!10 = !{!"S", %struct.netw zeroinitializer, i32 2, !5, !5} ; { %struct.test.01*, %struct.test.01* }

!intel.dtrans.types = !{!9, !10}
