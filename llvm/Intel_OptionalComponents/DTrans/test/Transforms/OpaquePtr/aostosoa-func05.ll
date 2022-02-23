; REQUIRES: asserts
; RUN: opt -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test AOS-to-SOA transformation that function attributes that are not
; permitted on the transformed type get removed. This is the same as
; aostosoa-func01.ll, but contains attributes that are valid for pointer
; parameters, that will not be valid for integer parameters.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, %struct.test01*, i32 }
%struct.test01dep = type { %struct.test01*, %struct.test01* }

@var01 = internal global %struct.test01dep zeroinitializer
define i32 @test01() {
  %p0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 0
  %v0 = load %struct.test01*, %struct.test01** %p0
  call void @test02(%struct.test01* nonnull nocapture %v0)
  ret i32 0
}
; CHECK-LABEL: define i32 @test01()
; CHECK-NONOPAQUE: %p0 = getelementptr %__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @var01, i64 0, i32 0
; CHECK-NONOPAQUE: call i64* @llvm.ptr.annotation.p0i64(i64* %p0, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: %v0 = load i64, i64* %p0, align 8
; CHECK-NONOPAQUE: call void @test02.1(i64 %v0)
; CHECK-NONOPAQUE: ret i32 0

; CHECK-OPAQUE: %p0 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 0
; CHECK-OPAQUE: call ptr @llvm.ptr.annotation.p0(ptr %p0, ptr getelementptr inbounds ([33 x i8], ptr @__intel_dtrans_aostosoa_index, i32 0, i32 0), ptr getelementptr inbounds ([1 x i8], ptr @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, ptr null)
; CHECK-OPAQUE: %v0 = load i64, ptr %p0, align 8
; CHECK-OPAQUE: call void @test02.1(i64 %v0)
; CHECK-OPAQUE: ret i32 0

define void @test02(%struct.test01* "intel_dtrans_func_index"="1" nonnull nocapture %in) !intel.dtrans.func.type !3 {
  %p1 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 1
  store  %struct.test01* %in, %struct.test01** %p1
  ret void
}
; The function should be cloned because the parameter type is changing.
; CHECK-LABEL: define void @test02.1(i64 %in)
; CHECK-NONOPAQUE: %p1 = getelementptr %__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @var01, i64 0, i32 1
; CHECK-NONOPAQUE: call i64* @llvm.ptr.annotation.p0i64(i64* %p1, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: store i64 %in, i64* %p1, align 8
; CHECK-NONOPAQUE: ret void

; CHECK-OPAQUE: %p1 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 1
; CHECK-OPAQUE: call ptr @llvm.ptr.annotation.p0(ptr %p1, ptr getelementptr inbounds ([33 x i8], ptr @__intel_dtrans_aostosoa_index, i32 0, i32 0), ptr getelementptr inbounds ([1 x i8], ptr @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, ptr null)
; CHECK-OPAQUE: store i64 %in, ptr %p1, align 8
; CHECK-OPAQUE: ret void

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!5 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !2} ; { %struct.test01*, %struct.test01* }

!intel.dtrans.types = !{!4, !5}
