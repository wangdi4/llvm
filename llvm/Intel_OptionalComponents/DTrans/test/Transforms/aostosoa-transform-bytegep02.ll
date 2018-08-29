; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; This tests the cases of a byte-flattened GEP used in a function that
; will be cloned.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i64, i32, i32 }

define i32 @main(i32 %argc, i8** %argv) {
  %mem = call i8* @malloc(i64 160)
  %st = bitcast i8* %mem to %struct.test01*
  call void @test01(%struct.test01* %st)
  ret i32 0
}

define void @test01(%struct.test01* %in) {
; CHECK-LABEL: define internal void @test01

  ; Get a byte-pointer to the structure.
  %p = bitcast %struct.test01* %in to i8*

  ; Get pointers to each field

  ; Equivalent to: getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
  %p8_A = getelementptr i8, i8* %p, i64 0
; CHECK:  [[AR0_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK:  [[AR0_BEGIN:%[0-9]+]] = load i64*, i64** [[AR0_PTR]]
; CHECK:  [[FIELD0_PTR:%[0-9]+]] = getelementptr i64, i64* [[AR0_BEGIN]], i64 %in
; CHECK:  %p8_A = bitcast i64* [[FIELD0_PTR]] to i8*

  ; Equivalent to: getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  %p8_B = getelementptr i8, i8* %p, i64 8
; CHECK:  [[AR1_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[AR1_BEGIN:%[0-9]+]] = load i32*, i32** [[AR1_PTR]]
; CHECK:  [[FIELD1_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 %in
; CHECK:  %p8_B = bitcast i32* [[FIELD1_PTR]] to i8*

  ; Equivalent to: getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 2
  %p8_C = getelementptr i8, i8* %p, i64 12
; CHECK:  [[AR2_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:  [[AR2_BEGIN:%[0-9]+]] = load i32*, i32** [[AR2_PTR]]
; CHECK:  [[FIELD2_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 %in
; CHECK:  %p8_C = bitcast i32* [[FIELD2_PTR]] to i8*

  %pA = bitcast i8* %p8_A to i64*
; CHECK:  %pA = bitcast i8* %p8_A to i64*

  %pB = bitcast i8* %p8_B to i32*
; CHECK:  %pB = bitcast i8* %p8_B to i32*

  %pC = bitcast i8* %p8_C to i32*
; CHECK:  %pC = bitcast i8* %p8_C to i32*

  ; Read and write fields
  store i64 1, i64* %pA
  %vA = load i64, i64* %pA
  store i32 1, i32* %pB
  %vB = load i32, i32* %pB
  store i32 1, i32* %pC
  %vC = load i32, i32* %pC

  ret void
}

declare i8* @malloc(i64)
