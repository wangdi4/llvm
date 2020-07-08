; RUN: opt -vpo-cfg-restructuring -vpo-paropt -basic-aa -scoped-noalias -aa-eval -evaluate-aa-metadata -print-all-alias-modref-info -disable-output %s 2>&1 | FileCheck %s
;
; Check that aliasing information is preserved in case when function has more
; than one region that needs outlining.
;
; void foo(double *glob)
; {
;   double tmp[5] = { 1.0, 2.0, 3.0, 4.0, 5.0 };
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

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(double* %glob) #0 {
entry:
  %glob.addr = alloca double*, align 8
  %tmp = alloca [5 x double], align 16
  %tmp1 = alloca double*, align 8
  %tmp2 = alloca double*, align 8
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
  store double* %glob, double** %glob.addr, align 8, !tbaa !2
  %tmp4 = bitcast [5 x double]* %tmp to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %tmp4, i8 0, i64 40, i1 false)
  %tmp5 = bitcast i8* %tmp4 to [5 x double]*
  %tmp6 = getelementptr inbounds [5 x double], [5 x double]* %tmp5, i32 0, i32 0
  store double 1.000000e+00, double* %tmp6, align 16
  %tmp7 = getelementptr inbounds [5 x double], [5 x double]* %tmp5, i32 0, i32 1
  store double 2.000000e+00, double* %tmp7, align 8
  %tmp8 = getelementptr inbounds [5 x double], [5 x double]* %tmp5, i32 0, i32 2
  store double 3.000000e+00, double* %tmp8, align 16
  %tmp10 = getelementptr inbounds [5 x double], [5 x double]* %tmp5, i32 0, i32 3
  store double 4.000000e+00, double* %tmp10, align 8
  %tmp11 = getelementptr inbounds [5 x double], [5 x double]* %tmp5, i32 0, i32 4
  store double 5.000000e+00, double* %tmp11, align 16
  %arrayidx = getelementptr inbounds [5 x double], [5 x double]* %tmp, i64 0, i64 0, !intel-tbaa !6
  store double* %arrayidx, double** %tmp1, align 8, !tbaa !2
  %arrayidx1 = getelementptr inbounds [5 x double], [5 x double]* %tmp, i64 0, i64 2, !intel-tbaa !6
  store double* %arrayidx1, double** %tmp2, align 8, !tbaa !2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !9
  store i32 999, i32* %.omp.ub, align 4, !tbaa !9
  %tmp12 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(double** %tmp1), "QUAL.OMP.SHARED"(double** %tmp2), "QUAL.OMP.SHARED"(double** %glob.addr) ]
  %tmp13 = load i32, i32* %.omp.lb, align 4, !tbaa !9
  store i32 %tmp13, i32* %.omp.iv, align 4, !tbaa !9
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %tmp14 = load i32, i32* %.omp.iv, align 4, !tbaa !9
  %tmp15 = load i32, i32* %.omp.ub, align 4, !tbaa !9
  %cmp = icmp sle i32 %tmp14, %tmp15
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %tmp16 = load i32, i32* %.omp.iv, align 4, !tbaa !9
  %mul = mul nsw i32 %tmp16, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !9
  %tmp17 = load double*, double** %tmp1, align 8, !tbaa !2
  %arrayidx4 = getelementptr inbounds double, double* %tmp17, i64 0
  %tmp18 = load double, double* %arrayidx4, align 8, !tbaa !11
  %tmp19 = load double*, double** %tmp2, align 8, !tbaa !2
  %arrayidx5 = getelementptr inbounds double, double* %tmp19, i64 0
  %tmp20 = load double, double* %arrayidx5, align 8, !tbaa !11
  %add6 = fadd double %tmp18, %tmp20
  %tmp21 = load double*, double** %glob.addr, align 8, !tbaa !2
  %tmp22 = load i32, i32* %i, align 4, !tbaa !9
  %idxprom = sext i32 %tmp22 to i64
  %arrayidx7 = getelementptr inbounds double, double* %tmp21, i64 %idxprom
  store double %add6, double* %arrayidx7, align 8, !tbaa !11
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %tmp23 = load i32, i32* %.omp.iv, align 4, !tbaa !9
  %add8 = add nsw i32 %tmp23, 1
  store i32 %add8, i32* %.omp.iv, align 4, !tbaa !9
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %tmp12) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  store i32 0, i32* %.omp.lb11, align 4, !tbaa !9
  store i32 999, i32* %.omp.ub12, align 4, !tbaa !9
  %tmp24 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb11), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv10), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub12), "QUAL.OMP.PRIVATE"(i32* %i16), "QUAL.OMP.SHARED"(double** %tmp1), "QUAL.OMP.SHARED"(double** %tmp2), "QUAL.OMP.SHARED"(double** %glob.addr) ]
  %tmp25 = load i32, i32* %.omp.lb11, align 4, !tbaa !9
  store i32 %tmp25, i32* %.omp.iv10, align 4, !tbaa !9
  br label %omp.inner.for.cond13

omp.inner.for.cond13:                             ; preds = %omp.inner.for.inc25, %omp.loop.exit
  %tmp26 = load i32, i32* %.omp.iv10, align 4, !tbaa !9
  %tmp27 = load i32, i32* %.omp.ub12, align 4, !tbaa !9
  %cmp14 = icmp sle i32 %tmp26, %tmp27
  br i1 %cmp14, label %omp.inner.for.body15, label %omp.inner.for.end27

omp.inner.for.body15:                             ; preds = %omp.inner.for.cond13
  %tmp28 = load i32, i32* %.omp.iv10, align 4, !tbaa !9
  %mul17 = mul nsw i32 %tmp28, 1
  %add18 = add nsw i32 0, %mul17
  store i32 %add18, i32* %i16, align 4, !tbaa !9
  %tmp29 = load double*, double** %tmp1, align 8, !tbaa !2
  %arrayidx19 = getelementptr inbounds double, double* %tmp29, i64 0
  %tmp30 = load double, double* %arrayidx19, align 8, !tbaa !11
  %tmp31 = load double*, double** %tmp2, align 8, !tbaa !2
  %arrayidx20 = getelementptr inbounds double, double* %tmp31, i64 1
  %tmp32 = load double, double* %arrayidx20, align 8, !tbaa !11
  %add21 = fadd double %tmp30, %tmp32
  %tmp33 = load double*, double** %glob.addr, align 8, !tbaa !2
  %tmp34 = load i32, i32* %i16, align 4, !tbaa !9
  %idxprom22 = sext i32 %tmp34 to i64
  %arrayidx23 = getelementptr inbounds double, double* %tmp33, i64 %idxprom22
  store double %add21, double* %arrayidx23, align 8, !tbaa !11
  br label %omp.body.continue24

omp.body.continue24:                              ; preds = %omp.inner.for.body15
  br label %omp.inner.for.inc25

omp.inner.for.inc25:                              ; preds = %omp.body.continue24
  %tmp35 = load i32, i32* %.omp.iv10, align 4, !tbaa !9
  %add26 = add nsw i32 %tmp35, 1
  store i32 %add26, i32* %.omp.iv10, align 4, !tbaa !9
  br label %omp.inner.for.cond13

omp.inner.for.end27:                              ; preds = %omp.inner.for.cond13
  br label %omp.loop.exit28

omp.loop.exit28:                                  ; preds = %omp.inner.for.end27
  call void @llvm.directive.region.exit(token %tmp24) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

; Function Attrs: argmemonly nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn writeonly }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPd", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !8, i64 0}
!7 = !{!"array@_ZTSA5_d", !8, i64 0}
!8 = !{!"double", !4, i64 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !4, i64 0}
!11 = !{!8, !8, i64 0}

; CHECK: NoAlias:   %tmp29 = load double*, double** %tmp1, align 8,{{.+}} <->   store double %add21, double* %arrayidx23, align 8
; CHECK: NoAlias:   %tmp31 = load double*, double** %tmp2, align 8,{{.+}} <->   store double %add21, double* %arrayidx23, align 8
; CHECK: NoAlias:   %tmp33 = load double*, double** %glob.addr, align 8,{{.+}} <->   store double %add21, double* %arrayidx23, align 8

; CHECK: NoAlias:   %tmp17 = load double*, double** %tmp1, align 8,{{.+}} <->   store double %add6, double* %arrayidx7, align 8
; CHECK: NoAlias:   %tmp19 = load double*, double** %tmp2, align 8,{{.+}} <->   store double %add6, double* %arrayidx7, align 8
; CHECK: NoAlias:   %tmp21 = load double*, double** %glob.addr, align 8,{{.+}} <->   store double %add6, double* %arrayidx7, align 8
