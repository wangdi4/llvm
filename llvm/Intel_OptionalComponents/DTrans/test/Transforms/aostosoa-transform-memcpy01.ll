; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; The test checks the transformation by the AOS-to-SOA transformation for
; memcpy calls.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i32, i32 }

define i32 @main(i32 %argc, i8** %argv) {
  %mem = call i8* @malloc(i64 120)
  %st = bitcast i8* %mem to %struct.test01*

  call void @test01(%struct.test01* %st, %struct.test01* %st)
  call void @test02(%struct.test01* %st, %struct.test01* %st, i32 5)
  call void @test03(%struct.test01* %st)
  ret i32 0
}

; Use memcpy with the size of one structure element
define void @test01(%struct.test01* %in1, %struct.test01* %in2) {
; CHECK-LABEL: define internal void @test01

  %ptr1 = bitcast %struct.test01* %in1 to i8*
  %ptr2 = bitcast %struct.test01* %in2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %ptr1, i8* %ptr2, i64 12, i1 false)
; CHECK:  [[AR0_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK:  [[AR0_BEGIN:%[0-9]+]] = load i32*, i32** [[AR0_PTR]]
; CHECK:  [[FIELD0_PTR1:%[0-9]+]] = getelementptr i32, i32* [[AR0_BEGIN]], i64 %in1
; CHECK:  [[FIELD0_PTR1_I8:%[0-9]+]] = bitcast i32* [[FIELD0_PTR1]] to i8*
; CHECK:  [[FIELD0_PTR2:%[0-9]+]] = getelementptr i32, i32* [[AR0_BEGIN]], i64 %in2
; CHECK:  [[FIELD0_PTR2_I8:%[0-9]+]] = bitcast i32* [[FIELD0_PTR2]] to i8*
; CHECK:  call void @llvm.memcpy.p0i8.p0i8.i64(i8* [[FIELD0_PTR1_I8]], i8* [[FIELD0_PTR2_I8]], i64 4, i1 false)

; CHECK:  [[AR1_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[AR1_BEGIN:%[0-9]+]] = load i32*, i32** [[AR1_PTR]]
; CHECK:  [[FIELD1_PTR1:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 %in1
; CHECK:  [[FIELD1_PTR1_I8:%[0-9]+]] = bitcast i32* [[FIELD1_PTR1]] to i8*
; CHECK:  [[FIELD1_PTR2:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 %in2
; CHECK:  [[FIELD1_PTR2_I8:%[0-9]+]] = bitcast i32* [[FIELD1_PTR2]] to i8*
; CHECK:  call void @llvm.memcpy.p0i8.p0i8.i64(i8* [[FIELD1_PTR1_I8]], i8* [[FIELD1_PTR2_I8]], i64 4, i1 false)

; CHECK:  [[AR2_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:  [[AR2_BEGIN:%[0-9]+]] = load i32*, i32** [[AR2_PTR]]
; CHECK:  [[FIELD2_PTR1:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 %in1
; CHECK:  [[FIELD2_PTR1_I8:%[0-9]+]] = bitcast i32* [[FIELD2_PTR1]] to i8*
; CHECK:  [[FIELD2_PTR2:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 %in2
; CHECK:  [[FIELD2_PTR2_I8:%[0-9]+]] = bitcast i32* [[FIELD2_PTR2]] to i8*
; CHECK:  call void @llvm.memcpy.p0i8.p0i8.i64(i8* [[FIELD2_PTR1_I8]], i8* [[FIELD2_PTR2_I8]], i64 4, i1 false)

  ret void
}

; Use memset with the size that is a multiple of the structure element size.
define void @test02(%struct.test01* %in1, %struct.test01* %in2, i32 %count) {
; CHECK-LABEL: define internal void @test02

 ; Use a different variable for the memcpy param than the variable that gets
 ; modified for this test.
  %numbytes = mul i32 12, %count
; CHECK: %numbytes = mul i32 1, %count

  %numbytes_i64 = sext i32 %numbytes to i64

  %ptr1 = bitcast %struct.test01* %in1 to i8*
  %ptr2 = bitcast %struct.test01* %in2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %ptr1, i8* %ptr2, i64 %numbytes_i64, i1 false)
; CHECK:  [[AR0_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK:  [[AR0_BEGIN:%[0-9]+]] = load i32*, i32** [[AR0_PTR]]
; CHECK:  [[FIELD0_PTR1:%[0-9]+]] = getelementptr i32, i32* [[AR0_BEGIN]], i64 %in1
; CHECK:  [[FIELD0_PTR1_I8:%[0-9]+]] = bitcast i32* [[FIELD0_PTR1]] to i8*
; CHECK:  [[FIELD0_PTR2:%[0-9]+]] = getelementptr i32, i32* [[AR0_BEGIN]], i64 %in2
; CHECK:  [[FIELD0_PTR2_I8:%[0-9]+]] = bitcast i32* [[FIELD0_PTR2]] to i8*
; CHECK:  [[BYTECOUNT0:%[0-9]+]] = mul i64 %numbytes_i64, 4
; CHECK:  call void @llvm.memcpy.p0i8.p0i8.i64(i8* [[FIELD0_PTR1_I8]], i8* [[FIELD0_PTR2_I8]], i64 [[BYTECOUNT0]], i1 false)

; CHECK:  [[AR1_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[AR1_BEGIN:%[0-9]+]] = load i32*, i32** [[AR1_PTR]]
; CHECK:  [[FIELD1_PTR1:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 %in1
; CHECK:  [[FIELD1_PTR1_I8:%[0-9]+]] = bitcast i32* [[FIELD1_PTR1]] to i8*
; CHECK:  [[FIELD1_PTR2:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 %in2
; CHECK:  [[FIELD1_PTR2_I8:%[0-9]+]] = bitcast i32* [[FIELD1_PTR2]] to i8*
; CHECK:  [[BYTECOUNT1:%[0-9]+]] = mul i64 %numbytes_i64, 4
; CHECK:  call void @llvm.memcpy.p0i8.p0i8.i64(i8* [[FIELD1_PTR1_I8]], i8* [[FIELD1_PTR2_I8]], i64 [[BYTECOUNT1]], i1 false)

; CHECK:  [[AR2_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:  [[AR2_BEGIN:%[0-9]+]] = load i32*, i32** [[AR2_PTR]]
; CHECK:  [[FIELD2_PTR1:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 %in1
; CHECK:  [[FIELD2_PTR1_I8:%[0-9]+]] = bitcast i32* [[FIELD2_PTR1]] to i8*
; CHECK:  [[FIELD2_PTR2:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 %in2
; CHECK:  [[FIELD2_PTR2_I8:%[0-9]+]] = bitcast i32* [[FIELD2_PTR2]] to i8*
; CHECK:  [[BYTECOUNT2:%[0-9]+]] = mul i64 %numbytes_i64, 4
; CHECK:  call void @llvm.memcpy.p0i8.p0i8.i64(i8* [[FIELD2_PTR1_I8]], i8* [[FIELD2_PTR2_I8]], i64 [[BYTECOUNT2]], i1 false)

  ret void
}

; Test with partial memcpy from indexed elements, such as:
;   memcpy(&in1[0].y, &in[2].y, 8)
define void @test03(%struct.test01* %in1) {
; CHECK-LABEL: define internal void @test03

  %dest_addr = getelementptr %struct.test01, %struct.test01* %in1, i64 0, i32 1
  %src_addr = getelementptr %struct.test01, %struct.test01* %in1, i64 2, i32 1

  ; We need to match the GEP conversion to get the right temp names because
  ; it's the same instruction sequence as the part used for the memset.
; CHECK:  %dest_addr = getelementptr
; CHECK: [[SRC_INDEX_ELEM:%[0-9]+]] = add i64 %in1, 2
; CHECK:  %src_addr = getelementptr

  %ptr1 = bitcast i32* %dest_addr to i8*
  %ptr2 = bitcast i32* %src_addr to i8*

  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %ptr1, i8* %ptr2, i64 8, i1 false)
; CHECK:  [[AR1_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[AR1_BEGIN:%[0-9]+]] = load i32*, i32** [[AR1_PTR]]
; CHECK:  [[FIELD1_PTR1:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 %in1
; CHECK:  [[FIELD1_PTR1_I8:%[0-9]+]] = bitcast i32* [[FIELD1_PTR1]] to i8*
; CHECK:  [[FIELD1_PTR2:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 [[SRC_INDEX_ELEM]]
; CHECK:  [[FIELD1_PTR2_I8:%[0-9]+]] = bitcast i32* [[FIELD1_PTR2]] to i8*
; CHECK:  call void @llvm.memcpy.p0i8.p0i8.i64(i8* [[FIELD1_PTR1_I8]], i8* [[FIELD1_PTR2_I8]], i64 4, i1 false)

; CHECK:  [[AR2_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:  [[AR2_BEGIN:%[0-9]+]] = load i32*, i32** [[AR2_PTR]]
; CHECK:  [[FIELD2_PTR1:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 %in1
; CHECK:  [[FIELD2_PTR1_I8:%[0-9]+]] = bitcast i32* [[FIELD2_PTR1]] to i8*
; CHECK:  [[FIELD2_PTR2:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 [[SRC_INDEX_ELEM]]
; CHECK:  [[FIELD2_PTR2_I8:%[0-9]+]] = bitcast i32* [[FIELD2_PTR2]] to i8*
; CHECK:  call void @llvm.memcpy.p0i8.p0i8.i64(i8* [[FIELD2_PTR1_I8]], i8* [[FIELD2_PTR2_I8]], i64 4, i1 false)

  ret void
}

declare i8* @malloc(i64)
declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)
