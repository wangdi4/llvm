; This test verifies that the field reordering transformation
; is applied to struct.test and a new struct is created with a
; different layout.

;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -passes=dtrans-reorderfieldsop | FileCheck %s

; CHECK: %__DFR_struct.test = type { i64, i64, i64, i32, i32, i32, i16 }
; CHECK-NOT: %struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

; CHECK: !intel.dtrans.types = !{![[MD_STRUCT:[0-9]+]]}

; CHECK: ![[MD_STRUCT]] = !{!"S", %__DFR_struct.test zeroinitializer, i32 7, ![[MD_I64:[0-9]+]], ![[MD_I64]], ![[MD_I64]], ![[MD_I32:[0-9]+]], ![[MD_I32]], ![[MD_I32]], ![[MD_I16:[0-9]+]]}
; CHECK: ![[MD_I64]] = !{i64 0, i32 0}
; CHECK: ![[MD_I32]] = !{i32 0, i32 0}
; CHECK: ![[MD_I16]] = !{i16 0, i32 0}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define i32 @main() {
entry:
  %call = tail call noalias i8* @calloc(i64 10, i64 48)
  %i = getelementptr %struct.test, ptr %call, i64 0, i32 0
  store i32 10, ptr %i, align 8
  ret i32 0
}

declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!intel.dtrans.types = !{!6}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = !{!"S", %struct.test zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !2} ; { i32, i64, i32, i32, i16, i64, i64 }
