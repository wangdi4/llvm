; This test verifies that size of fields that are marked with aostosoa index
; will be reduced to 2 bytes with a runtime check on number of elements
; allocated with aostosoa allocation call when DynClone is triggered (in
; init routine). This also verifies transformations of
; GEP/Load/Store/llvm.ptr.annotation for shrunken aostosoa index
; fields (in proc1 routine).

;  RUN: opt < %s -dtrans-dynclone-shrunken-type-width=16 -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes='internalize,dtrans-dyncloneop' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 2nd and 3rd fields of original struct will be converted to i16.
; They will become 4th and 5th fields after field reorder.
; CHECK: %__DYN_struct.test.01 = type <{ ptr, i64, i32, i16, i16, i16, i16 }>

; 1st field is shrunk to i16. 2nd and 3rd fields are
; marked with aostosoa index fields. Both will be reduced
; to 2 bytes when DynClone is triggered.
%struct.test.01 = type { i32, i64, i32, i32, i16, ptr, i64 }

; Type of AOSTOSOA variable.
%struct.ns = type { ptr, ptr, ptr, ptr }

; Memory allocation pointers, which are returned by calloc in "init" routine,
; are stored in 2nd and 3rd array fields of @n.
@n = internal global %struct.ns zeroinitializer, align 8

; AOSTOSOA allocation call is marked with this annotation.
@__intel_dtrans_aostosoa_alloc = private constant [38 x i8] c"{dtrans} AOS-to-SOA allocation {id:0}\00"

; AOSTOSOA allocation call is marked with this annotation.
@__intel_dtrans_aostosoa_index = private constant [41 x i8] c"{dtrans} AOS-to-SOA peeling index {id:0}\00"

; This routine has accesses to 2nd and 3rd fields of %struct.test.01, which
; are marked as aostosoa index fields.
; CHECK: define internal void @proc1()
define void @proc1() {
  %call1 = tail call ptr @calloc(i64 10, i64 48)

  %F2 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 2
; CHECK:  %F2 = getelementptr %__DYN_struct.test.01, ptr %call1, i32 0, i32 4

  %A2 = call ptr @llvm.ptr.annotation.p0.p0(ptr %F2, ptr getelementptr inbounds ([41 x i8], ptr @__intel_dtrans_aostosoa_index, i32 0, i32 0), ptr null, i32 0, ptr null)
; CHECK: %A2 = call ptr @llvm.ptr.annotation.p0.p0(ptr %F2,

  %L1 = load i32, ptr %F2
; CHECK: [[LD1:%[0-9]+]] = load i16, ptr %F2, align 2
; CHECK: %L1 = zext i16 [[LD1]] to i32

  %F3 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 3
; CHECK: %F3 = getelementptr %__DYN_struct.test.01, ptr %call1, i32 0, i32 5

  %A3 = call ptr @llvm.ptr.annotation.p0.p0(ptr %F3, ptr getelementptr inbounds ([41 x i8], ptr @__intel_dtrans_aostosoa_index, i32 0, i32 0), ptr null, i32 0, ptr null)
; CHECK: %A3 = call ptr @llvm.ptr.annotation.p0.p0(ptr %F3,

  store i32 0, ptr %F3
; CHECK: [[TRUNC1:%[0-9]+]] = trunc i32 0 to i16
; CHECK: store i16 [[TRUNC1]], ptr %F3, align 2

  %I6 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 6
  ret void
}

; Local variable for allocation size of aostosoa call.
; CHECK: [[APTRVAR:%dyn.alloc[0-9]*]] = alloca ptr
; CHECK: [[ASIZEVAR:%dyn.alloc[0-9]*]] = alloca i64

; Save allocation size of  aostosoa call.
; CHECK: %call0 = call ptr @calloc(i64 1000, i64 32)
; CHECK:  store i64 1000, ptr [[ASIZEVAR]]

; CHECK: [[LDASIZE:%dyn.alloc.ld[0-9]*]] = load i64, ptr [[ASIZEVAR]]

; Runtime check with 0xffff
; CHECK: [[OR1:%d.or[0-9]*]] = or i1
; CHECK: [[CMP1:%d.cmp[0-9]*]] = icmp ugt i64 [[LDASIZE]], 65535
; CHECK: [[OR2:%d.or[0-9]*]] = or i1 [[OR1]], [[CMP1]]

; CHECK: store i8 1, ptr @__Shrink__Happened__

; This routine is selected as InitRoutine.
define "intel_dtrans_func_index"="1" ptr @init() !intel.dtrans.func.type !7 {
  %call0 = call ptr @calloc(i64 1000, i64 32)
  %call.ptr = call ptr @llvm.ptr.annotation.p0.p0(ptr %call0, ptr getelementptr inbounds ([38 x i8], ptr @__intel_dtrans_aostosoa_alloc, i32 0, i32 0), ptr null, i32 0, ptr null)
  %C01 = getelementptr i8, ptr %call0, i64 0
  store ptr %C01, ptr getelementptr (%struct.ns, ptr @n, i64 0, i32 0)
  %call1 = tail call ptr @calloc(i64 10, i64 48)
  %tp2 = getelementptr %struct.test.01, ptr %call1, i64 2

; Below instructions are needed to select this routine as InitRoutine and
; increase field frequency of struct.test.01.
  %F1 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, ptr %F1, align 8
  %F6 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 6
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

declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0
declare ptr @llvm.ptr.annotation.p0.p0(ptr, ptr, ptr, i32, ptr)

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test.01 zeroinitializer, i32 2}  ; %struct.test.01**
!6 = !{%struct.test.01 zeroinitializer, i32 1}  ; %struct.test.01*
!7 = distinct !{!6}
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!8}
!10 = !{!"S", %struct.test.01 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !4, !2} ; { i32, i64, i32, i32, i16, i64*, i64 }
!11 = !{!"S", %struct.ns zeroinitializer, i32 4, !4, !5, !5, !4} ; { i64*, %struct.test.01**, %struct.test.01**, i64* }

!intel.dtrans.types = !{!10, !11}
