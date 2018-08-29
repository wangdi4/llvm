; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; This test verifies special cases where GetElementPointer
; instructions need to be transformed during the AOS to SOA
; transformation for cases where intermediate values
; need to be sign extended to be the proper size.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i32, i32, %struct.test01*, %struct.test01dep*, i8* }

; This structure will be used for verifying accesses within dependent data
; structures get handled.
%struct.test01dep = type { i16, %struct.test01*, %struct.test01* }

; Pointer to the type being transformed.
@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer


define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 480)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*
  call void @test01(i32 1)
  ret i32 0
}

; These test transformations of GEP instructions that require
; conversions prior to using the GEP index.
define void @test01(i32 %idx1) {
; CHECK define internal void@test01(i32 %idx1)

  %base = load %struct.test01*, %struct.test01** @g_test01ptr
; CHECK:  %base = load i64, i64* @g_test01ptr

  ; Test with GEP index type that differs from peeling index type, and requires
  ; a sign extension.
  %array_elem1 = getelementptr inbounds %struct.test01, %struct.test01* %base, i32 %idx1
; CHECK:  %1 = sext i32 %idx1 to i64
; CHECK:  %array_elem1 = add i64 %base, %1

  ; Test with GEP that accesses a field element where a GEP index is a
  ; different type than the peeled index type, and will require a
  ; sign extension prior to the addition.
  %y_addr = getelementptr inbounds %struct.test01, %struct.test01* %base, i32 %idx1, i32 1
  %y_val = load i32, i32* %y_addr
; CHECK:  %2 = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  %3 = load i32*, i32** %2
; CHECK:  %4 = sext i32 %idx1 to i64
; CHECK:  %5 = add i64 %base, %4
; CHECK:  %y_addr = getelementptr i32, i32* %3, i64 %5
; CHECK:  %y_val = load i32, i32* %y_addr

  ret void
}

declare i8* @calloc(i64, i64)
