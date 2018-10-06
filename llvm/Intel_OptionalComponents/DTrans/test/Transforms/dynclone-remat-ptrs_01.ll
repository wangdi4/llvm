; This test verifies that original pointers of shrunken structs, which are
; saved in global variable, are correctly rematerialized.

;  RUN: opt < %s -S -whole-program-assume -dtrans-dynclone 2>&1 | FileCheck %s
;  RUN: opt < %s -S -whole-program-assume -passes=dtrans-dynclone 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to i32.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

; Pointer of %struct.test.01 are saved in both fields in init routine.
; They will be rematerialized at the end of init routine.
%struct.netw = type { %struct.test.01*, %struct.test.01* }
@glob = internal global %struct.netw zeroinitializer, align 8

; This routine is selected as InitRoutine.
; Memory allocation pointers (i.e return value of calloc) are saved in both
; field 0 and 1 of @glob. These pointers will be rematerialized just before
; return if DynClone is triggered.
define void @init() {

; CHECK-LABEL:   define internal void @init

; CHECK:  [[ARET2:%dyn.alloc[0-9]*]] = alloca i8*

  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
; CHECK: store i8* %call1, i8** [[ARET2]]

  %tp1 = bitcast i8* %call1 to %struct.test.01*
  store %struct.test.01* %tp1, %struct.test.01** getelementptr (%struct.netw, %struct.netw* @glob, i64 0, i32 1)
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %tp2 = getelementptr %struct.test.01, %struct.test.01* %tp1, i64 5
  store %struct.test.01* %tp2, %struct.test.01** getelementptr (%struct.netw, %struct.netw* @glob, i64 0, i32 0)
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8

; CHECK: [[APTR:%dyn.alloc.ld[0-9]*]] = load i8*, i8** [[ARET2]]

; Saved pointers are fixed like below:
; if (glob->field0) {
;    glob->field0 = call1 + ((glob->field0 - call1) / old_size) * new_size;
; }
;
; CHECK: [[LD1:%[0-9]+]] = load %struct.test.01*
; CHECK: [[CMP1:%[0-9]+]] = icmp ne %struct.test.01* [[LD1]], null
; CHECK: br i1 [[CMP1]],
; CHECK: [[P11:%[0-9]+]] = ptrtoint %struct.test.01* [[LD1]] to i64
; CHECK: [[P12:%[0-9]+]]  = ptrtoint i8* [[APTR]] to i64
; CHECK: [[SUB1:%[0-9]+]] = sub i64 [[P11]], [[P12]]
; CHECK: [[SDIV1:%[0-9]+]] = sdiv i64 [[SUB1]], 48
; CHECK: [[BC11:%[0-9]+]] = bitcast i8* [[APTR]] to %__DYN_struct.test.01*
; CHECK: [[GEP1:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC11]], i64 [[SDIV1]]
; CHECK:  [[BC12:%[0-9]+]] = bitcast %__DYN_struct.test.01* [[GEP1]] to %struct.test.01*
; CHECK: store %struct.test.01* [[BC12]],

; CHECK: [[LD2:%[0-9]+]] = load %struct.test.01*
; CHECK: [[CMP2:%[0-9]+]] = icmp ne %struct.test.01* [[LD2]], null
; CHECK: br i1 [[CMP2]],
; CHECK: [[P21:%[0-9]+]] = ptrtoint %struct.test.01* [[LD2]] to i64
; CHECK: [[P22:%[0-9]+]]  = ptrtoint i8* [[APTR]] to i64
; CHECK: [[SUB2:%[0-9]+]] = sub i64 [[P21]], [[P22]]
; CHECK: [[SDIV2:%[0-9]+]] = sdiv i64 [[SUB2]], 48
; CHECK: [[BC21:%[0-9]+]] = bitcast i8* [[APTR]] to %__DYN_struct.test.01*
; CHECK: [[GEP2:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC21]], i64 [[SDIV2]]
; CHECK:  [[BC22:%[0-9]+]] = bitcast %__DYN_struct.test.01* [[GEP2]] to %struct.test.01*
; CHECK: store %struct.test.01* [[BC22]],

; CHECK:  store i8 1, i8* @__Shrink__Happened__

  ret void
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  call void @init();
  call void @proc1();
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
