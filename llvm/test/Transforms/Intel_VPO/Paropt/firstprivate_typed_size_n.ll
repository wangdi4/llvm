; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck --check-prefixes=CHECK,ALL %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck --check-prefixes=CHECK,ALL %s
; RUN: opt -opaque-pointers -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck --check-prefixes=OPQPTR,ALL %s
; RUN: opt -opaque-pointers -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck --check-prefixes=OPQPTR,ALL %s


; void foo(int n) {
; int e[n];
; #pragma omp parallel for firstprivate(e)
;   for(int d=0; d<100; d++);
; }

; ALL:  === VPOParopt Transform: PARALLEL LOOP construct
; ALL: collectNonPointerValuesToBeUsedInOutlinedRegion: Non-pointer values to be passed into the outlined region: 'i32 %n
; ALL: Enter VPOParoptTransform::genFirstPrivatizationCode
; CHECK: getItemInfo: Local Element Info for 'i32* %vla' (Typed):: Type: i32, NumElements: i32 %n
; OPQPTR: getItemInfo: Local Element Info for 'ptr %vla' (Typed):: Type: i32, NumElements: i32 %n
; ALL: Exit VPOParoptTransform::genFirstPrivatizationCode
; CHECK: %vla.fpriv = alloca i32, i32 %n{{.*}}
; OPQPTR: %vla.fpriv = alloca i32, i32 %n{{.*}}


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z3fooi(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca i8*, align 8
  %__vla_expr0 = alloca i64, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %omp.vla.tmp = alloca i64, align 8
  %d = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4, !tbaa !4
  %0 = load i32, i32* %n.addr, align 4, !tbaa !4
  %1 = zext i32 %0 to i64
  %2 = call i8* @llvm.stacksave()
  store i8* %2, i8** %saved_stack, align 8
  %vla = alloca i32, i64 %1, align 16
  store i64 %1, i64* %__vla_expr0, align 8
  %3 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #3
  %4 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #3
  store i32 0, i32* %.omp.lb, align 4, !tbaa !4
  %5 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #3
  store i32 99, i32* %.omp.ub, align 4, !tbaa !4
  store i64 %1, i64* %omp.vla.tmp, align 8, !tbaa !8
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %vla, i32 0, i32 %n), "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* %.omp.iv, i32 0), "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %.omp.lb, i32 0, i32 1), "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* %.omp.ub, i32 0), "QUAL.OMP.PRIVATE:TYPED"(i32* %d, i32 0, i32 1), "QUAL.OMP.SHARED"(i64* %omp.vla.tmp) ]
  %7 = load i64, i64* %omp.vla.tmp, align 8
  %8 = load i32, i32* %.omp.lb, align 4, !tbaa !4
  store i32 %8, i32* %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %10 = load i32, i32* %.omp.ub, align 4, !tbaa !4
  %cmp = icmp sle i32 %9, %10
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = bitcast i32* %d to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %11) #3
  %12 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %mul = mul nsw i32 %12, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %d, align 4, !tbaa !4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %13 = bitcast i32* %d to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #3
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %add1 = add nsw i32 %14, 1
  store i32 %add1, i32* %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %15 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15) #3
  %16 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %16) #3
  %17 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %17) #3
  %18 = load i8*, i8** %saved_stack, align 8
  call void @llvm.stackrestore(i8* %18)
  ret void
}

; Function Attrs: nofree nosync nounwind willreturn
declare i8* @llvm.stacksave() #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: nofree nosync nounwind willreturn
declare void @llvm.stackrestore(i8*) #1

attributes #0 = { mustprogress nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nofree nosync nounwind willreturn }
attributes #2 = { argmemonly nofree nosync nounwind willreturn }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"long", !6, i64 0}
