; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -S %s 2>&1 | FileCheck --check-prefixes=CHECK,ALL %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -S %s 2>&1 | FileCheck --check-prefixes=CHECK,ALL %s
; RUN: opt -opaque-pointers -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -S %s 2>&1 | FileCheck --check-prefixes=OPQPTR,ALL %s
; RUN: opt -opaque-pointers -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -S %s 2>&1 | FileCheck --check-prefixes=OPQPTR,ALL %s

; Original code:
;void foo(int &y) {
;#pragma omp for linear(y)
;  for (int i = 0; i < 10; i++);
;}

; ALL: Enter VPOParoptTransform::genLinearCode
; CHECK: getItemInfo: Local Element Info for 'i32** %y.addr' (Typed):: Type: i32
; CHECK: genPrivatizationAlloca: New Alloca for operand 'i32** %y.addr':: i32* %y.addr{{.*}}
; CHECK: genLinearCode: generated   %y.addr.deref = load i32*, i32** %y.addr
; OPQPTR: getItemInfo: Local Element Info for 'ptr %y.addr' (Typed):: Type: i32{{.*}}
; OPQPTR: genPrivatizationAlloca: New Alloca for operand 'ptr %y.addr':: ptr %y.addr{{.*}}
; OPQPTR: genLinearCode: generated   %y.addr.deref = load ptr, ptr %y.addr
; ALL: Exit VPOParoptTransform::genLinearCode
; ALL: %y.addr.linear = alloca i32
; CHECK: %y.addr.linear.ref = alloca i32*
; CHECK: store i32* %y.addr.linear, i32** %y.addr.linear.ref
; OPQPTR: %y.addr.linear.ref = alloca ptr
; OPQPTR: store ptr %y.addr.linear, ptr %y.addr.linear.ref


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z3fooRi(i32* nonnull align 4 dereferenceable(4) %y) #0 {
entry:
  %y.addr = alloca i32*, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32* %y, i32** %y.addr, align 8, !tbaa !4
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !8
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  store i32 9, i32* %.omp.ub, align 4, !tbaa !8
  %3 = load i32*, i32** %y.addr, align 8, !tbaa !4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.LINEAR:BYREF.TYPED"(i32** %y.addr, i32 0, i32 1, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* %.omp.iv, i32 0), "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %.omp.lb, i32 0, i32 1), "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* %.omp.ub, i32 0), "QUAL.OMP.PRIVATE:TYPED"(i32* %i, i32 0, i32 1) ]
  %5 = load i32, i32* %.omp.lb, align 4, !tbaa !8
  store i32 %5, i32* %.omp.iv, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i32, i32* %.omp.iv, align 4, !tbaa !8
  %7 = load i32, i32* %.omp.ub, align 4, !tbaa !8
  %cmp = icmp sle i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %8) #2
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !8
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %10 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %10) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, i32* %.omp.iv, align 4, !tbaa !8
  %add1 = add nsw i32 %11, 1
  store i32 %add1, i32* %.omp.iv, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.LOOP"() ]
  %12 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %12) #2
  %13 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #2
  %14 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %14) #2
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
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"clang version 13.0.0"}
!4 = !{!5, !5, i64 0}
!5 = !{!"any pointer", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !6, i64 0}
