; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-usecrulecompat -dtrans-fieldmodref-analysis -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-usecrulecompat -passes='require<dtrans-fieldmodref-analysis>' -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check handling of address taken functions for field based Mod/Ref analysis.
; In this case, the address taken function is not allowed because it is passed
; as a parameter to an indirect function call.
%struct.test01 = type { i32, i32*, i64 }
@funcAddr = global void (%struct.test01*, void (%struct.test01*)*)* zeroinitializer

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

  %fptr = load void (%struct.test01*, void (%struct.test01*)*)*, void (%struct.test01*, void (%struct.test01*)*)** @funcAddr

  ; Pass the address of a function to an indirect function call.
  call void %fptr(%struct.test01* %st, void (%struct.test01*)* @filter01a)
  ret void
}

define void @use01a(%struct.test01* %st, void (%struct.test01*)* nocapture %filter) {
  call void %filter(%struct.test01* %st)
  ret void
}

; Function that will be address taken.
define void @filter01a(%struct.test01* %st) {
  %fieldaddr = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  %ld1 = load i32, i32* %fieldaddr
  ret void
}

; Field 0 is accessed by a function that is address taken, so should get marked
; as 'bottom'.

; CHECK: ModRef candidate structures after analysis:
; CHECK-LABEL: LLVMType: %struct.test01 = type { i32, i32*, i64 }
; CHECK: 0)Field LLVM Type: i32
; CHECK: RWState: bottom
; CHECK: 1)Field LLVM Type: i32*
; CHECK: RWState: computed
; CHECK: 2)Field LLVM Type: i64
; CHECK: RWState: computed


declare i8* @malloc(i64)
