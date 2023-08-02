; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,inline' -disable-output -inline-report=0xf847 < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup,function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,inline,inlinereportemitter' -disable-output -inline-report=0xf8c6 < %s 2>&1 | FileCheck %s

; Check that the broker function __kmpc_fork_call and its target function
; main.DIR.OMP.PARALLEL.220 is recognized after VPOParoptPass.
; NOTE: Some EXTERNs will not appear in the metdata inlining report for now,
; because they have not been specifically instrumented.

; CHECK-LABEL: COMPILE FUNC: main
; CHECK: EXTERN: printf
; CHECK: BROKER: __kmpc_fork_call(main.DIR.OMP.PARALLEL.220)
; CHECK-LABEL: COMPILE FUNC: main.DIR.OMP.PARALLEL.220
; CHECK: DELETE: llvm.directive.region.entry
; CHECK-CL: EXTERN: __kmpc_for_static_init_4
; CHECK-CL: EXTERN: __kmpc_for_static_fini
; CHECK-CL: EXTERN: __kmpc_barrier
; CHECK: DELETE: llvm.directive.region.exit

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %a = alloca [1000000 x float], align 16
  %b = alloca [1000000 x float], align 16
  %c = alloca [1000000 x float], align 16
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4000000, ptr nonnull %a) #2
  call void @llvm.lifetime.start.p0(i64 4000000, ptr nonnull %b) #2
  call void @llvm.lifetime.start.p0(i64 4000000, ptr nonnull %c) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i) #2
  store i32 0, ptr %i, align 4, !tbaa !4
  %.pr = load i32, ptr %i, align 4, !tbaa !4
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %0 = phi i32 [ %inc, %for.body ], [ %.pr, %entry ]
  %cmp = icmp slt i32 %0, 1000000
  br i1 %cmp, label %for.body, label %DIR.OMP.PARALLEL.341

for.body:                                         ; preds = %for.cond
  %conv = sitofp i32 %0 to double
  %mul = fmul fast double %conv, 2.000000e+00
  %conv1 = fptrunc double %mul to float
  %idxprom = sext i32 %0 to i64
  %arrayidx = getelementptr inbounds [1000000 x float], ptr %a, i64 0, i64 %idxprom, !intel-tbaa !8
  store float %conv1, ptr %arrayidx, align 4, !tbaa !8
  %1 = load i32, ptr %i, align 4, !tbaa !4
  %conv2 = sitofp i32 %1 to double
  %mul3 = fmul fast double %conv2, 3.000000e+00
  %conv4 = fptrunc double %mul3 to float
  %idxprom5 = sext i32 %1 to i64
  %arrayidx6 = getelementptr inbounds [1000000 x float], ptr %b, i64 0, i64 %idxprom5, !intel-tbaa !8
  store float %conv4, ptr %arrayidx6, align 4, !tbaa !8
  %2 = load i32, ptr %i, align 4, !tbaa !4
  %inc = add nsw i32 %2, 1
  store i32 %inc, ptr %i, align 4, !tbaa !4
  br label %for.cond, !llvm.loop !11

DIR.OMP.PARALLEL.341:                             ; preds = %for.cond
  %end.dir.temp38 = alloca i1, align 1
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %DIR.OMP.PARALLEL.341
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %a, float 0.000000e+00, i64 1000000),
    "QUAL.OMP.SHARED:TYPED"(ptr %b, float 0.000000e+00, i64 1000000),
    "QUAL.OMP.SHARED:TYPED"(ptr %c, float 0.000000e+00, i64 1000000),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp38) ]

  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.2
  %temp.load39 = load volatile i1, ptr %end.dir.temp38, align 1
  br i1 %temp.load39, label %DIR.OMP.END.PARALLEL.9, label %DIR.OMP.LOOP.542

DIR.OMP.LOOP.542:                                 ; preds = %DIR.OMP.PARALLEL.3
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #2
  store i32 0, ptr %.omp.lb, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #2
  store volatile i32 999999, ptr %.omp.ub, align 4, !tbaa !4
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.LOOP.4

DIR.OMP.LOOP.4:                                   ; preds = %DIR.OMP.LOOP.542
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]

  br label %DIR.OMP.LOOP.543

DIR.OMP.LOOP.543:                                 ; preds = %DIR.OMP.LOOP.4
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.LOOP.7, label %DIR.OMP.LOOP.5

DIR.OMP.LOOP.5:                                   ; preds = %DIR.OMP.LOOP.543
  %5 = load i32, ptr %.omp.lb, align 4, !tbaa !4
  store volatile i32 %5, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.LOOP.5
  %6 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !4
  %7 = load volatile i32, ptr %.omp.ub, align 4, !tbaa !4
  %cmp9.not = icmp sgt i32 %6, %7
  br i1 %cmp9.not, label %DIR.OMP.END.LOOP.7.loopexit, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !4
  store i32 %8, ptr %i, align 4, !tbaa !4
  %idxprom12 = sext i32 %8 to i64
  %arrayidx13 = getelementptr inbounds [1000000 x float], ptr %a, i64 0, i64 %idxprom12, !intel-tbaa !8
  %9 = load float, ptr %arrayidx13, align 4, !tbaa !8
  %arrayidx15 = getelementptr inbounds [1000000 x float], ptr %b, i64 0, i64 %idxprom12, !intel-tbaa !8
  %10 = load float, ptr %arrayidx15, align 4, !tbaa !8
  %add16 = fadd fast float %9, %10
  %arrayidx18 = getelementptr inbounds [1000000 x float], ptr %c, i64 0, i64 %idxprom12, !intel-tbaa !8
  store float %add16, ptr %arrayidx18, align 4, !tbaa !8
  %11 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !4
  %add19 = add nsw i32 %11, 1
  store volatile i32 %add19, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

DIR.OMP.END.LOOP.7.loopexit:                      ; preds = %omp.inner.for.cond
  br label %DIR.OMP.END.LOOP.7

DIR.OMP.END.LOOP.7:                               ; preds = %DIR.OMP.END.LOOP.7.loopexit, %DIR.OMP.LOOP.543
  br label %DIR.OMP.END.LOOP.6

DIR.OMP.END.LOOP.6:                               ; preds = %DIR.OMP.END.LOOP.7
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.LOOP"() ]

  br label %DIR.OMP.END.LOOP.744

DIR.OMP.END.LOOP.744:                             ; preds = %DIR.OMP.END.LOOP.6
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.ub) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.lb) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #2
  br label %DIR.OMP.END.PARALLEL.9

DIR.OMP.END.PARALLEL.9:                           ; preds = %DIR.OMP.END.LOOP.744, %DIR.OMP.PARALLEL.3
  br label %DIR.OMP.END.PARALLEL.8

DIR.OMP.END.PARALLEL.8:                           ; preds = %DIR.OMP.END.PARALLEL.9
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]

  br label %DIR.OMP.END.PARALLEL.945

DIR.OMP.END.PARALLEL.945:                         ; preds = %DIR.OMP.END.PARALLEL.8
  %arrayidx20 = getelementptr inbounds [1000000 x float], ptr %c, i64 0, i64 10, !intel-tbaa !8
  %12 = load float, ptr %arrayidx20, align 8, !tbaa !8
  %conv21 = fpext float %12 to double
  %call = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str, double noundef nofpclass(nan inf) %conv21)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i) #2
  call void @llvm.lifetime.end.p0(i64 4000000, ptr nonnull %c) #2
  call void @llvm.lifetime.end.p0(i64 4000000, ptr nonnull %b) #2
  call void @llvm.lifetime.end.p0(i64 4000000, ptr nonnull %a) #2
  ret i32 0
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #3

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
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
!11 = distinct !{!11, !12}
!12 = !{!"llvm.loop.mustprogress"}
