; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline=basic-aa -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform,vpo-paropt-utils -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck %s

; Test src:
;
; void foo(int n) {
;   int e[n];
; #pragma omp parallel for lastprivate(e)
;   for (int d = 0; d < 100; d++)
;     ;
; }

; CHECK:  === VPOParopt Transform: PARALLEL LOOP construct
; CHECK: collectNonPointerValuesToBeUsedInOutlinedRegion: Non-pointer values to be passed into the outlined region: 'i64 %1
; CHECK: Enter VPOParoptTransform::genLastPrivatizationCode
; CHECK: getItemInfo: Local Element Info for 'ptr %vla' (Typed):: Type: i32, NumElements: i64 %1
; CHECK: Exit VPOParoptTransform::genLastPrivatizationCode
; CHECK: %vla.lpriv = alloca i32, i64 %0, align 16

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32 noundef %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %omp.vla.tmp = alloca i64, align 8
  %d = alloca i32, align 4
  store i32 %n, ptr %n.addr, align 4, !tbaa !4
  %0 = load i32, ptr %n.addr, align 4, !tbaa !4
  %1 = zext i32 %0 to i64
  %2 = call ptr @llvm.stacksave()
  store ptr %2, ptr %saved_stack, align 8
  %vla = alloca i32, i64 %1, align 16
  store i64 %1, ptr %__vla_expr0, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #3
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #3
  store i32 0, ptr %.omp.lb, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #3
  store i32 99, ptr %.omp.ub, align 4, !tbaa !4
  store i64 %1, ptr %omp.vla.tmp, align 8, !tbaa !8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %vla, i32 0, i64 %1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %d, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.vla.tmp, i64 0, i32 1) ]
  %4 = load i64, ptr %omp.vla.tmp, align 8
  %5 = load i32, ptr %.omp.lb, align 4, !tbaa !4
  store i32 %5, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %7 = load i32, ptr %.omp.ub, align 4, !tbaa !4
  %cmp = icmp sle i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %d) #3
  %8 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %d, align 4, !tbaa !4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 4, ptr %d) #3
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %add1 = add nsw i32 %9, 1
  store i32 %add1, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #3
  %10 = load ptr, ptr %saved_stack, align 8
  call void @llvm.stackrestore(ptr %10)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #1

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind willreturn }
attributes #2 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"long", !6, i64 0}
