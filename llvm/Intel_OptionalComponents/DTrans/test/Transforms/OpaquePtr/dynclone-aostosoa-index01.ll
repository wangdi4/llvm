; This test verifies that size of fields that are marked with aostosoa index
; will be reduced to 2 bytes with a runtime check on number of elements
; allocated with aostosoa allocation call when DynClone is triggered (in
; init routine). This also verifies transformations of
; GEP/Load/Store/llvm.ptr.annotation for shrunken aostosoa index
; fields (in proc1 routine).

;  RUN: opt < %s -dtransop-allow-typed-pointers -dtrans-dynclone-shrunken-type-width=16 -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -internalize -dtrans-dyncloneop 2>&1 | FileCheck %s
;  RUN: opt < %s -dtransop-allow-typed-pointers -dtrans-dynclone-shrunken-type-width=16 -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes='internalize,dtrans-dyncloneop' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 2nd and 3rd fields of original struct will be converted to i16.
; They will become 4th and 5th fields after field reorder.
; CHECK: %__DYN_struct.test.01 = type <{ i64*, i64, i32, i16, i16, i16, i16 }>

; 1st field is shrunk to i16. 2nd and 3rd fields are
; marked with aostosoa index fields. Both will be reduced
; to 2 bytes when DynClone is triggered.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

; Type of AOSTOSOA variable.
%struct.ns = type { i64*, %struct.test.01**, %struct.test.01**, i64* }

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
  %call1 = tail call i8* @calloc(i64 10, i64 48)
  %tp2 = bitcast i8* %call1 to %struct.test.01*

  %F2 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 2
; CHECK: [[BC1:%[0-9]+]] = bitcast %struct.test.01* %tp2 to %__DYN_struct.test.01*
; CHECK: [[GEP1:%[0-9]+]] = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC1]], i32 0, i32 4
; CHECK: %F2 = bitcast i16* [[GEP1]] to i32*

  %A2 = call i32* @llvm.ptr.annotation.p0i32(i32* %F2, i8* getelementptr inbounds ([41 x i8], [41 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* null, i32 0, i8* null)
; CHECK: %A2 = call i32* @llvm.ptr.annotation.p0i32(i32* %F2,

  %L1 = load i32, i32* %F2
; CHECK: [[BC2:%[0-9]+]] = bitcast i32* %F2 to i16*
; CHECK: [[LD1:%[0-9]+]] = load i16, i16* [[BC2]], align 2
; CHECK: %L1 = zext i16 [[LD1]] to i32

  %F3 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 3
; CHECK: [[BC3:%[0-9]+]] = bitcast %struct.test.01* %tp2 to %__DYN_struct.test.01*
; CHECK: [[GEP2:%[0-9]+]] = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC3]], i32 0, i32 5
; CHECK: %F3 = bitcast i16* [[GEP2]] to i32*

  %A3 = call i32* @llvm.ptr.annotation.p0i32(i32* %F3, i8* getelementptr inbounds ([41 x i8], [41 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* null, i32 0, i8* null)
; CHECK: %A3 = call i32* @llvm.ptr.annotation.p0i32(i32* %F3,

  store i32 0, i32* %F3
; CHECK: [[TRUNC1:%[0-9]+]] = trunc i32 0 to i16
; CHECK: [[BC4:%[0-9]+]] = bitcast i32* %F3 to i16*
; CHECK: store i16 [[TRUNC1]], i16* [[BC4]], align 2

  %I6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  ret void
}

; Local variable for allocation size of aostosoa call.
; CHECK: [[APTRVAR:%dyn.alloc[0-9]*]] = alloca i8*
; CHECK: [[ASIZEVAR:%dyn.alloc[0-9]*]] = alloca i64

; Save allocation size of  aostosoa call.
; CHECK: %call0 = call i8* @calloc(i64 1000, i64 32)
; CHECK:  store i64 1000, i64* [[ASIZEVAR]]

; CHECK: [[LDASIZE:%dyn.alloc.ld[0-9]*]] = load i64, i64* [[ASIZEVAR]]

; Runtime check with 0xffff
; CHECK: [[OR1:%d.or[0-9]*]] = or i1
; CHECK: [[CMP1:%d.cmp[0-9]*]] = icmp ugt i64 [[LDASIZE]], 65535
; CHECK: [[OR2:%d.or[0-9]*]] = or i1 [[OR1]], [[CMP1]]

; CHECK: store i8 1, i8* @__Shrink__Happened__

; This routine is selected as InitRoutine.
define "intel_dtrans_func_index"="1" %struct.test.01* @init() !intel.dtrans.func.type !7 {
  %call0 = call i8* @calloc(i64 1000, i64 32)
  %call.ptr = call i8* @llvm.ptr.annotation.p0i8(i8* %call0, i8* getelementptr inbounds ([38 x i8], [38 x i8]* @__intel_dtrans_aostosoa_alloc, i32 0, i32 0), i8* null, i32 0, i8* null)
  %C01 = getelementptr i8, i8* %call0, i64 0
  %C02 = bitcast i8* %C01 to i64*
  store i64* %C02, i64** getelementptr (%struct.ns, %struct.ns* @n, i64 0, i32 0)
  %call1 = tail call i8* @calloc(i64 10, i64 48)
  %tp1 = bitcast i8* %call1 to %struct.test.01*
  %tp2 = getelementptr %struct.test.01, %struct.test.01* %tp1, i64 2

; Below instructions are needed to select this routine as InitRoutine and
; increase field frequency of struct.test.01.
  %F1 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 1
  %g1 = select i1 undef, i64 500, i64 1000
  store i64 %g1, i64* %F1, align 8
  %F6 = getelementptr %struct.test.01, %struct.test.01* %tp1, i32 0, i32 6
  %g2 = select i1 undef, i64 -5000, i64 20000
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

declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64)
declare i8* @llvm.ptr.annotation.p0i8(i8*, i8*, i8*, i32, i8*)
declare i32* @llvm.ptr.annotation.p0i32(i32*, i8*, i8*, i32, i8*)

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
