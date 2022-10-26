;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -passes=dtrans-reorderfieldsop | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the reordering transformation is NOT applied to
; struct.test because a memory write is done to the first field of the structure
; using a bitcast of a pointer to the structure. The transformation does not
; currently support rewriting the store instruction if the structure fields were
; reordered.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }
@var = internal global ptr zeroinitializer, !intel_dtrans_type !4

; CHECK-NOT: %__DFR_struct.test = type { i64, i64, i64, i32, i32, i32, i16 }
; CHECK: %struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define i32 @main() {
  %call = call ptr @calloc(i64 10, i64 48)
  store ptr %call, ptr @var

  ; The following instructions write the structure field, but reorder fields
  ; does not support transforming this write to the location of a reordered
  ; structure.
  store i32 10, ptr %call
  ret i32 0
}

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !2} ; { i32, i64, i32, i32, i16, i64, i64 }

!intel.dtrans.types = !{!7}

