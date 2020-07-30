; REQUIRES: asserts
;
; Test DTrans integration in the old pass manager.
;
; This test is to verify that DTrans analysis gets recomputed if a transform
; modifies the structure types.

; RUN: opt -disable-verify -debug-pass=Executions -debug-only=dtransanalysis \
; RUN:     -whole-program-assume -enable-dtrans-soatoaos                     \
; RUN      -enable-dtrans-deletefield -enable-resolve-types                  \
; RUN:     -std-link-opts -S  %s -enable-dtrans                              \
; RUN:     2>&1 \
; RUN:     | FileCheck %s

; Make sure the DTrans analysis gets recomputed at the start of a
; transformation that follows one which changed the structure typess. In this
; case, DeleteField and ReorderFields are going to modify the IR.
; Also, DTrans analysis should not be run prior to the resolve types pass.

; CHECK-NOT: Executing Pass 'Data transformation analysis'
; CHECK: Executing Pass 'DTrans resolve types'
; CHECK: Running DTransAnalysisInfo::analyzeModule
; CHECK: Executing Pass 'DTrans Mem Init Trim Down'
; CHECK: Executing Pass 'DTrans struct of arrays to array of structs'
; CHECK-NOT: Running DTransAnalysisInfo::analyzeModule
; CHECK: Executing Pass 'DTrans weak align'
; CHECK-NOT: Running DTransAnalysisInfo::analyzeModule
; CHECK: Executing Pass 'DTrans delete field'
; CHECK-NOT: Running DTransAnalysisInfo::analyzeModule
; CHECK: Executing Pass 'DTrans reorder fields' on Module
; CHECK: Running DTransAnalysisInfo::analyzeModule
; CHECK: Executing Pass 'DTrans array of structs to struct of arrays'
; CHECK: Running DTransAnalysisInfo::analyzeModule
; CHECK: Executing Pass 'DTrans eliminate read only field access'
; CHECK-NOT: Running DTransAnalysisInfo::analyzeModule
; CHECK: Executing Pass 'DTrans dynamic cloning'
; CHECK-NOT: Running DTransAnalysisInfo::analyzeModule
; CHECK: Executing Pass 'DTrans annotator cleaner'

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i16, i64, i32, i64, i16 }

define i64 @doSomething(%struct.test* %p_test) {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 2
  %p_test_D = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 3
  %p_test_E = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 4

  ; read and write A, C and D
  store i16 1, i16* %p_test_A
  %valA = load i16, i16* %p_test_A
  store i32 2, i32* %p_test_C
  %valC = load i32, i32* %p_test_C
  store i64 3, i64* %p_test_D
  %valD = load i64, i64* %p_test_D
  store i16 4, i16* %p_test_E
  %valE = load i16, i16* %p_test_E

  %ext1 = sext i16 %valA to i64
  %ext2 = sext i32 %valC to i64
  %sum1 = add i64 %ext1, %ext2
  %sum2 = add i64 %sum1, %valD
  %ext3 = sext i16 %valE to i64
  %sum3 = add i64 %sum2, %ext3

  ret i64 %sum3
}

define i32 @main(i32 %argc, i8** %argv) {
  ; Allocate a structure.
  %p = call i8* @malloc(i64 40)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val = call i64 @doSomething(%struct.test* %p_test)

  ; Free the structure
  call void @free(i8* %p)
  ret i32 0
}

declare i8* @malloc(i64)
declare void @free(i8*)
