; REQUIRES: asserts
; RUN: opt -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test AOS-to-SOA transformation of storing a pointer to the type being
; transformed by AOS-to-SOA.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, %struct.test01*, i32 }
%struct.test01dep = type { %struct.test01*, %struct.test01* }

; Check the transformation when the field is within a dependent type.
@var01 = internal global %struct.test01dep zeroinitializer
define i32 @test01() {
  %p0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 0
  %p1 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 1
  %v0 = load %struct.test01*, %struct.test01** %p0
  %v1 = load %struct.test01*, %struct.test01** %p1
  store %struct.test01* %v0, %struct.test01** %p1
  store %struct.test01* %v1, %struct.test01** %p0
  ret i32 0
}
; CHECK-NONOPAQUE: %p0 = getelementptr %__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @var01, i64 0, i32 0
; CHECK-NONOPAQUE: %p1 = getelementptr %__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @var01, i64 0, i32 1
; CHECK-NONOPAQUE: call i64* @llvm.ptr.annotation.p0i64(i64* %p0, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: %v0 = load i64, i64* %p0
; CHECK-NONOPAQUE: call i64* @llvm.ptr.annotation.p0i64(i64* %p1, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: %v1 = load i64, i64* %p1
; CHECK-NONOPAQUE: call i64* @llvm.ptr.annotation.p0i64(i64* %p1, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: store i64 %v0, i64* %p1
; CHECK-NONOPAQUE: call i64* @llvm.ptr.annotation.p0i64(i64* %p0, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: store i64 %v1, i64* %p0
; CHECK-NONOPAQUE: ret i32 0

; CHECK-OPAQUE: %p0 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 0
; CHECK-OPAQUE: %p1 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 1
; CHECK-OPAQUE: call ptr @llvm.ptr.annotation.p0(ptr %p0, ptr getelementptr inbounds ([33 x i8], ptr @__intel_dtrans_aostosoa_index, i32 0, i32 0), ptr getelementptr inbounds ([1 x i8], ptr @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, ptr null)
; CHECK-OPAQUE: %v0 = load i64, ptr %p0
; CHECK-OPAQUE: call ptr @llvm.ptr.annotation.p0(ptr %p1, ptr getelementptr inbounds ([33 x i8], ptr @__intel_dtrans_aostosoa_index, i32 0, i32 0), ptr getelementptr inbounds ([1 x i8], ptr @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, ptr null)
; CHECK-OPAQUE: %v1 = load i64, ptr %p1
; CHECK-OPAQUE: call ptr @llvm.ptr.annotation.p0(ptr %p1, ptr getelementptr inbounds ([33 x i8], ptr @__intel_dtrans_aostosoa_index, i32 0, i32 0), ptr getelementptr inbounds ([1 x i8], ptr @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, ptr null)
; CHECK-OPAQUE: store i64 %v0, ptr %p1
; CHECK-OPAQUE: call ptr @llvm.ptr.annotation.p0(ptr %p0, ptr getelementptr inbounds ([33 x i8], ptr @__intel_dtrans_aostosoa_index, i32 0, i32 0), ptr getelementptr inbounds ([1 x i8], ptr @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, ptr null)
; CHECK-OPAQUE: store i64 %v1, ptr %p0
; CHECK-OPAQUE: ret i32 0

; Check the transformation when the field is a member of the type being transformed.
define i32 @test02() {
  %p0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 0
  %v0 = load %struct.test01*, %struct.test01** %p0

  %t0 = getelementptr %struct.test01, %struct.test01* %v0, i64 0, i32 1
  store %struct.test01* %v0, %struct.test01** %t0
  ret i32 0
}
; CHECK-LABEL: define i32 @test02
; CHECK-NONOPAQUE: %p0 = getelementptr %__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @var01, i64 0, i32 0
; CHECK-NONOPAQUE: call i64* @llvm.ptr.annotation.p0i64(i64* %p0, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: %v0 = load i64, i64* %p0
; CHECK-NONOPAQUE: [[ADDR1:%[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK-NONOPAQUE: [[BASE1:%[0-9]+]] = load i64*, i64** [[ADDR1]]
; CHECK-NONOPAQUE-SAME: !invariant.load
; CHECK-NONOPAQUE: %t0 = getelementptr i64, i64* [[BASE1]], i64 %v0
; CHECK-NONOPAQUE: call i64* @llvm.ptr.annotation.p0i64(i64* %t0, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: store i64 %v0, i64* %t0
; CHECK-NONOPAQUE: ret i32 0

; CHECK-OPAQUE: %p0 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 0
; CHECK-OPAQUE: call ptr @llvm.ptr.annotation.p0(ptr %p0, ptr getelementptr inbounds ([33 x i8], ptr @__intel_dtrans_aostosoa_index, i32 0, i32 0), ptr getelementptr inbounds ([1 x i8], ptr @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, ptr null)
; CHECK-OPAQUE: %v0 = load i64, ptr %p0
; CHECK-OPAQUE: [[ADDR1:%[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 1
; CHECK-OPAQUE: [[BASE1:%[0-9]+]] = load ptr, ptr [[ADDR1]]
; CHECK-OPAQUE-SAME: !invariant.load
; CHECK-OPAQUE: %t0 = getelementptr i64, ptr [[BASE1]], i64 %v0
; CHECK-OPAQUE: call ptr @llvm.ptr.annotation.p0(ptr %t0, ptr getelementptr inbounds ([33 x i8], ptr @__intel_dtrans_aostosoa_index, i32 0, i32 0), ptr getelementptr inbounds ([1 x i8], ptr @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, ptr null)
; CHECK-OPAQUE: store i64 %v0, ptr %t0
; CHECK-OPAQUE: ret i32 0

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!4 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !2} ; { %struct.test01*, %struct.test01* }

!intel.dtrans.types = !{!3, !4}
