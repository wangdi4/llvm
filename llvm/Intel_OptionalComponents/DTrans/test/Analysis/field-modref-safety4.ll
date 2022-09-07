; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-usecrulecompat -dtrans-fieldmodref-analysis -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-usecrulecompat -passes='require<dtrans-fieldmodref-analysis>' -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check handling of address taken functions for field based Mod/Ref analysis
; when the address taken function does not access the structure field, but calls
; a routine that accesses a structure field. This should cause the field to be
; set to 'bottom'.
%struct.test01 = type { i32, i32* }
@funcptr = global void (%struct.test01*)* @func_at

define internal void @test01() {
  %st_mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %st_mem to %struct.test01*

  %f0 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  store i32 8, i32* %f0

  %ar1_mem = call i8* @malloc(i64 64)
  %ar1_mem2 = bitcast i8* %ar1_mem to i32*
  %f1 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 1
  store i32* %ar1_mem2, i32** %f1

  call void @read01.0(%struct.test01* %st)
  ret void
}

define void @func_at(%struct.test01* %st) {
  call void @read01.0(%struct.test01* %st)
  ret void
}

define void @read01.0(%struct.test01* %st) {
  %fieldaddr = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  %ld1 = load i32, i32* %fieldaddr
  ret void
}
; Field 0 should get set to bottom because it is used by a function reachable
; from the address taken function.

; CHECK-LABEL: LLVMType: %struct.test01 = type { i32, i32* }
; CHECK: 0)Field LLVM Type: i32
; CHECK: RWState: bottom
; CHECK: 1)Field LLVM Type: i32*
; CHECK: RWState: computed

declare i8* @malloc(i64)
