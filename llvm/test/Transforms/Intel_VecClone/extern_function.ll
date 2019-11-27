; We do not clone extern functions.

; RUN: opt -vec-clone -S < %s
; RUN: opt -passes="vec-clone" -S < %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local global [4096 x float] zeroinitializer, align 16
@.str = private unnamed_addr constant [11 x i8] c"passed %f\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %omp.inner.for.body.lr.ph ]
  %1 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %1 to double
  %mul1 = fmul fast double %conv, 5.000000e-01
  %conv2 = fptrunc double %mul1 to float
  %arrayidx = getelementptr inbounds [4096 x float], [4096 x float]* @a, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store float %conv2, float* %arrayidx, align 4, !tbaa !2
  %call = tail call fast float @dowork(float* getelementptr inbounds ([4096 x float], [4096 x float]* @a, i64 0, i64 0), i32 %1) #1
  store float %call, float* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4096
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  %2 = load float, float* getelementptr inbounds ([4096 x float], [4096 x float]* @a, i64 0, i64 1024), align 16, !tbaa !2
  %conv6 = fpext float %2 to double
  %call7 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i64 0, i64 0), double %conv6)
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local float @dowork(float*, i32) local_unnamed_addr #2

; Function Attrs: nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #3

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" "vector-variants"="_ZGVbN4ul_dowork,_ZGVcN4ul_dowork,_ZGVdN4ul_dowork,_ZGVeN4ul_dowork" }
attributes #3 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang dba1f222b81823040e16528a12868e26b01725a8) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 3136500e59eb9f4febcf6ba130d030abdde64ac5)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA4096_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
