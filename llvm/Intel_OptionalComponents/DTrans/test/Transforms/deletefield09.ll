; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the size argument of realloc calls are correctly
; updated in a variety of cases.

%struct.test = type { i32, i64, i32 }

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

define i32 @main(i32 %argc, i8** %argv) {
  ; Allocate a structure.
  %p = call i8* @realloc(i8* null, i64 16)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val = call i32 @doSomething(%struct.test* %p_test)

  ; Grow the buffer
  %ra1 = call i8* @realloc(i8* %p, i64 32)
  ; FIXME: This should be necessary, but apparently we don't transfer type
  ;        alias information after a realloc.
  %p_test2 = bitcast i8* %ra1 to %struct.test*

  ; Calculate a new size
  %mul = mul i32 64, %val
  %sz = zext i32 %mul to i64

  ; Change the size of the buffer
  %ra2 = call i8* @realloc(i8* %ra1, i64 %sz)
  ; FIXME: This should be necessary, but apparently we don't transfer type
  ;        alias information after a realloc.
  %p_test3 = bitcast i8* %ra2 to %struct.test*

  ; Use the value where we had the size constant.
  icmp eq i32 128, %mul

  ; Free the buffer
  call void @free(i8* %ra2)
  ret i32 %val
}


; CHECK: %__DFT_struct.test = type { i32, i32 }

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; CHECK: %p = call i8* @realloc(i8* null, i64 8)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: %val = call i32 @doSomething.1(%__DFT_struct.test* %p_test)
; CHECK: %ra1 = call i8* @realloc(i8* %p, i64 16)
; CHECK: %mul.dt = mul i32 32, %val
; CHECK: %mul = mul i32 64, %val
; CHECK: %sz = zext i32 %mul.dt to i64
; CHECK: %ra2 = call i8* @realloc(i8* %ra1, i64 %sz)
; CHECK: icmp eq i32 128, %mul


; CHECK: define internal i32 @doSomething.1(%__DFT_struct.test* %p_test)
; CHECK: %p_test_A = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* %p_test, i64 0, i32 0
; CHECK-NOT: %p_test_B = getelementptr
; CHECK: %p_test_C = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* %p_test, i64 0, i32 1


declare i8* @realloc(i8*, i64)
declare void @free(i8*)
