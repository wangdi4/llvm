; RUN: opt < %s -S -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s

; This tests the AOS-to-SOA transform for handling load/stores that
; are done as generic i8* types instead of pointers of the
; the type being transformed when using a 32-bit index for the
; peeling index.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i64, %struct.test01* }

define i32 @main(i32 %argc, i8** %argv) {
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.test01*
  call void @test01(%struct.test01* %st)
  ret i32 0
}

; Test with cast to pointer sized int for uses in load/store instructions
define internal void @test01(%struct.test01* %st) {
; CHECK-LABEL: define internal void @test01

  %field = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 1

  ; Bitcast from ptr-to-ptr of type being transformed.
  %p_i8 = bitcast %struct.test01** %field to i8**

  ; Use the result of the bitcast to load a value. Because the transformation
  ; is using a 32-bit value for the index, the load needs to be transformed to
  ; avoid exceeding the memory of the element. The result of the load needs to
  ; be converted to be compatible with the uses, since the uses will stay as
  ; i8* operations.
  %val = load i8*, i8** %p_i8
; CHECK: [[BC1:%[0-9]+]] = bitcast i8** %p_i8 to i32*
; CHECK:  [[LOADED:%[0-9]+]] = load i32, i32* [[BC1]]
; CHECK:  %val = inttoptr i32 [[LOADED]] to i8*

  ; Use the result of the bitcast to store a value. In this case, the store
  ; needs to only write 32-bits into the peeling index field. The value being
  ; written will be truncated, however when using 32-bit indexing the user is
  ; guaranteeing it fits within 32-bits.
  store i8* %val, i8** %p_i8
; CHECK: [[TRUNC:%[0-9]+]] = ptrtoint i8* %val to i32
; CHECK: [[PTR_CAST:%[0-9]+]] = bitcast i8** %p_i8 to i32*
; CHECK: store i32 [[TRUNC]], i32* [[PTR_CAST]]

  ret void
}

declare i8* @malloc(i64)
