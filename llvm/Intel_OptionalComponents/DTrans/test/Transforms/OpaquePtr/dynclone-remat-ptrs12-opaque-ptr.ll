; This test verifies that pointers of shrunken struct, which are stored in
; AOSTOSOA global variable, are rematerialized correctly when DynClone is
; triggered.

;  RUN: opt < %s -opaque-pointers -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -dtrans-dyncloneop 2>&1 | FileCheck %s
;  RUN: opt < %s -opaque-pointers -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Pointers of %struct.test.01 are stored in 2nd array field of @n
; variable, which is AOSTOSOA global. Here, we check a loop is
; generated to rematerialize pointers of %struct.test.01 that are
; stored in 2nd array field of @n.

; Local variable to hold return value of calloc, which
; allocates memory for %struct.test.01
; CHECK: [[APTRVAR:%dyn.alloc[0-9]*]] = alloca ptr

; Local Variable to hold size of AOSTOSOA array.
; CHECK: [[AOSSIZEVAR:%dyn.alloc[0-9]*]] = alloca i64

; Storing size of AOSTOSOA array to local var.
; CHECK: %call0 = call ptr @calloc(i64 1000, i64 32)
; CHECK: store i64 1000, ptr [[AOSSIZEVAR]]

; Storing return ptr of alloc call for %struct.test.01 to local var.
; CHECK: %tp1 = tail call ptr @calloc(i64 10, i64 48)
; CHECK: store ptr %tp1, ptr [[APTRVAR]]

; Getting array size of AOSTOSOA alloc.
; CHECK: [[AOSSIZE:%dyn.alloc.ld[0-9]*]] = load i64, ptr [[AOSSIZEVAR]]

; Getting return pointer of alloc call for %struct.test.01.
; CHECK:   [[APTR:%dyn.alloc.ld[0-9]*]] = load ptr, ptr [[APTRVAR]]

; Pre-loop:
; Load of 3rd field address of @n.
; CHECK: [[L1:%[0-9]+]] = load ptr, ptr getelementptr inbounds (%struct.ns, ptr @n, i64 0, i32 2)

; Loop begin
; CHECK: [[LIndex:%rematidx[0-9]*]] = phi i64 [ 0,
; Load each element from 2nd array field of @n
; CHECK: [[GEP1:%[0-9]+]] = getelementptr inbounds ptr, ptr [[L1]], i64 [[LIndex]]
; CHECK: [[LOAD1:%[0-9]+]] = load ptr, ptr [[GEP1]]

; Rematerialize pointer if it is not null.
; CHECK: [[CMP1:%[0-9]+]] = icmp ne ptr [[LOAD1]], null
; CHECK: br i1 [[CMP1]],
; CHECK:  [[PTR1:%[0-9]+]] = ptrtoint ptr [[LOAD1]] to i64
; CHECK:  [[PTR2:%[0-9]+]] = ptrtoint ptr [[APTR]] to i64
; CHECK:  [[DIFF1:%[0-9]+]] = sub i64 [[PTR1]], [[PTR2]]
; CHECK:  [[IDX1:%[0-9]+]] = sdiv i64 [[DIFF1]], 48
; CHECK:  [[GEP2:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, ptr [[APTR]], i64 [[IDX1]]
; CHECK:  store ptr [[GEP2]], ptr [[GEP1]]

; Increment loop index and check against loop count.
; CHECK:  [[INC1:%[0-9]+]] = add i64 [[LIndex]], 1
; CHECK:  [[CMP2:%[0-9]+]] = icmp ult i64 [[INC1]], [[AOSSIZE]]
; CHECK:  br i1 [[CMP2]],

; CHECK: store i8 1, ptr @__Shrink__Happened__

; 1 and 6 fields are shrunk to i16.
%struct.test.01 = type { i32, i64, i32, i32, i16, ptr, i64 }

; Type of AOSTOSOA variable.
;   type { i32, i64*, %struct.test.01**, %struct.test.01**, i64* }
; Field 1
%struct.ns = type { i32, ptr, ptr, ptr, ptr }

; Memory allocation pointers, which are returned by calloc in "init" routine,
; are stored in 2nd and 3rd array fields of @n.
@n = internal global %struct.ns zeroinitializer, align 8

; AOSTOSOA allocation call is marked with this annotation.
@__intel_dtrans_aostosoa_alloc = private constant [38 x i8] c"{dtrans} AOS-to-SOA allocation {id:0}\00"

; This routine is selected as InitRoutine.
; Uses of Memory allocation pointer (%tp1 and %tp2) are stored in
; 2nd array fields of @n.
define "intel_dtrans_func_index"="1" ptr @init() !intel.dtrans.func.type !7 {
  %call0 = call ptr @calloc(i64 1000, i64 32)
  %call.ptr = call ptr @llvm.ptr.annotation.p0(ptr %call0, ptr getelementptr inbounds ([38 x i8], [38 x i8]* @__intel_dtrans_aostosoa_alloc, i32 0, i32 0), ptr null, i32 0, ptr null)
  %C01 = getelementptr i8, ptr %call0, i64 0
  ; DynClone uses pattern match to find the GEPOperator as the pointer parameter
  ; of the store. Use index 1 to avoid the GEP removal.
  store ptr %C01, ptr getelementptr (%struct.ns, ptr @n, i64 0, i32 1)
  %tp1 = tail call ptr @calloc(i64 10, i64 48)
  %tp2 = getelementptr %struct.test.01, ptr %tp1, i64 2

; Store %tp1 in the 3rd field of @n
  %N7 = getelementptr %struct.ns, ptr @n, i64 0, i32 2
  %LN7 = load ptr, ptr %N7
  %PN7 = getelementptr ptr, ptr %LN7, i64 1
  %NLD1 = load i64, ptr %PN7
  store ptr %tp1, ptr %PN7

; Store %tp2 in 3rd field of @n
  %N8 = getelementptr %struct.ns, ptr @n, i64 0, i32 2
  %LN8 = load ptr, ptr %N8
  %PN8 = getelementptr ptr, ptr %LN8, i64 1
  %NLD2 = load i64, ptr %PN8
  store ptr %tp2, ptr %PN8

; Below instructions are needed to select this routine as InitRoutine and
; increase field frequency of struct.test.01.
  %F1 = getelementptr %struct.test.01, ptr %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, ptr %F1, align 8
  store i64 %g1, ptr %F1, align 8
  store i64 %g1, ptr %F1, align 8
  %F6 = getelementptr %struct.test.01, ptr %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, ptr %F6, align 8
  store i64 %g2, ptr %F6, align 8
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

; This routine just accesses candidate field.
define void @proc1() {
  %call1 = tail call ptr @calloc(i64 10, i64 48)
  %F6 = getelementptr %struct.test.01, ptr %call1, i32 0, i32 6
  ret void
}

declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0
declare ptr @llvm.ptr.annotation.p0(ptr, ptr, ptr, i32, ptr)

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
!11 = !{!"S", %struct.ns zeroinitializer, i32 5, !1, !4, !5, !5, !4} ; { i32, i64*, %struct.test.01**, %struct.test.01**, i64* }

!intel.dtrans.types = !{!10, !11}
