; REQUIRES: asserts
; This test verifies dtrans field reordering transformation doesn't select
; struct.test as candidate due to safety conditions.

;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-reorderfieldsop -debug-only=dtrans-reorderfieldsop -disable-output 2>&1 | FileCheck %s

; CHECK: Rejecting struct.test based on safety data

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, i64, i32, i32, i16, %struct.test1, i64 }
%struct.test1 = type { i64 }

define void @foo(ptr "intel_dtrans_func_index"="1" %tp) !intel.dtrans.func.type !6 {
entry:
  %i = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 0
  %i1 = load i32, ptr %i, align 8
  %add = add nsw i32 %i1, 20
  %conv = sext i32 %add to i64
  %c = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 1
  store i64 %conv, ptr %c, align 8
  %add2 = add i32 %i1, 40
  %t = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 2
  store i32 %add2, ptr %t, align 8
  %add5 = add i32 %i1, 70
  %h = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 3
  store i32 %add5, ptr %h, align 4
  %i2 = trunc i32 %i1 to i16
  %conv8 = add i16 %i2, 110
  %d = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
  store i16 %conv8, ptr %d, align 8
  %conv10 = sext i16 %conv8 to i64
  %f = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 5, i32 0
  %i3 = insertelement <2 x i64> undef, i64 %conv10, i32 0
  %i4 = shufflevector <2 x i64> %i3, <2 x i64> undef, <2 x i32> zeroinitializer
  %i5 = add nsw <2 x i64> %i4, <i64 50, i64 110>
  store <2 x i64> %i5, ptr %f, align 8
  ret void
}

define i32 @main() {
entry:
  ret i32 0
}

!intel.dtrans.types = !{!0, !5}

!0 = !{!"S", %struct.test zeroinitializer, i32 7, !1, !2, !1, !1, !3, !4, !2}
!1 = !{i32 0, i32 0}
!2 = !{i64 0, i32 0}
!3 = !{i16 0, i32 0}
!4 = !{%struct.test1 zeroinitializer, i32 0}
!5 = !{!"S", %struct.test1 zeroinitializer, i32 1, !2}
!6 = distinct !{!7}
!7 = !{%struct.test zeroinitializer, i32 1}

