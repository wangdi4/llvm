; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test is used to check parsing of TYPED reduction clause
; with bool and operation for a float type.

; #include <stdio.h>
; #include <math.h>
;
; #define eps         1.0e-12
;
; int
; main() {
;
;    float v3;
;    int i;
;
;    v3 = 1;
;
;    #pragma omp parallel for reduction(&&: v3)
;       for( i = 0; i < 3; i++ ) {
;          v3 = v3 && 1;
;       }
;    if ( fabsl( v3 - 1 ) > eps ) {
;     printf("FAILED\n");
;      return 3;
;    }
;     printf("PASSED\n");
;    return 0;
; }

; CHECK: REDUCTION clause (size=1): (AND: float* %v3, TYPED (TYPE: float, NUM_ELEMENTS: i32 1))

@.str = private unnamed_addr constant [8 x i8] c"FAILED\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"PASSED\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %v3 = alloca float, align 4
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %cleanup.dest.slot = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = bitcast float* %v3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  store float 1.000000e+00, float* %v3, align 4, !tbaa !4
  %2 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  %3 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !8
  %4 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #2
  store i32 2, i32* %.omp.ub, align 4, !tbaa !8
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.AND:TYPED"(float* %v3, float 0.0, i32 1), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]
  %6 = load i32, i32* %.omp.lb, align 4, !tbaa !8
  store i32 %6, i32* %.omp.iv, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %7 = load i32, i32* %.omp.iv, align 4, !tbaa !8
  %8 = load i32, i32* %.omp.ub, align 4, !tbaa !8
  %cmp = icmp sle i32 %7, %8
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !8
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !8
  %10 = load float, float* %v3, align 4, !tbaa !4
  %tobool = fcmp fast une float %10, 0.000000e+00
  br i1 %tobool, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %omp.inner.for.body
  br label %land.end

land.end:                                         ; preds = %land.rhs, %omp.inner.for.body
  %11 = phi i1 [ false, %omp.inner.for.body ], [ true, %land.rhs ]
  %land.ext = zext i1 %11 to i32
  %conv = sitofp i32 %land.ext to float
  store float %conv, float* %v3, align 4, !tbaa !4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %land.end
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, i32* %.omp.iv, align 4, !tbaa !8
  %add1 = add nsw i32 %12, 1
  store i32 %add1, i32* %.omp.iv, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %13 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #2
  %14 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %14) #2
  %15 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15) #2
  %16 = load float, float* %v3, align 4, !tbaa !4
  %sub = fsub fast float %16, 1.000000e+00
  %conv2 = fpext float %sub to x86_fp80
  %17 = call fast x86_fp80 @llvm.fabs.f80(x86_fp80 %conv2)
  %cmp3 = fcmp fast ogt x86_fp80 %17, 0xK3FD78CBCCC096F508800
  br i1 %cmp3, label %if.then, label %if.end

if.then:                                          ; preds = %omp.loop.exit
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0))
  store i32 3, i32* %retval, align 4
  store i32 1, i32* %cleanup.dest.slot, align 4
  br label %cleanup

if.end:                                           ; preds = %omp.loop.exit
  %call5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i64 0, i64 0))
  store i32 0, i32* %retval, align 4
  store i32 1, i32* %cleanup.dest.slot, align 4
  br label %cleanup

cleanup:                                          ; preds = %if.end, %if.then
  %18 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %18) #2
  %19 = bitcast float* %v3 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #2
  %20 = load i32, i32* %retval, align 4
  ret i32 %20
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare x86_fp80 @llvm.fabs.f80(x86_fp80) #3

declare dso_local i32 @printf(i8*, ...) #4

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #4 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!4 = !{!5, !5, i64 0}
!5 = !{!"float", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !6, i64 0}
