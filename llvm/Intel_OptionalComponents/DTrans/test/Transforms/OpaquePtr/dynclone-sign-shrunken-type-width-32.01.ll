; This test verifies that fields 1 and 4 of %struct.test.01 are packed using
; bit-field in signed 32-bit.
; It also verifies that transformations are done correctly for
; the following
;    1. New layout
;    2. General loads / stores of field 2 in @proc1

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
; CHECK-NEXT:    [[DYN_ALLOC17:%.*]] = alloca i64, align 8
; CHECK-NEXT:    [[DYN_ALLOC:%.*]] = alloca ptr, align 8
; CHECK-NEXT:    [[DYN_SAFE:%.*]] = alloca i8, align 1
; CHECK-NEXT:    store i8 0, ptr [[DYN_SAFE]], align 1

; Initialize d_max = 0xffffffffe0000000, d_min = 0x10000000
; CHECK-NEXT:    [[D_MAX:%.*]] = alloca i64, align 8
; CHECK-NEXT:    store i64 -536870912, ptr [[D_MAX]], align 8
; CHECK-NEXT:    [[D_MIN:%.*]] = alloca i64, align 8
; CHECK-NEXT:    store i64 536870911, ptr [[D_MIN]], align 8

; CHECK:         [[CALL1:%.*]] = tail call noalias ptr @calloc(i64 10, i64 56)
  %call1 = tail call noalias ptr @calloc(i64 10, i64 56)

; CHECK:         [[F1:%.*]] = getelementptr [[STRUCT_TEST_01:%.*]], ptr [[CALL1]], i32 0, i32 1
  %F1 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 1

; CHECK-NEXT:    [[G1:%.*]] = select i1 undef, i64 500, i64 1000
  %g1 = select i1 undef, i64 500, i64 1000

; d_max = (d_max > g1) ? d_max : g1; d_min = (d_min < g1) ? d_min : g1;

; CHECK-NEXT:    [[D_LD:%.*]] = load i64, ptr [[D_MIN]], align 8
; CHECK-NEXT:    [[D_CMP:%.*]] = icmp slt i64 [[D_LD]], [[G1]]
; CHECK-NEXT:    [[D_SEL:%.*]] = select i1 [[D_CMP]], i64 [[D_LD]], i64 [[G1]]
; CHECK-NEXT:    store i64 [[D_SEL]], ptr [[D_MIN]], align 8
; CHECK-NEXT:    [[D_LD1:%.*]] = load i64, ptr [[D_MAX]], align 8
; CHECK-NEXT:    [[D_CMP2:%.*]] = icmp sgt i64 [[D_LD1]], [[G1]]
; CHECK-NEXT:    [[D_SEL3:%.*]] = select i1 [[D_CMP2]], i64 [[D_LD1]], i64 [[G1]]
; CHECK-NEXT:    store i64 [[D_SEL3]], ptr [[D_MAX]], align 8
; CHECK-NEXT:    store i64 [[G1]], ptr [[F1]], align 8
  store i64 %g1, i64* %F1, align 8

; CHECK-NEXT:    [[F6:%.*]] = getelementptr [[STRUCT_TEST_01]], ptr [[CALL1]], i32 0, i32 6
  %F6 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 6
; CHECK-NEXT:    [[G2:%.*]] = select i1 undef, i64 -5000, i64 20000
  %g2 = select i1 undef, i64 -5000, i64 20000

; d_max = (d_max > g2) ? d_max : g2; d_min = (d_min < g2) ? d_min : g2;
; CHECK:         [[D_LD4:%.*]] = load i64, ptr [[D_MIN]], align 8
; CHECK-NEXT:    [[D_CMP5:%.*]] = icmp slt i64 [[D_LD4]], [[G2]]
; CHECK-NEXT:    [[D_SEL6:%.*]] = select i1 [[D_CMP5]], i64 [[D_LD4]], i64 [[G2]]
; CHECK-NEXT:    store i64 [[D_SEL6]], ptr [[D_MIN]], align 8
; CHECK-NEXT:    [[D_LD7:%.*]] = load i64, ptr [[D_MAX]], align 8
; CHECK-NEXT:    [[D_CMP8:%.*]] = icmp sgt i64 [[D_LD7]], [[G2]]
; CHECK-NEXT:    [[D_SEL9:%.*]] = select i1 [[D_CMP8]], i64 [[D_LD7]], i64 [[G2]]
; CHECK-NEXT:    store i64 [[D_SEL9]], ptr [[D_MAX]], align 8
; CHECK-NEXT:    store i64 [[G2]], ptr [[F6]], align 8
  store i64 %g2, ptr %F6, align 8

; if (d_min < 0xffffffffe0000000 || d_max > 0x10000000) bail out;
; CHECK:         [[D_LD10:%.*]] = load i64, ptr [[D_MIN]], align 8
; CHECK-NEXT:    [[D_CMP11:%.*]] = icmp slt i64 [[D_LD10]], -536870912
; CHECK-NEXT:    [[D_LD12:%.*]] = load i64, ptr [[D_MAX]], align 8
; CHECK-NEXT:    [[D_CMP13:%.*]] = icmp sgt i64 [[D_LD12]], 536870911
; CHECK-NEXT:    [[D_OR:%.*]] = or i1 [[D_CMP11]], [[D_CMP13]]

; CHECK:       copydata:
; CHECK-NEXT:    [[LINDEX:%.*]] = phi i64

; CHECK:         [[TMP5:%.*]] = getelementptr inbounds [[STRUCT_TEST_01]], ptr [[TMP1:%.*]], i64 [[LINDEX]], i32 1
; CHECK-NEXT:    [[TMP6:%.*]] = load i64, ptr [[TMP5]], align 8

; CHECK:         [[TMP11:%.*]] = getelementptr inbounds [[STRUCT_TEST_01]], ptr [[TMP1]], i64 [[LINDEX]], i32 4
; CHECK-NEXT:    [[TMP12:%.*]] = load i16, ptr [[TMP11]], align 2

; CHECK:         [[TMP15:%.*]] = getelementptr inbounds [[STRUCT_TEST_01]], ptr [[TMP1]], i64 [[LINDEX]], i32 6
; CHECK-NEXT:    [[TMP16:%.*]] = load i64, ptr [[TMP15]], align 8

; Field 1 and field 4 are packed to field 2 of %__DYN_struct.test.01.
; CHECK:         [[TMP20:%.*]] = getelementptr inbounds [[__DYN_STRUCT_TEST_01:%.*]], ptr [[TMP2:%.*]], i64 [[LINDEX]], i32 2
; CHECK-NEXT:    [[TMP21:%.*]] = trunc i64 [[TMP6]] to i32
; CHECK-NEXT:    [[TMP22:%.*]] = load i32, ptr [[TMP20]], align 4
; As the input can be negative number now, we must mask 0x30000000, otherwise, it will pollute another packed value.
; CHECK-NEXT:    [[TMP23:%.*]] = and i32 [[TMP21]], 1073741823
; CHECK-NEXT:    [[TMP24:%.*]] = and i32 [[TMP22]], -1073741824
; CHECK-NEXT:    [[TMP25:%.*]] = or i32 [[TMP24]], [[TMP23]]
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP20]], align 4

; CHECK:         [[TMP28:%.*]] = getelementptr inbounds [[__DYN_STRUCT_TEST_01]], ptr [[TMP2]], i64 [[LINDEX]], i32 2
; CHECK-NEXT:    [[TMP29:%.*]] = zext i16 [[TMP12]] to i32
; CHECK-NEXT:    [[TMP30:%.*]] = load i32, ptr [[TMP28]], align 4
; CHECK-NEXT:    [[TMP31:%.*]] = and i32 [[TMP29]], 3
; CHECK-NEXT:    [[TMP32:%.*]] = shl i32 [[TMP31]], 30
; CHECK-NEXT:    [[TMP33:%.*]] = and i32 [[TMP30]], 1073741823
; CHECK-NEXT:    [[TMP34:%.*]] = or i32 [[TMP33]], [[TMP32]]
; CHECK-NEXT:    store i32 [[TMP34]], ptr [[TMP28]], align 4

; CHECK:         [[TMP36:%.*]] = getelementptr inbounds [[__DYN_STRUCT_TEST_01]], ptr [[TMP2]], i64 [[LINDEX]], i32 5
; CHECK-NEXT:    [[TMP37:%.*]] = trunc i64 [[TMP16]] to i32
; CHECK-NEXT:    store i32 [[TMP37]], ptr [[TMP36]], align 4
; CHECK-NEXT:    [[TMP38:%.*]] = getelementptr inbounds [[__DYN_STRUCT_TEST_01]], ptr [[TMP2]], i64 [[LINDEX]], i32 6

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

; L1 = (int64_t)((int32_t)(*F6))
; CHECK-NEXT:    [[TMP1:%.*]] = load i32, ptr [[F6]], align 4
; This data is shrinked to i32, since we support negative number now, need sign extend the number.
; CHECK-NEXT:    [[L1:%.*]] = sext i32 [[TMP1]] to i64
  %L1 = load i64, ptr %F6

; CHECK-NEXT:    [[F1:%.*]] = getelementptr [[__DYN_STRUCT_TEST_01]], ptr [[CALL1]], i32 0, i32 2
  %F1 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 1

; *F1 = (((*F1) & (0xc0000000)) | (L1 & 0x3fffffff));
; CHECK-NEXT:    [[TMP7:%.*]] = trunc i64 [[L1]] to i32
; CHECK-NEXT:    [[TMP9:%.*]] = load i32, ptr [[F1]], align 4
; CHECK-NEXT:    [[TMP10:%.*]] = and i32 [[TMP7]], 1073741823
; CHECK-NEXT:    [[TMP11:%.*]] = and i32 [[TMP9]], -1073741824
; CHECK-NEXT:    [[TMP12:%.*]] = or i32 [[TMP11]], [[TMP10]]
; CHECK-NEXT:    store i32 [[TMP12]], ptr [[F1]], align 4
  store i64 %L1, ptr %F1

; L2 = sext i30 ((*F1) & (0x3fffffff)), i64;
; CHECK-NEXT:    [[TMP14:%.*]] = load i32, ptr [[F1]], align 4
; CHECK-NEXT:    [[TMP15:%.*]] = trunc i32 [[TMP14]] to i30
; CHECK-NEXT:    [[TMP16:%.*]] = sext i30 [[TMP15]] to i32
; CHECK-NEXT:    [[L2:%.*]] = sext i32 [[TMP16]] to i64
  %L2 = load i64, ptr %F1

; CHECK-NEXT:    [[F4:%.*]] = getelementptr [[__DYN_STRUCT_TEST_01]], ptr [[CALL1]], i32 0, i32 2
  %F4 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 4

; *((int32_t*)F4) = (*((int32_t*)(F4)) & 0x30000000) | ((((int)(0)) & 0x3) << 30);
; CHECK-NEXT:    [[TMP19:%.*]] = sext i16 0 to i32
; CHECK-NEXT:    [[TMP21:%.*]] = load i32, ptr [[F4]], align 4
; CHECK-NEXT:    [[TMP22:%.*]] = and i32 [[TMP19]], 3
; CHECK-NEXT:    [[TMP23:%.*]] = shl i32 [[TMP22]], 30
; CHECK-NEXT:    [[TMP24:%.*]] = and i32 [[TMP21]], 1073741823
; CHECK-NEXT:    [[TMP25:%.*]] = or i32 [[TMP24]], [[TMP23]]
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[F4]], align 4
  store i16 0, ptr %F4

; CHECK-NEXT:    [[C1:%.*]] = icmp eq i64 [[L2]], 2
  %C1 = icmp eq i64 %L2, 2

; CHECK-NEXT:    [[S1:%.*]] = select i1 [[C1]], i16 1, i16 2
  %S1 = select i1 %C1, i16 1, i16 2

; *((int32_t*)F4) = (((int32_t)((int16_t)(S1)) & 3) << 30) | (*((int32_t*)F4) & 0x3fffffff);
; CHECK-NEXT:    [[TMP26:%.*]] = sext i16 [[S1]] to i32
; CHECK-NEXT:    [[TMP28:%.*]] = load i32, ptr [[F4]], align 4
; CHECK-NEXT:    [[TMP29:%.*]] = and i32 [[TMP26]], 3
; CHECK-NEXT:    [[TMP30:%.*]] = shl i32 [[TMP29]], 30
; CHECK-NEXT:    [[TMP31:%.*]] = and i32 [[TMP28]], 1073741823
; CHECK-NEXT:    [[TMP32:%.*]] = or i32 [[TMP31]], [[TMP30]]
; CHECK-NEXT:    store i32 [[TMP32]], ptr [[F4]], align 4
  store i16 %S1, ptr %F4

; CHECK-NEXT:    [[TMP34:%.*]] = load i32, ptr [[F4]], align 4
; CHECK-NEXT:    [[TMP35:%.*]] = lshr i32 [[TMP34]], 30
; CHECK-NEXT:    [[L3:%.*]] = trunc i32 [[TMP35]] to i16
  %L3 = load i16, ptr %F4

; CHECK-NEXT:    ret void
  ret void
}

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
