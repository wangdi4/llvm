; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; This code tests TYPED clause
; The test is passed if NORMALIZED.IV:TYPED and NORMALIZED.UB:TYPED clauses are parsed correctly

; #include <omp.h>
; int main() {
;    int x = 5;
;     int i = 0;
;     #pragma omp parallel for lastprivate(x)
;     for (i = 0; i < 10; i++)
;     { x += 10; }
; }

; CHECK: IV clause: {{.*}}, TYPED (TYPE: i32, NUM_ELEMENTS: i32 1)
; CHECK: UB clause: {{.*}}, TYPED (TYPE: i32, NUM_ELEMENTS: i32 1)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = bitcast i32* %x to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  store i32 5, i32* %x, align 4, !tbaa !4
  %1 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  store i32 0, i32* %i, align 4, !tbaa !4
  %2 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  %3 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !4
  %4 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #2
  store i32 9, i32* %.omp.ub, align 4, !tbaa !4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.LASTPRIVATE:TYPED"(i32* %x, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(i32* %i, i32 0, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* %.omp.iv, i32 0), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* %.omp.ub, i32 0) ]
  %6 = load i32, i32* %.omp.lb, align 4, !tbaa !4
  store i32 %6, i32* %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %7 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %8 = load i32, i32* %.omp.ub, align 4, !tbaa !4
  %cmp = icmp sle i32 %7, %8
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !4
  %10 = load i32, i32* %x, align 4, !tbaa !4
  %add1 = add nsw i32 %10, 10
  store i32 %add1, i32* %x, align 4, !tbaa !4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %add2 = add nsw i32 %11, 1
  store i32 %add2, i32* %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %12 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %12) #2
  %13 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #2
  %14 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %14) #2
  %15 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15) #2
  %16 = bitcast i32* %x to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %16) #2
  %17 = load i32, i32* %retval, align 4
  ret i32 %17
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"clang version 10.0.0"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
