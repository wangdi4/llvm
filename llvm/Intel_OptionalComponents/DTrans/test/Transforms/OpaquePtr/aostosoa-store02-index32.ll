; REQUIRES: asserts
; RUN: opt -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test AOS-to-SOA transformation of storing a null pointer to the type being
; transformed by AOS-to-SOA. When opaque pointers are used, there will only be a
; single constant value for 'null' within a function that can represent any
; pointer type. We want to make sure that only specific store instructions get
; changed to be integers.
;
; This case is the same as aostosoa-store02.ll, but is using 32-bit integers for
; the index type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, %struct.test01*, i32 }
%struct.test01dep = type { %struct.test01dep*, %struct.test01* }

@var01 = internal global %struct.test01dep zeroinitializer
define i32 @test01() {
  %p0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 0
  %p1 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 1

  ; This store should stay as storing a pointer null
  store %struct.test01dep* null, %struct.test01dep** %p0

  ; This store should be changed to store the integer zero
  store %struct.test01* null, %struct.test01** %p1
  ret i32 0
}
; CHECK-LABEL: define i32 @test01
; CHECK-NONOPAQUE: %p0 = getelementptr %__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @var01, i64 0, i32 0
; CHECK-NONOPAQUE: %p1 = getelementptr %__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @var01, i64 0, i32 1
; CHECK-NONOPAQUE: store %__SOADT_struct.test01dep* null, %__SOADT_struct.test01dep** %p0
; CHECK-NONOPAQUE: call i32* @llvm.ptr.annotation.p0i32(i32* %p1, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: store i32 0, i32* %p1
; CHECK-NONOPAQUE: ret i32 0

; CHECK-OPAQUE: %p0 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 0
; CHECK-OPAQUE: %p1 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 1
; CHECK-OPAQUE: store ptr null, ptr %p0
; CHECK-OPAQUE: call ptr @llvm.ptr.annotation.p0(ptr %p1, ptr getelementptr inbounds ([33 x i8], ptr @__intel_dtrans_aostosoa_index, i32 0, i32 0), ptr getelementptr inbounds ([1 x i8], ptr @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, ptr null)
; CHECK-OPAQUE: store i32 0, ptr %p1
; CHECK-OPAQUE: ret i32 0

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{%struct.test01dep zeroinitializer, i32 1}  ; %struct.test01dep*
!4 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!5 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !3, !2} ; { %struct.test01dep*, %struct.test01* }

!intel.dtrans.types = !{!4, !5}
