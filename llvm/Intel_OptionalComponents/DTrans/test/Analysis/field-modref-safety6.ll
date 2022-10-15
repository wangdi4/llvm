; REQUIRES: asserts

; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -passes='require<dtrans-fieldmodref-analysis>' -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check handling of address taken functions for field based Mod/Ref analysis.
; In this case, the address taken function is not allowed because it is passed
; as a parameter to another function, which stores the address to a global
; variable, which could allow the function to be called from some other
; function.
%struct.test01 = type { i32, i32*, i64 }

@current_func = global void (%struct.test01*)* zeroinitializer
define internal void @test01() {
  %st_mem = call i8* @malloc(i64 24)
  %st = bitcast i8* %st_mem to %struct.test01*

  %f0 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  store i32 8, i32* %f0

  %ar1_mem = call i8* @malloc(i64 64)
  %ar1_mem2 = bitcast i8* %ar1_mem to i32*
  %f1 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 1
  store i32* %ar1_mem2, i32** %f1

  %f2 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 2
  store i64 1, i64* %f2

  ; Create an address taken condition by passing function addresses to a
  ; function.
  call void @use01a(%struct.test01* %st, void (%struct.test01*)* @filter01a)
  call void @use01a(%struct.test01* %st, void (%struct.test01*)* @filter01b)
  ret void
}

; Function pointer is not 'nocapture' in this case, and gets saved to memory,
; which will cause the structure fields that are accessible to be set to
; 'bottom'.
define void @use01a(%struct.test01* %st, void (%struct.test01*)* %filter) {
  store void (%struct.test01*)* %filter, void (%struct.test01*)** @current_func
  call void %filter(%struct.test01* %st)
  ret void
}

; Function that will be address taken
define void @filter01a(%struct.test01* %st) {
  call void @read01.0(%struct.test01* %st)
  ret void
}

; Function that will be address taken
define void @filter01b(%struct.test01* %st) {
  call void @read01.1(%struct.test01* %st)
  ret void
}

; Function called by address taken function that accesses structure fields.
define void @read01.0(%struct.test01* %st) {
  %fieldaddr = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  %ld1 = load i32, i32* %fieldaddr
  ret void
}

; Function called by address taken function that accesses structure fields.
define void @read01.1(%struct.test01* %st) {
  %fieldaddr = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 1
  %ld1 = load i32*, i32** %fieldaddr
  ret void
}

; Fields 0 and 1 are accessed from functions that reachable from the address
; taken functions, and the function pointers are captured into a memory
; location. This should result in these fields being marked as 'bottom'.

; CHECK: ModRef candidate structures after analysis:
; CHECK-LABEL: LLVMType: %struct.test01 = type { i32, i32*, i64 }
; CHECK: 0)Field LLVM Type: i32
; CHECK: RWState: bottom
; CHECK: 1)Field LLVM Type: i32*
; CHECK: RWState: bottom
; CHECK: 2)Field LLVM Type: i64
; CHECK: RWState: computed


declare i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
