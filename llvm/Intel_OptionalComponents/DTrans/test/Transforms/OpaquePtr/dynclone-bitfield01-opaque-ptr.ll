; This test verifies that fields 1 and 4 of %struct.test.01 are packed using
; bit-fields. It also verifies that transformations are done correctly for
; the following
;    1. New layout
;    2. General loads / stores of packed fields in @proc1
;    3. In Init routine, copying packed fields from original to new layout.

;  RUN: opt < %s -opaque-pointers -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -internalize -dtrans-dyncloneop 2>&1 | FileCheck %s
;  RUN: opt < %s -opaque-pointers -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes='internalize,dtrans-dyncloneop' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to i16. 1 and 4 fields are packed using
; bit-fields. The new index for the packed field will be 5.

%struct.test.01 = type { i32, i64, i32, i32, i16, ptr, i64, i32 }

; New layout for DynClone.
; TODO: The following check line is disabled because the AsmWriter to not
; currently find all the structure types to be emitted when printing IR.
; TODO-CHECK: %__DYN_struct.test.01 = type <{ ptr, i32, i32, i32, i32, i16, i16 }>

; This routine is selected as InitRoutine.
define void @init() {

; CHECK-LABEL:   define internal void @init

; CHECK:  [[ARET1:%dyn.alloc[0-9]*]] = alloca i64
; CHECK:  [[ARET2:%dyn.alloc[0-9]*]] = alloca ptr

  %tp1 = tail call ptr @calloc(i64 10, i64 56)
; CHECK: store i64 10, ptr [[ARET1]]
; CHECK: store ptr %tp1, ptr [[ARET2]]

  %F1 = getelementptr %struct.test.01, ptr %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, ptr %F1, align 8
  %F6 = getelementptr %struct.test.01, ptr %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, ptr %F6, align 8

; CHECK: [[APTR:%dyn.alloc.ld[0-9]*]] = load ptr, ptr [[ARET2]]
; CHECK: [[ASIZE:%dyn.alloc.ld[0-9]*]] = load i64, ptr [[ARET1]]

; CHECK: [[LI:%lindex[0-9]*]] = phi i64 [ 0,
; CHECK: [[SG0:%[0-9]+]] = getelementptr inbounds %struct.test.01, ptr [[APTR]], i64 [[LI]], i32 0
; CHECK: [[LD0:%[0-9]+]] = load i32, ptr [[SG0]]
; CHECK: [[SG1:%[0-9]+]] = getelementptr inbounds %struct.test.01, ptr [[APTR]], i64 [[LI]], i32 1
; CHECK: [[LD1:%[0-9]+]] = load i64, ptr [[SG1]]
; CHECK: [[SG2:%[0-9]+]] = getelementptr inbounds %struct.test.01, ptr [[APTR]], i64 [[LI]], i32 2
; CHECK: [[LD2:%[0-9]+]] = load i32, ptr [[SG2]]
; CHECK: [[SG3:%[0-9]+]] = getelementptr inbounds %struct.test.01, ptr [[APTR]], i64 [[LI]], i32 3
; CHECK: [[LD3:%[0-9]+]] = load i32, ptr [[SG3]]
; CHECK: [[SG4:%[0-9]+]] = getelementptr inbounds %struct.test.01, ptr [[APTR]], i64 [[LI]], i32 4
; CHECK: [[LD4:%[0-9]+]] = load i16, ptr [[SG4]]
; CHECK: [[SG5:%[0-9]+]] = getelementptr inbounds %struct.test.01, ptr [[APTR]], i64 [[LI]], i32 5
; CHECK: [[LD5:%[0-9]+]] = load ptr, ptr [[SG5]]
; CHECK: [[SG6:%[0-9]+]] = getelementptr inbounds %struct.test.01, ptr [[APTR]], i64 [[LI]], i32 6
; CHECK: [[LD6:%[0-9]+]] = load i64, ptr [[SG6]]
; CHECK: [[SG7:%[0-9]+]] = getelementptr inbounds %struct.test.01, ptr [[APTR]], i64 [[LI]], i32 7
; CHECK: [[LD7:%[0-9]+]] = load i32, ptr [[SG7]]
; CHECK: [[DG1:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[LI]], i32 1
; CHECK: store i32 [[LD0]], ptr [[DG1]]
;
; Copying packed field.
;
; CHECK: [[DG2:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[LI]], i32 5
; CHECK: [[BC1:%[0-9]+]] = trunc i64 [[LD1]] to i16
; CHECK: [[NLI1:%[0-9]+]] = load i16, ptr [[DG2]]
; CHECK: [[AND1:%[0-9]+]] = and i16 [[NLI1]], -16384
; CHECK: [[OR1:%[0-9]+]] = or i16 [[AND1]], [[BC1]]
; CHECK: store i16 [[OR1]], ptr [[DG2]]
; CHECK: [[DG3:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[LI]], i32 2
; CHECK: store i32 [[LD2]], ptr [[DG3]]
; CHECK: [[DG4:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[LI]], i32 3
; CHECK: store i32 [[LD3]], ptr [[DG4]]
;
; Copying packed field.
;
; CHECK: [[BDG5:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[LI]], i32 5
; CHECK: [[NLD2:%[0-9]+]] = load i16, ptr [[BDG5]]
; CHECK: [[SHL2:%[0-9]+]] = shl i16 [[LD4]], 14
; CHECK: [[AND2:%[0-9]+]] = and i16 [[NLD2]], 16383
; CHECK: [[OR2:%[0-9]+]] = or i16 [[AND2]], [[SHL2]]
; CHECK: store i16 [[OR2]], ptr [[BDG5]]
; CHECK: [[DG0:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[LI]], i32 0
; CHECK: store ptr [[LD5]], ptr [[DG0]]
; CHECK: [[DG5:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[LI]], i32 6
; CHECK: [[BC2:%[0-9]+]] = trunc i64 [[LD6]] to i16
; CHECK: store i16 [[BC2]], ptr [[DG5]]
; CHECK: [[DG6:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[LI]], i32 4
; CHECK: store i32 [[LD7]], ptr [[DG6]]
; CHECK: [[ADD1:%[0-9]+]] = add i64 [[LI]], 1
; CHECK: [[CMP1:%[0-9]+]] = icmp ult i64 [[ADD1]], [[ASIZE]]

; CHECK:  store i8 1, ptr @__Shrink__Happened__

  ret void
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  call void @init();
;  call void @proc1();
  ret i32 0
}

; CHECK-LABEL:   define internal void @proc1()

; This routine just accesses candidate field.
define void @proc1() {
  %tp2 = tail call ptr @calloc(i64 10, i64 56)
  %F6 = getelementptr %struct.test.01, ptr %tp2, i32 0, i32 6
  %L1 = load i64, ptr %F6
  %F1 = getelementptr %struct.test.01, ptr %tp2, i32 0, i32 1

;
; Storing shrunken packed field.
;
; CHECK: %F1 = getelementptr %__DYN_struct.test.01, ptr %tp2, i32 0, i32 5
; CHECK: [[PT1:%[0-9]+]] = trunc i64 %L1 to i16
; CHECK: [[PLD1:%[0-9]+]] = load i16, ptr %F1
; CHECK: [[PAND1:%[0-9]+]] = and i16 [[PLD1]], -16384
; CHECK: [[POR1:%[0-9]+]] = or i16 [[PAND1]], [[PT1]]
; CHECK: store i16 [[POR1]], ptr %F1, align 2

  store i64 %L1, ptr %F1

;
; Loading shrunken packed field.
;
; CHECK: [[PLD2:%[0-9]+]] = load i16, ptr %F1, align 2
; CHECK: [[PAND2:%[0-9]+]] = and i16 [[PLD2]], 16383
; CHECK: %L2 = zext i16 [[PAND2]] to i64

  %L2 = load i64, ptr %F1
  %F4 = getelementptr %struct.test.01, ptr %tp2, i32 0, i32 4

;
; Storing packed bit-field field.
;
; CHECK: %F4 = getelementptr %__DYN_struct.test.01, ptr %tp2, i32 0, i32 5
; CHECK: [[PC1:%[0-9]+]] = bitcast i16 0 to i16
; CHECK: [[PLD3:%[0-9]+]] = load i16, ptr %F4
; CHECK: [[PSHL3:%[0-9]+]] = shl i16 [[PC1]], 14
; CHECK: [[PAND3:%[0-9]+]] = and i16 [[PLD3]], 16383
; CHECK: [[POR3:%[0-9]+]] = or i16 [[PAND3]], [[PSHL3]]
; CHECK: store i16 [[POR3]], ptr %F4, align 2

  store i16 0, ptr %F4

;
; Storing packed bit-field field.
;
; CHECK: [[PC2:%[0-9]+]] = bitcast i16 %S1 to i16
; CHECK: [[PLD4:%[0-9]+]] = load i16, ptr %F4
; CHECK: [[PSHL4:%[0-9]+]] = shl i16 [[PC2]], 14
; CHECK: [[PAND4:%[0-9]+]] = and i16 [[PLD4]], 16383
; CHECK: [[POR4:%[0-9]+]] = or i16 [[PAND4]], [[PSHL4]]
; CHECK: store i16 [[POR4]], ptr %F4, align 2

  %C1 = icmp eq i64 %L2, 2
  %S1 = select i1 %C1, i16 1, i16 2
  store i16 %S1, ptr %F4

;
; Loading packed bit-field field.
;
; CHECK: [[PLD5:%[0-9]+]] = load i16, ptr %F4, align 2
; CHECK: [[PSHL5:%[0-9]+]] = lshr i16 [[PLD5]], 14
; CHECK: %L3 = bitcast i16 [[PSHL5]] to i16

  %L3 = load i16, ptr %F4
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

