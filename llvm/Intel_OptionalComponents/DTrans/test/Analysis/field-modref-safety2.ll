; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=true -dtrans-fieldmodref-analysis -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=true -passes='require<dtrans-fieldmodref-analysis>' -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; These tests are to check the behavior of the field based Mod/Ref analysis
; when -dtrans-outofboundsok is enabled.

; Test a simple case where a field is address taken.
; In this case, %stuct.test01 should not be selected as a candidate for
; further analysis.
%struct.test01 = type { i32, i32* }
@gTest01 = internal global i32* zeroinitializer
define internal void @test01() {
  %st_mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %st_mem to %struct.test01*

  %f0 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  store i32 8, i32* %f0

  %f1 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 1

  %ar1_mem = call i8* @malloc(i64 64)
  %cmp1 = icmp eq i8* %ar1_mem, null
  br i1 %cmp1, label %no_mem1, label %good1

no_mem1:
  store i32* null, i32** %f1
  br label %done

good1:
  call void @llvm.memset.p0i8.i64(i8* %ar1_mem, i8 0, i64 64, i1 false)

  %ar1_mem2 = bitcast i8* %ar1_mem to i32*
  store i32* %ar1_mem2, i32** %f1
  br label %done

done:
  ; Take the address of the first field.
  store i32* %f0, i32** @gTest01
  ret void
}
; CHECK: ModRef candidate structures after analysis:
; CHECK-NOT: LLVMType: %struct.test01 = type { i32, i32* }


; Test allocating storage for a field that does escape. Because
; -dtrans-outofboundsok is enabled, this should disqualify all fields.
%struct.test02 = type { i32, i32* }
@gAr02 = internal global i32* zeroinitializer

define internal void @test02() {
  %st_mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %st_mem to %struct.test02*

  %f0 = getelementptr %struct.test02, %struct.test02* %st, i64 0, i32 0
  store i32 8, i32* %f0

  %f1 = getelementptr %struct.test02, %struct.test02* %st, i64 0, i32 1
  %f1_i8 = bitcast i32** %f1 to i8**

  %ar1_mem = call i8* @malloc(i64 64)
  %cmp1 = icmp eq i8* %ar1_mem, null
  br i1 %cmp1, label %no_mem1, label %good1

no_mem1:
  store i8* null, i8** %f1_i8
  br label %done

good1:
  store i8* %ar1_mem, i8** %f1_i8
  %ar1_mem2 = bitcast i8* %ar1_mem to i32*

  ; Escape the allocated memory
  store i32* %ar1_mem2, i32** @gAr02
  br label %done

done:
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test02 = type { i32, i32* }
; CHECK: 0)Field LLVM Type: i32
; CHECK: RWState: bottom
; CHECK: 1)Field LLVM Type: i32*
; CHECK: RWState: bottom


declare i8* @malloc(i64)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
