; This test verifies that pointers of shrunken struct, which are stored in
; AOSTOSOA global variable, are rematerialized correctly when DynClone is
; triggered.

; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dynclone 2>&1 | FileCheck %s
; RUN: opt < %s -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dynclone 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Pointers of %struct.test.01 are stored in 2nd array field of @n
; variable, which is AOSTOSOA global. Here, we check a loop is
; generated to rematerialize pointers of %struct.test.01 that are
; stored in 2nd array field of @n.

; Local variable to hold return value of calloc, which
; allocates memory for %struct.test.01
; CHECK: [[APTRVAR:%dyn.alloc[0-9]*]] = alloca i8*

; Local Variable to hold size of AOSTOSOA array.
; CHECK: [[AOSSIZEVAR:%dyn.alloc[0-9]*]] = alloca i64

; Storing size of AOSTOSOA array to local var.
; CHECK: %call0 = call noalias i8* @calloc(i64 1000, i64 32)
; CHECK: store i64 1000, i64* [[AOSSIZEVAR]]

; Storing return ptr of alloc call for %struct.test.01 to local var.
; CHECK: %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
; CHECK: store i8* %call1, i8** [[APTRVAR]]

; Getting array size of AOSTOSOA alloc.
; CHECK: [[AOSSIZE:%dyn.alloc.ld[0-9]*]] = load i64, i64* [[AOSSIZEVAR]]

; Getting return pointer of alloc call for %struct.test.01.
; CHECK:   [[APTR:%dyn.alloc.ld[0-9]*]] = load i8*, i8** [[APTRVAR]]

; Pre-loop:
; Load of 2nd field address of @n.
; CHECK: [[L1:%[0-9]+]] = load %struct.test.01**, %struct.test.01*** getelementptr inbounds (%struct.ns, %struct.ns* @n, i64 0, i32 1)

; Loop begin
; CHECK: [[LIndex:%rematidx[0-9]*]] = phi i64 [ 0,
; Load each element from 2nd array field of @n
; CHECK: [[GEP1:%[0-9]+]] = getelementptr inbounds %struct.test.01*, %struct.test.01** [[L1]], i64 [[LIndex]]
;CHECK: [[LOAD1:%[0-9]+]] = load %struct.test.01*, %struct.test.01** [[GEP1]]

; Rematerialize pointer if it is not null.
; CHECK: [[CMP1:%[0-9]+]] = icmp ne %struct.test.01* [[LOAD1]], null
; CHECK: br i1 [[CMP1]],
; CHECK:  [[PTR1:%[0-9]+]] = ptrtoint %struct.test.01* [[LOAD1]] to i64
; CHECK:  [[PTR2:%[0-9]+]] = ptrtoint i8* [[APTR]] to i64
; CHECK:  [[DIFF1:%[0-9]+]] = sub i64 [[PTR1]], [[PTR2]]
; CHECK:  [[IDX1:%[0-9]+]] = sdiv i64 [[DIFF1]], 48
; CHECK:  [[BC1:%[0-9]+]] = bitcast i8* [[APTR]] to %__DYN_struct.test.01*
; CHECK:  [[GEP2:%[0-9]+]] = getelementptr inbounds %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC1]], i64 [[IDX1]]
; CHECK:  [[BC2:%[0-9]+]] = bitcast %__DYN_struct.test.01* [[GEP2]] to %struct.test.01*
; CHECK:  store %struct.test.01* [[BC2]], %struct.test.01** [[GEP1]]

; Increment loop index and check against loop count.
; CHECK:  [[INC1:%[0-9]+]] = add i64 [[LIndex]], 1
; CHECK:  [[CMP2:%[0-9]+]] = icmp ult i64 [[INC1]], [[AOSSIZE]]
; CHECK:  br i1 [[CMP2]],

; CHECK: store i8 1, i8* @__Shrink__Happened__

; 1 and 6 fields are shrunk to i16.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

; Type of AOSTOSOA variable.
%struct.ns = type { i64*, %struct.test.01**, %struct.test.01**, i64* }

; Memory allocation pointers, which are returned by calloc in "init" routine,
; are stored in 2nd and 3rd array fields of @n.
@n = internal global %struct.ns zeroinitializer, align 8

; AOSTOSOA allocation call is marked with this annotation.
@__intel_dtrans_aostosoa_alloc = private constant [38 x i8] c"{dtrans} AOS-to-SOA allocation {id:0}\00"

; This routine is selected as InitRoutine.
; Uses of Memory allocation pointer (%tp1 and %tp2) are stored in
; 2nd array fields of @n.
define %struct.test.01* @init() {
  %call0 = call noalias i8* @calloc(i64 1000, i64 32)
  %call.ptr = call i8* @llvm.ptr.annotation.p0i8(i8* %call0, i8* getelementptr inbounds ([38 x i8], [38 x i8]* @__intel_dtrans_aostosoa_alloc, i32 0, i32 0), i8* null, i32 0, i8* null)
  %C01 = getelementptr i8, i8* %call0, i64 0
  %C02 = bitcast i8* %C01 to i64*
  store i64* %C02, i64** getelementptr (%struct.ns, %struct.ns* @n, i64 0, i32 0)
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %tp2 = getelementptr %struct.test.01, %struct.test.01* %tp1, i64 2

; Store %tp1 in 2nd array field of @n
  %N7 = getelementptr %struct.ns, %struct.ns* @n, i64 0, i32 1
  %LN7 = load %struct.test.01**, %struct.test.01*** %N7
  %PN7 = getelementptr %struct.test.01*, %struct.test.01** %LN7, i64 1
  %BC1 = bitcast %struct.test.01** %PN7 to i64*
  %NLD1 = load i64, i64* %BC1
  store %struct.test.01* %tp1, %struct.test.01** %PN7

; Store %tp2 in 2nd array field of @n
  %N8 = getelementptr %struct.ns, %struct.ns* @n, i64 0, i32 1
  %LN8 = load %struct.test.01**, %struct.test.01*** %N8
  %PN8 = getelementptr %struct.test.01*, %struct.test.01** %LN8, i64 1
  %BC2 = bitcast %struct.test.01** %PN8 to i64*
  %NLD2 = load i64, i64* %BC2
  store %struct.test.01* %tp2, %struct.test.01** %PN8

; Below instructions are needed to select this routine as InitRoutine and
; increase field frequency of struct.test.01.
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  store i64 %g1, i64* %F1, align 8
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
  store i64 %g2, i64* %F6, align 8
  store i64 %g2, i64* %F6, align 8
  store i64 %g2, i64* %F6, align 8

  ret %struct.test.01* null
}

; Call to "init" routine is qualified as InitRoutine for DynClone.
define i32 @main() {
entry:
  %tp2 = call %struct.test.01* @init();
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
declare dso_local noalias i8* @llvm.ptr.annotation.p0i8(i8*, i8*, i8*, i32, i8*)
