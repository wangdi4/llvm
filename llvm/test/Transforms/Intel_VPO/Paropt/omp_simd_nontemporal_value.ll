; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
; void test_01(float *a, float **b, int size) {
; #pragma omp simd nontemporal(a)
;   for (int i = 0; i < size; ++i)
;     b[i] = a + i;
; }
;
; Check that !nontemporal metadata is not attached to stores when a
; pointer to nontemporal memory is stored as a value.
;
; CHECK-NOT: !nontemporal
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @test_01(float* %a, float** %b, i32 %size) #0 {
entry:
  %a.addr = alloca float*, align 8
  %b.addr = alloca float**, align 8
  %size.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store float* %a, float** %a.addr, align 8, !tbaa !2
  store float** %b, float*** %b.addr, align 8, !tbaa !6
  store i32 %size, i32* %size.addr, align 4, !tbaa !8
  %0 = bitcast i32* %.capture_expr.0 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = load i32, i32* %size.addr, align 4, !tbaa !8
  store i32 %1, i32* %.capture_expr.0, align 4, !tbaa !8
  %2 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  %3 = load i32, i32* %.capture_expr.0, align 4, !tbaa !8
  %sub = sub nsw i32 %3, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.1, align 4, !tbaa !8
  %4 = load i32, i32* %.capture_expr.0, align 4, !tbaa !8
  %cmp = icmp slt i32 0, %4
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %5 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #2
  %6 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #2
  %7 = load i32, i32* %.capture_expr.1, align 4, !tbaa !8
  store i32 %7, i32* %.omp.ub, align 4, !tbaa !8
  %8 = load float*, float** %a.addr, align 8, !tbaa !2
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.precond.then
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NONTEMPORAL"(float* %8), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.LINEAR:IV"(i32* %i, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  store i32 0, i32* %.omp.iv, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.2
  %10 = load i32, i32* %.omp.iv, align 4, !tbaa !8
  %11 = load i32, i32* %.omp.ub, align 4, !tbaa !8
  %cmp3 = icmp sle i32 %10, %11
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %12 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %12) #2
  %13 = load i32, i32* %.omp.iv, align 4, !tbaa !8
  %mul = mul nsw i32 %13, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, i32* %i, align 4, !tbaa !8
  %14 = load float*, float** %a.addr, align 8, !tbaa !2
  %15 = load i32, i32* %i, align 4, !tbaa !8
  %idx.ext = sext i32 %15 to i64
  %add.ptr = getelementptr inbounds float, float* %14, i64 %idx.ext, !intel-tbaa !10
  %16 = load float**, float*** %b.addr, align 8, !tbaa !6
  %17 = load i32, i32* %i, align 4, !tbaa !8
  %idxprom = sext i32 %17 to i64
  %ptridx = getelementptr inbounds float*, float** %16, i64 %idxprom
  store float* %add.ptr, float** %ptridx, align 8, !tbaa !2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %18 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %18) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %19 = load i32, i32* %.omp.iv, align 4, !tbaa !8
  %add5 = add nsw i32 %19, 1
  store i32 %add5, i32* %.omp.iv, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %20 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %20) #2
  %21 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %21) #2
  %22 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %22) #2
  %23 = bitcast i32* %.capture_expr.0 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %23) #2
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPf", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"pointer@_ZTSPPf", !4, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !4, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"float", !4, i64 0}
