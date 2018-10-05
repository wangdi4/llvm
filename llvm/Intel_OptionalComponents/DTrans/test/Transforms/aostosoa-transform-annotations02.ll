; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=true -dtrans-aostosoa-heur-override=struct.test01,struct.test02 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=true -dtrans-aostosoa-heur-override=struct.test01,struct.test02 2>&1 | FileCheck %s

; Test AOS-to-SOA transformation insertion of annotations for the dynamic
; cloning pass to identify the memory allocation of the structure of arrays,
; and the memory locations that correspond to the peeling index. This test
; verifies that separate annotation variables are used when there are
; multiple structures being transformed.

; CHECK: @__intel_dtrans_aostosoa_alloc = private constant [38 x i8] c"{dtrans} AOS-to-SOA allocation {id:0}\00"
; CHECK: @__intel_dtrans_aostosoa_index = private constant [41 x i8] c"{dtrans} AOS-to-SOA peeling index {id:0}\00"
; CHECK: @__intel_dtrans_aostosoa_alloc.1 = private constant [38 x i8] c"{dtrans} AOS-to-SOA allocation {id:1}\00"
; CHECK: @__intel_dtrans_aostosoa_index.1 = private constant [41 x i8] c"{dtrans} AOS-to-SOA peeling index {id:1}\00"

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { %struct.test02*, %struct.test01dep* }
%struct.test01dep = type { %struct.test01*, %struct.test02* }
%struct.test02 = type { %struct.test01*, %struct.test01dep* }

@g_test01depptr = internal unnamed_addr global %struct.test01dep zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01(i64 15)
  call void @test02(i64 15)
  ret i32 0
}

; Verify annotation gets placed on the converted allocation call
define internal void @test01(i64 %num) {
; CHECK-LABEL: define internal void @test01

  ; Allocate %num elements. (structure size = 16)
  %size = mul nsw i64 %num, 16
  %mem = call i8* @malloc(i64 %size)
; CHECK:   call i8* @llvm.ptr.annotation.p0i8(i8* %mem
; CHECK-SAME: @__intel_dtrans_aostosoa_alloc,

  store i8* %mem, i8** bitcast (%struct.test01** getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 0)  to i8**)
  ret void
}

define internal void @test02(i64 %num) {
; CHECK-LABEL: define internal void @test02

  ; Allocate %num elements. (structure size = 16)
  %size = mul nsw i64 %num, 16
  %mem = call i8* @malloc(i64 %size)
; CHECK:   call i8* @llvm.ptr.annotation.p0i8(i8* %mem
; CHECK-SAME: @__intel_dtrans_aostosoa_alloc.1,

  bitcast i8* %mem to %struct.test02*
  ret void
}

; Verify that annotation is put on the pointers used to access the
; peeling index
define internal void @test03() {
; CHECK-LABEL: define internal void @test03

  ; test with constant object GEP
; CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @g_test01depptr
; CHECK-SAME: __intel_dtrans_aostosoa_index,
  %ptr1_to_st01 = load %struct.test01*, %struct.test01** getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 0)

  ; test with a field from the peeled structure
  %ptr1_to_st02 = getelementptr %struct.test01, %struct.test01* %ptr1_to_st01, i64 0, i32 0
; CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* %ptr1_to_st02
; CHECK-SAME: @__intel_dtrans_aostosoa_index.1,

  %st02 = load %struct.test02*, %struct.test02** %ptr1_to_st02
  %ptr2_to_st01 = getelementptr %struct.test02, %struct.test02* %st02, i64 0, i32 0
; CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* %ptr2_to_st01
; CHECK-SAME: @__intel_dtrans_aostosoa_index,

  %st01 = load %struct.test01*, %struct.test01** %ptr2_to_st01

  ; test with a field from a dependent struture
  %ptr3_to_st01 = getelementptr %struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 0
; CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* %ptr3_to_st01
; CHECK-SAME: @__intel_dtrans_aostosoa_index,

  store %struct.test01* %st01, %struct.test01** %ptr3_to_st01

  ret void
}

declare i8* @malloc(i64)
