; REQUIRES: asserts
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes='require<dtrans-fieldmodrefop-analysis>' -debug-only=dtrans-fmr -disable-output 2>&1 | FileCheck %s

; Verify that literal structure type that is a member of
; a structure that gets analyzed by the field mod/ref analysis
; does not cause the compiler to crash. (CMPLRLLVM-43666)

; CHECK: Disqualifying all fields of structure based on safety data: %struct.test01

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; In this case, the safety analyzer set safety bits that
; indicate the structure is invalid for field mod/ref. This
; should cause all the structure to not be a candidate.
%struct.test01 = type { i32, ptr, { i32, i64, i32 } }
@gAr02 = internal global %struct.test01* zeroinitializer, !intel_dtrans_type !6
define internal void @test01() {
  %st_mem = call ptr @malloc(i64 16)

  %f0 = getelementptr %struct.test01, ptr %st_mem, i64 0, i32 0
  store i32 8, ptr %f0

  %f1 = getelementptr %struct.test01, ptr %st_mem, i64 0, i32 1
  %f1_i8 = bitcast ptr %f1 to ptr

  %ar1_mem = call ptr @malloc(i64 64)
  %cmp1 = icmp eq ptr %ar1_mem, null
  br i1 %cmp1, label %no_mem1, label %good1

no_mem1:
  store ptr null, ptr %f1_i8
  br label %done

good1:
  store ptr %ar1_mem, ptr %f1_i8

  ; Escape the allocated memory
  store ptr %ar1_mem, ptr @gAr02
  br label %done

done:
  ret void
}

declare !intel.dtrans.func.type !8  "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{double 0.0e+00, i32 1}  ; double*
!3 = !{!"L", i32 3, !1, !4, !1}  ; { i32, i64, i32 }
!4 = !{i64 0, i32 0}  ; i64
!6 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, double*, { i32, i64, i32 } }

!intel.dtrans.types = !{!9}
