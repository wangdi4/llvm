; This test verifies that reuse field transformation applied to 4-th field of struct.test
; and replace it to 3-th field in both cal_0 and cal_1.
; REQUIRES: asserts
; RUN: opt -opaque-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume < %s -passes=dtrans-reusefieldop -debug-only=dtrans-reusefieldop -disable-output 2>&1 | FileCheck %s

; CHECK:Reused structure: %struct.test
; CHECK-NEXT:    Field mapping are (From:To): { 3:3 4:3 }
; CHECK-NEXT:GEP (before):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
; CHECK-NEXT:New GEP (after):
; CHECK-NEXT:  %0 = getelementptr %struct.test, ptr %tp, i64 0, i32 3
; CHECK-NEXT:Load:
; CHECK-NEXT:  %b = load i64, ptr %0, align 8
; CHECK-NEXT:GEP (before):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
; CHECK-NEXT:New GEP (after):
; CHECK-NEXT:  %0 = getelementptr %struct.test, ptr %tp, i64 0, i32 3
; CHECK-NEXT:Load:
; CHECK-NEXT:  %b = load i64, ptr %0, align 8

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, i64, i32, i64, i64 }

define void @foo_0(%struct.test* "intel_dtrans_func_index"="1" %tp) !intel.dtrans.func.type !6 {
entry:
  %i = getelementptr inbounds i8, ptr %tp, i32 0
  %0 = load i32, ptr %i, align 8
  %add = add nsw i32 %0, 20
  %conv = sext i32 %add to i64
  %c = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 1
  store i64 %conv, ptr %c, align 8
  %add2 = add i32 %0, 40
  %t = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 2
  store i32 %add2, ptr %t, align 8
  %conv10 = sext i32 %add2 to i64
  %add11 = add nsw i64 %conv10, 50
  %f = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 3
  store i64 %add11, ptr %f, align 8
  %o = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
  store i64 %add11, ptr %o, align 8
  ret void
}

define i64 @cal_0(%struct.test* "intel_dtrans_func_index"="1" %tp) !intel.dtrans.func.type !6 {
entry:
  %f = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 3
  %a = load i64, ptr %f, align 8
  %g = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
  %b = load i64, ptr %g, align 8
  %c = add i64 %a, %b
  ret i64 %c
}


define void @foo_1(ptr "intel_dtrans_func_index"="1" %tp, i64 %num) !intel.dtrans.func.type !6 {
entry:
  %add11 = add nsw i64 %num, 50
  %f = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 3
  store i64 %add11, ptr %f, align 8
  %add14 = add nsw i64 %num, 110
  %o = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
  store i64 %add11, ptr %o, align 8
  ret void
}

define i64 @cal_1(%struct.test* "intel_dtrans_func_index"="1" %tp) !intel.dtrans.func.type !6 {
entry:
  %f = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 3
  %a = load i64, ptr %f, align 8
  %g = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
  %b = load i64, ptr %g, align 8
  %c = sub i64 %a, %b
  ret i64 %c
}

define i64 @main() {
entry:
  %call = tail call noalias i8* @calloc(i64 10, i64 40)
  %i = getelementptr %struct.test, ptr %call, i64 0, i32 0
  store i32 10, ptr %i, align 8
  tail call void @foo_0(ptr %call)
  %res_0 = tail call i64 @cal_0(ptr %call)
  tail call void @foo_1(ptr %call, i64 101)
  %res_1 = tail call i64 @cal_1(ptr %call)
  %res = add i64 %res_0, %res_1
  ret i64 %res
}

; Function Attrs: nounwind
declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!6 = distinct !{!5}
!7 = distinct !{!5}
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!8}
!10 = distinct !{!8}
!11 = !{!"S", %struct.test zeroinitializer, i32 5, !1, !2, !1, !2, !2} ; { i32, i64, i32, i64, i64 }

!intel.dtrans.types = !{!11}
