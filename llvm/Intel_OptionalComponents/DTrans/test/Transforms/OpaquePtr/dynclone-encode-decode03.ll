; This test verifies that encoding/decoding is skipped for load/store
; instructions related to arguments if possible when DynClone transformation
; is triggered.
;
; proc4 routine: Since argument %cost is not only used by store instruction
; (but also used by "add" instruction), encoding/decoding can't be skipped
; for load/store instructions related to %cost. This test verifies that
; __DYN_encoder is called in proc4 before storing the %cost value and
; __DYN_decoder is called in proc2 before passing %LF1 argument at callsite
; of proc4.
;
; proc3 routine: Encoding/decoding is skipped for load/store instructions
; related to %cost. This test verifies that __DYN_encoder is not called in
; proc3 before storing the %cost value and __DYN_decoder is not called in
; proc1 before passing %LF1 and 30 arguments at callsites of proc3.


;  RUN: opt < %s -dtransop-allow-typed-pointers -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -internalize -dtrans-dyncloneop 2>&1 | FileCheck %s
;  RUN: opt < %s -dtransop-allow-typed-pointers -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes='internalize,dtrans-dyncloneop' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to i16.
; Size of original %struct.test.01: 56
; Size after transformation: 30
;
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i32 }

%struct.nw = type { i32, i64 }
; Variable for candidate fields initialization.
@nw = internal global %struct.nw zeroinitializer, align 8


; CHECK: define internal void @proc4

; inc = cost + 1;
; field_1 = cost;
define void @proc4(i64 %cost, %struct.test.01* "intel_dtrans_func_index"="1" %tp4) !intel.dtrans.func.type !6 {
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp4, i32 0, i32 1
  %inc = add nsw i64 %cost, 1
  store i64 %cost, i64* %F1, align 8
; CHECK: [[CALL0:%[0-9]+]] = call i16 @__DYN_encoder(i64 %cost)
; CHECK: [[BC2:%[0-9]+]] = bitcast i64* %F1 to i16*
; CHECK: store i16 [[CALL0]], i16* [[BC2]], align 2

  ret void
}

; CHECK: define internal void @proc3

; field_1 = cost;
define void @proc3(i64 %cost, %struct.test.01* "intel_dtrans_func_index"="1" %tp4) !intel.dtrans.func.type !7 {
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp4, i32 0, i32 1
  store i64 %cost, i64* %F1, align 8
; CHECK: [[BC0:%[0-9]+]] = trunc i64 %cost to i16
; CHECK: [[BC1:%[0-9]+]] = bitcast i64* %F1 to i16*
; CHECK:  store i16 [[BC0]], i16* [[BC1]], align 2
  ret void
}


; CHECK: define internal void @proc1()

;  call proc3(field_1, %tp2);
;  call proc3(30, %tp2);
define void @proc1() {
  %call1 = tail call i8* @calloc(i64 10, i64 56)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
  %LF1 = load i64, i64* %F1, align 8
  call void @proc3(i64 %LF1, %struct.test.01* %tp2);
  call void @proc3(i64 30, %struct.test.01* %tp2);

; CHECK: [[LD0:%[0-9]+]] = load i16, i16* %3, align 2
; CHECK:  %LF1 = zext i16 [[LD0]] to i64
; CHECK:  call void @proc3(i64 %LF1, %struct.test.01* %tp2)
; CHECK:  call void @proc3(i64 30, %struct.test.01* %tp2)

  ret void
}

; CHECK: define internal void @proc2()

;  call proc4(field_1, %tp2);
define void @proc2() {
  %call1 = tail call i8* @calloc(i64 10, i64 56)
  %tp2 = bitcast i8* %call1 to %struct.test.01*

; Accessing 1st field, which is shrunken from i64 to i16.
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 1
  %LF1 = load i64, i64* %F1, align 8
  call void @proc4(i64 %LF1, %struct.test.01* %tp2);

; CHECK: [[LD1:%[0-9]+]] = load i16, i16* %3, align 2
; CHECK:  %LF1 = call i64 @__DYN_decoder(i16 [[LD1]])
; CHECK: call void @proc4(i64 %LF1, %struct.test.01* %tp2)

  ret void
}

; CHECK-LABEL: define internal void @init()

; This routine is selected as InitRoutine.
define void @init() {
; Initialize nw.field_1
  store i64 2000000, i64* getelementptr (%struct.nw, %struct.nw* @nw, i64 0, i32 1), align 8
  %call1 = tail call i8* @calloc(i64 10, i64 56)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
  %L = load i64, i64* getelementptr (%struct.nw, %struct.nw* @nw, i64 0, i32 1)
  %S1 = mul i64 2, %L
  store i64 %S1, i64* %F1, align 8
  %S2 = add i64 15, %S1
  store i64 %S2, i64* %F6, align 8
  ret void
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  store i64 1000000, i64* getelementptr (%struct.nw, %struct.nw* @nw, i64 0, i32 1), align 8
  call void @init();
  ret i32 0
}

declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64)
attributes #0 = { "target-features"="+avx2" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test.01 zeroinitializer, i32 1}  ; %struct.test.01*
!6 = distinct !{!5}
!7 = distinct !{!5}
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!8}
!10 = !{!"S", %struct.test.01 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !1} ; { i32, i64, i32, i32, i16, i64*, i64, i32 }
!11 = !{!"S", %struct.nw zeroinitializer, i32 2, !1, !2} ; { i32, i64 }

!intel.dtrans.types = !{!10, !11}
