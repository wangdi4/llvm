; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=true -passes='require<dtrans-fieldmodref-analysis>' -dtrans-fieldmodref-eval -disable-output 2>&1 | FileCheck %s

; This test is to check the getModRefInfo interface function does not crash
; for 'InlineAsm' calls.
; TODO: It may be possible to parse any constraints specified for inline asm statements
; to provide less conservative results in the future.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i32, i32* }
define %struct.test01* @test01() {
  %st_mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %st_mem to %struct.test01*

  %scalar_field_addr = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  store i32 8, i32* %scalar_field_addr

  %array_field_addr = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 1

  %ar1_mem = call i8* @malloc(i64 64)
  %cmp1 = icmp eq i8* %ar1_mem, null
  br i1 %cmp1, label %no_mem1, label %good1

no_mem1:
  store i32* null, i32** %array_field_addr
  br label %done

good1:
  call void @llvm.memset.p0i8.i64(i8* %ar1_mem, i8 0, i64 64, i1 false)

  %ar1_mem2 = bitcast i8* %ar1_mem to i32*
  store i32* %ar1_mem2, i32** %array_field_addr
  br label %done

done:
  ret %struct.test01* %st
}

; Function that will contain calls to check results for.
define void @test01process(%struct.test01* %in) {
  %scalar_field_addr = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
  %array_field_addr = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  %scalar_field_value = load i32, i32* %scalar_field_addr
  %array_begin = load i32*, i32** %array_field_addr

  ; Verify this reports NOMODREF for both fields to be sure the
  ; structure being checked is of interest.
  call void @test01none()

  ; Should report MODREF for both fields due to reachable inline asm.
  call void @test01escape(%struct.test01* %in)
  ret void
}

; Function that does not use the structure
define void @test01none() {
  ret void
}

; Function containing a call to a function using inline asm
define void @test01escape(%struct.test01* %in) {
  call void @test01stall()
  ret void
}

; Function using inline asm.
define void @test01stall() {
  tail call void asm sideeffect ".byte 0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00", "~{dirflag},~{fpsr},~{flags}"()
  ret void
}

define i32 @main() {
  %st = call %struct.test01* @test01()
  call void @test01process(%struct.test01* %st)

  ret i32 0
}

declare i8* @malloc(i64) #0
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

; CHECK: FieldModRefQuery: - NoModRef   : [test01process]   %scalar_field_value = load i32, i32* %scalar_field_addr, align 4 --   call void @test01none()
; CHECK: FieldModRefQuery: - NoModRef   : [test01process]   %array_begin = load i32*, i32** %array_field_addr, align 8 --   call void @test01none()
; CHECK: FieldModRefQuery: - ModRef     : [test01process]   %scalar_field_value = load i32, i32* %scalar_field_addr, align 4 --   call void @test01escape(%struct.test01* %in)
; CHECK: FieldModRefQuery: - ModRef     : [test01process]   %array_begin = load i32*, i32** %array_field_addr, align 8 --   call void @test01escape(%struct.test01* %in)
