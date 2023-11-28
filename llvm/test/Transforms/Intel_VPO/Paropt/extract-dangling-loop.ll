; RUN: opt -passes="vpo-paropt" -S %s 2>&1 | FileCheck %s

; Test src:
;
; int a, b, c;
;
; void foo() {
;   int e;
;   for (int i = 0; i < a; ++i)
;     if (e)
;       e = c;
; }
;
; void bar() {
; #pragma omp for
;   for (int i = 0; i < b; ++i)
; #pragma omp task
;     foo();
; }

; After outlining, the LoopInfo for the "omp for" loop, contains dangling
; pointers to the outlined blocks in the omp task. This leads to an incorrect
; transformation of the already outlined code.
; The CHECK lines verify that the task code is not transformed semantically
; after outlining.

; CHECK: define{{.*}}TASK
; ...
; CHECK: %tobool.i = icmp eq i32 %e.0.i, 0
; CHECK: [[RES:%[0-9]+]] = load i32, ptr @c
; CHECK: %spec.select.i = select i1 %tobool.i, i32 0, i32 [[RES]]
; CHECK: %inc.i = add nuw nsw i32 %i.0.i, 1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global i32 0, align 4
@b = dso_local local_unnamed_addr global i32 0, align 4
@c = dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: nounwind uwtable
define dso_local void @_Z3foov() local_unnamed_addr #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %e.0 = phi i32 [ undef, %entry ], [ %spec.select, %for.body ]
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %0 = load i32, ptr @a, align 4
  %cmp = icmp slt i32 %i.0, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

for.body:                                         ; preds = %for.cond
  %tobool = icmp eq i32 %e.0, 0
  %1 = load i32, ptr @c, align 4
  %spec.select = select i1 %tobool, i32 0, i32 %1
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond
}

; Function Attrs: nounwind uwtable
define dso_local void @_Z3barv() local_unnamed_addr #1 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = load i32, ptr @b, align 4
  %cmp = icmp sgt i32 %0, 0
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %sub2 = add nsw i32 %0, -1
  %1 = bitcast ptr %.omp.iv to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %1) #2
  %2 = bitcast ptr %.omp.lb to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %2) #2
  store i32 0, ptr %.omp.lb, align 4
  %3 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %3) #2
  store volatile i32 %sub2, ptr %.omp.ub, align 4
  br label %DIR.OMP.LOOP.1

DIR.OMP.LOOP.1:                                   ; preds = %omp.precond.then
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  br label %DIR.OMP.LOOP.216

DIR.OMP.LOOP.216:                                 ; preds = %DIR.OMP.LOOP.1
  br label %DIR.OMP.LOOP.2

DIR.OMP.LOOP.2:                                   ; preds = %DIR.OMP.LOOP.216
  %5 = load i32, ptr %.omp.lb, align 4
  store volatile i32 %5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %DIR.OMP.END.TASK.7, %DIR.OMP.LOOP.2
  %6 = load volatile i32, ptr %.omp.iv, align 4
  %7 = load volatile i32, ptr %.omp.ub, align 4
  %cmp4 = icmp sgt i32 %6, %7
  br i1 %cmp4, label %omp.loop.exit.split, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %8) #2
  %9 = load volatile i32, ptr %.omp.iv, align 4
  store i32 %9, ptr %i, align 4
  br label %DIR.OMP.TASK.3

DIR.OMP.TASK.3:                                   ; preds = %omp.inner.for.body
  br label %DIR.OMP.TASK.4

DIR.OMP.TASK.4:                                   ; preds = %DIR.OMP.TASK.3
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"() ]
  br label %DIR.OMP.TASK.5

DIR.OMP.TASK.5:                                   ; preds = %DIR.OMP.TASK.4
  br label %for.cond.i

for.cond.i:                                       ; preds = %for.body.i, %DIR.OMP.TASK.5
  %e.0.i = phi i32 [ %spec.select.i, %for.body.i ], [ undef, %DIR.OMP.TASK.5 ]
  %i.0.i = phi i32 [ %inc.i, %for.body.i ], [ 0, %DIR.OMP.TASK.5 ]
  %11 = load i32, ptr @a, align 4
  %cmp.i = icmp slt i32 %i.0.i, %11
  br i1 %cmp.i, label %for.body.i, label %DIR.OMP.END.TASK.6.split

for.body.i:                                       ; preds = %for.cond.i
  %tobool.i = icmp eq i32 %e.0.i, 0
  %12 = load i32, ptr @c, align 4
  %spec.select.i = select i1 %tobool.i, i32 0, i32 %12
  %inc.i = add nuw nsw i32 %i.0.i, 1
  br label %for.cond.i

DIR.OMP.END.TASK.6.split:                         ; preds = %for.cond.i
  br label %DIR.OMP.END.TASK.6

DIR.OMP.END.TASK.6:                               ; preds = %DIR.OMP.END.TASK.6.split
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.TASK"() ]
  br label %DIR.OMP.END.TASK.7

DIR.OMP.END.TASK.7:                               ; preds = %DIR.OMP.END.TASK.6
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %8) #2
  %13 = load volatile i32, ptr %.omp.iv, align 4
  %add6 = add nsw i32 %13, 1
  store volatile i32 %add6, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.loop.exit.split:                              ; preds = %omp.inner.for.cond
  br label %DIR.OMP.END.LOOP.8

DIR.OMP.END.LOOP.8:                               ; preds = %omp.loop.exit.split
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.LOOP.8, %entry
  %14 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %14) #2
  %15 = bitcast ptr %.omp.lb to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %15) #2
  %16 = bitcast ptr %.omp.iv to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %16) #2
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #3

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #3

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { argmemonly nocallback nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
