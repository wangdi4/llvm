; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -dtransanalysis -whole-program-assume -dtrans-print-types -dtrans-outofboundsok=false -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="require<dtransanalysis>" -whole-program-assume -dtrans-print-types -dtrans-outofboundsok=false -disable-output < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test checks that an access to structure field based on select with constant arguments is treated without setting Bad pointer manipulation.

; Test1
; CHECK-LABEL: LLVMType: %struct.s1 = type
; CHECK: Safety data:
; CHECK-NOT: Bad pointer manipulation

%struct.s1 = type { i32, i64, i32, i64 }
define dso_local void @test1(i32 %x) {
entry:
  %base = tail call i8* @malloc(i64 32)
  %bc = bitcast i8* %base to %struct.s1*
  %cmp = icmp sgt i32 %x, 1
  %sel = select i1 %cmp, i64 0, i64 16
  %gep = getelementptr inbounds i8, i8* %base, i64 %sel
  ret void
}

; Select with one non-constant argument.

; Test2
; CHECK-LABEL: LLVMType: %struct.s2 = type
; CHECK: Safety data:
; CHECK: Bad pointer manipulation

%struct.s2 = type { i32, i64, i32, i64 }
define dso_local void @test2(i64 %x) {
entry:
  %base = tail call i8* @malloc(i64 32)
  %bc = bitcast i8* %base to %struct.s2*
  %cmp = icmp sgt i64 %x, 1
  %sel = select i1 %cmp, i64 0, i64 %x
  %gep = getelementptr inbounds i8, i8* %base, i64 %sel
  ret void
}

; Non-constant GEP offset which is not select.

; Test3
; CHECK-LABEL: LLVMType: %struct.s3 = type
; CHECK: Safety data:
; CHECK: Bad pointer manipulation

%struct.s3 = type { i32, i64, i32, i64 }
define dso_local void @test3(i64 %x) {
entry:
  %base = tail call i8* @malloc(i64 32)
  %bc = bitcast i8* %base to %struct.s3*
  %gep = getelementptr inbounds i8, i8* %base, i64 %x
  ret void
}

declare noalias i8* @malloc(i64)
