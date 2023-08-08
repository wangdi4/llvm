; REQUIRES: asserts
; RUN: opt -passes='vpo-paropt,print<inline-report>' -inline-report=0xf847 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup,vpo-paropt,inlinereportemitter' -inline-report=0xf8c6 -disable-output < %s 2>&1 | FileCheck %s

; Check that the asserts that OldFunction->getName() == "" and
; NewFunction->getName() != "" are respected when replaceFunctionWithFunction
; is called from the inlining reports.

; CHECK: COMPILE FUNC: main
; CHECK: EXTERN: printf
; CHECK: BROKER: __kmpc_fork_call(main.DIR.OMP.PARALLEL.2)
; CHECK: COMPILE FUNC: main.DIR.OMP.PARALLEL.2
; CHECK: DELETE: llvm.directive.region.entry
; CHECK: DELETE: llvm.directive.region.exit

; ModuleID = 'sm1.c'
source_filename = "sm1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
DIR.OMP.PARALLEL.321:
  %a = alloca [1000000 x float], align 16
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4000000, ptr nonnull %a) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i) #2
  %end.dir.temp18 = alloca i1, align 1
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %DIR.OMP.PARALLEL.321
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED:TYPED"(ptr %a, float 0.000000e+00, i64 1000000), "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1), "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp18) ]
  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.2
  %temp.load19 = load volatile i1, ptr %end.dir.temp18, align 1
  br i1 %temp.load19, label %DIR.OMP.END.PARALLEL.9, label %DIR.OMP.LOOP.522

DIR.OMP.LOOP.522:                                 ; preds = %DIR.OMP.PARALLEL.3
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #2
  store i32 0, ptr %.omp.lb, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #2
  store volatile i32 999999, ptr %.omp.ub, align 4, !tbaa !4
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.LOOP.4

DIR.OMP.LOOP.4:                                   ; preds = %DIR.OMP.LOOP.522
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0), "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0), "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]
  br label %DIR.OMP.LOOP.523

DIR.OMP.LOOP.523:                                 ; preds = %DIR.OMP.LOOP.4
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.LOOP.7, label %DIR.OMP.LOOP.5

DIR.OMP.LOOP.5:                                   ; preds = %DIR.OMP.LOOP.523
  %2 = load i32, ptr %.omp.lb, align 4, !tbaa !4
  store volatile i32 %2, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.LOOP.5
  %3 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !4
  %4 = load volatile i32, ptr %.omp.ub, align 4, !tbaa !4
  %cmp.not = icmp sgt i32 %3, %4
  br i1 %cmp.not, label %DIR.OMP.END.LOOP.7.loopexit, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !4
  store i32 %5, ptr %i, align 4, !tbaa !4
  %conv = sitofp i32 %5 to float
  %idxprom = sext i32 %5 to i64
  %arrayidx = getelementptr inbounds [1000000 x float], ptr %a, i64 0, i64 %idxprom, !intel-tbaa !8
  store float %conv, ptr %arrayidx, align 4, !tbaa !8
  %6 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !4
  %add1 = add nsw i32 %6, 1
  store volatile i32 %add1, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

DIR.OMP.END.LOOP.7.loopexit:                      ; preds = %omp.inner.for.cond
  br label %DIR.OMP.END.LOOP.7

DIR.OMP.END.LOOP.7:                               ; preds = %DIR.OMP.END.LOOP.7.loopexit, %DIR.OMP.LOOP.523
  br label %DIR.OMP.END.LOOP.6

DIR.OMP.END.LOOP.6:                               ; preds = %DIR.OMP.END.LOOP.7
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.LOOP.724

DIR.OMP.END.LOOP.724:                             ; preds = %DIR.OMP.END.LOOP.6
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.ub) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.lb) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #2
  br label %DIR.OMP.END.PARALLEL.9

DIR.OMP.END.PARALLEL.9:                           ; preds = %DIR.OMP.PARALLEL.3, %DIR.OMP.END.LOOP.724
  br label %DIR.OMP.END.PARALLEL.8

DIR.OMP.END.PARALLEL.8:                           ; preds = %DIR.OMP.END.PARALLEL.9
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.925

DIR.OMP.END.PARALLEL.925:                         ; preds = %DIR.OMP.END.PARALLEL.8
  %arrayidx2 = getelementptr inbounds [1000000 x float], ptr %a, i64 0, i64 10, !intel-tbaa !8
  %7 = load float, ptr %arrayidx2, align 8, !tbaa !8
  %conv3 = fpext float %7 to double
  %call = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str, double noundef nofpclass(nan inf) %conv3)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i) #2
  call void @llvm.lifetime.end.p0(i64 4000000, ptr nonnull %a) #2
  ret i32 0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #3

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nounwind }
attributes #3 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !10, i64 0}
!9 = !{!"array@_ZTSA1000000_f", !10, i64 0}
!10 = !{!"float", !6, i64 0}
