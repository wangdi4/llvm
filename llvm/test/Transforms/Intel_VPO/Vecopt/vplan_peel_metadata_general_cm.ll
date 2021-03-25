; RUN: opt -S -mattr=avx2 -VPlanDriver < %s \
; RUN:     -vplan-enable-peeling -vplan-enable-general-peeling-cost-model \
; RUN: | FileCheck %s

; This is a test for VPlanPeelingCostModelGeneral. The test checks
; that the cost model works correctly at least in some cases. The cost
; model needs to be properly tuned before it can be applied to more
; cases.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: test_load
define void @test_load(i32* nocapture %dst, i32* nocapture %src, i64 %size) {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %for.body

for.body:
  %counter = phi i64 [ 0, %entry ], [ %counter.next, %for.body ]
  %src.ptr = getelementptr inbounds i32, i32* %src, i64 %counter
  ; CHECK: load <8 x i32>, <8 x i32>* %{{.*}}, align 4, !intel.preferred_alignment ![[MD_LD:.*]]
  %src.val = load i32, i32* %src.ptr, align 4
  %dst.val = add i32 %src.val, 42
  %counter_times_two = mul nsw nuw i64 %counter, 2
  %dst.ptr = getelementptr inbounds i32, i32* %dst, i64 %counter_times_two
  store i32 %dst.val, i32* %dst.ptr, align 4
  %counter.next = add nsw i64 %counter, 1
  %exitcond = icmp sge i64 %counter.next, %size
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; CHECK: ![[MD_LD]] = !{i32 32}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
