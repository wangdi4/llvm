; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output < %s 2>&1 | FileCheck %s

; The test checks that a memset call updates an alloc function as following:
;   1) Non-zero value: sets to bottom
;   2) Zero value: keeps an existing alloc func

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test_01 = type { i8* }
%struct.test_02 = type { i8* }

; memset a pointer field with non-zero value (16)
define dso_local void @foo_01(%struct.test_01* %a) {
entry:
  %call = tail call noalias i8* @malloc(i64 100)
  %0 = bitcast %struct.test_01* %a to i8**
  store i8* %call, i8** %0, align 8
  %b2 = getelementptr inbounds %struct.test_01, %struct.test_01* %a, i64 0, i32 0
  %1 = bitcast i8** %b2 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull align 8 %1, i8 16, i64 8, i1 false)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test_01
; CHECK: Bottom Alloc Function

; memset a pointer field with zero value
define dso_local void @foo_02(%struct.test_02* %a) {
entry:
  %call = tail call noalias i8* @malloc(i64 100) #3
  %0 = bitcast %struct.test_02* %a to i8**
  store i8* %call, i8** %0, align 8
  %b2 = getelementptr inbounds %struct.test_02, %struct.test_02* %a, i64 0, i32 0
  %1 = bitcast i8** %b2 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull align 8 %1, i8 0, i64 8, i1 false)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test_02
; CHECK: Single Alloc Function: i8* (i64)* @malloc

declare dso_local noalias i8* @malloc(i64) local_unnamed_addr
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1)

