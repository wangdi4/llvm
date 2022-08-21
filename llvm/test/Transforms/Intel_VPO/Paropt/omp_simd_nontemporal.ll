; RUN: opt -enable-new-pm=0 -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; void test_01(float *a, float *b, int size) {
; #pragma omp simd nontemporal(a, b)
;   for (int i = 0; i < size; ++i) {
;     float tmp = b[i];
;     a[i] = tmp * tmp + tmp + 1.0;
;   }
; }

; Check that we can parse the NONTEMPORAL clause and translate it into
; !nontemporal metadata.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @test_01(ptr noundef %a, ptr noundef %b, i32 noundef %size) #0 {
entry:
  %a.addr = alloca ptr, align 8
  %b.addr = alloca ptr, align 8
  %size.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp5 = alloca float, align 4
  store ptr %a, ptr %a.addr, align 8, !tbaa !4
  store ptr %b, ptr %b.addr, align 8, !tbaa !4
  store i32 %size, ptr %size.addr, align 4, !tbaa !8
  call void @llvm.lifetime.start.p0(i64 4, ptr %.capture_expr.0) #2
  %0 = load i32, ptr %size.addr, align 4, !tbaa !8
  store i32 %0, ptr %.capture_expr.0, align 4, !tbaa !8
  call void @llvm.lifetime.start.p0(i64 4, ptr %.capture_expr.1) #2
  %1 = load i32, ptr %.capture_expr.0, align 4, !tbaa !8
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.1, align 4, !tbaa !8
  %2 = load i32, ptr %.capture_expr.0, align 4, !tbaa !8
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #2
  %3 = load i32, ptr %.capture_expr.1, align 4, !tbaa !8
  store i32 %3, ptr %.omp.ub, align 4, !tbaa !8
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(ptr %a.addr),
    "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(ptr %b.addr),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp5, float 0.000000e+00, i32 1) ]
  store i32 0, ptr %.omp.iv, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %5 = load i32, ptr %.omp.iv, align 4, !tbaa !8
  %6 = load i32, ptr %.omp.ub, align 4, !tbaa !8
  %cmp3 = icmp sle i32 %5, %6
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #2
  %7 = load i32, ptr %.omp.iv, align 4, !tbaa !8
  %mul = mul nsw i32 %7, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %i, align 4, !tbaa !8
  call void @llvm.lifetime.start.p0(i64 4, ptr %tmp5) #2
  %8 = load ptr, ptr %b.addr, align 8, !tbaa !4
  %9 = load i32, ptr %i, align 4, !tbaa !8
  %idxprom = sext i32 %9 to i64
  %arrayidx = getelementptr inbounds float, ptr %8, i64 %idxprom
; CHECK: load float, ptr %arrayidx, align 4, !tbaa !{{[0-9]*}}, !nontemporal
  %10 = load float, ptr %arrayidx, align 4, !tbaa !10
  store float %10, ptr %tmp5, align 4, !tbaa !10
  %11 = load float, ptr %tmp5, align 4, !tbaa !10
  %12 = load float, ptr %tmp5, align 4, !tbaa !10
  %mul6 = fmul fast float %11, %12
  %13 = load float, ptr %tmp5, align 4, !tbaa !10
  %add7 = fadd fast float %mul6, %13
  %conv = fpext float %add7 to double
  %add8 = fadd fast double %conv, 1.000000e+00
  %conv9 = fptrunc double %add8 to float
  %14 = load ptr, ptr %a.addr, align 8, !tbaa !4
  %15 = load i32, ptr %i, align 4, !tbaa !8
  %idxprom10 = sext i32 %15 to i64
  %arrayidx11 = getelementptr inbounds float, ptr %14, i64 %idxprom10
; CHECK: store float %conv9, ptr %arrayidx11, align 4, !tbaa !{{[0-9]*}}, !nontemporal
  store float %conv9, ptr %arrayidx11, align 4, !tbaa !10
  call void @llvm.lifetime.end.p0(i64 4, ptr %tmp5) #2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %16 = load i32, ptr %.omp.iv, align 4, !tbaa !8
  %add12 = add nsw i32 %16, 1
  store i32 %add12, ptr %.omp.iv, align 4, !tbaa !8
  br label %omp.inner.for.cond, !llvm.loop !12

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.capture_expr.1) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.capture_expr.0) #2
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"pointer@_ZTSPf", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !6, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"float", !6, i64 0}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.vectorize.enable", i1 true}
