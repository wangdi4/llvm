; RUN: opt -passes="function(vpo-cfg-restructuring),vpo-paropt,aa-eval" -aa-pipeline="basic-aa,scoped-noalias-aa" -evaluate-aa-metadata -print-all-alias-modref-info -disable-output %s 2>&1 | FileCheck %s

; Test src:
;
; void foo(double *glob) {
;   double tmp[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
;   double *tmp1 = &tmp[0];
;   double *tmp2 = &tmp[2];
;
; #pragma omp parallel for
;   for (int i = 0; i < 1000; ++i)
;     glob[i] = tmp1[0] + tmp2[0];
;
; #pragma omp parallel for
;   for (int i = 0; i < 1000; ++i)
;     glob[i] = tmp1[0] + tmp2[1];
; }

; Check that aliasing information is preserved in case when function has more
; than one region that needs outlining.

; CHECK-DAG: NoAlias:   %2 = load ptr, ptr %tmp1, align 8, {{.+}} <->   store double %add21, ptr %arrayidx23, align 8
; CHECK-DAG: NoAlias:   %4 = load ptr, ptr %tmp2, align 8, {{.+}} <->   store double %add21, ptr %arrayidx23, align 8
; CHECK-DAG: NoAlias:   %6 = load ptr, ptr %glob.addr, align 8, {{.+}} <->   store double %add21, ptr %arrayidx23, align 8

; CHECK-DAG: NoAlias:   %2 = load ptr, ptr %tmp1, align 8, {{.+}} <->   store double %add6, ptr %arrayidx7, align 8
; CHECK-DAG: NoAlias:   %4 = load ptr, ptr %tmp2, align 8, {{.+}} <->   store double %add6, ptr %arrayidx7, align 8
; CHECK-DAG: NoAlias:   %6 = load ptr, ptr %glob.addr, align 8, {{.+}} <->   store double %add6, ptr %arrayidx7, align 8

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr noundef %glob) #0 {
entry:
  %glob.addr = alloca ptr, align 8
  %tmp = alloca [5 x double], align 16
  %tmp1 = alloca ptr, align 8
  %tmp2 = alloca ptr, align 8
  %tmp3 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp9 = alloca i32, align 4
  %.omp.iv10 = alloca i32, align 4
  %.omp.lb11 = alloca i32, align 4
  %.omp.ub12 = alloca i32, align 4
  %i16 = alloca i32, align 4
  store ptr %glob, ptr %glob.addr, align 8, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 40, ptr %tmp) #3
  call void @llvm.memset.p0.i64(ptr align 16 %tmp, i8 0, i64 40, i1 false)
  %0 = getelementptr inbounds [5 x double], ptr %tmp, i32 0, i32 0
  store double 1.000000e+00, ptr %0, align 16
  %1 = getelementptr inbounds [5 x double], ptr %tmp, i32 0, i32 1
  store double 2.000000e+00, ptr %1, align 8
  %2 = getelementptr inbounds [5 x double], ptr %tmp, i32 0, i32 2
  store double 3.000000e+00, ptr %2, align 16
  %3 = getelementptr inbounds [5 x double], ptr %tmp, i32 0, i32 3
  store double 4.000000e+00, ptr %3, align 8
  %4 = getelementptr inbounds [5 x double], ptr %tmp, i32 0, i32 4
  store double 5.000000e+00, ptr %4, align 16
  call void @llvm.lifetime.start.p0(i64 8, ptr %tmp1) #3
  %arrayidx = getelementptr inbounds [5 x double], ptr %tmp, i64 0, i64 0, !intel-tbaa !8
  store ptr %arrayidx, ptr %tmp1, align 8, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 8, ptr %tmp2) #3
  %arrayidx1 = getelementptr inbounds [5 x double], ptr %tmp, i64 0, i64 2, !intel-tbaa !8
  store ptr %arrayidx1, ptr %tmp2, align 8, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #3
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #3
  store i32 0, ptr %.omp.lb, align 4, !tbaa !11
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #3
  store i32 999, ptr %.omp.ub, align 4, !tbaa !11
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %glob.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %tmp1, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %tmp2, ptr null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  %6 = load i32, ptr %.omp.lb, align 4, !tbaa !11
  store i32 %6, ptr %.omp.iv, align 4, !tbaa !11
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %7 = load i32, ptr %.omp.iv, align 4, !tbaa !11
  %8 = load i32, ptr %.omp.ub, align 4, !tbaa !11
  %cmp = icmp sle i32 %7, %8
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #3
  %9 = load i32, ptr %.omp.iv, align 4, !tbaa !11
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4, !tbaa !11
  %10 = load ptr, ptr %tmp1, align 8, !tbaa !4
  %arrayidx4 = getelementptr inbounds double, ptr %10, i64 0
  %11 = load double, ptr %arrayidx4, align 8, !tbaa !13
  %12 = load ptr, ptr %tmp2, align 8, !tbaa !4
  %arrayidx5 = getelementptr inbounds double, ptr %12, i64 0
  %13 = load double, ptr %arrayidx5, align 8, !tbaa !13
  %add6 = fadd fast double %11, %13
  %14 = load ptr, ptr %glob.addr, align 8, !tbaa !4
  %15 = load i32, ptr %i, align 4, !tbaa !11
  %idxprom = sext i32 %15 to i64
  %arrayidx7 = getelementptr inbounds double, ptr %14, i64 %idxprom
  store double %add6, ptr %arrayidx7, align 8, !tbaa !13
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #3
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %16 = load i32, ptr %.omp.iv, align 4, !tbaa !11
  %add8 = add nsw i32 %16, 1
  store i32 %add8, ptr %.omp.iv, align 4, !tbaa !11
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #3
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv10) #3
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb11) #3
  store i32 0, ptr %.omp.lb11, align 4, !tbaa !11
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub12) #3
  store i32 999, ptr %.omp.ub12, align 4, !tbaa !11
  %17 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %glob.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %tmp1, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %tmp2, ptr null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv10, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb11, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub12, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i16, i32 0, i32 1) ]
  %18 = load i32, ptr %.omp.lb11, align 4, !tbaa !11
  store i32 %18, ptr %.omp.iv10, align 4, !tbaa !11
  br label %omp.inner.for.cond13

omp.inner.for.cond13:                             ; preds = %omp.inner.for.inc25, %omp.loop.exit
  %19 = load i32, ptr %.omp.iv10, align 4, !tbaa !11
  %20 = load i32, ptr %.omp.ub12, align 4, !tbaa !11
  %cmp14 = icmp sle i32 %19, %20
  br i1 %cmp14, label %omp.inner.for.body15, label %omp.inner.for.end27

omp.inner.for.body15:                             ; preds = %omp.inner.for.cond13
  call void @llvm.lifetime.start.p0(i64 4, ptr %i16) #3
  %21 = load i32, ptr %.omp.iv10, align 4, !tbaa !11
  %mul17 = mul nsw i32 %21, 1
  %add18 = add nsw i32 0, %mul17
  store i32 %add18, ptr %i16, align 4, !tbaa !11
  %22 = load ptr, ptr %tmp1, align 8, !tbaa !4
  %arrayidx19 = getelementptr inbounds double, ptr %22, i64 0
  %23 = load double, ptr %arrayidx19, align 8, !tbaa !13
  %24 = load ptr, ptr %tmp2, align 8, !tbaa !4
  %arrayidx20 = getelementptr inbounds double, ptr %24, i64 1
  %25 = load double, ptr %arrayidx20, align 8, !tbaa !13
  %add21 = fadd fast double %23, %25
  %26 = load ptr, ptr %glob.addr, align 8, !tbaa !4
  %27 = load i32, ptr %i16, align 4, !tbaa !11
  %idxprom22 = sext i32 %27 to i64
  %arrayidx23 = getelementptr inbounds double, ptr %26, i64 %idxprom22
  store double %add21, ptr %arrayidx23, align 8, !tbaa !13
  br label %omp.body.continue24

omp.body.continue24:                              ; preds = %omp.inner.for.body15
  call void @llvm.lifetime.end.p0(i64 4, ptr %i16) #3
  br label %omp.inner.for.inc25

omp.inner.for.inc25:                              ; preds = %omp.body.continue24
  %28 = load i32, ptr %.omp.iv10, align 4, !tbaa !11
  %add26 = add nsw i32 %28, 1
  store i32 %add26, ptr %.omp.iv10, align 4, !tbaa !11
  br label %omp.inner.for.cond13

omp.inner.for.end27:                              ; preds = %omp.inner.for.cond13
  br label %omp.loop.exit28

omp.loop.exit28:                                  ; preds = %omp.inner.for.end27
  call void @llvm.directive.region.exit(token %17) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub12) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb11) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv10) #3
  call void @llvm.lifetime.end.p0(i64 8, ptr %tmp2) #3
  call void @llvm.lifetime.end.p0(i64 8, ptr %tmp1) #3
  call void @llvm.lifetime.end.p0(i64 40, ptr %tmp) #3
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { argmemonly nofree nounwind willreturn writeonly }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"pointer@_ZTSPd", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !10, i64 0}
!9 = !{!"array@_ZTSA5_d", !10, i64 0}
!10 = !{!"double", !6, i64 0}
!11 = !{!12, !12, i64 0}
!12 = !{!"int", !6, i64 0}
!13 = !{!10, !10, i64 0}
