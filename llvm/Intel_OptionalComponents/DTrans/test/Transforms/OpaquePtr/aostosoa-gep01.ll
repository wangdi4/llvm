; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -S -whole-program-assume -dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -dtransop-allow-typed-pointers -S -whole-program-assume -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -S -whole-program-assume -dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -S -whole-program-assume -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test AOS-to-SOA transformation for GEP instructions

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, %struct.test01*, i32 }
%struct.test01dep = type { %struct.test01*, i32 }
; CHECK-NONOPAQUE-DAG: %__SOA_struct.test01 = type { i32*, i64*, i32* }
; CHECK-NONOPAQUE-DAG: %__SOADT_struct.test01dep = type { i64, i32 }

; CHECK-OPAQUE-DAG: %__SOA_struct.test01 = type { ptr, ptr, ptr }
; CHECK-OPAQUE-DAG: %__SOADT_struct.test01dep = type { i64, i32 }

@var01 = internal global %struct.test01dep zeroinitializer
define i32 @test01() {
  %p0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 0
  %p1 = load %struct.test01*, %struct.test01** %p0
  %p2 = getelementptr %struct.test01, %struct.test01* %p1, i64 0, i32 2
  %v = load i32, i32* %p2
  ret i32 0
}
; CHECK-LABEL: define i32 @test01
; CHECK-NONOPAQUE: %p0 = getelementptr %__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @var01, i64 0, i32 0
; CHECK-NONOPAQUE: %alloc_idx = call i64* @llvm.ptr.annotation.p0i64(i64* %p0, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: %p1 = load i64, i64* %p0
; CHECK-NONOPAQUE: %[[GEP1:[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK-NONOPAQUE: %[[LOAD1:[0-9]+]] = load i32*, i32** %[[GEP1]]
; CHECK-NONOPAQUE-SAME: !invariant.load
; CHECK-NONOPAQUE: %p2 = getelementptr i32, i32* %[[LOAD1]], i64 %p1
; CHECK-NONOPAQUE: %v = load i32, i32* %p2
; CHECK-NONOPAQUE: ret i32 0

; CHECK-OPAQUE: %p0 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 0
; CHECK-OPAQUE: %alloc_idx = call ptr @llvm.ptr.annotation.p0(ptr %p0, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK-OPAQUE: %p1 = load i64, ptr %p0
; CHECK-OPAQUE: %[[GEP1:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 2
; CHECK-OPAQUE: %[[LOAD1:[0-9]+]] = load ptr, ptr %[[GEP1]]
; CHECK-OPAQUE-SAME: !invariant.load
; CHECK-OPAQUE: %p2 = getelementptr i32, ptr %[[LOAD1]], i64 %p1
; CHECK-OPAQUE: %v = load i32, ptr %p2
; CHECK-OPAQUE: ret i32 0

; Use a single index GEP on the type being transformed.
define i32 @test02(i64 %n) {
  %p0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 0
  %p1 = load %struct.test01*, %struct.test01** %p0
  %p2 = getelementptr %struct.test01, %struct.test01* %p1, i64 %n
  %p3 = getelementptr %struct.test01, %struct.test01* %p2, i64 0, i32 2
  %v = load i32, i32* %p3
  ret i32 0
}

; CHECK-LABEL: define i32 @test02
; CHECK-NONOPAQUE: %p0 = getelementptr %__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @var01, i64 0, i32 0
; CHECK-NONOPAQUE: %alloc_idx = call i64* @llvm.ptr.annotation.p0i64(i64* %p0, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: %p1 = load i64, i64* %p0
; CHECK-NONOPAQUE: %p2 = add i64 %p1, %n
; CHECK-NONOPAQUE: %[[GEP1:[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK-NONOPAQUE: %[[LOAD1:[0-9]+]] = load i32*, i32** %[[GEP1]]
; CHECK-NONOPAQUE-SAME: !invariant.load
; CHECK-NONOPAQUE: %p3 = getelementptr i32, i32* %[[LOAD1]], i64 %p2
; CHECK-NONOPAQUE: %v = load i32, i32* %p3
; CHECK-NONOPAQUE: ret i32 0

; CHECK-OPAQUE: %p0 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 0
; CHECK-OPAQUE: %alloc_idx = call ptr @llvm.ptr.annotation.p0(ptr %p0, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK-OPAQUE: %p1 = load i64, ptr %p0
; CHECK-OPAQUE: %p2 = add i64 %p1, %n
; CHECK-OPAQUE: %[[GEP1:[0-9]+]]  = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 2
; CHECK-OPAQUE: %[[LOAD1:[0-9]+]] = load ptr, ptr %[[GEP1]]
; CHECK-OPAQUE-SAME: !invariant.load
; CHECK-OPAQUE: %p3 = getelementptr i32, ptr %[[LOAD1]], i64 %p2
; CHECK-OPAQUE: %v = load i32, ptr %p3
; CHECK-OPAQUE: ret i32 0

; Use a variable for the first GEP index on the type being transformed.
define i32 @test03(i64 %n) {
  %p0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 0
  %p1 = load %struct.test01*, %struct.test01** %p0
  %p2 = getelementptr %struct.test01, %struct.test01* %p1, i64 %n, i32 2
  %v = load i32, i32* %p2
  ret i32 0
}

; CHECK-LABEL: define i32 @test03

; CHECK-NONOPAQUE: %p0 = getelementptr %__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @var01, i64 0, i32 0
; CHECK-NONOPAQUE: %alloc_idx = call i64* @llvm.ptr.annotation.p0i64(i64* %p0, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: %p1 = load i64, i64* %p0
; CHECK-NONOPAQUE: %[[GEP1:[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK-NONOPAQUE: %[[LOAD1:[0-9]+]] = load i32*, i32** %[[GEP1]]
; CHECK-NONOPAQUE-SAME: !invariant.load
; CHECK-NONOPAQUE: %[[ADD1:[0-9]+]] = add i64 %p1, %n
; CHECK-NONOPAQUE: %p2 = getelementptr i32, i32* %[[LOAD1]], i64 %[[ADD1]]
; CHECK-NONOPAQUE: %v = load i32, i32* %p2
; CHECK-NONOPAQUE: ret i32 0

; CHECK-OPAQUE: %p0 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 0
; CHECK-OPAQUE: %alloc_idx = call ptr @llvm.ptr.annotation.p0(ptr %p0, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK-OPAQUE: %p1 = load i64, ptr %p0
; CHECK-OPAQUE: %[[GEP1:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 2
; CHECK-OPAQUE: %[[LOAD1:[0-9]+]] = load ptr, ptr %[[GEP1]]
; CHECK-OPAQUE-SAME: !invariant.load
; CHECK-OPAQUE: %[[ADD1:[0-9]+]] = add i64 %p1, %n
; CHECK-OPAQUE: %p2 = getelementptr i32, ptr %[[LOAD1]], i64 %[[ADD1]]
; CHECK-OPAQUE: %v = load i32, ptr %p2
; CHECK-OPAQUE: ret i32 0


!intel.dtrans.types = !{!3, !4}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!4 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !1} ; { %struct.test01*, i32 }

