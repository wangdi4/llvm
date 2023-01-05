; This test verifies that the field reordering transformation is applied
; correctly to byte flattened GEP instructions related to %struct.test.

;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -passes=dtrans-reorderfieldsop | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.01 = type { i32, i64, i32, i32, i16, i64, i64 }
@var01 = internal global ptr zeroinitializer, !intel_dtrans_type !4

; This struct is used to verify that no GEPs related to it are modified
; by reordering-fields.
%struct.test.02 = type { i32, i64 }
@var02 = internal global ptr zeroinitializer, !intel_dtrans_type !5

define i32 @main() {
entry:
  %call = tail call noalias ptr @calloc(i64 10, i64 48)
  store ptr %call, ptr @var01

  %F2 = getelementptr i8, ptr %call, i64 8
; CHECK: %F2 = getelementptr i8, ptr %call, i64 0

  %F3 = getelementptr i8, ptr %call, i64 16
; CHECK: %F3 = getelementptr i8, ptr %call, i64 28

  %F4 = getelementptr i8, ptr %call, i64 20
; CHECK: %F4 = getelementptr i8, ptr %call, i64 32

  %F5 = getelementptr i8, ptr %call, i64 24
; CHECK: %F5 = getelementptr i8, ptr %call, i64 36

  %call1 = tail call noalias ptr @calloc(i64 20, i64 16)
  store ptr %call1, ptr @var02
  tail call void @foo(ptr %call, ptr %call1)

; GEP shouldn't be modified
  %F9 = getelementptr i8, ptr %call1, i64 8
; CHECK: %F9 = getelementptr i8, ptr %call1, i64 8

  ret i32 0
}

define void @foo(ptr "intel_dtrans_func_index"="1" %tp, ptr "intel_dtrans_func_index"="2" %tp3) !intel.dtrans.func.type !6 {
entry:
  %F1 = getelementptr i8, ptr %tp, i64 0
; CHECK: %F1 = getelementptr i8, ptr %tp, i64 24

  %F6 = getelementptr i8, ptr %tp, i64 32
; CHECK: %F6 = getelementptr i8, ptr %tp, i64 8

  %F7 = getelementptr i8, ptr %tp, i64 40
; CHECK: %F7 = getelementptr i8, ptr %tp, i64 16

  %bp3 = bitcast ptr %tp3 to ptr

; GEP shouldn't be modified
  %F8 = getelementptr i8, ptr %bp3, i64 0
; CHECK: %F8 = getelementptr i8, ptr %bp3, i64 0

  ret void
}

declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{%struct.test.01 zeroinitializer, i32 1}  ; %struct.test.01*
!5 = !{%struct.test.02 zeroinitializer, i32 1}  ; %struct.test.02*
!6 = distinct !{!4, !5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = !{!"S", %struct.test.01 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !2} ; { i32, i64, i32, i32, i16, i64, i64 }
!10 = !{!"S", %struct.test.02 zeroinitializer, i32 2, !1, !2} ; { i32, i64 }

!intel.dtrans.types = !{!9, !10}
