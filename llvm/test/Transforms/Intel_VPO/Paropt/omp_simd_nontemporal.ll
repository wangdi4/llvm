; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
; void test_01(float *a, float *b, int size) {
; #pragma omp simd nontemporal(a, b)
;   for (int i = 0; i < size; ++i) {
;     float tmp = b[i];
;     a[i] = tmp * tmp + tmp + 1.0;
;   }
; }
;
; Check that we can parse the NONTEMPORAL clause and translate it into
; !nontemporal metadata.
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z7test_01PfS_i(float* %a, float* %b, i32 %size) #0 {
entry:
  %a.addr = alloca float*, align 8
  %b.addr = alloca float*, align 8
  %size.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp5 = alloca float, align 4
  store float* %a, float** %a.addr, align 8, !tbaa !2
  store float* %b, float** %b.addr, align 8, !tbaa !2
  store i32 %size, i32* %size.addr, align 4, !tbaa !6
  %0 = bitcast i32* %.capture_expr.0 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = load i32, i32* %size.addr, align 4, !tbaa !6
  store i32 %1, i32* %.capture_expr.0, align 4, !tbaa !6
  %2 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  %3 = load i32, i32* %.capture_expr.0, align 4, !tbaa !6
  %sub = sub nsw i32 %3, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.1, align 4, !tbaa !6
  %4 = load i32, i32* %.capture_expr.0, align 4, !tbaa !6
  %cmp = icmp slt i32 0, %4
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %5 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #2
  %6 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #2
  %7 = load i32, i32* %.capture_expr.1, align 4, !tbaa !6
  store i32 %7, i32* %.omp.ub, align 4, !tbaa !6
  %8 = load float*, float** %a.addr, align 8, !tbaa !2
  %9 = load float*, float** %b.addr, align 8, !tbaa !2
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.precond.then
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NONTEMPORAL"(float* %8, float* %9), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.LINEAR:IV"(i32* %i, i32 1), "QUAL.OMP.PRIVATE"(float* %tmp5) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  store i32 0, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.2
  %11 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %12 = load i32, i32* %.omp.ub, align 4, !tbaa !6
  %cmp3 = icmp sle i32 %11, %12
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %13 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %13) #2
  %14 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %mul = mul nsw i32 %14, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, i32* %i, align 4, !tbaa !6
  %15 = bitcast float* %tmp5 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %15) #2
  %16 = load float*, float** %b.addr, align 8, !tbaa !2
  %17 = load i32, i32* %i, align 4, !tbaa !6
  %idxprom = sext i32 %17 to i64
  %ptridx = getelementptr inbounds float, float* %16, i64 %idxprom
  ; CHECK: load float, float* %ptridx, align 4, !tbaa !{{[0-9]}}, !nontemporal
  %18 = load float, float* %ptridx, align 4, !tbaa !8
  store float %18, float* %tmp5, align 4, !tbaa !8
  %19 = load float, float* %tmp5, align 4, !tbaa !8
  %20 = load float, float* %tmp5, align 4, !tbaa !8
  %mul6 = fmul float %19, %20
  %21 = load float, float* %tmp5, align 4, !tbaa !8
  %add7 = fadd float %mul6, %21
  %conv = fpext float %add7 to double
  %add8 = fadd double %conv, 1.000000e+00
  %conv9 = fptrunc double %add8 to float
  %22 = load float*, float** %a.addr, align 8, !tbaa !2
  %23 = load i32, i32* %i, align 4, !tbaa !6
  %idxprom10 = sext i32 %23 to i64
  %ptridx11 = getelementptr inbounds float, float* %22, i64 %idxprom10
  ; CHECK: store float %conv9, float* %ptridx11, align 4, !tbaa !{{[0-9]}}, !nontemporal
  store float %conv9, float* %ptridx11, align 4, !tbaa !8
  %24 = bitcast float* %tmp5 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %24) #2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %25 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %25) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %26 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %add12 = add nsw i32 %26, 1
  store i32 %add12, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %27 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %27) #2
  %28 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %28) #2
  %29 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %29) #2
  %30 = bitcast i32* %.capture_expr.0 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %30) #2
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

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPf", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"float", !4, i64 0}
