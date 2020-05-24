; RUN: opt -S -VPlanDriver -vplan-enable-peeling < %s | FileCheck %s

; CHECK-LABEL: test_store
define void @test_store(i64* nocapture %ary) {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptr = getelementptr inbounds i64, i64* %ary, i64 %indvars.iv
  ; CHECK: store <2 x i64> %{{.*}}, <2 x i64>* %{{.*}}, align 8, !intel.preferred_alignment ![[MD_ST:.*]]
  store i64 %indvars.iv, i64* %ptr, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

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

; CHECK: ![[MD_ST]] = !{i32 16}
; CHECK: ![[MD_LD]] = !{i32 32}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
