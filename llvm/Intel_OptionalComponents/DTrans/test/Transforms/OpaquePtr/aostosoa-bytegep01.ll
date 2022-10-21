; REQUIRES: asserts
; RUN: opt -S -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true  -debug-only=dtrans-aostosoaop %s | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -S -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true  -debug-only=dtrans-aostosoaop %s | FileCheck %s  --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA transformation for byte-flattened GEP instructions

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, %struct.test01*, i32 }
%struct.test01dep = type { %struct.test01*, i32 }

@var01 = internal global %struct.test01dep zeroinitializer
define i32 @test01() {
  %p0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 0
  %p1 = load %struct.test01*, %struct.test01** %p0

  %flat = bitcast %struct.test01* %p1 to i8*
  %fa0 = getelementptr i8, i8* %flat, i64 0
  %fa0.t = bitcast i8* %fa0 to i32*
  %v0 = load i32, i32* %fa0.t

  %fa1 = getelementptr i8, i8* %flat, i64 8
  %fa1.t = bitcast i8* %fa1 to %struct.test01**
  %v1 = load %struct.test01*, %struct.test01** %fa1.t

  %fa2 = getelementptr i8, i8* %flat, i64 16
  %fa2.t = bitcast i8* %fa2 to i32*
  %v2 = load i32, i32* %fa2.t

  ret i32 0
}
; CHECK-LABEL: define i32 @test01
; CHECK-NONOPAQUE: %[[FADDR0:[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK-NONOPAQUE: %[[LOAD0:[0-9]+]] = load i32*, i32** %[[FADDR0]]
; CHECK-NONOPAQUE: %[[GEP0:[0-9]+]] = getelementptr i32, i32* %[[LOAD0]], i64 %p1
; CHECK-NONOPAQUE: %fa0 = bitcast i32* %[[GEP0]] to i8*
; CHECK-NONOPAQUE: %fa0.t = bitcast i8* %fa0 to i32*
; CHECK-NONOPAQUE: %v0 = load i32, i32* %fa0.t
; CHECK-NONOPAQUE: %[[FADDR1:[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK-NONOPAQUE: %[[LOAD1:[0-9]+]] = load i64*, i64** %[[FADDR1]]
; CHECK-NONOPAQUE: %[[GEP1:[0-9]+]] = getelementptr i64, i64* %[[LOAD1]], i64 %p1
; CHECK-NONOPAQUE: %fa1 = bitcast i64* %[[GEP1]] to i8*
; CHECK-NONOPAQUE: %fa1.t = bitcast i8* %fa1 to i64*
; CHECK-NONOPAQUE: %alloc_idx1 = call i64* @llvm.ptr.annotation.p0i64(i64* %fa1.t, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: %v1 = load i64, i64* %fa1.t
; CHECK-NONOPAQUE: %[[FADDR2:[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK-NONOPAQUE: %[[LOAD2:[0-9]+]] = load i32*, i32** %[[FADDR2]]
; CHECK-NONOPAQUE: %[[GEP2:[0-9]+]] = getelementptr i32, i32* %[[LOAD2]], i64 %p1
; CHECK-NONOPAQUE: %fa2 = bitcast i32* %[[GEP2]] to i8*
; CHECK-NONOPAQUE: %fa2.t = bitcast i8* %fa2 to i32*
; CHECK-NONOPAQUE: %v2 = load i32, i32* %fa2.t

; CHECK-OPAQUE: %[[FADDR0:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 0
; CHECK-OPAQUE: %[[LOAD0:[0-9]+]] = load ptr, ptr %[[FADDR0]]
; CHECK-OPAQUE: %fa0 = getelementptr i32, ptr %[[LOAD0]], i64 %p1
; CHECK-OPAQUE: %fa0.t = bitcast ptr %fa0 to ptr
; CHECK-OPAQUE: %v0 = load i32, ptr %fa0.t
; CHECK-OPAQUE: %[[FADDR1:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 1
; CHECK-OPAQUE: %[[LOAD1:[0-9]+]] = load ptr, ptr %[[FADDR1]]
; CHECK-OPAQUE: %fa1 = getelementptr i64, ptr %[[LOAD1]], i64 %p1
; CHECK-OPAQUE: %fa1.t = bitcast ptr %fa1 to ptr
; CHECK-OPAQUE: %alloc_idx1 = call ptr @llvm.ptr.annotation.p0(ptr %fa1.t, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK-OPAQUE: %v1 = load i64, ptr %fa1.t
; CHECK-OPAQUE: %[[FADDR2:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 2
; CHECK-OPAQUE: %[[LOAD2:[0-9]+]] = load ptr, ptr %[[FADDR2]]
; CHECK-OPAQUE: %fa2 = getelementptr i32, ptr %[[LOAD2]], i64 %p1
; CHECK-OPAQUE: %fa2.t = bitcast ptr %fa2 to ptr
; CHECK-OPAQUE: %v2 = load i32, ptr %fa2.t

!intel.dtrans.types = !{!3, !4}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!4 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !1} ; { %struct.test01*, i32 }

