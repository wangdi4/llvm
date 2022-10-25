; REQUIRES: asserts
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types < %s 2>&1

; Test the DTrans safety analyzer when a GEP that addresses a field within a
; structure by using an offset that is a multiple of the size of a pointer,
; rather than the normal multi-operand indexing method to get the address of
; structure fields. Currently, the structure type that is determined to be
; an element pointee will be marked as 'Unhandled use'

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.BMLog = type { ptr, ptr, ptr, %struct.ListBase, ptr }
%struct.RangeTreeUInt = type { i64 }
%struct.ListBase = type { ptr, ptr }

define void @test(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !7 {
  ; The normal way of accessing field 4
  %field4 = getelementptr %struct.BMLog, ptr %in, i64 0, i32 4
  %val = load ptr, ptr %field4

  ; Another way of accessing field 4 by computing a 40-byte offset from the
  ; start of the structure.
  %also_field4 = getelementptr ptr, ptr %in, i64 5
  store ptr %val, ptr %also_field4
  ret void
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.BMLog
; CHECK: Safety data: Contains nested structure | Unhandled use{{$}}
; CHECK: End LLVMType: %struct.BMLog

!intel.dtrans.types = !{!8, !9, !10}

!1 = !{%struct.RangeTreeUInt zeroinitializer, i32 1}  ; %struct.RangeTreeUInt*
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{%struct.ListBase zeroinitializer, i32 0}  ; %struct.ListBase
!4 = !{i64 0, i32 0}  ; i64
!5 = !{i8 0, i32 1}  ; i8*
!6 = !{%struct.BMLog zeroinitializer, i32 1}  ; %struct.BMLog*
!7 = distinct !{!6}
!8 = !{!"S", %struct.BMLog zeroinitializer, i32 5, !1, !2, !2, !3, !2} ; { %struct.RangeTreeUInt*, i32*, i32*, %struct.ListBase, i32* }
!9 = !{!"S", %struct.RangeTreeUInt zeroinitializer, i32 1, !4} ; { i64 }
!10 = !{!"S", %struct.ListBase zeroinitializer, i32 2, !5, !5} ; { i8*, i8* }
