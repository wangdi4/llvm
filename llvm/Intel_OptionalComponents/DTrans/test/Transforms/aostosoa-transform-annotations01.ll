; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=true -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=true -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; Test AOS-to-SOA transformation insertion of annotations for the dynamic
; cloning pass to identify the memory allocation of the structure of arrays,
; and the memory locations that correspond to the peeling index.

; CHECK: @__intel_dtrans_aostosoa_alloc = private constant [38 x i8] c"{dtrans} AOS-to-SOA allocation {id:0}\00"
; CHECK: @__intel_dtrans_aostosoa_index = private constant [41 x i8] c"{dtrans} AOS-to-SOA peeling index {id:0}\00"

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { %struct.test01*, %struct.test01dep* }
%struct.test01dep = type { %struct.test01* }
@g_test01depptr = internal unnamed_addr global %struct.test01dep zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01(i64 15)
  call void @test02()
  ret i32 0
}

; Verify annotation gets placed on the converted allocation call
define internal void @test01(i64 %num) {
; CHECK-LABEL: define internal void @test01

  ; Allocate %num elements. (structure size = 16)
  %size = mul nsw i64 %num, 16
  %mem = call i8* @malloc(i64 %size)
; CHECK:   call i8* @llvm.ptr.annotation.p0i8(i8* %mem
; CHECK-SAME: @__intel_dtrans_aostosoa_alloc

  store i8* %mem, i8** bitcast (%struct.test01** getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 0)  to i8**)
  ret void
}

; Verify that annotation is put on the pointers used to access the
; peeling index
define internal void @test02() {
; CHECK-LABEL: define internal void @test02

  ; test with constant object GEP
; CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* getelementptr inbounds (%__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @g_test01depptr
; CHECK-SAME: __intel_dtrans_aostosoa_index

  %st01_ptr = load %struct.test01*, %struct.test01** getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 0)

  ; test with a field from the peeled structure
  %ptr1 = getelementptr %struct.test01, %struct.test01* %st01_ptr, i64 0, i32 0
; CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* %ptr1
; CHECK-SAME: @__intel_dtrans_aostosoa_index

  %val1 = load %struct.test01*, %struct.test01** %ptr1

  ; test with a field from a dependent struture
  %ptr2 = getelementptr %struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 0
; CHECK: call i32* @llvm.ptr.annotation.p0i32(i32* %ptr2
; CHECK-SAME: @__intel_dtrans_aostosoa_index

  store %struct.test01* %val1, %struct.test01** %ptr2

  ret void
}

declare i8* @malloc(i64)
