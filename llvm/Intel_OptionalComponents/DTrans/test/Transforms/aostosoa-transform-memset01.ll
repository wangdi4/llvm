; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; The test checks the transformation by the AOS-to-SOA transformation for
; memset calls.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i32, i32 }

define i32 @main(i32 %argc, i8** %argv) {
  %mem = call i8* @malloc(i64 120)
  %st = bitcast i8* %mem to %struct.test01*

  call void @test01(%struct.test01* %st)
  call void @test02(%struct.test01* %st)
  call void @test03(%struct.test01* %st, i64 5)
  call void @test04(%struct.test01* %st, %struct.test01* %st, i64 5)
  call void @test05(%struct.test01* %st, %struct.test01* %st, i64 5)
  call void @test06(%struct.test01* %st)
  call void @test07(%struct.test01* %st)

  ret i32 0
}

; Use memset with the size of one structure element
define void @test01(%struct.test01* %in) {
; CHECK-LABEL: define internal void @test01
  %ptr = bitcast %struct.test01* %in to i8*

  ; Currently, we will generate one memset per field of the original
  ; structure. This could be modified in the future to perform an
  ; element copy when only a single element is being copied, but just
  ; running the memcpy optimizer should simplify the calls in this case.
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 12, i1 false)
; CHECK:  [[AR0_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK:  [[AR0_BEGIN:%[0-9]+]] = load i32*, i32** [[AR0_PTR]]
; CHECK:  [[FIELD0_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR0_BEGIN]], i64 %in
; CHECK:  [[FIELD0_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD0_PTR]] to i8*
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD0_PTR_I8]], i8 0, i64 4, i1 false)

; CHECK:  [[AR1_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[AR1_BEGIN:%[0-9]+]] = load i32*, i32** [[AR1_PTR]]
; CHECK:  [[FIELD1_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 %in
; CHECK:  [[FIELD1_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD1_PTR]] to i8*
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD1_PTR_I8]], i8 0, i64 4, i1 false)

; CHECK:  [[AR2_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:  [[AR2_BEGIN:%[0-9]+]] = load i32*, i32** [[AR2_PTR]]
; CHECK:  [[FIELD2_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 %in
; CHECK:  [[FIELD2_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD2_PTR]] to i8*
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD2_PTR_I8]], i8 0, i64 4, i1 false)

  ret void
}

; Use memset with a constant size and a non-zero value being set
define void @test02(%struct.test01* %in) {
; CHECK-LABEL: define internal void @test02
  %ptr = bitcast %struct.test01* %in to i8*

  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 -1, i64 48, i1 false)
; CHECK:  [[AR0_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK:  [[AR0_BEGIN:%[0-9]+]] = load i32*, i32** [[AR0_PTR]]
; CHECK:  [[FIELD0_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR0_BEGIN]], i64 %in
; CHECK:  [[FIELD0_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD0_PTR]] to i8*
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD0_PTR_I8]], i8 -1, i64 16, i1 false)

; CHECK:  [[AR1_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[AR1_BEGIN:%[0-9]+]] = load i32*, i32** [[AR1_PTR]]
; CHECK:  [[FIELD1_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 %in
; CHECK:  [[FIELD1_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD1_PTR]] to i8*
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD1_PTR_I8]], i8 -1, i64 16, i1 false)

; CHECK:  [[AR2_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:  [[AR2_BEGIN:%[0-9]+]] = load i32*, i32** [[AR2_PTR]]
; CHECK:  [[FIELD2_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 %in
; CHECK:  [[FIELD2_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD2_PTR]] to i8*
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD2_PTR_I8]], i8 -1, i64 16, i1 false)

  ret void
}

; Use memset with the size that is a multiple of the structure element size.
define void @test03(%struct.test01* %in, i64 %count) {
; CHECK-LABEL: define internal void @test03

  %ptr = bitcast %struct.test01* %in to i8*

  ; Check that the multiply gets updated so that the size of the structure
  ; has been divided by the original size multiple to determine the number
  ; of elements to set in each array.
  %numbytes = mul i64 12, %count
; CHECK:  [[NUM_ELEM:%numbytes]] = mul i64 1, %count

  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 %numbytes, i1 false)
; CHECK:  [[AR0_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK:  [[AR0_BEGIN:%[0-9]+]] = load i32*, i32** [[AR0_PTR]]
; CHECK:  [[FIELD0_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR0_BEGIN]], i64 %in
; CHECK:  [[FIELD0_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD0_PTR]] to i8*
; CHECK:  [[BYTECOUNT0:%[0-9]+]] = mul i64 [[NUM_ELEM]], 4
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD0_PTR_I8]], i8 0, i64 [[BYTECOUNT0]], i1 false)

; CHECK:  [[AR1_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[AR1_BEGIN:%[0-9]+]] = load i32*, i32** [[AR1_PTR]]
; CHECK:  [[FIELD1_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 %in
; CHECK:  [[FIELD1_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD1_PTR]] to i8*
; CHECK:  [[BYTECOUNT1:%[0-9]+]] = mul i64 [[NUM_ELEM]], 4
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD1_PTR_I8]], i8 0, i64 [[BYTECOUNT1]], i1 false)

; CHECK:  [[AR2_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:  [[AR2_BEGIN:%[0-9]+]] = load i32*, i32** [[AR2_PTR]]
; CHECK:  [[FIELD2_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 %in
; CHECK:  [[FIELD2_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD2_PTR]] to i8*
; CHECK:  [[BYTECOUNT2:%[0-9]+]] = mul i64 [[NUM_ELEM]], 4
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD2_PTR_I8]], i8 0, i64 [[BYTECOUNT2]], i1 false)

  ret void
}

; Test with memset parameter that needs to be updated being used in multiple
; memset calls.
define void @test04(%struct.test01* %in1, %struct.test01* %in2, i64 %count) {
; CHECK-LABEL: define internal void @test04

  %ptr1 = bitcast %struct.test01* %in1 to i8*
  %ptr2 = bitcast %struct.test01* %in2 to i8*

  ; Currently a clone will be made when the first memset is updated. The replication
  ; of the memset will be the same for both.
  %numbytes = mul i64 12, %count
; CHECK: %numbytes.dt = mul i64 1, %count
; CHECK: %numbytes = mul i64 1, %count

  call void @llvm.memset.p0i8.i64(i8* %ptr1, i8 0, i64 %numbytes, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %ptr2, i8 0, i64 %numbytes, i1 false)

  ret void
}

; Use memset with on a case where the transform tracks the type through
; a select statement
define void @test05(%struct.test01* %in1, %struct.test01* %in2, i64 %count) {
; CHECK-LABEL: define internal void @test05
  %ptr1 = bitcast %struct.test01* %in1 to i8*
  %ptr2 = bitcast %struct.test01* %in2 to i8*
  %ptr = select i1 undef, i8* %ptr1, i8* %ptr2
; CHECK: [[SEL_RESULT:%[0-9]+]] = select i1 undef, i64 %in1, i64 %in2

  %numbytes = mul i64 12, %count
; CHECK:  [[NUM_ELEM:%numbytes]] = mul i64 1, %count

  tail call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 %numbytes, i1 false)
; CHECK:  [[AR0_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK:  [[AR0_BEGIN:%[0-9]+]] = load i32*, i32** [[AR0_PTR]]
; CHECK:  [[FIELD0_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR0_BEGIN]], i64 [[SEL_RESULT]]
; CHECK:  [[FIELD0_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD0_PTR]] to i8*
; CHECK:  [[BYTECOUNT0:%[0-9]+]] = mul i64 [[NUM_ELEM]], 4
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD0_PTR_I8]], i8 0, i64 [[BYTECOUNT0]], i1 false)

; CHECK:  [[AR1_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[AR1_BEGIN:%[0-9]+]] = load i32*, i32** [[AR1_PTR]]
; CHECK:  [[FIELD1_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 [[SEL_RESULT]]
; CHECK:  [[FIELD1_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD1_PTR]] to i8*
; CHECK:  [[BYTECOUNT1:%[0-9]+]] = mul i64 [[NUM_ELEM]], 4
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD1_PTR_I8]], i8 0, i64 [[BYTECOUNT1]], i1 false)

; CHECK:  [[AR2_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:  [[AR2_BEGIN:%[0-9]+]] = load i32*, i32** [[AR2_PTR]]
; CHECK:  [[FIELD2_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 [[SEL_RESULT]]
; CHECK:  [[FIELD2_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD2_PTR]] to i8*
; CHECK:  [[BYTECOUNT2:%[0-9]+]] = mul i64 [[NUM_ELEM]], 4
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD2_PTR_I8]], i8 0, i64 [[BYTECOUNT2]], i1 false)

  ret void
}

; Use memset starting with the address of a field that is not array element 0
define void @test06(%struct.test01* %in) {
; CHECK-LABEL: define internal void @test06

  %field0_addr = getelementptr inbounds %struct.test01, %struct.test01* %in, i64 2, i32 0

  ; The GEP instruction will produce two separate add instructions, one for
  ; the GEP instruction processing, and one for determining the index to use
  ; for the memset.
  ; TODO: This is something that can hopefully be cleaned up in the future,
  ; since the GEP and bitcast will effectively be dead instructions.
  ; Because of the order instructions are processed, the first one will
  ; be the one used for memset transformation.
; CHECK: [[INDEX_ELEM:%[0-9]+]] = add i64 %in, 2

; We need to locate the setting of the %field0_addr before matching the
; memset sequence because there will be some common instructions used
; by the GEP transformation.
; CHECK:  %field0_addr = getelementptr

  %ptr = bitcast i32* %field0_addr to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 12, i1 false)
; CHECK:  [[AR0_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK:  [[AR0_BEGIN:%[0-9]+]] = load i32*, i32** [[AR0_PTR]]
; CHECK:  [[FIELD0_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR0_BEGIN]], i64 [[INDEX_ELEM]]
; CHECK:  [[FIELD0_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD0_PTR]] to i8*
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD0_PTR_I8]], i8 0, i64 4, i1 false)

; CHECK:  [[AR1_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[AR1_BEGIN:%[0-9]+]] = load i32*, i32** [[AR1_PTR]]
; CHECK:  [[FIELD1_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 [[INDEX_ELEM]]
; CHECK:  [[FIELD1_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD1_PTR]] to i8*
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD1_PTR_I8]], i8 0, i64 4, i1 false)

; CHECK:  [[AR2_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:  [[AR2_BEGIN:%[0-9]+]] = load i32*, i32** [[AR2_PTR]]
; CHECK:  [[FIELD2_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 [[INDEX_ELEM]]
; CHECK:  [[FIELD2_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD2_PTR]] to i8*
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD2_PTR_I8]], i8 0, i64 4, i1 false)

  ret void
}

; Test with partial memset from indexed elements, such as:
;   memset(&in1[0].y, 0, 8)
define void @test07(%struct.test01* %in1) {
; CHECK-LABEL: define internal void @test07

  %field1_addr = getelementptr %struct.test01, %struct.test01* %in1, i64 0, i32 1

; We need to match the GEP conversion to get the right temp names because
; it's the same instruction sequence as the part used for the memset.
; CHECK:  %field1_addr = getelementptr

  %ptr = bitcast i32* %field1_addr to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 8, i1 false)
; CHECK:  [[AR1_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[AR1_BEGIN:%[0-9]+]] = load i32*, i32** [[AR1_PTR]]
; CHECK:  [[FIELD1_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 %in
; CHECK:  [[FIELD1_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD1_PTR]] to i8*
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD1_PTR_I8]], i8 0, i64 4, i1 false)

; CHECK:  [[AR2_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:  [[AR2_BEGIN:%[0-9]+]] = load i32*, i32** [[AR2_PTR]]
; CHECK:  [[FIELD2_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 %in
; CHECK:  [[FIELD2_PTR_I8:%[0-9]+]] = bitcast i32* [[FIELD2_PTR]] to i8*
; CHECK:  call void @llvm.memset.p0i8.i64(i8* [[FIELD2_PTR_I8]], i8 0, i64 4, i1 false)

  ret void
}

declare i8* @malloc(i64)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
