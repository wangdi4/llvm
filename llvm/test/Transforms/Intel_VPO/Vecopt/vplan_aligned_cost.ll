; Checks that vectorizer will use minimal load/store cost 1 in case "#pragma vector aligned" is specified

; Test source code:
; void foo(int *restrict a, int *restrict b, int c) {
;   int i;
; #pragma omp simd
; #pragma vector aligned
;   for (i = 0; i < 1024; i++) {
;     a[i] = b[i] + c;
;   }
; }

; RUN: opt %s -S -passes='vplan-vec' -disable-output -vplan-cost-model-print-analysis-for-vf=8 -vplan-force-vf=8 2>&1 | FileCheck %s
; RUN: opt %s -S -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec' -disable-output -vplan-cost-model-print-analysis-for-vf=8 -vplan-force-vf=8 2>&1 | FileCheck %s

; CHECK: Cost 1 for i32 {{%.*}} = load ptr {{%.*}}
; CHECK: Cost 1 for store i32 {{%.*}} ptr {{%.*}}

define dso_local void @foo(ptr noalias nocapture noundef writeonly %a, ptr noalias nocapture noundef readonly %b, i32 noundef %c) local_unnamed_addr #0 {
DIR.OMP.SIMD.1:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.115

DIR.OMP.SIMD.115:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.115, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.115 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4
  %add1 = add nsw i32 %1, %c
  %arrayidx3 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 %add1, ptr %arrayidx3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.114, label %omp.inner.for.body, !llvm.loop !0

DIR.OMP.END.SIMD.114:                             ; preds = %omp.inner.for.body
  store i32 1024, ptr %i.linear.iv, align 4
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.114
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

!0 = distinct !{!0, !1, !2}
!1 = !{!"llvm.loop.mustprogress"}
!2 = !{!"llvm.loop.intel.vector.aligned"}
