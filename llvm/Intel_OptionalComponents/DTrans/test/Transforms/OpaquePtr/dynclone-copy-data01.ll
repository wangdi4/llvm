; This test verifies that data is copied correctly from original layout
; to new layout in "init" routine for DynClone transformation.

;  RUN: opt < %s -dtrans-dynclone-shrunken-type-width=16 -dtrans-dynclone-sign-shrunken-int-type=false -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes='internalize,dtrans-dyncloneop' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to i16. Mapping of fields
; between original and new layouts:
;   0 --> 1
;   1 --> 5
;   2 --> 2
;   3 --> 3
;   4 --> 6
;   5 --> 0
;   6 --> 7
;   7 --> 4
; Loop is generated to copy data from old layout to new layout
; in init routine below.

%struct.test.01 = type { i32, i64, i32, i32, i16, ptr, i64, i32 }

; This routine is selected as InitRoutine.
define void @init() {

; CHECK-LABEL:   define internal void @init

; CHECK:  [[ARET1:%dyn.alloc[0-9]*]] = alloca i64
; CHECK:  [[ARET2:%dyn.alloc[0-9]*]] = alloca ptr

  %call1 = tail call ptr @calloc(i64 10, i64 56)
; CHECK: store i64 10, ptr [[ARET1]]
; CHECK: store ptr %call1, ptr [[ARET2]]

  %F1 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, ptr %F1, align 8
  %F6 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 6
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
; CHECK: [[DG2:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[LI]], i32 5
; CHECK: [[BC1:%[0-9]+]] = trunc i64 [[LD1]] to i16
; CHECK: store i16 [[BC1]], ptr [[DG2]]
; CHECK: [[DG3:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[LI]], i32 2
; CHECK: store i32 [[LD2]], ptr [[DG3]]
; CHECK: [[DG4:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[LI]], i32 3
; CHECK: store i32 [[LD3]], ptr [[DG4]]
; CHECK: [[DG6:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[LI]], i32 6
; CHECK: store i16 [[LD4]], ptr [[DG6]]
; CHECK: [[DG0:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[LI]], i32 0
; CHECK: store ptr [[LD5]], ptr [[DG0]]
; CHECK: [[DG5:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[LI]], i32 7
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

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call ptr @calloc(i64 10, i64 56)
  %F6 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 6
  %L = load i64, ptr %F6
  %F1 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 1
  store i64 %L, ptr %F1
  ret void
}

; Function Attrs: nounwind
declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test.01 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !1} ; { i32, i64, i32, i32, i16, i64*, i64, i32 }

!intel.dtrans.types = !{!7}
