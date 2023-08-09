; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="vpo-paropt" -S %s | FileCheck %s

; This test checks if the dispatch header takes care of loop carried dependence of temporaries.
; The temporary here is defined in the loop body, and used in a PHI in the loop header with an
; incoming edge from the Loop Latch and another from Loop preheader. (PHI-%LOCK_COUNT.0 and def-%add3)

; Test src:
;
; #include "omp.h"
; #include <stdio.h>
;
; void Subr(omp_lock_t &LCK) {
;   int i, LOCK_COUNT = 0;
;   int MAX_ITER = 10;
; #pragma omp for nowait schedule(static, 3)
;   for (i = 1; i <= MAX_ITER; i++) {
;     LOCK_COUNT = LOCK_COUNT + 1;
;     if (LOCK_COUNT == 4) {
;       printf("Thread number: %d Before lock; I= %d \n", omp_get_thread_num(),
;              i);
;       omp_set_lock(&LCK);
;       printf("Thread number: %d After lock; I= %d \n", omp_get_thread_num(), i);
;     }
;   }
;   printf("Thread number: %d Free Lock\n", omp_get_thread_num());
;   omp_unset_lock(&LCK);
; }
;
; int main() {
;   omp_lock_t LCK;
;   omp_set_dynamic(0);
;   omp_init_lock(&LCK);
;   omp_set_num_threads(2);
; #pragma omp parallel
;   { Subr(LCK); }
;   omp_destroy_lock(&LCK);
;   printf("Test Passed\n");
; }

; Outlined Function for parallelized loop
; CHECK-LABEL: @_Z4SubrR10omp_lock_t(
; CHECK: dispatch.header:
; CHECK-NEXT: [[ADDED_PHI:%.*]] = phi i32 [ [[DEF:%.*]], %dispatch.inc ], [ 0, %omp.inner.for.body.lr.ph ]
;
; CHECK: [[LOOP_PREHEADER:omp.inner.for.body]]:
; CHECK-NEXT: [[ORIGINAL_PHI:%.*]] = phi i32 [ [[ADDED_PHI]], %dispatch.body ], [ [[DEF]], %[[LOOP_LATCH:omp.inner.for.inc]] ]
; CHECK: [[DEF]] = add nuw nsw i32 [[ORIGINAL_PHI]], 1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.omp_lock_t = type { ptr }

@.str = private unnamed_addr constant [39 x i8] c"Thread number: %d Before lock; I= %d \0A\00", align 1
@.str.1 = private unnamed_addr constant [38 x i8] c"Thread number: %d After lock; I= %d \0A\00", align 1
@.str.2 = private unnamed_addr constant [29 x i8] c"Thread number: %d Free Lock\0A\00", align 1

; Function Attrs: mustprogress uwtable
define dso_local void @_Z4SubrR10omp_lock_t(ptr noundef nonnull align 8 dereferenceable(8) %LCK) local_unnamed_addr #0 {
DIR.OMP.LOOP.217:
  %i = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.lb) #2
  store i32 0, ptr %.omp.lb, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.ub) #2
  store volatile i32 9, ptr %.omp.ub, align 4, !tbaa !4
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.LOOP.1

DIR.OMP.LOOP.1:                                   ; preds = %DIR.OMP.LOOP.217
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.NOWAIT"(),
    "QUAL.OMP.SCHEDULE.STATIC"(i32 3),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]
  br label %DIR.OMP.LOOP.218

DIR.OMP.LOOP.218:                                 ; preds = %DIR.OMP.LOOP.1
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %omp.precond.end, label %DIR.OMP.LOOP.2

DIR.OMP.LOOP.2:                                   ; preds = %DIR.OMP.LOOP.218
  %1 = load i32, ptr %.omp.lb, align 4, !tbaa !4
  store volatile i32 %1, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.LOOP.2
  %LOCK_COUNT.0 = phi i32 [ 0, %DIR.OMP.LOOP.2 ], [ %add3, %omp.inner.for.inc ]
  %2 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !4
  %3 = load volatile i32, ptr %.omp.ub, align 4, !tbaa !4
  %cmp2.not = icmp sgt i32 %2, %3
  br i1 %cmp2.not, label %omp.precond.end.loopexit, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !4
  %add = add nsw i32 %4, 1
  store i32 %add, ptr %i, align 4, !tbaa !4
  %add3 = add nuw nsw i32 %LOCK_COUNT.0, 1
  %cmp4 = icmp eq i32 %add3, 4
  br i1 %cmp4, label %if.then, label %omp.inner.for.inc

if.then:                                          ; preds = %omp.inner.for.body
  %call = call i32 @omp_get_thread_num() #2
  %call5 = call i32 (ptr, ...) @printf(ptr noundef nonnull @.str, i32 noundef %call, i32 noundef %add) #2
  call void @omp_set_lock(ptr noundef nonnull %LCK) #2
  %call6 = call i32 @omp_get_thread_num() #2
  %5 = load i32, ptr %i, align 4, !tbaa !4
  %call7 = call i32 (ptr, ...) @printf(ptr noundef nonnull @.str.1, i32 noundef %call6, i32 noundef %5) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.then, %omp.inner.for.body
  %6 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !4
  %add8 = add nsw i32 %6, 1
  store volatile i32 %add8, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.precond.end.loopexit:                         ; preds = %omp.inner.for.cond
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.precond.end.loopexit, %DIR.OMP.LOOP.218
  br label %DIR.OMP.END.LOOP.3

DIR.OMP.END.LOOP.3:                               ; preds = %omp.precond.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.LOOP.4

DIR.OMP.END.LOOP.4:                               ; preds = %DIR.OMP.END.LOOP.3
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.ub) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.lb) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.iv) #2
  %call9 = call i32 @omp_get_thread_num() #2
  %call10 = call i32 (ptr, ...) @printf(ptr noundef nonnull @.str.2, i32 noundef %call9)
  call void @omp_unset_lock(ptr noundef nonnull %LCK)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i) #2
  ret void
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #3

; Function Attrs: nofree nounwind readonly
declare dso_local i32 @omp_get_thread_num() local_unnamed_addr #4

; Function Attrs: nofree nounwind
declare dso_local void @omp_set_lock(ptr noundef) local_unnamed_addr #3

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind
declare dso_local void @omp_unset_lock(ptr noundef) local_unnamed_addr #3

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
