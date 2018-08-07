; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; Test memset with a structure that contains different types of fields to
; verify the memsets write the expected number of bytes for each.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i16, i8, i32*, i64 }

define i32 @main(i32 %argc, i8** %argv) {
  %mem = call i8* @malloc(i64 240)
  %st = bitcast i8* %mem to %struct.test01*

  call void @test01(%struct.test01* %st)
  ret i32 0
}

; Use memmset with a constant size that represents 10 structure elements, on a
; structure of mixed data types.
define void @test01(%struct.test01* %in) {
; CHECK-LABEL: define internal void @test01

  %ptr = bitcast %struct.test01* %in to i8*
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 240, i1 false)
; CHECK:  [[AR0_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK:  [[AR0_BEGIN:%[0-9]+]] = load i16*, i16** [[AR0_PTR]]
; CHECK:  [[FIELD0_PTR:%[0-9]+]] = getelementptr i16, i16* [[AR0_BEGIN]], i64 %in
; CHECK:  [[FIELD0_PTR_I8:%[0-9]+]] = bitcast i16* [[FIELD0_PTR]] to i8*
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD0_PTR_I8]], i8 0, i64 20, i1 false)

; CHECK:  [[AR1_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[AR1_BEGIN:%[0-9]+]] = load i8*, i8** [[AR1_PTR]]
; CHECK:  [[FIELD1_PTR_I8:%[0-9]+]] = getelementptr i8, i8* [[AR1_BEGIN]], i64 %in
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD1_PTR_I8]], i8 0, i64 10, i1 false)

; Note: this field is a pointer type, so one extra level of indirection on these
; CHECK:  [[AR2_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:  [[AR2_BEGIN:%[0-9]+]] = load i32**, i32*** [[AR2_PTR]]
; CHECK:  [[FIELD2_PTR:%[0-9]+]] = getelementptr i32*, i32** [[AR2_BEGIN]], i64 %in
; CHECK:  [[FIELD2_PTR_I8:%[0-9]+]] = bitcast i32** [[FIELD2_PTR]] to i8*
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD2_PTR_I8]], i8 0, i64 80, i1 false)

; CHECK:  [[AR3_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 3
; CHECK:  [[AR3_BEGIN:%[0-9]+]] = load i64*, i64** [[AR3_PTR]]
; CHECK:  [[FIELD3_PTR:%[0-9]+]] = getelementptr i64, i64* [[AR3_BEGIN]], i64 %in
; CHECK:  [[FIELD3_PTR_I8:%[0-9]+]] = bitcast i64* [[FIELD3_PTR]] to i8*
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD3_PTR_I8]], i8 0, i64 80, i1 false)

  ret void
}


declare i8* @malloc(i64)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
