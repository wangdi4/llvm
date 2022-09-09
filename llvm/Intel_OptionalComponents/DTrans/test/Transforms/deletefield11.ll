; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt -whole-program-assume -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the size argument of realloc calls are correctly
; updated in a cloned function.

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

  ; Calculate a new size
  %mul = mul i32 64, %valC
  %sz = zext i32 %mul to i64

  ; Change the size of the buffer
  %p = bitcast %struct.test* %p_test to i8*
  %ra = call i8* @realloc(i8* %p, i64 %sz)
  ; FIXME: This bitcast should NOT be necessary, but we don't transfer the
  ;        local pointer analyzer type alias information after a realloc.
  %p_test2 = bitcast i8* %ra to %struct.test*

  ; Use the value where we had the size constant.
  icmp eq i32 128, %mul

  ; Free the buffer
  call void @free(i8* %ra)

  ret i32 %valA
}

define i32 @main(i32 %argc, i8** %argv) {
  ; Allocate a structure.
  %p = call i8* @realloc(i8* null, i64 16)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val = call i32 @doSomething(%struct.test* %p_test)

  ret i32 %val
}


; CHECK: %__DFT_struct.test = type { i32, i32 }

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; CHECK: %p = call i8* @realloc(i8* null, i64 8)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: %val = call i32 @doSomething.1(%__DFT_struct.test* %p_test)


; CHECK: define internal i32 @doSomething.1(%__DFT_struct.test* %p_test)
; CHECK: %p_test_A = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* %p_test, i64 0, i32 0
; CHECK-NOT: %p_test_B = getelementptr
; CHECK: %p_test_C = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* %p_test, i64 0, i32 1
; CHECK: %mul.dt = mul i32 32, %valC
; CHECK: %mul = mul i32 64, %valC
; CHECK: %sz = zext i32 %mul.dt to i64
; CHECK: %ra = call i8* @realloc(i8* %p, i64 %sz)
; CHECK: icmp eq i32 128, %mul

declare i8* @realloc(i8*, i64)
declare void @free(i8*)
