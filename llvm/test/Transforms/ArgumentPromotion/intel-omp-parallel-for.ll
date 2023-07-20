; RUN: opt -S -passes=argpromotion %s | FileCheck %s
;
;    void bar(int, float, double);
;
;    void foo(int N) {
;      float p = 3;
;      double q = 5;
;      N = 7;
;
;    #pragma omp parallel for firstprivate(q)
;      for (int i = 2; i < N; i++) {
;        bar(i, p, q);
;      }
;    }
;
; Verify that argument promotion is happening for the outlined function when using
; opaque pointers.
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%__struct.ident_t = type { i32, i32, i32, i32, ptr }

@.source.0.0 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private unnamed_addr global %__struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr getelementptr inbounds ([22 x i8], ptr @.source.0.0, i32 0, i32 0) }
@.source.0.0.1 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.2 = private unnamed_addr global %__struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr getelementptr inbounds ([22 x i8], ptr @.source.0.0.1, i32 0, i32 0) }
@.source.0.0.3 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.4 = private unnamed_addr global %__struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr getelementptr inbounds ([22 x i8], ptr @.source.0.0.3, i32 0, i32 0) }
@.source.0.0.5 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.6 = private unnamed_addr global %__struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr getelementptr inbounds ([22 x i8], ptr @.source.0.0.5, i32 0, i32 0) }
@.source.0.0.7 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.8 = private unnamed_addr global %__struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr getelementptr inbounds ([22 x i8], ptr @.source.0.0.7, i32 0, i32 0) }

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo(i32 %N) local_unnamed_addr {
entry:
  %0 = alloca i32, align 4
  store i32 0, ptr %0, align 4
  %tid.val = tail call i32 @__kmpc_global_thread_num(ptr nonnull @.kmpc_loc.0.0.8)
  %1 = alloca i32, align 4
  store i32 %tid.val, ptr %1, align 4
  %p = alloca float, align 4
  %q = alloca double, align 8
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store float 3.000000e+00, ptr %p, align 4
  store double 5.000000e+00, ptr %q, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 4, ptr %.omp.ub, align 4
  %fork.test = tail call i32 @__kmpc_ok_to_fork(ptr nonnull @.kmpc_loc.0.0.6)
  %fork.test20 = icmp eq i32 %fork.test, 0
  br i1 %fork.test20, label %if.else.call.4, label %if.then.fork.4

if.then.fork.4:                                   ; preds = %entry
  call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr nonnull @.kmpc_loc.0.0.4, i32 4, ptr @foo.DIR.OMP.PARALLEL.LOOP.2.split17, ptr nonnull %p, ptr nonnull %.omp.lb, ptr nonnull %q, ptr nonnull %.omp.ub)
  br label %DIR.OMP.END.PARALLEL.LOOP.5

if.else.call.4:                                   ; preds = %entry
  call void @foo.DIR.OMP.PARALLEL.LOOP.2.split17(ptr nonnull %1, ptr nonnull %0, ptr nonnull %p, ptr nonnull %.omp.lb, ptr nonnull %q, ptr nonnull %.omp.ub)
  br label %DIR.OMP.END.PARALLEL.LOOP.5

DIR.OMP.END.PARALLEL.LOOP.5:                      ; preds = %if.else.call.4, %if.then.fork.4
  ret void
}

declare dso_local void @bar(i32, float, double) local_unnamed_addr

declare void @__kmpc_for_static_init_4(ptr, i32, i32, ptr, ptr, ptr, ptr, i32, i32) local_unnamed_addr

declare void @__kmpc_for_static_fini(ptr, i32) local_unnamed_addr

; Function Attrs: noinline nounwind uwtable
define internal void @foo.DIR.OMP.PARALLEL.LOOP.2.split17(ptr nocapture readonly %tid, ptr nocapture readnone %bid, ptr nocapture readonly %p, ptr nocapture readonly %.omp.lb, ptr nocapture readonly %q, ptr nocapture readonly %.omp.ub) {
; CHECK: define internal void @foo.DIR.OMP.PARALLEL.LOOP.2.split17(ptr nocapture readonly %tid, ptr nocapture readnone %bid, ptr nocapture readonly %p, i64 [[LBVAL:%.*]], i64 [[QVAL:%.*]], i64 [[UBVAL:%.*]])
; CHECK:   %0 = trunc i64 [[UBVAL]] to i32
; CHECK:   %1 = bitcast i64 [[QVAL]] to double
; CHECK:   %2 = trunc i64 [[LBVAL]] to i32
newFuncRoot:
  %is.last = alloca i32, align 4
  %lower.bnd = alloca i32, align 4
  %upper.bnd = alloca i32, align 4
  %stride = alloca i32, align 4
  store i32 0, ptr %is.last, align 4
  %0 = load i32, ptr %.omp.lb, align 4
  %1 = load double, ptr %q, align 8
  %2 = load i32, ptr %.omp.ub, align 4
  %cmp415 = icmp sgt i32 %0, %2
  br i1 %cmp415, label %DIR.OMP.END.PARALLEL.LOOP.4, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %newFuncRoot
  %my.tid = load i32, ptr %tid, align 4
  store i32 %0, ptr %lower.bnd, align 4
  store i32 %2, ptr %upper.bnd, align 4
  store i32 1, ptr %stride, align 4
  call void @__kmpc_for_static_init_4(ptr nonnull @.kmpc_loc.0.0, i32 %my.tid, i32 34, ptr nonnull %is.last, ptr nonnull %lower.bnd, ptr nonnull %upper.bnd, ptr nonnull %stride, i32 1, i32 1)
  %lb.new = load i32, ptr %lower.bnd, align 4
  %ub.new = load i32, ptr %upper.bnd, align 4
  %omp.ztt = icmp sgt i32 %lb.new, %ub.new
  br i1 %omp.ztt, label %loop.region.exit, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body.lr.ph, %omp.inner.for.body
  %.omp.iv.local.016 = phi i32 [ %add6, %omp.inner.for.body ], [ %lb.new, %omp.inner.for.body.lr.ph ]
  %add5 = add nsw i32 %.omp.iv.local.016, 2
  %3 = load float, ptr %p, align 4
  call void @bar(i32 %add5, float %3, double %1)
  %add6 = add nsw i32 %.omp.iv.local.016, 1
  %cmp4 = icmp slt i32 %.omp.iv.local.016, %ub.new
  br i1 %cmp4, label %omp.inner.for.body, label %loop.region.exit

loop.region.exit:                                 ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  call void @__kmpc_for_static_fini(ptr nonnull @.kmpc_loc.0.0.2, i32 %my.tid)
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:                      ; preds = %newFuncRoot, %loop.region.exit
  ret void
}

declare !callback !0 void @__kmpc_fork_call(ptr, i32, ptr, ...) local_unnamed_addr

declare i32 @__kmpc_ok_to_fork(ptr) local_unnamed_addr

declare i32 @__kmpc_global_thread_num(ptr) local_unnamed_addr

!0 = !{!1}
!1 = !{i64 2, i64 -1, i64 -1, i1 true}
