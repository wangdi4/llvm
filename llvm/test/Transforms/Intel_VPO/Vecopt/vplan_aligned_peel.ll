; Checks that vectorizer will perform vector peeling in case "#pragma vector aligned" is not specified so that
; llvm/test/Transforms/Intel_VPO/Vecopt/vplan_aligned_no_peel.ll test will remain valid

; Test source code:
; void foo(int *restrict a, int *restrict b, int c) {
;   int i;
; #pragma omp simd
;   for (i = 0; i < 1024; i++) {
;     a[i] = b[i] + c;
;   }
; }

; RUN: opt %s -S -passes='vplan-vec' -vplan-enable-peeling -vplan-force-vf=8 2>&1 | FileCheck %s --check-prefix=O2
; RUN: opt %s -S -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -vplan-enable-peeling -vplan-force-vf=8 2>&1 | FileCheck %s --check-prefix=O3

; O2: peel.check
; O3: vector-peel

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
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.114, label %omp.inner.for.body

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

