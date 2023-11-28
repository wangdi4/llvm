; RUN: opt -passes="function(vpo-cfg-restructuring),vpo-paropt,aa-eval" -aa-pipeline="basic-aa,scoped-noalias-aa" -evaluate-aa-metadata -print-all-alias-modref-info -disable-output %s 2>&1 | FileCheck %s

; It tests whether the alias analysis information is kept after the outlining.
; For example, after the following loop is outlined, the compiler can still
; understand *glob is not aliased with *tmp1 and *tmp2.

; Test src:
;
; void foo(double *glob) {
;   double tmp[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
;   double *tmp1 = &tmp[0];
;   double *tmp2 = &tmp[2];
;
; #pragma omp target map(tofrom : tmp1 [0:4], tmp2 [0:2], glob [0:999])
;   {
;     for (int i = 0; i < 1000; ++i) {
;       glob[i] = tmp1[0] * tmp1[1] + tmp2[0] * tmp2[1] + tmp2[2];
;     }
;   }
; }

; CHECK:  NoAlias:   %1 = load i32, ptr %i.priv, align 4, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store double %add11, ptr %arrayidx12, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}
; CHECK:  MustAlias:   %1 = load i32, ptr %i.priv, align 4, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store i32 %inc, ptr %i.priv, align 4, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}
; CHECK:  NoAlias:   %2 = load double, ptr %arrayidx, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store i32 0, ptr %i.priv, align 4, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}
; CHECK:  NoAlias:   %2 = load double, ptr %arrayidx, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store double %add11, ptr %arrayidx12, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}
; CHECK:  NoAlias:   %2 = load double, ptr %arrayidx, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store i32 %inc, ptr %i.priv, align 4, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}
; CHECK:  NoAlias:   %3 = load double, ptr %arrayidx6, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store i32 0, ptr %i.priv, align 4, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}
; CHECK:  NoAlias:   %3 = load double, ptr %arrayidx6, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store double %add11, ptr %arrayidx12, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}
; CHECK:  NoAlias:   %3 = load double, ptr %arrayidx6, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store i32 %inc, ptr %i.priv, align 4, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}
; CHECK:  NoAlias:   %4 = load double, ptr %arrayidx1, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store i32 0, ptr %i.priv, align 4, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}
; CHECK:  NoAlias:   %4 = load double, ptr %arrayidx1, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store double %add11, ptr %arrayidx12, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}
; CHECK:  NoAlias:   %4 = load double, ptr %arrayidx1, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store i32 %inc, ptr %i.priv, align 4, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}
; CHECK:  NoAlias:   %5 = load double, ptr %arrayidx8, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store i32 0, ptr %i.priv, align 4, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}
; CHECK:  NoAlias:   %5 = load double, ptr %arrayidx8, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store double %add11, ptr %arrayidx12, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}
; CHECK:  NoAlias:   %5 = load double, ptr %arrayidx8, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store i32 %inc, ptr %i.priv, align 4, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}
; CHECK:  NoAlias:   %6 = load double, ptr %arrayidx10, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store i32 0, ptr %i.priv, align 4, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}
; CHECK:  NoAlias:   %6 = load double, ptr %arrayidx10, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store double %add11, ptr %arrayidx12, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}
; CHECK:  NoAlias:   %6 = load double, ptr %arrayidx10, align 8, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}} <->   store i32 %inc, ptr %i.priv, align 4, !tbaa !{{[0-9]+}}, !alias.scope !{{[0-9]+}}, !noalias !{{[0-9]+}}

target triple = "x86_64-mic"
target device_triples = "x86_64-mic"

@"@tid.addr" = external global i32

; Function Attrs: nounwind uwtable
define void @foo(ptr %glob) #0 {
entry:
  %tmp = alloca [5 x double], align 16
  %i = alloca i32, align 4
  %0 = bitcast ptr %tmp to ptr
  call void @llvm.lifetime.start.p0(i64 40, ptr %0) #1
  call void @llvm.memset.p0.i64(ptr align 16 %0, i8 0, i64 40, i1 false)
  %1 = getelementptr [5 x double], ptr %tmp, i32 0, i32 0
  store double 1.000000e+00, ptr %1, align 8
  %2 = getelementptr [5 x double], ptr %tmp, i32 0, i32 1
  store double 2.000000e+00, ptr %2, align 8
  %3 = getelementptr [5 x double], ptr %tmp, i32 0, i32 2
  store double 3.000000e+00, ptr %3, align 8
  %4 = getelementptr [5 x double], ptr %tmp, i32 0, i32 3
  store double 4.000000e+00, ptr %4, align 8
  %5 = getelementptr [5 x double], ptr %tmp, i32 0, i32 4
  store double 5.000000e+00, ptr %5, align 8
  %arrayidx = getelementptr inbounds [5 x double], ptr %tmp, i64 0, i64 0
  %arrayidx1 = getelementptr inbounds [5 x double], ptr %tmp, i64 0, i64 2
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(ptr %arrayidx, ptr %arrayidx, i64 32),
    "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(ptr %arrayidx1, ptr %arrayidx1, i64 16),
    "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(ptr %glob, ptr %glob, i64 7992),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  %7 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %7) #1
  store i32 0, ptr %i, align 4, !tbaa !4
  br label %for.cond

for.cond:                                         ; preds = %for.body, %DIR.OMP.TARGET.1
  %8 = load i32, ptr %i, align 4, !tbaa !4
  %cmp = icmp slt i32 %8, 1000
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0(i64 4, ptr %7) #1
  br label %for.end

for.body:                                         ; preds = %for.cond
  %9 = load double, ptr %arrayidx, align 8, !tbaa !8
  %arrayidx6 = getelementptr inbounds double, ptr %arrayidx, i64 1
  %10 = load double, ptr %arrayidx6, align 8, !tbaa !8
  %mul = fmul double %9, %10
  %11 = load double, ptr %arrayidx1, align 8, !tbaa !8
  %arrayidx8 = getelementptr inbounds double, ptr %arrayidx1, i64 1
  %12 = load double, ptr %arrayidx8, align 8, !tbaa !8
  %mul9 = fmul double %11, %12
  %add = fadd double %mul, %mul9
  %arrayidx10 = getelementptr inbounds double, ptr %arrayidx1, i64 2
  %13 = load double, ptr %arrayidx10, align 8, !tbaa !8
  %add11 = fadd double %add, %13
  %idxprom = sext i32 %8 to i64
  %arrayidx12 = getelementptr inbounds double, ptr %glob, i64 %idxprom
  store double %add11, ptr %arrayidx12, align 8, !tbaa !8
  %14 = load i32, ptr %i, align 4, !tbaa !4
  %inc = add nsw i32 %14, 1
  store i32 %inc, ptr %i, align 4, !tbaa !4
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.lifetime.end.p0(i64 40, ptr %0) #1
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #3 = { argmemonly nofree nounwind willreturn writeonly }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = !{i32 0, i32 54, i32 -698850821, !"foo", i32 47, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"double", !6, i64 0}
