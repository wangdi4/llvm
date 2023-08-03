; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="vpo-paropt" -S %s | FileCheck %s

; This test checks if the dispatch header takes care of loop carried dependence of temporaries.
; The temporary here is a PHi with constant value input from Loop Header and Loop Latch.

;Cpp Source Code:
;void enclosed(int thread) {
;  int i;
;  int flag = 0;
;
;#pragma omp for schedule(static, 2)
;  for (i = 0; i < 6; i++) {
;    if (flag == 0) {
;      flag = 1;
;      printf("thread=%d flag=%d i=%d\n", thread, flag, i);
;    }
;  }
;}
;int main() {
;#pragma omp parallel num_threads(2)
;  {
;    int thread = omp_get_thread_num();
;    enclosed(thread);
;  };
;  puts("*** TEST PASSED ***");
;  exit(EXIT_SUCCESS);
;}

; Outlined Function for parallelized loop
; CHECK-LABEL: define internal void @main.DIR.OMP.PARALLEL.2(
; CHECK: dispatch.header:
; CHECK-NEXT: [[ADDED_PHI:%.*]] = phi i32 [ [[ADDED_PHI0:%.*]], %dispatch.inc ], [ 0, %omp.inner.for.body.i.lr.ph ]
;
; CHECK: [[LOOP_PREHEADER:omp.inner.for.body.i]]:
; CHECK-NEXT: [[ORIGINAL_PHI:%.*]] = phi i32 [ [[ADDED_PHI]], %dispatch.body ], [ [[ADDED_PHI0]], %[[LOOP_LATCH:omp.inner.for.inc.i]] ]
;
; CHECK: [[LOOP_LATCH]]:
; CHECK-NEXT:  [[ADDED_PHI0]] = phi i32 [ 1, %if.then.i ], [ 1, %[[LOOP_PREHEADER]] ]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, ptr }

@.str = private unnamed_addr constant [24 x i8] c"thread=%d flag=%d i=%d\0A\00", align 1
@.str.1 = private unnamed_addr constant [20 x i8] c"*** TEST PASSED ***\00", align 1

define dso_local i32 @main() local_unnamed_addr {
entry:
  %thread = alloca i32, align 4
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 2),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %thread, i32 0, i64 1) ]
  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.2
  call void @llvm.lifetime.start.p0(i64 4, ptr %thread)
  %call = call i32 @omp_get_thread_num()
  store i32 %call, ptr %thread, align 4, !tbaa !2
  %i.i = alloca i32, align 4
  %.omp.iv.i = alloca i32, align 4
  %.omp.lb.i = alloca i32, align 4
  %.omp.ub.i = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i.i)
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.iv.i)
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.lb.i)
  store i32 0, ptr %.omp.lb.i, align 4, !tbaa !2
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.ub.i)
  store volatile i32 5, ptr %.omp.ub.i, align 4, !tbaa !2
  br label %DIR.OMP.LOOP.4

DIR.OMP.LOOP.4:                                   ; preds = %DIR.OMP.PARALLEL.3
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.SCHEDULE.STATIC"(i32 2),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb.i, i32 0, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv.i, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub.i, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i.i, i32 0, i64 1) ]
  br label %DIR.OMP.LOOP.2.i

DIR.OMP.LOOP.2.i:                                 ; preds = %DIR.OMP.LOOP.4
  %2 = load i32, ptr %.omp.lb.i, align 4, !tbaa !2
  store volatile i32 %2, ptr %.omp.iv.i, align 4, !tbaa !2
  br label %omp.inner.for.cond.i

omp.inner.for.cond.i:                             ; preds = %omp.inner.for.inc.i, %DIR.OMP.LOOP.2.i
  %flag.0.i = phi i32 [ 0, %DIR.OMP.LOOP.2.i ], [ 1, %omp.inner.for.inc.i ]
  %3 = load volatile i32, ptr %.omp.iv.i, align 4, !tbaa !2
  %4 = load volatile i32, ptr %.omp.ub.i, align 4, !tbaa !2
  %cmp.i = icmp sgt i32 %3, %4
  br i1 %cmp.i, label %enclosed.exit, label %omp.inner.for.body.i

omp.inner.for.body.i:                             ; preds = %omp.inner.for.cond.i
  %5 = load volatile i32, ptr %.omp.iv.i, align 4, !tbaa !2
  store i32 %5, ptr %i.i, align 4, !tbaa !2
  %cmp1.i = icmp eq i32 %flag.0.i, 0
  br i1 %cmp1.i, label %if.then.i, label %omp.inner.for.inc.i

if.then.i:                                        ; preds = %omp.inner.for.body.i
  %call.i = call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @.str, i32 %call, i32 1, i32 %5)
  br label %omp.inner.for.inc.i

omp.inner.for.inc.i:                              ; preds = %if.then.i, %omp.inner.for.body.i
  %6 = load volatile i32, ptr %.omp.iv.i, align 4, !tbaa !2
  %add2.i = add nsw i32 %6, 1
  store volatile i32 %add2.i, ptr %.omp.iv.i, align 4, !tbaa !2
  br label %omp.inner.for.cond.i

enclosed.exit:                                    ; preds = %omp.inner.for.cond.i
  br label %DIR.OMP.END.LOOP.6

DIR.OMP.END.LOOP.6:                               ; preds = %enclosed.exit
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.LOOP.7

DIR.OMP.END.LOOP.7:                               ; preds = %DIR.OMP.END.LOOP.6
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.ub.i)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.lb.i)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.iv.i)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i.i)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %thread)
  br label %DIR.OMP.END.PARALLEL.8

DIR.OMP.END.PARALLEL.8:                           ; preds = %DIR.OMP.END.LOOP.7
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.9

DIR.OMP.END.PARALLEL.9:                           ; preds = %DIR.OMP.END.PARALLEL.8
  %call1 = call i32 @puts(ptr nonnull dereferenceable(1) @.str.1)
  call void @exit(i32 0)
  unreachable
}

declare dso_local i32 @omp_get_thread_num() local_unnamed_addr
declare dso_local i32 @puts(ptr nocapture readonly) local_unnamed_addr
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)
declare dso_local void @exit(i32)

!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
