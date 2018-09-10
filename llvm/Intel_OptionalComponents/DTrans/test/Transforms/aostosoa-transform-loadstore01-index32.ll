; RUN: opt < %s -S -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s

; This tests the AOS-to-SOA transform for handling load/stores that
; are done as pointer sized int types instead of pointers of the type
; being transformed when using a 32-bit index for the peeling index.

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
  %p_i64 = bitcast %struct.test01** %field to i64*

  ; Use the result of the bitcast to load a value. Because the transformation
  ; is using a 32-bit value for the index, the load needs to be transformed to
  ; avoid exceeding the element. Once the structure is transformed, %field will
  ; be an i32*, so that address should be used directly, rather than casting it
  ; to an i64* and back to an i32*. The result of the load may need to be extended
  ; to be compatible with the uses when the uses will stay as 64-bit operations.
  ; In this case, the icmp will use the 64-bit value, but the store needs to
  ; use the 32-bit result.
  %val_i64 = load i64, i64* %p_i64
  %cmp = icmp eq i64 %val_i64, 0
; CHECK:  [[LOADED:%[0-9]+]] = load i32, i32* %field
; CHECK:  %val_i64 = zext i32 [[LOADED]] to i64

  ; Use the result of the bitcast to store a value. In this case, the store
  ; needs to only write 32-bits into the peeling index field.
  store i64 %val_i64, i64* %p_i64
; CHECK: store i32 [[LOADED]], i32* %field

  ret void
}

declare i8* @malloc(i64)
