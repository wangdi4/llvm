; RUN: opt -S -passes=vplan-vec -disable-output -vplan-print-after-all-zero-bypass -vplan-force-vf=16  -enable-intel-advanced-opts < %s | FileCheck %s

; XFAIL: *
; TODO: the test needs to be enabled back once cost modelling for uniform icmp is fixed. Please see CMPLRLLVM-32328 for details.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @test.positive(i64 %uniform) #0 {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %latch ]
  %varying = icmp eq i64 %iv, 42
  br i1 %varying, label %latch, label %bb0

bb0:
; Verify that uniform condition has lower threshold to be bypassed for all-zero
; because probability of that is usually higher.
;
; CHECK-LABEL: VPlan IR for: test.positive:header
; CHECK: all-zero-check
  %uni.cmp = icmp eq i64 %uniform, 11
  br i1 %uni.cmp, label %latch, label %bb1

bb1:
  %add0 = add i64 %iv, 42
  %add1 = add i64 %iv, 42
  %add2 = add i64 %iv, 42
  %add3 = add i64 %iv, 42
  %add4 = add i64 %iv, 42
  br label %latch

latch:
  %iv.next = add i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 64
  br i1 %exitcond, label %exit, label %header

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret void
}

define dso_local void @test.negative(i64 %uniform) #0 {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %latch ]
  %varying = icmp eq i64 %iv, 42
  br i1 %varying, label %latch, label %bb0

bb0:
; CHECK-LABEL: VPlan IR for: test.negative:header
; CHECK-NOT: all-zero-check
  %div.cmp = icmp eq i64 %uniform, %iv
  br i1 %div.cmp, label %latch, label %bb1

bb1:
  %add0 = add i64 %iv, 42
  %add1 = add i64 %iv, 42
  %add2 = add i64 %iv, 42
  %add3 = add i64 %iv, 42
  %add4 = add i64 %iv, 42
  br label %latch

latch:
  %iv.next = add i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 64
  br i1 %exitcond, label %exit, label %header

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret void
}


declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "target-cpu"="x86-64" "target-features"="+avx2"  }
