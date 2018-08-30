; This test verifies that data is copied correctly from original layout
; to new layout in "init" routine for DynClone transformation.

;  RUN: opt < %s -S -whole-program-assume -dtrans-dynclone 2>&1 | FileCheck %s
;  RUN: opt < %s -S -whole-program-assume -passes=dtrans-dynclone 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to i32. Mapping of fields
; between original and new layouts:
;   0 --> 1
;   1 --> 2
;   2 --> 3
;   3 --> 4
;   4 --> 6
;   5 --> 0
;   6 --> 5
; Loop is generated to copy data from old layout to new layout
; in init routine below.

%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

; This routine is selected as InitRoutine.
define void @init() {

; CHECK-LABEL:   define internal void @init

  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8

; CHECK:  [[SRC:%[0-9]+]] = bitcast i8* %call1 to %struct.test.01*
; CHECK: [[DST:%[0-9]+]] = bitcast i8* %call1 to %__DYN_struct.test.01*

; CHECK: [[LI:%lindex[0-9]*]] = phi i64 [ 0,
; CHECK: [[SG0:%[0-9]+]] = getelementptr inbounds %struct.test.01, %struct.test.01* [[SRC]], i64 [[LI]], i32 0
; CHECK: [[LD0:%[0-9]+]] = load i32, i32* [[SG0]]
; CHECK: [[SG1:%[0-9]+]] = getelementptr inbounds %struct.test.01, %struct.test.01* [[SRC]], i64 [[LI]], i32 1
; CHECK: [[LD1:%[0-9]+]] = load i64, i64* [[SG1]]
; CHECK: [[SG2:%[0-9]+]] = getelementptr inbounds %struct.test.01, %struct.test.01* [[SRC]], i64 [[LI]], i32 2
; CHECK: [[LD2:%[0-9]+]] = load i32, i32* [[SG2]]
; CHECK: [[SG3:%[0-9]+]] = getelementptr inbounds %struct.test.01, %struct.test.01* [[SRC]], i64 [[LI]], i32 3
; CHECK: [[LD3:%[0-9]+]] = load i32, i32* [[SG3]]
; CHECK: [[SG4:%[0-9]+]] = getelementptr inbounds %struct.test.01, %struct.test.01* [[SRC]], i64 [[LI]], i32 4
; CHECK: [[LD4:%[0-9]+]] = load i16, i16* [[SG4]]
; CHECK: [[SG5:%[0-9]+]] = getelementptr inbounds %struct.test.01, %struct.test.01* [[SRC]], i64 [[LI]], i32 5
; CHECK: [[LD5:%[0-9]+]] = load i64*, i64** [[SG5]]
; CHECK: [[SG6:%[0-9]+]] = getelementptr inbounds %struct.test.01, %struct.test.01* [[SRC]], i64 [[LI]], i32 6
; CHECK: [[LD6:%[0-9]+]] = load i64, i64* [[SG6]]
; CHECK: [[DG1:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, %__DYN_struct.test.01* [[DST]], i64 [[LI]], i32 1
; CHECK: store i32 [[LD0]], i32* [[DG1]]
; CHECK: [[DG2:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, %__DYN_struct.test.01* [[DST]], i64 [[LI]], i32 2
; CHECK: [[BC1:%[0-9]+]] = trunc i64 [[LD1]] to i32
; CHECK: store i32 [[BC1]], i32* [[DG2]]
; CHECK: [[DG3:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, %__DYN_struct.test.01* [[DST]], i64 [[LI]], i32 3
; CHECK: store i32 [[LD2]], i32* [[DG3]]
; CHECK: [[DG4:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, %__DYN_struct.test.01* [[DST]], i64 [[LI]], i32 4
; CHECK: store i32 [[LD3]], i32* [[DG4]]
; CHECK: [[DG6:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, %__DYN_struct.test.01* [[DST]], i64 [[LI]], i32 6
; CHECK: store i16 [[LD4]], i16* [[DG6]]
; CHECK: [[DG0:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, %__DYN_struct.test.01* [[DST]], i64 [[LI]], i32 0
; CHECK: store i64* [[LD5]], i64** [[DG0]]
; CHECK: [[DG5:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, %__DYN_struct.test.01* [[DST]], i64 [[LI]], i32 5
; CHECK: [[BC2:%[0-9]+]] = trunc i64 [[LD6]] to i32
; CHECK: store i32 [[BC2]], i32* [[DG5]]
; CHECK: [[ADD1:%[0-9]+]] = add i64 [[LI]], 1
; CHECK: [[CMP1:%[0-9]+]] = icmp ult i64 [[ADD1]], 10

; CHECK:  store i8 1, i8* @__Shrink__Happened__

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
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  ret void
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
