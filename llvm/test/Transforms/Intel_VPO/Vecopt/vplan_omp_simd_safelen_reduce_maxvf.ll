; Check that VPlan reduces the MaxVF (in search space) when the safelen specified via `#pragma omp simd safelen(x)` is lesser than originally chosen MaxVF

; Input C code:
; int a[1024];
; int b[1024];
; int c[1024];
;
; void foo()
; {
;     int i;
; #pragma omp simd safelen(4)
;     for (i=4; i<1024; i++) {
;         a[i] = a[i-4] + b[i] * c[i];
;     }
; }


; REQUIRES: asserts

; RUN: opt -S -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -debug-only=LoopVectorizationPlanner -print-after=hir-vplan-vec < %s 2>&1 | FileCheck %s --check-prefix=CHECK-HIR

; RUN: opt -S -vplan-vec -debug-only=LoopVectorizationPlanner < %s 2>&1 | FileCheck %s --check-prefix=CHECK-LLVM

; Check debug info from LoopVectorizationPlanner
; CHECK-HIR: LVP: Safelen: [[Safelen:[0-9]+]]
; CHECK-HIR-NEXT: LVP: Orig MinVF: [[MinVF:[0-9]+]] Orig MaxVF: {{[0-9]+}}
; CHECK-HIR-NEXT: LVP: MinVF: [[MinVF]] MaxVF: [[Safelen]]
; Check loop is vectorized
; CHECK-HIR-NOT: DO i1 = 0, 1020, 1


; CHECK-LLVM: LVP: Safelen: [[Safelen:[0-9]+]]
; CHECK-LLVM-NEXT: LVP: Orig MinVF: [[MinVF:[0-9]+]] Orig MaxVF: {{[0-9]+}}
; CHECK-LLVM-NEXT: LVP: MinVF: [[MinVF]] MaxVF: [[Safelen]]
; Check loop is vectorized
; CHECK-LLVM: mul nsw <{{[0-9]+}} x i32>


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@c = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo() local_unnamed_addr {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SAFELEN"(i32 4), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %omp.inner.for.body.lr.ph ]
  %1 = add nuw nsw i64 %indvars.iv, 4
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %arrayidx2 = getelementptr inbounds [1024 x i32], [1024 x i32]* @b, i64 0, i64 %1, !intel-tbaa !2
  %3 = load i32, i32* %arrayidx2, align 4, !tbaa !2
  %arrayidx4 = getelementptr inbounds [1024 x i32], [1024 x i32]* @c, i64 0, i64 %1, !intel-tbaa !2
  %4 = load i32, i32* %arrayidx4, align 4, !tbaa !2
  %mul5 = mul nsw i32 %4, %3
  %add6 = add nsw i32 %mul5, %2
  %arrayidx8 = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %1, !intel-tbaa !2
  store i32 %add6, i32* %arrayidx8, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1020
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }


!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
