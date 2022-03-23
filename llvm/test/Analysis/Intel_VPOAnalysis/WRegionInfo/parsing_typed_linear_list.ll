; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; This code tests TYPED for the LINEAR clause
; The test is passed if LINEAR:TYPED clauses are parsed correctly

; #include <omp.h>
; int x[10];
; int y = 1;
; int z = 2;
; int main() {
;   int m = 3;
;   int arr[] = {0,1,2,3,4};
;   int *p = arr;
; #pragma omp for linear (y,z,m,p:3)
;   for (int i = 0; i < 10; i++) {
;     x[i] = y+p[1];
;   }
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: LINEAR clause (size=4): {{.*}}, TYPED (TYPE: i32, NUM_ELEMENTS: i32 1), i32 3) (i32* @z, TYPED (TYPE: i32, NUM_ELEMENTS: i32 1), i32 3) (i32* %m, TYPED (TYPE: i32, NUM_ELEMENTS: i32 1), i32 3) (i32** %p, TYPED (TYPE: i32, NUM_ELEMENTS: i32 4), {{.*}}

@x = dso_local global [10 x i32] zeroinitializer, align 16
@y = dso_local global i32 1, align 4
@z = dso_local global i32 2, align 4
@__const.main.arr = private unnamed_addr constant [5 x i32] [i32 0, i32 1, i32 2, i32 3, i32 4], align 16

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %m = alloca i32, align 4
  %arr = alloca [5 x i32], align 16
  %p = alloca i32*, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = bitcast i32* %m to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 3, i32* %m, align 4, !tbaa !4
  %1 = bitcast [5 x i32]* %arr to i8*
  call void @llvm.lifetime.start.p0i8(i64 20, i8* %1) #3
  %2 = bitcast [5 x i32]* %arr to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 16 %2, i8* align 16 bitcast ([5 x i32]* @__const.main.arr to i8*), i64 20, i1 false)
  %3 = bitcast i32** %p to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %3) #3
  %arraydecay = getelementptr inbounds [5 x i32], [5 x i32]* %arr, i64 0, i64 0
  store i32* %arraydecay, i32** %p, align 8, !tbaa !8
  %4 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #3
  %5 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #3
  store i32 0, i32* %.omp.lb, align 4, !tbaa !4
  %6 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #3
  store i32 9, i32* %.omp.ub, align 4, !tbaa !4
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.LINEAR:TYPED"(i32* @y, i32 0, i32 1, i32 3), "QUAL.OMP.LINEAR:TYPED"(i32* @z, i32 0, i32 1, i32 3), "QUAL.OMP.LINEAR:TYPED"(i32* %m, i32 0, i32 1, i32 3), "QUAL.OMP.LINEAR:TYPED"(i32** %p, i32 0, i32 4, i32 3), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %8 = load i32, i32* %.omp.lb, align 4, !tbaa !4
  store i32 %8, i32* %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %10 = load i32, i32* %.omp.ub, align 4, !tbaa !4
  %cmp = icmp sle i32 %9, %10
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %11) #3
  %12 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %mul = mul nsw i32 %12, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !4
  %13 = load i32, i32* @y, align 4, !tbaa !4
  %14 = load i32*, i32** %p, align 8, !tbaa !8
  %arrayidx = getelementptr inbounds i32, i32* %14, i64 1
  %15 = load i32, i32* %arrayidx, align 4, !tbaa !4
  %add1 = add nsw i32 %13, %15
  %16 = load i32, i32* %i, align 4, !tbaa !4
  %idxprom = sext i32 %16 to i64
  %arrayidx2 = getelementptr inbounds [10 x i32], [10 x i32]* @x, i64 0, i64 %idxprom, !intel-tbaa !10
  store i32 %add1, i32* %arrayidx2, align 4, !tbaa !10
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %17 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %17) #3
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %18 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %add3 = add nsw i32 %18, 1
  store i32 %add3, i32* %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.LOOP"() ]
  %19 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #3
  %20 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %20) #3
  %21 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %21) #3
  %22 = bitcast i32** %p to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %22) #3
  %23 = bitcast [5 x i32]* %arr to i8*
  call void @llvm.lifetime.end.p0i8(i64 20, i8* %23) #3
  %24 = bitcast i32* %m to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %24) #3
  %25 = load i32, i32* %retval, align 4
  ret i32 %25
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { argmemonly nofree nounwind willreturn }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"pointer@_ZTSPi", !6, i64 0}
!10 = !{!11, !5, i64 0}
!11 = !{!"array@_ZTSA10_i", !5, i64 0}
