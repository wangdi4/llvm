; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -S %s 2>&1 | FileCheck --check-prefixes=CHECK,ALL %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -S %s 2>&1 | FileCheck --check-prefixes=CHECK,ALL %s
; RUN: opt -opaque-pointers -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -S %s 2>&1 | FileCheck --check-prefixes=OPQPTR,ALL %s
; RUN: opt -opaque-pointers -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -S %s 2>&1 | FileCheck --check-prefixes=OPQPTR,ALL %s


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; void foo()
; {
;   int l = 0;
; #pragma omp simd reduction(+:l)
;   for (int i = 0; i < 1000; ++i)
;     ++l;
; }


; ALL: Enter VPOParoptTransform::genReductionCode
; CHECK: getItemInfo: Local Element Info for 'i32* %l' (Typed):: Type: i32{{.*}}
; OPQPTR: getItemInfo: Local Element Info for 'ptr %l' (Typed):: Type: i32{{.*}}
; ALL: Exit VPOParoptTransform::genReductionCode
; ALL: %l.red = alloca i32, align 4


; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %l = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = bitcast i32* %l to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  store i32 0, i32* %l, align 4, !tbaa !4
  %1 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  store i32 999, i32* %.omp.ub, align 4, !tbaa !4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(i32* %l, i32 0, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* %.omp.iv, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* %.omp.ub, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(i32* %i, i32 0, i32 1, i32 1) ]
  store i32 0, i32* %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %5 = load i32, i32* %.omp.ub, align 4, !tbaa !4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #2
  %7 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !4
  %8 = load i32, i32* %l, align 4, !tbaa !4
  %inc = add nsw i32 %8, 1
  store i32 %inc, i32* %l, align 4, !tbaa !4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %9 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %9) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %add1 = add nsw i32 %10, 1
  store i32 %add1, i32* %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SIMD"() ]
  %11 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %11) #2
  %12 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %12) #2
  %13 = bitcast i32* %l to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #2
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { mustprogress nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
