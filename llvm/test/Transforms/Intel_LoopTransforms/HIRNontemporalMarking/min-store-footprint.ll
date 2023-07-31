; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -hir-nontemporal-cacheline-count=0 -hir-nontemporal-min-store-footprint=40 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-nontemporal-marking,print<hir>" -aa-pipeline="basic-aa" -hir-details -disable-output < %s 2>&1 | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

; Check that the -hir-nontemporal-min-store-footprint threshold works as
; expected. With the threshold set to 40 B, a 32-bit store should only be marked
; if it is in a loop with a trip count that can be at least 10. The maximum loop
; trip count can be a constant trip count or it can be set using either of our
; two maximum trip count metadata, llvm.loop.intel.loopcount_minimum or
; llvm.loop.intel.max.trip_count. This test also checks that overflows don't
; lead to incorrect behavior when trip counts are extremely high.

define void @const10(ptr %dest) "target-features"="+avx512f" {
; CHECK-LABEL: const10
;     CHECK: BEGIN REGION { modified }
;     CHECK:       + DO i32 i1 = 0, 9, 1   <DO_LOOP>
;     CHECK:           !nontemporal
;     CHECK:       + END LOOP
;     CHECK:          @llvm.x86.sse.sfence();
;     CHECK: END REGION

entry:
  br label %loop

loop:
  %index = phi i32 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i32 %index, 1
  %addr = getelementptr inbounds i32, ptr %dest, i32 %index
  store i32 %index, ptr %addr, align 8
  %cond = icmp eq i32 %index.next, 10
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

define void @const9(ptr %dest) "target-features"="+avx512f" {
; CHECK-LABEL: const9
;     CHECK: BEGIN REGION { }
;     CHECK:       + DO i32 i1 = 0, 8, 1   <DO_LOOP>
; CHECK-NOT:           !nontemporal
;     CHECK:       + END LOOP
; CHECK-NOT:          @llvm.x86.sse.sfence();
;     CHECK: END REGION

entry:
  br label %loop

loop:
  %index = phi i32 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i32 %index, 1
  %addr = getelementptr inbounds i32, ptr %dest, i32 %index
  store i32 %index, ptr %addr, align 8
  %cond = icmp eq i32 %index.next, 9
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

define void @loopcount_maximum10(ptr %dest, i32 %n) "target-features"="+avx512f" {
; CHECK-LABEL: loopcount_maximum10
;     CHECK: BEGIN REGION { modified }
;     CHECK:       + DO i32 i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>  <LEGAL_MAX_TC = 10> <max_trip_count = 10>
;     CHECK:           !nontemporal
;     CHECK:       + END LOOP
;     CHECK:          @llvm.x86.sse.sfence();
;     CHECK: END REGION

entry:
  br label %loop

loop:
  %index = phi i32 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i32 %index, 1
  %addr = getelementptr inbounds i32, ptr %dest, i32 %index
  store i32 %index, ptr %addr, align 8
  %cond = icmp eq i32 %index.next, %n
  br i1 %cond, label %exit, label %loop, !llvm.loop !0

exit:
  ret void
}

define void @loopcount_maximum9(ptr %dest, i32 %n) "target-features"="+avx512f" {
; CHECK-LABEL: loopcount_maximum9
;     CHECK: BEGIN REGION { }
;     CHECK:       + DO i32 i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 9>  <LEGAL_MAX_TC = 9> <max_trip_count = 9>
; CHECK-NOT:           !nontemporal
;     CHECK:       + END LOOP
; CHECK-NOT:          @llvm.x86.sse.sfence();
;     CHECK: END REGION

entry:
  br label %loop

loop:
  %index = phi i32 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i32 %index, 1
  %addr = getelementptr inbounds i32, ptr %dest, i32 %index
  store i32 %index, ptr %addr, align 8
  %cond = icmp eq i32 %index.next, %n
  br i1 %cond, label %exit, label %loop, !llvm.loop !2

exit:
  ret void
}

define void @max_trip_count10(ptr %dest, i32 %n) "target-features"="+avx512f" {
; CHECK-LABEL: max_trip_count10
;     CHECK: BEGIN REGION { modified }
;     CHECK:       + DO i32 i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>  <LEGAL_MAX_TC = 10>
;     CHECK:           !nontemporal
;     CHECK:       + END LOOP
;     CHECK:          @llvm.x86.sse.sfence();
;     CHECK: END REGION

entry:
  br label %loop

loop:
  %index = phi i32 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i32 %index, 1
  %addr = getelementptr inbounds i32, ptr %dest, i32 %index
  store i32 %index, ptr %addr, align 8
  %cond = icmp eq i32 %index.next, %n
  br i1 %cond, label %exit, label %loop, !llvm.loop !4

exit:
  ret void
}

define void @max_trip_count9(ptr %dest, i32 %n) "target-features"="+avx512f" {
; CHECK-LABEL: max_trip_count9
;     CHECK: BEGIN REGION { }
;     CHECK:       + DO i32 i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 9>  <LEGAL_MAX_TC = 9>
; CHECK-NOT:           !nontemporal
;     CHECK:       + END LOOP
; CHECK-NOT:          @llvm.x86.sse.sfence();
;     CHECK: END REGION

entry:
  br label %loop

loop:
  %index = phi i32 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i32 %index, 1
  %addr = getelementptr inbounds i32, ptr %dest, i32 %index
  store i32 %index, ptr %addr, align 8
  %cond = icmp eq i32 %index.next, %n
  br i1 %cond, label %exit, label %loop, !llvm.loop !6

exit:
  ret void
}

define void @overflow(ptr %dest) "target-features"="+avx512f" {
; CHECK-LABEL: overflow
;     CHECK: BEGIN REGION { modified }
;     CHECK:       + DO i64 i1 = 0, 9223372036854775807, 1   <DO_LOOP>
;     CHECK:           !nontemporal
;     CHECK:       + END LOOP
;     CHECK:          @llvm.x86.sse.sfence();
;     CHECK: END REGION

entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %addr = getelementptr inbounds i64, ptr %dest, i64 %index
  store i64 %index, ptr %addr, align 8
  %cond = icmp eq i64 %index.next, 9223372036854775808 ; 2^63
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.intel.loopcount_maximum", i32 10}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.intel.loopcount_maximum", i32 9}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.intel.max.trip_count", i32 10}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.intel.max.trip_count", i32 9}
