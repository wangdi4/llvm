; This test verifies that the field reordering transformation is applied
; correctly to sdiv and udiv instructions with constant and non-constant sizes
; related to %struct.T, which a structure nested within it is transformed.

;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-reorderfieldop-enable-legal-test=0 -S -passes=dtrans-reorderfieldsop | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@G =  global i32 20, align 4
%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }
%struct.T = type { %struct.test, i32 }


define void @bar(ptr "intel_dtrans_func_index"="1" %tp, ptr "intel_dtrans_func_index"="2" %tp2) !intel.dtrans.func.type !6 {
entry:
  %p1 = ptrtoint ptr %tp2 to i64
  %p2 = ptrtoint ptr %tp to i64
  %diff = sub i64  %p2, %p1
  %num = sdiv i64 %diff, 56
; CHECK: %num = sdiv i64 %diff, 48

  %p3 = ptrtoint ptr %tp2 to i64
  %p4 = ptrtoint ptr %tp to i64
  %diff1 = sub i64  %p3, %p4
  %ld = load i32, ptr @G, align 4
  %conv = sext i32 %ld to i64
  %mul = mul nsw i64 %conv, 56
  %num1 = udiv i64 %diff1, %mul
; CHECK: %0 = sdiv exact i64 %mul, 56
; CHECK: %1 = mul i64 %0, 48
; CHECK: %num1 = udiv i64 %diff1, %1

  ret void
}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{%struct.test zeroinitializer, i32 0}  ; %struct.test
!5 = !{%struct.T zeroinitializer, i32 1}  ; %struct.T*
!6 = distinct !{!5, !5}
!7 = !{!"S", %struct.test zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !2} ; { i32, i64, i32, i32, i16, i64, i64 }
!8 = !{!"S", %struct.T zeroinitializer, i32 2, !4, !1} ; { %struct.test, i32 }

!intel.dtrans.types = !{!7, !8}
