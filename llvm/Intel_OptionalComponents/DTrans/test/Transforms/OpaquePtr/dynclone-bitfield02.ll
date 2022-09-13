; This test is same as dynclone-bitfield01.ll except more values (0, 1, 2,
; 3 and 4) are stored to 4th field of %struct.test.01 in @proc1. Since all
; possible values of 4th field can't fit in 2-bits, fields 1 and 4 can't be
; packed together. This test verifies that new layout doesn't pack fields
; 1 and 4 using bit-fields.

;  RUN: opt < %s -dtransop-allow-typed-pointers -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -internalize -dtrans-dyncloneop 2>&1 | FileCheck %s
;  RUN: opt < %s -dtransop-allow-typed-pointers -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes='internalize,dtrans-dyncloneop' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to i16.

%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i32 }

; New layout for DynClone.
; CHECK: %__DYN_struct.test.01 = type <{ i64*, i32, i32, i32, i32, i16, i16, i16 }>
; Makes sure bit-field layout is not triggered.
; CHECK-NOT: %__DYN_struct.test.01 = type <{ i64*, i32, i32, i32, i32, i16, i16 }>

; This routine is selected as InitRoutine.
define void @init() {
  %call1 = tail call i8* @calloc(i64 10, i64 56)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
  ret void
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  call void @init();
;  call void @proc1();
  ret i32 0
}

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call i8* @calloc(i64 10, i64 56)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  %L1 = load i64, i64* %F6
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
  store i64 %L1, i64* %F1
  %L2 = load i64, i64* %F1
  %F4 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 4
  store i16 0, i16* %F4
  %C1 = icmp eq i64 %L2, 2
  %S1 = select i1 %C1, i16 1, i16 2
  store i16 %S1, i16* %F4
  store i16 3, i16* %F4
  store i16 4, i16* %F4
  %L3 = load i16, i16* %F4
  ret void
}

; Function Attrs: nounwind
declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test.01 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !1} ; { i32, i64, i32, i32, i16, i64*, i64, i32 }

!intel.dtrans.types = !{!7}
