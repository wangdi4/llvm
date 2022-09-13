; This test verifies that fields 1 and 4 of %struct.test.01 are packed using
; bit-field in signed 32-bit,.
; It also verifies that transformations are done correctly for
; the following
;    1. New layout
;    2. General loads / stores of field 2 in @proc1
;    3. Encoder/Decoder transformations with signed 32-bit.

;  RUN: opt < %s -opaque-pointers -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -internalize -dtrans-dyncloneop 2>&1 | FileCheck %s
;  RUN: opt < %s -opaque-pointers -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes='internalize,dtrans-dyncloneop' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i32 }

; Data layout
; Oringal             New
; f0 i32              f5 i64*
; f1 i64              f0 i32
; f2 i32              f4 i16->i2 & f1 i32->i30
; f3 i32              f2 i32
; f4 i16              f3 i32
; f5 i64*             f6 i64->i32
; f6 i64              f7 i32
; f7 i32

; New layout for DynClone.
; CHECK: %__DYN_struct.test.01 = type <{ ptr, i32, i32, i32, i32, i32, i32 }>

; This routine is selected as InitRoutine.
define void @init() {
; CHECK-LABEL: @init(
; CHECK-LABEL: copydata:
; CHECK:         [[TMP5:%.*]] = getelementptr inbounds [[STRUCT_TEST_01:%.*]], ptr [[TMP1:%.*]], i64 [[LINDEX:%.*]], i32 1
; CHECK-NEXT:    [[TMP6:%.*]] = load i64, ptr [[TMP5]], align 8

; CHECK:         [[TMP15:%.*]] = getelementptr inbounds [[STRUCT_TEST_01]], ptr [[TMP1]], i64 [[LINDEX]], i32 6
; CHECK-NEXT:    [[TMP16:%.*]] = load i64, ptr [[TMP15]], align 8

; CHECK:         [[TMP20:%.*]] = getelementptr inbounds [[__DYN_STRUCT_TEST_01:%.*]], ptr [[TMP2:%.*]], i64 [[LINDEX]], i32 2
; CHECK-NEXT:    [[TMP21:%.*]] = call i32 @__DYN_encoder(i64 [[TMP6]])
; CHECK-NEXT:    [[TMP22:%.*]] = load i32, ptr [[TMP20]], align 4
; CHECK-NEXT:    [[TMP23:%.*]] = and i32 [[TMP21]], 1073741823
; CHECK-NEXT:    [[TMP24:%.*]] = and i32 [[TMP22]], -1073741824
; CHECK-NEXT:    [[TMP25:%.*]] = or i32 [[TMP24]], [[TMP23]]
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP20]], align 4

; CHECK:         [[TMP36:%.*]] = getelementptr inbounds [[__DYN_STRUCT_TEST_01]], ptr [[TMP2]], i64 [[LINDEX]], i32 5
; CHECK-NEXT:    [[TMP37:%.*]] = call i32 @__DYN_encoder(i64 [[TMP16]])
; CHECK-NEXT:    store i32 [[TMP37]], ptr [[TMP36]], align 4
;
  %call1 = tail call noalias i8* @calloc(i64 10, i64 56)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, ptr %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, ptr %F1, align 8
  %F6 = getelementptr %struct.test.01, ptr %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, ptr %F6, align 8
  store i64 20000000000, ptr %F6, align 8
  store i64 20000000001, ptr %F6, align 8
  store i64 -20000000003, ptr %F6, align 8
  store i64 -20000000004, ptr %F6, align 8
  store i64 -20000000005, ptr %F6, align 8
  ret void
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  call void @init();
  ret i32 0
}

; This routine just accesses candidate field.
define void @proc1() {
; CHECK-LABEL: @proc1(
; CHECK-NEXT:    [[CALL1:%.*]] = tail call noalias ptr @calloc(i64 10, i64 32)
  %call1 = tail call noalias ptr @calloc(i64 10, i64 56)

; CHECK-NEXT:    [[F6:%.*]] = getelementptr [[__DYN_STRUCT_TEST_01:%.*]], ptr [[CALL1]], i32 0, i32 5
  %F6 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 6

; CHECK-NEXT:    [[TMP4:%.*]] = load i32, ptr [[F6]], align 4
; CHECK-NEXT:    [[L1:%.*]] = sext i32 [[TMP4]] to i64
  %L1 = load i64, ptr %F6

; CHECK-NEXT:    [[F1:%.*]] = getelementptr [[__DYN_STRUCT_TEST_01]], ptr [[CALL1]], i32 0, i32 2
  %F1 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 1

; CHECK-NEXT:    [[TMP7:%.*]] = trunc i64 [[L1]] to i32
; CHECK-NEXT:    [[TMP9:%.*]] = load i32, ptr [[F1]], align 4
; CHECK-NEXT:    [[TMP10:%.*]] = and i32 [[TMP7]], 1073741823
; CHECK-NEXT:    [[TMP11:%.*]] = and i32 [[TMP9]], -1073741824
; CHECK-NEXT:    [[TMP12:%.*]] = or i32 [[TMP11]], [[TMP10]]
; CHECK-NEXT:    store i32 [[TMP12]], ptr [[F1]], align 4
  store i64 %L1, ptr %F1

; CHECK-NEXT:    [[TMP14:%.*]] = load i32, ptr [[F1]], align 4
; CHECK-NEXT:    [[TMP15:%.*]] = trunc i32 [[TMP14]] to i30
; CHECK-NEXT:    [[TMP16:%.*]] = sext i30 [[TMP15]] to i32
; CHECK-NEXT:    [[L2:%.*]] = call i64 @__DYN_decoder(i32 [[TMP16]])
  %L2 = load i64, ptr %F1

  %F4 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 4
  store i16 0, ptr %F4
  %C1 = icmp eq i64 %L2, 2
  %S1 = select i1 %C1, i16 1, i16 2
  store i16 %S1, ptr %F4
  %L3 = load i16, ptr %F4
  ret void
}

; CHECK-LABEL:  @__DYN_encoder
; CHECK:  entry:
; CHECK:    %1 = icmp sge i64 %0, -536870912
; CHECK:    %2 = icmp ule i64 %0, 536870906
; CHECK:    %3 = and i1 %1, %2
; CHECK:    br i1 %3, label %default, label %switch_bb
; CHECK:  switch_bb:                                        ; preds = %entry
; CHECK:    switch i64 %0, label %default [
; CHECK:      i64 -20000000005, label %case
; CHECK:      i64 -20000000004, label %case1
; CHECK:      i64 -20000000003, label %case2
; CHECK:      i64 20000000000, label %case3
; CHECK:      i64 20000000001, label %case4
; CHECK:    ]
; CHECK:  default:                                          ; preds = %entry, %switch_bb
; CHECK:    %4 = trunc i64 %0 to i32
; CHECK:    br label %return
; CHECK:  return:                                           ; preds = %case4, %case3, %case2, %case1, %case, %default
; CHECK:    %phival = phi i32 [ %4, %default ], [ 536870907, %case ], [ 536870908, %case1 ], [ 536870909, %case2 ], [ 536870910, %case3 ], [ 536870911, %case4 ]
; CHECK:    ret i32 %phival
; CHECK:  case:                                             ; preds = %switch_bb
; CHECK:    br label %return
; CHECK:  case1:                                            ; preds = %switch_bb
; CHECK:    br label %return
; CHECK:  case2:                                            ; preds = %switch_bb
; CHECK:    br label %return
; CHECK:  case3:                                            ; preds = %switch_bb
; CHECK:    br label %return
; CHECK:  case4:                                            ; preds = %switch_bb
; CHECK:    br label %return

; CHECK-LABEL:  @__DYN_decoder
; CHECK:  entry:
; CHECK:    %1 = icmp sge i32 %0, -536870912
; CHECK:    %2 = icmp ule i32 %0, 536870906
; CHECK:    %3 = and i1 %1, %2
; CHECK:    br i1 %3, label %default, label %switch_bb
; CHECK:  switch_bb:                                        ; preds = %entry
; CHECK:    switch i32 %0, label %default [
; CHECK:      i32 536870907, label %case
; CHECK:      i32 536870908, label %case1
; CHECK:      i32 536870909, label %case2
; CHECK:      i32 536870910, label %case3
; CHECK:      i32 536870911, label %case4
; CHECK:    ]
; CHECK:  default:                                          ; preds = %entry, %switch_bb
; CHECK:    %4 = sext i32 %0 to i64
; CHECK:    br label %return
; CHECK:  return:                                           ; preds = %case4, %case3, %case2, %case1, %case, %default
; CHECK:    %phival = phi i64 [ %4, %default ], [ -20000000005, %case ], [ -20000000004, %case1 ], [ -20000000003, %case2 ], [ 20000000000, %case3 ], [ 20000000001, %case4 ]
; CHECK:    ret i64 %phival
; CHECK:  case:                                             ; preds = %switch_bb
; CHECK:    br label %return
; CHECK:  case1:                                            ; preds = %switch_bb
; CHECK:    br label %return
; CHECK:  case2:                                            ; preds = %switch_bb
; CHECK:    br label %return
; CHECK:  case3:                                            ; preds = %switch_bb
; CHECK:    br label %return
; CHECK:  case4:                                            ; preds = %switch_bb
; CHECK:    br label %return

; Function Attrs: nounwind
declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test.01 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !1} ; { i32, i64, i32, i32, i16, i64*, i64, i32 }

!intel.dtrans.types = !{!7}
