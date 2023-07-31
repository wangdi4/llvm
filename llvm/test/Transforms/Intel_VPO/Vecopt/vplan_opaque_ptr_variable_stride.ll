; RUN: opt -passes='hir-vplan-vec' -vplan-force-vf=4 -vplan-print-after-vpentity-instrs -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIR
; RUN: opt -passes='vplan-vec' -vplan-force-vf=4 -vplan-print-after-vpentity-instrs -disable-output < %s 2>&1 | FileCheck %s --check-prefix=LLVM

; This test checks to see that the step multiplier for opaque ptr
; induction is generated without a type mismatch assertion.

; HIR: [[STEP:%.*]] = {%0 + 1}
; HIR: [[ADJUSTED_STEP:%.*]] = mul i{{[0-9]+}} [[STEP]]
; HIR-NEXT: induction-init{getelementptr} ptr {{.*}} i{{[0-9]+}} [[ADJUSTED_STEP]]

; LLVM: [[ADJUSTED_STEP:%.*]] = mul i{{[0-9]+}} %add
; LLVM-NEXT: induction-init{getelementptr} ptr {{.*}} i{{[0-9]+}} [[ADJUSTED_STEP]]

define dso_local noundef i32 @main(i32 %0) local_unnamed_addr #1 {
entry:
  %lp.linear = alloca ptr, align 8
  %a = alloca [128 x i32], align 16
  br label %DIR.OMP.SIMD.227

DIR.OMP.SIMD.227:                                 ; preds = %for.body
  store ptr %a, ptr %lp.linear, align 8
  %add = add nsw i32 %0, 1
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.227
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:PTR_TO_PTR.TYPED"(ptr %lp.linear, i32 0, i32 1, i32 %add) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %idx.ext = sext i32 %add to i64
  %lp.linear.promoted = load ptr, ptr %lp.linear, align 8
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %2 = phi ptr [ %lp.linear.promoted, %DIR.OMP.SIMD.2 ], [ %add.ptr, %omp.inner.for.body ]
  %.omp.iv.local.022 = phi i32 [ 0, %DIR.OMP.SIMD.2 ], [ %add6, %omp.inner.for.body ]
  call void @"?baz@@YAXPEAPEAJ@Z"(ptr noundef nonnull %lp.linear)
  %add.ptr = getelementptr inbounds i32, ptr %2, i64 %idx.ext
  store ptr %add.ptr, ptr %lp.linear, align 8
  %add6 = add nuw nsw i32 %.omp.iv.local.022, 1
  %exitcond26.not = icmp eq i32 %add6, 64
  br i1 %exitcond26.not, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body

DIR.OMP.END.SIMD.3:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.328

DIR.OMP.END.SIMD.328:                             ; preds = %DIR.OMP.END.SIMD.3
  ret i32 0
}

declare void @"?baz@@YAXPEAPEAJ@Z"(ptr)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
