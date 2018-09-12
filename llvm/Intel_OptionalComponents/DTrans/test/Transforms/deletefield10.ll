; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt  -whole-program-assume -dtrans-identify-unused-values=false -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the size argument of memfunc calls are correctly.

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
  ; Allocate an array of structures.
  %base = zext i32 %argc to i64
  %n = add i64 %base, 4
  %sz = mul i64 %n, 16
  %p = call i8* @malloc(i64 %sz)
  %p_test = bitcast i8* %p to %struct.test*

  ; Zero initialize the structures
  call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 %sz, i1 false)

  ; Call a function to do something to the first structure.
  %val = call i32 @doSomething(%struct.test* %p_test)

  ; Copy the first structure to the second.
  %p2 = getelementptr %struct.test, %struct.test* %p_test, i64 1
  %p2_i8 = bitcast %struct.test* %p2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p, i8* %p2_i8, i64 16, i1 false)

  ; Move the contents of the first two structures to the third and fourth
  %p3 = getelementptr %struct.test, %struct.test* %p_test, i64 2
  %p3_i8 = bitcast %struct.test* %p3 to i8*
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %p, i8* %p3_i8, i64 32, i1 false)

  ; Free the buffer
  call void @free(i8* %p)
  ret i32 %val
}


; CHECK: %__DFT_struct.test = type { i32, i32 }

; CHECK-LABEL: define i32 @main(i32 %argc, i8** %argv)
; CHECK: %base = zext i32 %argc to i64
; CHECK: %n = add i64 %base, 4
; FIXME: This is kind of unfortunate. We're cloning this because it has two uses
;        but both uses are as size computations that we're updating for the same
;        structure. It would be nice to recognize that and not clone it.
; CHECK: %sz.dt = mul i64 %n, 8
; CHECK: %sz = mul i64 %n, 8
; CHECK: %p = call i8* @malloc(i64 %sz.dt)
; CHECK: %p_test = bitcast i8* %p to %__DFT_struct.test*
; CHECK: call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 %sz, i1 false)
; CHECK: %val = call i32 @doSomething.1(%__DFT_struct.test* %p_test)
; CHECK: %p2 = getelementptr %__DFT_struct.test, %__DFT_struct.test* %p_test, i64 1
; CHECK: %p2_i8 = bitcast %__DFT_struct.test* %p2 to i8*
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p, i8* %p2_i8, i64 8, i1 false)
; CHECK: %p3 = getelementptr %__DFT_struct.test, %__DFT_struct.test* %p_test, i64 2
; CHECK: %p3_i8 = bitcast %__DFT_struct.test* %p3 to i8*
; CHECK: call void @llvm.memmove.p0i8.p0i8.i64(i8* %p, i8* %p3_i8, i64 16, i1 false)


; CHECK: define internal i32 @doSomething.1(%__DFT_struct.test* %p_test)
; CHECK: %p_test_A = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* %p_test, i64 0, i32 0
; CHECK-NOT: %p_test_B = getelementptr
; CHECK: %p_test_C = getelementptr %__DFT_struct.test,
; CHECK-SAME:                      %__DFT_struct.test* %p_test, i64 0, i32 1


declare i8* @malloc(i64)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)
declare void @llvm.memmove.p0i8.p0i8.i64(i8* , i8*, i64, i1)
declare void @free(i8*)
