; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; This tests the AOS-to-SOA transform for handling bitcasts that
; are either from or to a pointer of the type being transformed.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i64, i32, i32 }

@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  %mem = call i8* @malloc(i64 160)
  store i8* %mem, i8** bitcast (%struct.test01** @g_test01ptr to i8**)
  call void @test01()
  ret i32 0
}

define void @test01() {
; CHECK-LABEL: define internal void @test01

  %var = load %struct.test01*, %struct.test01** @g_test01ptr
; CHECK:  %var = load i64, i64* @g_test01ptr

  ; Bitcast from pointer to type being transformed.
  %p_i8 = bitcast %struct.test01* %var to i8*
; CHECK: %p_i8 = inttoptr i64 %var to i8*

  ; Bitcast to type pointer to type being transformed.
  %p_st = bitcast i8* %p_i8 to %struct.test01*
; CHECK: %p_st = ptrtoint i8* %p_i8 to i64

  ; Bitcast from pointer to type being transformed. This one
  ; will have no uses, and should be removed.
  %p2_i8 = bitcast %struct.test01* %p_st to i8*

  ret void
; CHECK-NEXT: ret void
}

declare i8* @malloc(i64)
