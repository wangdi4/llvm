; This test verifies that the field reordering transformation is applied
; correctly to GEP and calloc instructions related to %struct.test.

;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -passes=dtrans-reorderfieldsop | FileCheck %s

; CHECK-LABEL: define void @foo
; CHECK: getelementptr inbounds %__DFR_struct.test, ptr %tp, i64 0, i32 3
; CHECK: getelementptr inbounds %__DFR_struct.test, ptr %tp, i64 0, i32 0
; CHECK: getelementptr inbounds %__DFR_struct.test, ptr %tp, i64 0, i32 4
; CHECK: getelementptr inbounds %__DFR_struct.test, ptr %tp, i64 0, i32 5
; CHECK: getelementptr inbounds %__DFR_struct.test, ptr %tp, i64 0, i32 6
; CHECK: getelementptr inbounds %__DFR_struct.test, ptr %tp, i64 0, i32 1
; CHECK: getelementptr inbounds %__DFR_struct.test, ptr %tp, i64 0, i32 2

; CHECK-LABEL: define i32 @main
; CHECK: calloc(i64 10, i64 40)
; CHECK-NOT: calloc(i64 10, i64 48)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define void @foo(ptr "intel_dtrans_func_index"="1" %tp) !intel.dtrans.func.type !5 {
entry:
  %i = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 0
  %0 = load i32, ptr %i, align 8
  %add = add nsw i32 %0, 20
  %conv = sext i32 %add to i64
  %c = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 1
  store i64 %conv, ptr %c, align 8
  %add2 = add i32 %0, 40
  %t = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 2
  store i32 %add2, ptr %t, align 8
  %add5 = add i32 %0, 70
  %h = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 3
  store i32 %add5, ptr %h, align 4
  %1 = trunc i32 %0 to i16
  %conv8 = add i16 %1, 110
  %d = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
  store i16 %conv8, ptr %d, align 8
  %conv10 = sext i16 %conv8 to i64
  %add11 = add nsw i64 %conv10, 50
  %f = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 5
  store i64 %add11, ptr %f, align 8
  %add14 = add nsw i64 %conv10, 110
  %o = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 6
  store i64 %add14, ptr %o, align 8
  ret void
}

define i32 @main() {
entry:
  %call = tail call noalias ptr @calloc(i64 10, i64 48)
  %i = getelementptr %struct.test, ptr %call, i64 0, i32 0
  store i32 10, ptr %i, align 8
  tail call void @foo(ptr %call)
  ret i32 0
}

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!5 = distinct !{!4}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !2} ; { i32, i64, i32, i32, i16, i64, i64 }

!intel.dtrans.types = !{!8}
