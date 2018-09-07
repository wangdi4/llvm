; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the size argument of a calloc calls are correctly
; updated in a few different combinations.

%struct.test = type { i32, i64, i32 }

; CHECK: %__DFT_struct.test = type { i32, i32 }

; calloc a buffer with the size in the first argument.
define void @test1() {
  ; Allocate an array of structures.
  %p = call i8* @calloc(i64 16, i64 8)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val = call i32 @doSomething(%struct.test* %p_test)

  ; Free the structure
  call void @free(i8* %p)
  ret void
}

; CHECK define internal void@test1()
; CHECK: %p = call i8* @calloc(i64 8, i64 8)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: %val = call i32 @doSomething.1(%__DFT_struct.test* %p_test)

; calloc a buffer with the size in the second argument.
define void @test2() {
  ; Allocate an array of structures.
  %p = call i8* @calloc(i64 8, i64 16)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val = call i32 @doSomething(%struct.test* %p_test)

  ; Free the structure
  call void @free(i8* %p)
  ret void
}

; CHECK define internal void@test2()
; CHECK: %p = call i8* @calloc(i64 8, i64 8)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: %val = call i32 @doSomething.1(%__DFT_struct.test* %p_test)

; calloc a buffer with a variable multiple of the size in the first argument.
define void @test3(i64 %val) {
  ; Allocate an array of structures.
  %sz = mul i64 %val, 16
  %p = call i8* @calloc(i64 %sz, i64 8)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val2 = call i32 @doSomething(%struct.test* %p_test)

  ; Free the structure
  call void @free(i8* %p)
  ret void
}

; CHECK define internal void@test3(i64 %val)
; CHECK: %sz = mul i64 %val, 8
; CHECK: %p = call i8* @calloc(i64 %sz, i64 8)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: %val2 = call i32 @doSomething.1(%__DFT_struct.test* %p_test)

; calloc a buffer with a variable multiple of the size in the second argument.
define void @test4(i64 %val) {
  ; Allocate an array of structures.
  %sz = mul i64 %val, 16
  %p = call i8* @calloc(i64 8, i64 %sz)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val2 = call i32 @doSomething(%struct.test* %p_test)

  ; Free the structure
  call void @free(i8* %p)
  ret void
}

; CHECK define internal void@test4(i64 %val)
; CHECK: %sz = mul i64 %val, 8
; CHECK: %p = call i8* @calloc(i64 8, i64 %sz)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: %val2 = call i32 @doSomething.1(%__DFT_struct.test* %p_test)

; calloc a buffer with an extended multiple of the size in the first argument.
define void @test5(i32 %val) {
  ; Allocate an array of structures.
  %sz = mul i32 %val, 16
  %sz64 = sext i32 %sz to i64
  %p = call i8* @calloc(i64 %sz64, i64 8)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val2 = call i32 @doSomething(%struct.test* %p_test)

  ; Free the structure
  call void @free(i8* %p)
  ret void
}

; CHECK define internal void@test5(i32 %val)
; CHECK: %sz = mul i32 %val, 8
; CHECK: %sz64 = sext i32 %sz to i64
; CHECK: %p = call i8* @calloc(i64 %sz64, i64 8)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: %val2 = call i32 @doSomething.1(%__DFT_struct.test* %p_test)

; calloc a buffer with an extended multiple of the size in the second argument.
define void @test6(i32 %val) {
  ; Allocate an array of structures.
  %sz = mul i32 %val, 16
  %sz64 = sext i32 %sz to i64
  %p = call i8* @calloc(i64 8, i64 %sz64)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val2 = call i32 @doSomething(%struct.test* %p_test)

  ; Free the structure
  call void @free(i8* %p)
  ret void
}

; CHECK define internal void@test6(i32 %val)
; CHECK: %sz = mul i32 %val, 8
; CHECK: %sz64 = sext i32 %sz to i64
; CHECK: %p = call i8* @calloc(i64 8, i64 %sz64)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: %val2 = call i32 @doSomething.1(%__DFT_struct.test* %p_test)

; calloc a buffer with both arguments matching the size.
define void @test7() {
  ; Allocate an array of structures.
  %p = call i8* @calloc(i64 16, i64 16)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val = call i32 @doSomething(%struct.test* %p_test)

  ; Free the structure
  call void @free(i8* %p)
  ret void
}

; CHECK define internal void@test7()
; CHECK: %p = call i8* @calloc(i64 8, i64 16)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: %val = call i32 @doSomething.1(%__DFT_struct.test* %p_test)

; calloc a buffer with a variable multiple of the size used for both arguments.
define void @test8(i64 %val) {
  ; Allocate an array of structures.
  %sz = mul i64 %val, 16
  %p = call i8* @calloc(i64 %sz, i64 %sz)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val2 = call i32 @doSomething(%struct.test* %p_test)

  ; Free the structure
  call void @free(i8* %p)
  ret void
}

; CHECK define internal void@test8(i64 %val)
; CHECK: %sz.dt = mul i64 %val, 8
; CHECK: %sz = mul i64 %val, 16
; CHECK: %p = call i8* @calloc(i64 %sz.dt, i64 %sz)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: %val2 = call i32 @doSomething.1(%__DFT_struct.test* %p_test)

; calloc a buffer with a variable multiple of the size which has another use.
define void @test9(i64 %val) {
  ; Allocate an array of structures.
  %sz = mul i64 %val, 16
  %other = add i64 %sz, 32
  %p = call i8* @calloc(i64 %sz, i64 %other)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val2 = call i32 @doSomething(%struct.test* %p_test)

  ; Free the structure
  call void @free(i8* %p)
  ret void
}

; CHECK define internal void@test9(i64 %val)
; CHECK: %sz.dt = mul i64 %val, 8
; CHECK: %sz = mul i64 %val, 16
; CHECK: %other = add i64 %sz
; CHECK: %p = call i8* @calloc(i64 %sz.dt, i64 %other)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: %val2 = call i32 @doSomething.1(%__DFT_struct.test* %p_test)

define i32 @main(i32 %argc, i8** %argv) {
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


; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)

define i32 @doSomething(%struct.test* %p_test) {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 2

  ; read and write A and C
  store i32 1, i32* %p_test_A
  %valA = load i32, i32* %p_test_A
  store i32 2, i32* %p_test_C
  %valC = load i32, i32* %p_test_C

  ret i32 %valA
}

; CHECK: define internal i32 @doSomething.1(%__DFT_struct.test* %p_test)
; CHECK: %p_test_A = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* %p_test, i64 0, i32 0
; CHECK-NOT: %p_test_B = getelementptr
; CHECK: %p_test_C = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* %p_test, i64 0, i32 1

declare i8* @calloc(i64, i64)
declare void @free(i8*)
