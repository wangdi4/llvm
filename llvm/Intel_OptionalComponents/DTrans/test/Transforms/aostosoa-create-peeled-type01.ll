; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"


; This test verifies the basic functionality for creating a peeled type
; for the AOS to SOA transformation.

; In this test, we will select %struct.test01 as the type to be transformed.
; This will result in a new data type being created that holds pointers to
; the original types in the structure (with the exception of pointers to
; types being converted, those should get pointers to the type that will
; be used for indices, which will default to the native pointer size).

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
%struct.test01 = type { i32, i64, %struct.test01*, %struct.test02*, i16** }
%struct.test02 = type { %struct.test01*, %struct.test01*, %struct.test01** }

; Verify the contents of the new types and dependent types
; CHECK: %__SOA_struct.test01 = type { i32*, i64*, i64*, %__SOADT_struct.test02**, i16*** }
; CHECK: %__SOADT_struct.test02 = type { i64, i64, i64* }

; Verify that a new global variable has been created for the type being
; converted.
; CHECK: @__soa_struct.test01 = internal global %__SOA_struct.test01 zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 40)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  %alloc02 = call i8* @calloc(i64 10, i64 24)
  %struct02_mem = bitcast i8* %alloc02 to %struct.test02*

  call void @test01();
  call void @test02(%struct.test02*  %struct02_mem);

  ret i32 0
}


; Uses of the pointers to the type will be converted to pointers to the index
; type. This is a basic test that relies on the DTransOptBase class to
; convert the type and does not require the transformation specific IR.
define void @test01() {
  %local = alloca %struct.test01*
  ret void
}
; Verify that a scalar integer is now used for the local.
; CHECK define internal void@test01()
; CHECK:   %local = alloca i64


; Uses of the pointers to the type will be converted to pointers to the index
; type. This is a basic test that relies on the DTransOptBase class to handle
; pointers to the types being transformed and does not require the transformation
; specific IR.
define void @test02(%struct.test02* %in1) {
  %field1_addr = getelementptr %struct.test02, %struct.test02* %in1, i64 0, i32 0
  %field2_addr = getelementptr %struct.test02, %struct.test02* %in1, i64 0, i32 1
  %field1_val = load %struct.test01*, %struct.test01** %field1_addr
  %field2_val = load %struct.test01*, %struct.test01** %field2_addr
  store %struct.test01* %field2_val, %struct.test01** %field1_addr
  store %struct.test01* %field1_val, %struct.test01** %field2_addr
  ret void
}
; Verify that accesses to read and write pointers to the type being converted
; are changed to be reads and writes of integer index values.
; CHECK define internal void@test02.1(%__SOADT_struct.test02* %in1)
; CHECK:   %field1_addr = getelementptr %__SOADT_struct.test02, %__SOADT_struct.test02* %in1, i64 0, i32 0
; CHECK:   %field2_addr = getelementptr %__SOADT_struct.test02, %__SOADT_struct.test02* %in1, i64 0, i32 1
; CHECK:   %field1_val = load i64, i64* %field1_addr
; CHECK:   %field2_val = load i64, i64* %field2_addr
; CHECK:   store i64 %field2_val, i64* %field1_addr
; CHECK:   store i64 %field1_val, i64* %field2_addr

declare i8* @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }
