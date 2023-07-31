; Check that VPlan adjusts the search space for VF based of safelen specified via `#pragma omp simd safelen(x)`. If the MinVF in search space is greater than safelen, then it
; has to be lowered to the nereast power of 2 lower than or equal to safelen. MaxVF should also be adjusted accordingly.

; NOTE: This can be tested only on LLVM-IR path since getTypesWidthRangeInBits() for HIR returns hard-coded values.

; Input C code:
; int a[1024];
; int b[1024];
; int c[1024];
;
; void foo()
; {
;     int i;
; #pragma omp simd safelen(2)
;     for (i=2; i<1024; i++) {
;         a[i] = a[i-2] + b[i] * c[i];
;     }
; }

; REQUIRES: asserts

; RUN: opt -S -passes=vplan-vec -debug-only=LoopVectorizationPlanner < %s 2>&1 | FileCheck %s --check-prefix=CHECK-LLVM
; RUN: opt -S -passes="vplan-vec" -debug-only=LoopVectorizationPlanner < %s 2>&1 | FileCheck %s --check-prefix=CHECK-LLVM

; CHECK-LLVM: LVP: Safelen: [[Safelen:[0-9]+]]
; CHECK-LLVM-NEXT: LVP: Orig MinVF: 4  Orig MaxVF: {{[0-9]+}}
; CHECK-LLVM-NEXT: LVP: MinVF: [[Safelen]] MaxVF: [[Safelen]]
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
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SAFELEN"(i32 2), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i32 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %omp.inner.for.body.lr.ph ]
  %1 = add nuw nsw i32 %indvars.iv, 2
  %arrayidx = getelementptr inbounds [1024 x i32], ptr @a, i32 0, i32 %indvars.iv, !intel-tbaa !2
  %2 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %arrayidx3 = getelementptr inbounds [1024 x i32], ptr @b, i32 0, i32 %1, !intel-tbaa !2
  %3 = load i32, ptr %arrayidx3, align 4, !tbaa !2
  %arrayidx5 = getelementptr inbounds [1024 x i32], ptr @c, i32 0, i32 %1, !intel-tbaa !2
  %4 = load i32, ptr %arrayidx5, align 4, !tbaa !2
  %mul6 = mul nsw i32 %4, %3
  %add7 = add nsw i32 %mul6, %2
  %arrayidx9 = getelementptr inbounds [1024 x i32], ptr @a, i32 0, i32 %1, !intel-tbaa !2
  store i32 %add7, ptr %arrayidx9, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond = icmp eq i32 %indvars.iv.next, 1022
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


!2 = !{!3, !4, i32 0}
!3 = !{!"array@_ZTSA1024_i", !4, i32 0}
!4 = !{!"int", !5, i32 0}
!5 = !{!"omnipotent char", !6, i32 0}
!6 = !{!"Simple C/C++ TBAA"}
