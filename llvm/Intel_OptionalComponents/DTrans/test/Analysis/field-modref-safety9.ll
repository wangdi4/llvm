; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-usecrulecompat -dtrans-fieldmodref-analysis -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-usecrulecompat -passes='require<dtrans-fieldmodref-analysis>' -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check handling of address taken functions for field based Mod/Ref analysis
; for a case using a broker function.

; Pass function address to a broker function as arguments that are
; not the callback function, and not pass-through arguments. These
; functions should be treated as address-taken for the analysis, rather
; than as asafe calls.

%struct.test01 = type { i32, i32*, i64 }

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

  ; Based on the callback metadata descriptor, arguments 0 and 1, are not forwarded
  ; to the callback routine as nocapture arguments, and should cause fields accessed
  ; by them to be set to 'bottom'.
  tail call void (void (%struct.test01*)*, void (%struct.test01*)*, void (i32*, i32*, ...)*, ...) @broker(
    void (%struct.test01*)* @filter01a,
    void (%struct.test01*)* @filter01b,
    void (i32*, i32*, ...)* bitcast
      (void (i32*, i32*, i64, %struct.test01*)* @use01a to void (i32*, i32*, ...)*),
    i64 1,
    %struct.test01* %st
  )

  ret void
}

; Function that will be address taken because it is a callback for the broker
; function.
define void @use01a(i32* %in0, i32* %in1, i64 %in2, %struct.test01* %in3) {
  ret void
}

; Function that will be address taken because it is a pass-through to the
; callback function.
define void @filter01a(%struct.test01* %st) {
  %fieldaddr = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  %ld1 = load i32, i32* %fieldaddr
  ret void
}

; Function that will be address taken because it is a pass-through to the
; callback function.
define void @filter01b(%struct.test01* %st) {
  %fieldaddr = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 2
  store i64 0, i64* %fieldaddr
  ret void
}

declare !callback !0 void @broker(void (%struct.test01*)*, void (%struct.test01*)*, void (i32*, i32*, ...)*, ...)
declare i8* @malloc(i64)

!0 = !{!1}
!1 = !{i64 2, i64 -1, i64 -1, i1 true}


; CHECK: ModRef candidate structures after analysis:
; CHECK-LABEL: LLVMType: %struct.test01 = type { i32, i32*, i64 }
; CHECK: 0)Field LLVM Type: i32
; CHECK: RWState: bottom
; CHECK: 1)Field LLVM Type: i32*
; CHECK: RWState: computed
; CHECK: 2)Field LLVM Type: i64
; CHECK: RWState: bottom
