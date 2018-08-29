; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; The test checks the transformation by the AOS-to-SOA transformation for
; memmove calls when the structure contains a mix of data types. The memmove
; handling is identical to the memcpy case, so limited testing is done
; for memmove.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { %struct.test01*, i32 }

define i32 @main(i32 %argc, i8** %argv) {
  %mem = call i8* @malloc(i64 160)
  %st = bitcast i8* %mem to %struct.test01*

  call void @test01(%struct.test01* %st, %struct.test01* %st)
  ret i32 0
}

; Use memmove with the size of one structure element
define void @test01(%struct.test01* %in1, %struct.test01* %in2) {
; CHECK-LABEL: define internal void @test01

  %ptr1 = bitcast %struct.test01* %in1 to i8*
  %ptr2 = bitcast %struct.test01* %in2 to i8*
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %ptr1, i8* %ptr2, i64 16, i1 false)

  ; The first field is transformed to the peeling index type.
; CHECK:  [[AR0_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK:  [[AR0_BEGIN:%[0-9]+]] = load i64*, i64** [[AR0_PTR]]
; CHECK:  [[FIELD0_PTR1:%[0-9]+]] = getelementptr i64, i64* [[AR0_BEGIN]], i64 %in1
; CHECK:  [[FIELD0_PTR1_I8:%[0-9]+]] = bitcast i64* [[FIELD0_PTR1]] to i8*
; CHECK:  [[FIELD0_PTR2:%[0-9]+]] = getelementptr i64, i64* [[AR0_BEGIN]], i64 %in2
; CHECK:  [[FIELD0_PTR2_I8:%[0-9]+]] = bitcast i64* [[FIELD0_PTR2]] to i8*
; CHECK:  call void @llvm.memmove.p0i8.p0i8.i64(i8* [[FIELD0_PTR1_I8]], i8* [[FIELD0_PTR2_I8]], i64 8, i1 false)

; CHECK:  [[AR1_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[AR1_BEGIN:%[0-9]+]] = load i32*, i32** [[AR1_PTR]]
; CHECK:  [[FIELD1_PTR1:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 %in1
; CHECK:  [[FIELD1_PTR1_I8:%[0-9]+]] = bitcast i32* [[FIELD1_PTR1]] to i8*
; CHECK:  [[FIELD1_PTR2:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 %in2
; CHECK:  [[FIELD1_PTR2_I8:%[0-9]+]] = bitcast i32* [[FIELD1_PTR2]] to i8*
; CHECK:  call void @llvm.memmove.p0i8.p0i8.i64(i8* [[FIELD1_PTR1_I8]], i8* [[FIELD1_PTR2_I8]], i64 4, i1 false)

  ret void
}

declare i8* @malloc(i64)
declare void @llvm.memmove.p0i8.p0i8.i64(i8*, i8*, i64, i1)
