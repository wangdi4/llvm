; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that metadata type attachment for a global variable gets updated when
; its type is changed due to deleting a field from the structure stored within
; the array.

%struct.test = type { i64, i64, i64 }
@gArray = internal global [10 x %struct.test] zeroinitializer, !intel_dtrans_type !2
; CHECK: %__DFT_struct.test = type { i64, i64 } 
; CHECK: @gArray = internal global [10 x %__DFT_struct.test] zeroinitializer, !intel_dtrans_type ![[MD_ARRAY:[0-9]+]]

define i64 @test(i64 %idx) {
  %addr0 = getelementptr inbounds [10 x %struct.test], ptr @gArray, i64 0, i64 %idx, i32 0
  %v0 = load i64, ptr %addr0
  %addr1 = getelementptr inbounds [10 x %struct.test], ptr @gArray, i64 0, i64 %idx, i32 1
  %v1 = load i64, ptr %addr1
  %sum = add i64 %v0, %v1
  ret i64 %sum
}

!intel.dtrans.types = !{!4}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"A", i32 10, !3}  ; [10 x %struct.test]
!3 = !{%struct.test zeroinitializer, i32 0}  ; %struct.test
!4 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !1, !1} ; { i64, i64, i64 }

; CHECK: ![[MD_ARRAY]] = !{!"A", i32 10, ![[MD_STRUCT:[0-9]+]]}
; CHECK: ![[MD_STRUCT]] = !{%__DFT_struct.test zeroinitializer, i32 0}
