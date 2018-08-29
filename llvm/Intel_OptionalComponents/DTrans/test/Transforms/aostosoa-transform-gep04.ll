; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; This test verifies replacements when the GEP result is used
; in more than one basic blocks.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i32, i32, %struct.test01*, %struct.test01*, i8* }

; Pointer to the type being transformed.
@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer


define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 480)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  %tmp = call i32 @test01(i64 1)
  ret i32 0
}

define i32 @test01(i64 %idx1) {
; CHECK: define internal i32 @test01(i64 %idx1)

  %base = load %struct.test01*, %struct.test01** @g_test01ptr
; CHECK:  %base = load i64, i64* @g_test01ptr

  ; Test with GEP that will be used in multiple basic blocks
  ; There is nothing unique to the transformed IR compared to
  ; the other test. We just want to be sure the new %y_addr
  ; gets to all the users.
  %y_addr = getelementptr inbounds %struct.test01, %struct.test01* %base, i64 %idx1, i32 1
; CHECK:  %1 = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  %2 = load i32*, i32** %1
; CHECK:  %3 = add i64 %base, %idx1
; CHECK:  %y_addr = getelementptr i32, i32* %2, i64 %3

  br i1 undef, label %label_one, label %label_two
label_one:
  store i32 1, i32* %y_addr
  br label %label_end
label_two:
  store i32 2, i32* %y_addr
  br label %label_end

label_end:
  %res = load i32, i32* %y_addr
  ret i32 %res
}

declare i8* @calloc(i64, i64)
