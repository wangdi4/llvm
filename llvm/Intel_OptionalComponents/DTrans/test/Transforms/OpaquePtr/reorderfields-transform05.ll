; This test verifies that the field reordering transformation is applied
; correctly to a structure that has a field which is a pointer to another
; structure type that is being transformed by reorder fields.

;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -passes=dtrans-reorderfieldsop | FileCheck %s

; CHECK-DAG: %__DFR_struct.test01 = type { i64, i64, ptr, i32, i32, i32, i16 }
; CHECK-DAG: %__DFR_struct.test02 = type { i64, ptr, i64, i32, i32, i32, i16 }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i32, i64, i32, i32, i16, i64, ptr }
%struct.test02 = type { i32, i64, i32, i32, i16, ptr, i64 }

define i32 @main() {
entry:
  %call = tail call noalias ptr @calloc(i64 10, i64 48)
  %i = getelementptr %struct.test01, ptr %call, i64 0, i32 3
  store i32 10, ptr %i, align 8
  %call1 = tail call noalias ptr @calloc(i64 10, i64 48)
  %j = getelementptr %struct.test02, ptr %call1, i64 0, i32 2
  store i32 10, ptr %j, align 8
  ret i32 0
}

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !4} ; { i32, i64, i32, i32, i16, i64, %struct.test02* }
!9 = !{!"S", %struct.test02 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !5, !2} ; { i32, i64, i32, i32, i16, %struct.test01*, i64 }

!intel.dtrans.types = !{!8, !9}
