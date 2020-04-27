; RUN: opt -S -VPlanDriver -vplan-enable-peeling < %s | FileCheck %s

define void @foo(i64* nocapture %ary) {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptr = getelementptr inbounds i64, i64* %ary, i64 %indvars.iv
  ; CHECK: store <2 x i64> %{{.*}}, <2 x i64>* %{{.*}}, align 4, !intel.preferred_alignment ![[MD:.*]]
  ; CHECK: ![[MD]] = !{i32 16}
  store i64 %indvars.iv, i64* %ptr, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
