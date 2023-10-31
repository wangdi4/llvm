; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

; This test verifies that the size argument of a calloc calls are correctly
; updated in a few different combinations.

%struct.test = type { i32, i64, i32 }

; CHECK: %__DFT_struct.test = type { i32, i32 }

; calloc a buffer with the size in the first argument.
define void @test1() {
  ; Allocate an array of structures.
  %p = call ptr @calloc(i64 24, i64 8)

  ; Call a function to do something.
  %val = call i32 @doSomething(ptr %p)

  ; Free the structure
  call void @free(ptr %p)
  ret void
}
; CHECK define void@test1()
; CHECK: %p = call ptr @calloc(i64 8, i64 8)

; calloc a buffer with the size in the second argument.
define void @test2() {
  ; Allocate an array of structures.
  %p = call ptr @calloc(i64 8, i64 24)

  ; Call a function to do something.
  %val = call i32 @doSomething(ptr %p)

  ; Free the structure
  call void @free(ptr %p)
  ret void
}
; CHECK define void@test2()
; CHECK: %p = call ptr @calloc(i64 8, i64 8)


; calloc a buffer with a variable multiple of the size in the first argument.
define void @test3(i64 %val) {
  ; Allocate an array of structures.
  %sz = mul i64 %val, 24
  %p = call ptr @calloc(i64 %sz, i64 8)

  ; Call a function to do something.
  %val2 = call i32 @doSomething(ptr %p)

  ; Free the structure
  call void @free(ptr %p)
  ret void
}
; CHECK define void@test3(i64 %val)
; CHECK: %sz = mul i64 %val, 8
; CHECK: %p = call ptr @calloc(i64 %sz, i64 8)

; calloc a buffer with a variable multiple of the size in the second argument.
define void @test4(i64 %val) {
  ; Allocate an array of structures.
  %sz = mul i64 %val, 24
  %p = call ptr @calloc(i64 8, i64 %sz)

  ; Call a function to do something.
  %val2 = call i32 @doSomething(ptr %p)

  ; Free the structure
  call void @free(ptr %p)
  ret void
}
; CHECK define void@test4(i64 %val)
; CHECK: %sz = mul i64 %val, 8
; CHECK: %p = call ptr @calloc(i64 8, i64 %sz)

; calloc a buffer with an extended multiple of the size in the first argument.
define void @test5(i32 %val) {
  ; Allocate an array of structures.
  %sz = mul i32 %val, 24
  %sz64 = sext i32 %sz to i64
  %p = call ptr @calloc(i64 %sz64, i64 8)

  ; Call a function to do something.
  %val2 = call i32 @doSomething(ptr %p)

  ; Free the structure
  call void @free(ptr %p)
  ret void
}
; CHECK define void@test5(i32 %val)
; CHECK: %sz = mul i32 %val, 8
; CHECK: %sz64 = sext i32 %sz to i64
; CHECK: %p = call ptr @calloc(i64 %sz64, i64 8)

; calloc a buffer with an extended multiple of the size in the second argument.
define void @test6(i32 %val) {
  ; Allocate an array of structures.
  %sz = mul i32 %val, 24
  %sz64 = sext i32 %sz to i64
  %p = call ptr @calloc(i64 8, i64 %sz64)

  ; Call a function to do something.
  %val2 = call i32 @doSomething(ptr %p)

  ; Free the structure
  call void @free(ptr %p)
  ret void
}
; CHECK define void@test6(i32 %val)
; CHECK: %sz = mul i32 %val, 8
; CHECK: %sz64 = sext i32 %sz to i64
; CHECK: %p = call ptr @calloc(i64 8, i64 %sz64)

; calloc a buffer with both arguments matching the size.
define void @test7() {
  ; Allocate an array of structures.
  %p = call ptr @calloc(i64 24, i64 16)

  ; Call a function to do something.
  %val = call i32 @doSomething(ptr %p)

  ; Free the structure
  call void @free(ptr %p)
  ret void
}
; CHECK define void@test7()
; CHECK: %p = call ptr @calloc(i64 8, i64 16)

; calloc a buffer with a variable multiple of the size used for both arguments.
define void @test8(i64 %val) {
  ; Allocate an array of structures.
  %sz = mul i64 %val, 24
  %p = call ptr @calloc(i64 %sz, i64 %sz)

  ; Call a function to do something.
  %val2 = call i32 @doSomething(ptr %p)

  ; Free the structure
  call void @free(ptr %p)
  ret void
}
; CHECK define void@test8(i64 %val)
; CHECK: %sz.dt = mul i64 %val, 8
; CHECK: %sz = mul i64 %val, 24
; CHECK: %p = call ptr @calloc(i64 %sz.dt, i64 %sz)

; calloc a buffer with a variable multiple of the size which has another use.
define void @test9(i64 %val) {
  ; Allocate an array of structures.
  %sz = mul i64 %val, 24
  %other = add i64 %sz, 32
  %p = call ptr @calloc(i64 %sz, i64 %other)

  ; Call a function to do something.
  %val2 = call i32 @doSomething(ptr %p)

  ; Free the structure
  call void @free(ptr %p)
  ret void
}
; CHECK define void@test9(i64 %val)
; CHECK: %sz.dt = mul i64 %val, 8
; CHECK: %sz = mul i64 %val, 24
; CHECK: %other = add i64 %sz
; CHECK: %p = call ptr @calloc(i64 %sz.dt, i64 %other)

define i32 @main(i32 %argc, ptr "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !4 {
  call void @test1()
  call void @test2()
  %val = sext i32 %argc to i64
  call void @test3(i64 %val)
  call void @test4(i64 %val)
  call void @test5(i32 %argc)
  call void @test6(i32 %argc)
  call void @test7()
  call void @test8(i64 %val)
  call void @test9(i64 %val)
  ret i32 0
}

define i32 @doSomething(ptr "intel_dtrans_func_index"="1" %p_test) !intel.dtrans.func.type !6 {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, ptr %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, ptr %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, ptr %p_test, i64 0, i32 2

  ; read and write A and C
  store i32 1, ptr %p_test_A
  %valA = load i32, ptr %p_test_A
  store i32 2, ptr %p_test_C
  %valC = load i32, ptr %p_test_C
  %sum = add i32 %valA, %valC

  ret i32 %sum
}

declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0
declare !intel.dtrans.func.type !9 void @free(ptr "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i8 0, i32 2}  ; i8**
!4 = distinct !{!3}
!5 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = distinct !{!7}
!10 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }

!intel.dtrans.types = !{!10}
