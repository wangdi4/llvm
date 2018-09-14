; RUN: opt < %s -vpo-paropt -S | FileCheck %s
; RUN: opt < %s -passes='vpo-paropt'  -S | FileCheck %s

; This test checks that the "simd" directive in a "parallel for simd" is still
; represented after the "parallel for" part is consumed for codegen.

; int aaa[1000];
; void foo() {
;   #pragma omp parallel for simd
;   for (int i = 0; i<1000; ++i)
;     aaa[i] = 123;
; }

; ModuleID = 'par_for_simd.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@aaa = dso_local global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @_Z3foov() local_unnamed_addr #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !2
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  store volatile i32 999, i32* %.omp.ub, align 4, !tbaa !2
  br label %DIR.OMP.PARALLEL.LOOP.1

; Verify that the PARALLEL FOR is consumed and that a fork_call is emitted
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"() {{.*}}
; CHECK: call void {{.*}} @__kmpc_fork_call({{.*}})

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %entry
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.SHARED"([1000 x i32]* @aaa) ]
  br label %DIR.OMP.PARALLEL.LOOP.15

DIR.OMP.PARALLEL.LOOP.15:                         ; preds = %DIR.OMP.PARALLEL.LOOP.1
  %4 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([1000 x i32]* @aaa to i8*))
  %5 = bitcast i8* %4 to [1000 x i32]*
  br label %DIR.OMP.PARALLEL.LOOP.2

; Verify that the SIMD directive is preserved
; CHECK: %{{.*}} = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]

DIR.OMP.PARALLEL.LOOP.2:                          ; preds = %DIR.OMP.PARALLEL.LOOP.15
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.PARALLEL.LOOP.2
  %7 = load i32, i32* %.omp.lb, align 4, !tbaa !2
  store volatile i32 %7, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %8 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  %9 = load volatile i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp = icmp sgt i32 %8, %9
  br i1 %cmp, label %omp.loop.exit, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  %idxprom = sext i32 %10 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* %5, i64 0, i64 %idxprom
  store i32 123, i32* %arrayidx, align 4, !tbaa !6
  %11 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  %add1 = add nsw i32 %11, 1
  store volatile i32 %add1, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.3

DIR.OMP.END.PARALLEL.LOOP.3:                      ; preds = %DIR.OMP.END.SIMD.4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %2) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %1) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %0) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: inaccessiblememonly nounwind speculatable
declare i8* @llvm.launder.invariant.group.p0i8(i8*) #3

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { inaccessiblememonly nounwind speculatable }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 0bc2dcdd432fcebd3a6fb90d6ddade4c117d08da) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm c652fdc7e5d2080f2c394dfee95e24b39c000e0d)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA1000_i", !3, i64 0}
