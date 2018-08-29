; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01,struct.test02 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01,struct.test02 2>&1 | FileCheck %s

; This test verifies that GetElementPointer instructions get transformed
; when there are multiple structures to be converted.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; These data structures are going to be transformed.
%struct.test01 = type { i32, i32, i32, %struct.test01*, %struct.test02*, i8* }
%struct.test02 = type { i32, %struct.test01* }

; Pointers to the types being transformed.
@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer
@g_test02ptr = internal unnamed_addr global %struct.test02* zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 480)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  %alloc02 = call i8* @calloc(i64 10, i64 160)
  %struct02_mem = bitcast i8* %alloc02 to %struct.test02*

  call void @test01(i64 0)
  ret i32 0
}

define void @test01(i64 %idx1) {
; CHECK define internal void@test01(i64 %idx1) {

  %base1 = load %struct.test01*, %struct.test01** @g_test01ptr
;CHECK:   %base1 = load i64, i64* @g_test01ptr

  %base2 = load %struct.test02*, %struct.test02** @g_test02ptr
; CHECK:  %base2 = load i64, i64* @g_test02ptr

  %array_elem1 = getelementptr %struct.test01, %struct.test01* %base1, i64 0
; CHECK:   %array_elem1 = add i64 %base1, 0

  %array_elem2 = getelementptr %struct.test02, %struct.test02* %base2, i64 0
; CHECK:   %array_elem2 = add i64 %base2, 0

  %test01_field_addr = getelementptr %struct.test01, %struct.test01* %array_elem1, i64 %idx1, i32 1
; CHECK:   %1 = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  %2 = load i32*, i32** %1
; CHECK:  %3 = add i64 %array_elem1, %idx1
; CHECK:  %test01_field_addr = getelementptr i32, i32* %2, i64 %3

  %test02_field_addr = getelementptr %struct.test02, %struct.test02* %array_elem2, i64 %idx1, i32 0
; CHECK:   %4 = getelementptr %__SOA_struct.test02, %__SOA_struct.test02* @__soa_struct.test02, i64 0, i32 0
; CHECK:  %5 = load i32*, i32** %4
; CHECK:  %6 = add i64 %array_elem2, %idx1
; CHECK:  %test02_field_addr = getelementptr i32, i32* %5, i64 %6

  ret void
}

declare i8* @calloc(i64, i64)
