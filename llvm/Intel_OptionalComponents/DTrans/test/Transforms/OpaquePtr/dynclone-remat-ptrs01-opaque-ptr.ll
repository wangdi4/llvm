; This test verifies that original pointers of shrunken structs, which are
; saved in global variable, are correctly rematerialized.

;  RUN: opt < %s -opaque-pointers -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes='internalize,dtrans-dyncloneop' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to i16.
%struct.test.01 = type { i32, i64, i32, i32, i16, ptr, i64 }

; Pointer of %struct.test.01 are saved in both fields in init routine.
; They will be rematerialized at the end of init routine.
%struct.netw = type { ptr, ptr }
@glob = internal global %struct.netw zeroinitializer, align 8

; This routine is selected as InitRoutine.
; Memory allocation pointers (i.e return value of calloc) are saved in both
; field 0 and 1 of @glob. These pointers will be rematerialized just before
; return if DynClone is triggered.
define void @init() {

; CHECK-LABEL:   define internal void @init

; CHECK:  [[ARET2:%dyn.alloc[0-9]*]] = alloca ptr

  %tp1 = tail call ptr @calloc(i64 10, i64 48)
; CHECK: store ptr %tp1, ptr [[ARET2]]

  store ptr %tp1, ptr getelementptr (%struct.netw, ptr @glob, i64 0, i32 1)
  %F1 = getelementptr %struct.test.01, ptr %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, ptr %F1, align 8
  %tp2 = getelementptr %struct.test.01, ptr %tp1, i64 5
  %field0 = getelementptr %struct.netw, ptr @glob, i64 0, i32 0
  store ptr %tp2, ptr %field0

  %F6 = getelementptr %struct.test.01, ptr %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, ptr %F6, align 8

; CHECK: [[APTR:%dyn.alloc.ld[0-9]*]] = load ptr, ptr [[ARET2]]

; Saved pointers are fixed like below:
; if (glob->field0) {
;    glob->field0 = call1 + ((glob->field0 - call1) / old_size) ptr new_size;
; }
;
; CHECK: postloop:
; CHECK: [[LD1:%[0-9]+]] = load ptr
; CHECK: [[CMP1:%[0-9]+]] = icmp ne ptr [[LD1]], null
; CHECK: br i1 [[CMP1]],
; CHECK: [[P11:%[0-9]+]] = ptrtoint ptr [[LD1]] to i64
; CHECK: [[P12:%[0-9]+]]  = ptrtoint ptr [[APTR]] to i64
; CHECK: [[SUB1:%[0-9]+]] = sub i64 [[P11]], [[P12]]
; CHECK: [[SDIV1:%[0-9]+]] = sdiv i64 [[SUB1]], 48
; CHECK: [[GEP1:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[SDIV1]]
; CHECK: store ptr [[GEP1]],

; CHECK: [[LD2:%[0-9]+]] = load ptr
; CHECK: [[CMP2:%[0-9]+]] = icmp ne ptr [[LD2]], null
; CHECK: br i1 [[CMP2]],
; CHECK: [[P21:%[0-9]+]] = ptrtoint ptr [[LD2]] to i64
; CHECK: [[P22:%[0-9]+]]  = ptrtoint ptr [[APTR]] to i64
; CHECK: [[SUB2:%[0-9]+]] = sub i64 [[P21]], [[P22]]
; CHECK: [[SDIV2:%[0-9]+]] = sdiv i64 [[SUB2]], 48
; CHECK: [[GEP2:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[SDIV2]]
; CHECK: store ptr [[GEP2]],

; CHECK:  store i8 1, ptr @__Shrink__Happened__

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
  %tp2 = tail call ptr @calloc(i64 10, i64 48)
  %F6 = getelementptr %struct.test.01, ptr %tp2, i32 0, i32 6
  ret void
}

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test.01 zeroinitializer, i32 1}  ; %struct.test.01*
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test.01 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !4, !2} ; { i32, i64, i32, i32, i16, i64*, i64 }
!9 = !{!"S", %struct.netw zeroinitializer, i32 2, !5, !5} ; { %struct.test.01*, %struct.test.01* }

!intel.dtrans.types = !{!8, !9}
