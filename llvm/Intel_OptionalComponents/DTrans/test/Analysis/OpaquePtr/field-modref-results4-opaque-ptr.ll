; REQUIRES: asserts

; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=true -passes='require<dtrans-fieldmodrefop-analysis>' -dtrans-fieldmodref-eval -disable-output 2>&1 | FileCheck %s

; This test is to check the getModRefInfo interface function does not crash
; for 'InlineAsm' calls.
; TODO: It may be possible to parse any constraints specified for inline asm statements
; to provide less conservative results in the future.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i32, ptr }
define "intel_dtrans_func_index"="1" ptr @test01() !intel.dtrans.func.type !4 {
  %st = call ptr @malloc(i64 16)

  %scalar_field_addr = getelementptr %struct.test01, ptr %st, i64 0, i32 0
  store i32 8, ptr %scalar_field_addr

  %array_field_addr = getelementptr %struct.test01, ptr %st, i64 0, i32 1

  %ar1_mem = call ptr @malloc(i64 64)
  %cmp1 = icmp eq ptr %ar1_mem, null
  br i1 %cmp1, label %no_mem1, label %good1

no_mem1:
  store ptr null, ptr %array_field_addr
  br label %done

good1:
  call void @llvm.memset.p0i8.i64(ptr %ar1_mem, i8 0, i64 64, i1 false)

  store ptr %ar1_mem, ptr %array_field_addr
  br label %done

done:
  ret ptr %st
}

; Function that will contain calls to check results for.
define void @test01process(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !5 {
  %scalar_field_addr = getelementptr %struct.test01, ptr %in, i64 0, i32 0
  %array_field_addr = getelementptr %struct.test01, ptr %in, i64 0, i32 1
  %scalar_field_value = load i32, ptr %scalar_field_addr
  %array_begin = load ptr, ptr %array_field_addr

  ; Verify this reports NOMODREF for both fields to be sure the
  ; structure being checked is of interest.
  call void @test01none()

  ; Should report MODREF for both fields due to reachable inline asm.
  call void @test01escape(ptr %in)
  ret void
}

; Function that does not use the structure
define void @test01none() {
  ret void
}

; Function containing a call to a function using inline asm
define void @test01escape(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !6 {
  call void @test01stall()
  ret void
}

; Function using inline asm.
define void @test01stall() {
  tail call void asm sideeffect ".byte 0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00", "~{dirflag},~{fpsr},~{flags}"()
  ret void
}

define i32 @main() {
  %st = call ptr @test01()
  call void @test01process(ptr %st)

  ret i32 0
}

declare !intel.dtrans.func.type !15 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0
declare !intel.dtrans.func.type !16 void @llvm.memset.p0i8.i64(ptr "intel_dtrans_func_index"="1", i8, i64, i1)

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

; CHECK: FieldModRefQuery: - NoModRef   : [test01process]   %scalar_field_value = load i32, ptr %scalar_field_addr, align 4 --   call void @test01none()
; CHECK: FieldModRefQuery: - NoModRef   : [test01process]   %array_begin = load ptr, ptr %array_field_addr, align 8 --   call void @test01none()
; CHECK: FieldModRefQuery: - ModRef     : [test01process]   %scalar_field_value = load i32, ptr %scalar_field_addr, align 4 --   call void @test01escape(ptr %in)
; CHECK: FieldModRefQuery: - ModRef     : [test01process]   %array_begin = load ptr, ptr %array_field_addr, align 8 --   call void @test01escape(ptr %in)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = distinct !{!3}
!5 = distinct !{!3}
!6 = distinct !{!3}
!14 = !{i8 0, i32 1}  ; i8*
!15 = distinct !{!14}
!16 = distinct !{!14}
!17 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32, i32* }

!intel.dtrans.types = !{!17}

