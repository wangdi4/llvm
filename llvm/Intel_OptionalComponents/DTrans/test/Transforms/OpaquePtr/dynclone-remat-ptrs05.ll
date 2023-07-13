; This test verifies that DynClone is not triggered because it is unable to
; track all uses of memory allocation pointer in init routine because
; arguments of phi in init routine are not from same memory allocation.

; REQUIRES: asserts
;  RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop -debug-only=dtrans-dynclone 2>&1 | FileCheck %s

; CHECK: Track uses of AllocCalls Failed
; CHECK-NOT: store i8 1, ptr @__Shrink__Happened__

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.01 = type { i32, i64, i32, i32, i16, ptr, i64 }

; Memory allocation pointer, which is returned by calloc in "init" routine,
; is saved in @glob.
%struct.netw = type { ptr, ptr }
@glob = internal global %struct.netw zeroinitializer, align 8

; This routine is selected as InitRoutine.
; DynClone will be disabled since arguments of phi node are
; not coming from the same memory allocation.
define "intel_dtrans_func_index"="1" ptr @init() !intel.dtrans.func.type !6 {
blk0:
  %call0 = tail call ptr @calloc(i64 10, i64 48)
  %call1 = tail call ptr @calloc(i64 10, i64 48)
  store ptr %call1, ptr getelementptr (%struct.netw, ptr @glob, i64 0, i32 1)
  br i1 undef, label %blk1, label %blk2

blk1:
  %tp2 = load ptr, ptr getelementptr (%struct.netw, ptr @glob, i64 0, i32 1)
  br label %blk2

blk2:
  %tp3 = phi ptr [%call0, %blk0], [%tp2, %blk1]
  %tp4 = getelementptr %struct.test.01, ptr %tp3, i64 2
  %F1 = getelementptr %struct.test.01, ptr %tp4, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, ptr %F1, align 8
  %F6 = getelementptr %struct.test.01, ptr %tp4, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, ptr %F6, align 8
  ret ptr null
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  %tp2 = call ptr @init();
  call void @proc1();
  ret i32 0
}

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call ptr @calloc(i64 10, i64 48)
  %F6 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 6
  ret void
}

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
!9 = !{!"S", %struct.test.01 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !4, !2} ; { i32, i64, i32, i32, i16, i64*, i64 }
!10 = !{!"S", %struct.netw zeroinitializer, i32 2, !5, !5} ; { %struct.test.01*, %struct.test.01* }

!intel.dtrans.types = !{!9, !10}
