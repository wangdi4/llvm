; This test verifies that DynClone handles MultiElem load/store instructions
; correctly during transformation when all fields involved in load/store are
; marked with aostosoa index fields and analysis able to detect all fields
; involved (in proc1).

;  RUN: opt < %s -dtransop-allow-typed-pointers -S -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes='internalize,dtrans-dyncloneop' 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1 and 6 fields are shrunk to i16. 2nd and 3rd fields are marked
; with aostosoa index fields.
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

 ; CHECK: define internal void @proc1()

; %L (load) and a store to %M involves multiple fields of %struct.test.01
; (2nd and 3rd), which are marked as aostosoa index fields. Analysis able
; to detect that 2nd and 3rd fields are involved in the load and store
; instructions.
define void @proc1() {
  %call1 = tail call i8* @calloc(i64 10, i64 48)
  %tp2 = bitcast i8* %call1 to %struct.test.01*
  %F2 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 2
; CHECK: [[BC:%[0-9]+]] = bitcast %struct.test.01* %tp2 to %__DYN_struct.test.01*
; CHECK: [[GEP1:%[0-9]+]] = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC]], i32 0, i32 4
; CHECK: %F2 = bitcast i16* [[GEP1]] to i32*

  %A2 = call i32* @llvm.ptr.annotation.p0i32(i32* %F2, i8* getelementptr inbounds ([41 x i8], [41 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* null, i32 0, i8* null)
  %F3 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 3
; CHECK: [[BC2:%[0-9]+]] = bitcast %struct.test.01* %tp2 to %__DYN_struct.test.01*
; CHECK: [[GEP2:%[0-9]+]] = getelementptr %__DYN_struct.test.01, %__DYN_struct.test.01* [[BC2]], i32 0, i32 5
; CHECK: %F3 = bitcast i16* [[GEP2]] to i32*
  %A3 = call i32* @llvm.ptr.annotation.p0i32(i32* %F3, i8* getelementptr inbounds ([41 x i8], [41 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* null, i32 0, i8* null)
  %M = select i1 undef, i32* %F2, i32* %F3
  %L = load i32, i32* %M
; CHECK:  [[BC3:%[0-9]+]] = bitcast i32* %M to i16*
; CHECK: [[LD:%[0-9]+]] = load i16, i16* [[BC3]], align 2
; CHECK: %L = zext i16 [[LD]] to i32
; CHECK-NOT: %L = load i32, i32* %M

  store i32 0, i32* %M
; CHECK:  [[TRUNC:%[0-9]+]] = trunc i32 0 to i16
; CHECK:  [[BC4:%[0-9]+]] = bitcast i32* %M to i16*
; CHECK:  store i16 [[TRUNC]], i16* [[BC4]], align 2
; CHECK-NOT: store i32 0, i32* %M

  %I6 = getelementptr %struct.test.01, %struct.test.01* %tp2, i32 0, i32 6
  ret void
}

; CHECK: store i8 1, i8* @__Shrink__Happened__

; This routine is selected as InitRoutine.
; Uses of Memory allocation pointer (%tp1 and %tp2) are stored in
; 2nd array fields of @n.
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

; CHECK: void @proc1.

declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #0
declare i8* @llvm.ptr.annotation.p0i8(i8*, i8*, i8*, i32, i8*)
declare i32* @llvm.ptr.annotation.p0i32(i32*, i8*, i8*, i32, i8*)

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
