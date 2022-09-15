; This test verifies only candidate selection for field reusing
; REQUIRES: asserts
; RUN: opt -opaque-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume < %s -passes=dtrans-reusefieldop -debug-only=dtrans-reusefieldop -disable-output 2>&1 | FileCheck %s

; CHECK: Reuse field: looking for candidate structure field.
; CHECK: LLVM Type: %struct.test
; CHECK:   Candidate structure for reuse field: %struct.test

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define void @foo(%struct.test* "intel_dtrans_func_index"="1" %tp) !intel.dtrans.func.type !6 {
entry:
  %i = getelementptr inbounds %struct.test, ptr %tp, i64 0
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
  %call = tail call noalias i8* @calloc(i64 10, i64 48)
  %i = getelementptr %struct.test, ptr %call, i64 0, i32 0
  store i32 10, ptr %i, align 8
  tail call void @foo(ptr %call)
  ret i32 0
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
!11 = !{!"S", %struct.test zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !2} ; { i32, i64, i32, i32, i16, i64, i64 }

!intel.dtrans.types = !{!11}

