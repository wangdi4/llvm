; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck --check-prefixes=CHECK,ALL %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck --check-prefixes=CHECK,ALL %s
; RUN: opt -opaque-pointers -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck --check-prefixes=OPQPTR,ALL %s
; RUN: opt -opaque-pointers -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck --check-prefixes=OPQPTR,ALL %s


; int main() {
; int e[100];
; #pragma omp parallel for firstprivate(e)
;   for(int d=0; d<100; d++);
; }

; ALL: Enter VPOParoptTransform::genFirstPrivatizationCode
; CHECK: getItemInfo: Local Element Info for '[100 x i32]* %e' (Typed):: Type: i32, NumElements: i32 100
; OPQPTR: getItemInfo: Local Element Info for 'ptr %e' (Typed):: Type: i32, NumElements: i32 100
; ALL: Exit VPOParoptTransform::genFirstPrivatizationCode
; CHECK: %e.fpriv = alloca [100 x i32]{{.*}}
; OPQPTR: %e.fpriv = alloca [100 x i32]{{.*}}


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress norecurse nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %e = alloca [100 x i32], align 16
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %d = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = bitcast [100 x i32]* %e to i8*
  call void @llvm.lifetime.start.p0i8(i64 400, i8* %0) #2
  %1 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  %2 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !4
  %3 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #2
  store i32 99, i32* %.omp.ub, align 4, !tbaa !4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE:TYPED"([100 x i32]* %e, i32 0, i32 100), "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* %.omp.iv, i32 0), "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %.omp.lb, i32 0, i32 1), "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* %.omp.ub, i32 0), "QUAL.OMP.PRIVATE:TYPED"(i32* %d, i32 0, i32 1) ]
  %5 = load i32, i32* %.omp.lb, align 4, !tbaa !4
  store i32 %5, i32* %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %7 = load i32, i32* %.omp.ub, align 4, !tbaa !4
  %cmp = icmp sle i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = bitcast i32* %d to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %8) #2
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %d, align 4, !tbaa !4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %10 = bitcast i32* %d to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %10) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %add1 = add nsw i32 %11, 1
  store i32 %add1, i32* %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %12 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %12) #2
  %13 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #2
  %14 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %14) #2
  %15 = bitcast [100 x i32]* %e to i8*
  call void @llvm.lifetime.end.p0i8(i64 400, i8* %15) #2
  %16 = load i32, i32* %retval, align 4
  ret i32 %16
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { mustprogress norecurse nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
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
