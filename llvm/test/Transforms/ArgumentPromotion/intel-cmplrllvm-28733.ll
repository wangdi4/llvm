; RUN: opt < %s -passes=argpromotion -S | FileCheck %s

; CHECK-DAG: define{{.*}}@b
; CHECK-DAG: define{{.*}}@.omp_outlined.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, ptr }

@a = dso_local local_unnamed_addr global i32 0, align 4
@0 = private unnamed_addr constant [23 x i8] c";unknown;unknown;0;0;;\00", align 1
@1 = private unnamed_addr constant %struct.ident_t { i32 0, i32 514, i32 0, i32 0, ptr getelementptr inbounds ([23 x i8], ptr @0, i32 0, i32 0) }, align 8
@2 = private unnamed_addr constant %struct.ident_t { i32 0, i32 2, i32 0, i32 0, ptr getelementptr inbounds ([23 x i8], ptr @0, i32 0, i32 0) }, align 8

; Function Attrs: nounwind uwtable
define dso_local i32 @b() local_unnamed_addr #0 {
entry:
  %c = alloca i32, align 4
  %0 = bitcast ptr %c to ptr
  call void @llvm.lifetime.start.p0i8(i64 4, ptr nonnull %0) #4
  call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr nonnull @2, i32 1, ptr bitcast (ptr @.omp_outlined. to ptr), ptr %c)
  call void @llvm.lifetime.end.p0i8(i64 4, ptr nonnull %0) #4
  ret i32 undef
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.lifetime.start.p0i8(i64 immarg %0, ptr nocapture %1) #1

; Function Attrs: nofree norecurse nounwind uwtable
define internal void @.omp_outlined.(ptr noalias nocapture readonly %.global_tid., ptr noalias nocapture readnone %.bound_tid., ptr nocapture nonnull readonly align 4 dereferenceable(4) %c) #2 {
entry:
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %0 = load i32, ptr %c, align 4, !tbaa !3
  %sub2 = add nsw i32 %0, -1
  %cmp = icmp sgt i32 %0, 0
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %1 = bitcast ptr %.omp.lb to ptr
  call void @llvm.lifetime.start.p0i8(i64 4, ptr nonnull %1) #4
  store i32 0, ptr %.omp.lb, align 4, !tbaa !3
  %2 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.start.p0i8(i64 4, ptr nonnull %2) #4
  store i32 %sub2, ptr %.omp.ub, align 4, !tbaa !3
  %3 = bitcast ptr %.omp.stride to ptr
  call void @llvm.lifetime.start.p0i8(i64 4, ptr nonnull %3) #4
  store i32 1, ptr %.omp.stride, align 4, !tbaa !3
  %4 = bitcast ptr %.omp.is_last to ptr
  call void @llvm.lifetime.start.p0i8(i64 4, ptr nonnull %4) #4
  store i32 0, ptr %.omp.is_last, align 4, !tbaa !3
  %5 = load i32, ptr %.global_tid., align 4, !tbaa !3
  call void @__kmpc_for_static_init_4(ptr nonnull @1, i32 %5, i32 34, ptr nonnull %.omp.is_last, ptr nonnull %.omp.lb, ptr nonnull %.omp.ub, ptr nonnull %.omp.stride, i32 1, i32 1)
  %6 = load i32, ptr %.omp.ub, align 4, !tbaa !3
  %cmp4.not = icmp slt i32 %6, %0
  %7 = select i1 %cmp4.not, i32 %6, i32 %sub2
  store i32 %7, ptr %.omp.ub, align 4, !tbaa !3
  %8 = load i32, ptr %.omp.lb, align 4, !tbaa !3
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %omp.precond.then
  %.omp.iv.0 = phi i32 [ %8, %omp.precond.then ], [ %add6, %omp.inner.for.body ]
  %9 = load i32, ptr %.omp.ub, align 4, !tbaa !3
  %cmp5.not = icmp sgt i32 %.omp.iv.0, %9
  br i1 %cmp5.not, label %omp.loop.exit, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %add6 = add nsw i32 %.omp.iv.0, 1
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  %10 = load i32, ptr %.global_tid., align 4, !tbaa !3
  call void @__kmpc_for_static_fini(ptr nonnull @1, i32 %10)
  call void @llvm.lifetime.end.p0i8(i64 4, ptr nonnull %4) #4
  call void @llvm.lifetime.end.p0i8(i64 4, ptr nonnull %3) #4
  call void @llvm.lifetime.end.p0i8(i64 4, ptr nonnull %2) #4
  call void @llvm.lifetime.end.p0i8(i64 4, ptr nonnull %1) #4
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.lifetime.end.p0i8(i64 immarg %0, ptr nocapture %1) #1

; Function Attrs: nofree nounwind
declare dso_local void @__kmpc_for_static_init_4(ptr nocapture readonly %0, i32 %1, i32 %2, ptr nocapture %3, ptr nocapture %4, ptr nocapture %5, ptr nocapture %6, i32 %7, i32 %8) local_unnamed_addr #3

; Function Attrs: nofree nounwind
declare void @__kmpc_for_static_fini(ptr nocapture readonly %0, i32 %1) local_unnamed_addr #3

; Function Attrs: nofree nounwind
declare !callback !7 void @__kmpc_fork_call(ptr %0, i32 %1, ptr %2, ...) local_unnamed_addr #3

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn mustprogress }
attributes #2 = { nofree norecurse nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nofree nounwind }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}
!nvvm.annotations = !{}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8}
!8 = !{i64 2, i64 -1, i64 -1, i1 true}
