; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt -basicaa -scoped-noalias -aa-eval -evaluate-aa-metadata -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
target triple = "x86_64-mic"
target device_triples = "x86_64-mic"
;
; It tests whether the alias analysis information is kept after the outlining.
; For example, after the following loop is outlined, the compiler can still
; understand *glob is not aliased with *tmp1 and *tmp2.
;
; void foo(double *glob)
; {
;   double tmp[5] = { 1.0, 2.0, 3.0, 4.0, 5.0 };
;   double *tmp1 = &tmp[0];
;   double *tmp2 = &tmp[2];
;
;   #pragma omp target map(tofrom:tmp1[0:4], tmp2[0:2], glob[0:999])
;   {
;     for (int i = 0; i < 1000; ++i) {
;       glob[i] = tmp1[0] * tmp1[1] + tmp2[0] * tmp2[1] + tmp2[2];
;     }
;   }
;}
@"@tid.addr" = external global i32

; Function Attrs: nounwind uwtable
define void @foo(double* %glob) #0 {
entry:
  %tmp = alloca [5 x double], align 16
  %i = alloca i32, align 4
  %0 = bitcast [5 x double]* %tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 40, i8* %0) #2
  call void @llvm.memset.p0i8.i64(i8* align 16 %0, i8 0, i64 40, i1 false)
  %1 = getelementptr [5 x double], [5 x double]* %tmp, i32 0, i32 0
  store double 1.000000e+00, double* %1
  %2 = getelementptr [5 x double], [5 x double]* %tmp, i32 0, i32 1
  store double 2.000000e+00, double* %2
  %3 = getelementptr [5 x double], [5 x double]* %tmp, i32 0, i32 2
  store double 3.000000e+00, double* %3
  %4 = getelementptr [5 x double], [5 x double]* %tmp, i32 0, i32 3
  store double 4.000000e+00, double* %4
  %5 = getelementptr [5 x double], [5 x double]* %tmp, i32 0, i32 4
  store double 5.000000e+00, double* %5
  %arrayidx = getelementptr inbounds [5 x double], [5 x double]* %tmp, i64 0, i64 0
  %arrayidx1 = getelementptr inbounds [5 x double], [5 x double]* %tmp, i64 0, i64 2
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(double* %arrayidx, double* %arrayidx, i64 32), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(double* %arrayidx1, double* %arrayidx1, i64 16), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(double* %glob, double* %glob, i64 7992), "QUAL.OMP.PRIVATE"(i32* %i) ], !omp_offload.entry !10
  %7 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #2
  store i32 0, i32* %i, align 4, !tbaa !3
  br label %for.cond

for.cond:                                         ; preds = %for.body, %DIR.OMP.TARGET.1
  %8 = load i32, i32* %i, align 4, !tbaa !3
  %cmp = icmp slt i32 %8, 1000
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %7) #2
  br label %for.end

for.body:                                         ; preds = %for.cond
  %9 = load double, double* %arrayidx, align 8, !tbaa !7
  %arrayidx6 = getelementptr inbounds double, double* %arrayidx, i64 1
  %10 = load double, double* %arrayidx6, align 8, !tbaa !7
  %mul = fmul double %9, %10
  %11 = load double, double* %arrayidx1, align 8, !tbaa !7
  %arrayidx8 = getelementptr inbounds double, double* %arrayidx1, i64 1
  %12 = load double, double* %arrayidx8, align 8, !tbaa !7
  %mul9 = fmul double %11, %12
  %add = fadd double %mul, %mul9
  %arrayidx10 = getelementptr inbounds double, double* %arrayidx1, i64 2
  %13 = load double, double* %arrayidx10, align 8, !tbaa !7
  %add11 = fadd double %add, %13
  %idxprom = sext i32 %8 to i64
  %arrayidx12 = getelementptr inbounds double, double* %glob, i64 %idxprom
  store double %add11, double* %arrayidx12, align 8, !tbaa !7
  %14 = load i32, i32* %i, align 4, !tbaa !3
  %inc = add nsw i32 %14, 1
  store i32 %inc, i32* %i, align 4, !tbaa !3
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.lifetime.end.p0i8(i64 40, i8* %0) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!omp_offload.info = !{!9}
!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 7c6feb7b57a1fb6fb93d81865c58ebbd9b8f4401) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 10fdf56ea1f6de596ff5e1e7a34cb9d88ca43968)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"double", !5, i64 0}
!9 = !{i32 0, i32 54, i32 -698850821, !"foo", i32 47, i32 0}
!10 = distinct !{i32 0}

; CHECK:  NoAlias:   %1 = load i32, i32* %i.priv, align 4, !tbaa !4, !alias.scope !11, !noalias !18 <->   store double %add11, double* %arrayidx12, align 8, !tbaa !19, !alias.scope !18, !noalias !22
; CHECK:  MustAlias:   %1 = load i32, i32* %i.priv, align 4, !tbaa !4, !alias.scope !11, !noalias !18 <->   store i32 %inc, i32* %i.priv, align 4, !tbaa !4, !alias.scope !8, !noalias !12
; CHECK:  NoAlias:   %2 = load double, double* %arrayidx, align 8, !tbaa !19, !alias.scope !13, !noalias !21 <->   store i32 0, i32* %i.priv, align 4, !tbaa !4, !alias.scope !8, !noalias !12
; CHECK:  NoAlias:   %2 = load double, double* %arrayidx, align 8, !tbaa !19, !alias.scope !13, !noalias !21 <->   store double %add11, double* %arrayidx12, align 8, !tbaa !19, !alias.scope !18, !noalias !22
; CHECK:  NoAlias:   %2 = load double, double* %arrayidx, align 8, !tbaa !19, !alias.scope !13, !noalias !21 <->   store i32 %inc, i32* %i.priv, align 4, !tbaa !4, !alias.scope !8, !noalias !12
; CHECK:  NoAlias:   %3 = load double, double* %arrayidx6, align 8, !tbaa !19, !alias.scope !14, !noalias !21 <->   store i32 0, i32* %i.priv, align 4, !tbaa !4, !alias.scope !8, !noalias !12
; CHECK:  NoAlias:   %3 = load double, double* %arrayidx6, align 8, !tbaa !19, !alias.scope !14, !noalias !21 <->   store double %add11, double* %arrayidx12, align 8, !tbaa !19, !alias.scope !18, !noalias !22
; CHECK:  NoAlias:   %3 = load double, double* %arrayidx6, align 8, !tbaa !19, !alias.scope !14, !noalias !21 <->   store i32 %inc, i32* %i.priv, align 4, !tbaa !4, !alias.scope !8, !noalias !12
; CHECK:  NoAlias:   %4 = load double, double* %arrayidx1, align 8, !tbaa !19, !alias.scope !15, !noalias !21 <->   store i32 0, i32* %i.priv, align 4, !tbaa !4, !alias.scope !8, !noalias !12
; CHECK:  NoAlias:   %4 = load double, double* %arrayidx1, align 8, !tbaa !19, !alias.scope !15, !noalias !21 <->   store double %add11, double* %arrayidx12, align 8, !tbaa !19, !alias.scope !18, !noalias !22
; CHECK:  NoAlias:   %4 = load double, double* %arrayidx1, align 8, !tbaa !19, !alias.scope !15, !noalias !21 <->   store i32 %inc, i32* %i.priv, align 4, !tbaa !4, !alias.scope !8, !noalias !12
; CHECK:  NoAlias:   %5 = load double, double* %arrayidx8, align 8, !tbaa !19, !alias.scope !16, !noalias !21 <->   store i32 0, i32* %i.priv, align 4, !tbaa !4, !alias.scope !8, !noalias !12
; CHECK:  NoAlias:   %5 = load double, double* %arrayidx8, align 8, !tbaa !19, !alias.scope !16, !noalias !21 <->   store double %add11, double* %arrayidx12, align 8, !tbaa !19, !alias.scope !18, !noalias !22
; CHECK:  NoAlias:   %5 = load double, double* %arrayidx8, align 8, !tbaa !19, !alias.scope !16, !noalias !21 <->   store i32 %inc, i32* %i.priv, align 4, !tbaa !4, !alias.scope !8, !noalias !12
; CHECK:  NoAlias:   %6 = load double, double* %arrayidx10, align 8, !tbaa !19, !alias.scope !17, !noalias !21 <->   store i32 0, i32* %i.priv, align 4, !tbaa !4, !alias.scope !8, !noalias !12
; CHECK:  NoAlias:   %6 = load double, double* %arrayidx10, align 8, !tbaa !19, !alias.scope !17, !noalias !21 <->   store double %add11, double* %arrayidx12, align 8, !tbaa !19, !alias.scope !18, !noalias !22
; CHECK:  NoAlias:   %6 = load double, double* %arrayidx10, align 8, !tbaa !19, !alias.scope !17, !noalias !21 <->   store i32 %inc, i32* %i.priv, align 4, !tbaa !4, !alias.scope !8, !noalias !12
