; RUN: opt -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="vpo-paropt" -S %s | FileCheck %s

; This test checks if the dispatch header takes care of loop carried dependence of temporaries.
; The temporary here is defined in the loop body, and used in a PHI in the loop header with an
; incoming edge from the Loop Latch and another from Loop preheader. (PHI-%LOCK_COUNT.0 and def-%add5)
;
;Cpp Source Code
;#include "omp.h"
;#include <stdio.h>
;
;void Subr(omp_lock_t &LCK) {
;  int i, LOCK_COUNT = 0;
;  int MAX_ITER = 10;
;#pragma omp for nowait schedule(static, 3)
;  for (i = 1; i <= MAX_ITER; i++) {
;    LOCK_COUNT = LOCK_COUNT + 1;
;    if (LOCK_COUNT == 4) {
;      printf("Thread number: %d Before lock; I= %d \n", omp_get_thread_num(), i);
;      omp_set_lock(&LCK);
;      printf("Thread number: %d After lock; I= %d \n", omp_get_thread_num(), i);
;    }
;  }
;  printf("Thread number: %d Free Lock\n", omp_get_thread_num());
;  omp_unset_lock(&LCK);
;}
;
;int main() {
;  omp_lock_t LCK;
;  omp_set_dynamic(0);
;  omp_init_lock(&LCK);
;  omp_set_num_threads(2);
;#pragma omp parallel
;  {
;    Subr(LCK);
;  }
;  omp_destroy_lock(&LCK);
;  printf("Test Passed\n");
;}
;

target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, i8* }
%struct.omp_lock_t = type { i8* }

@.str = private unnamed_addr constant [39 x i8] c"Thread number: %d Before lock; I= %d \0A\00", align 1
@.str.1 = private unnamed_addr constant [38 x i8] c"Thread number: %d After lock; I= %d \0A\00", align 1
@.str.2 = private unnamed_addr constant [29 x i8] c"Thread number: %d Free Lock\0A\00", align 1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nofree nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #3

; Function Attrs: nofree nounwind readonly
declare dso_local i32 @omp_get_thread_num() local_unnamed_addr #4

; Function Attrs: nofree nounwind
declare dso_local void @omp_set_lock(%struct.omp_lock_t*) local_unnamed_addr #3

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nofree nounwind
declare dso_local void @omp_unset_lock(%struct.omp_lock_t*) local_unnamed_addr #3

define dso_local void @_Z4SubrR10omp_lock_t(%struct.omp_lock_t* dereferenceable(8) %LCK) local_unnamed_addr #0 {
entry:
  %i = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  %1 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #2
  %2 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !2
  %3 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3) #2
  store volatile i32 9, i32* %.omp.ub, align 4, !tbaa !2
  br label %DIR.OMP.LOOP.1

DIR.OMP.LOOP.1:                                   ; preds = %entry
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.NOWAIT"(), "QUAL.OMP.SCHEDULE.STATIC"(i32 3), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]
  br label %DIR.OMP.LOOP.223

DIR.OMP.LOOP.223:                                 ; preds = %DIR.OMP.LOOP.1
  br label %DIR.OMP.LOOP.2

DIR.OMP.LOOP.2:                                   ; preds = %DIR.OMP.LOOP.223
  %5 = load i32, i32* %.omp.lb, align 4, !tbaa !2
  store volatile i32 %5, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.LOOP.2
  %LOCK_COUNT.0 = phi i32 [ 0, %DIR.OMP.LOOP.2 ], [ %add5, %omp.inner.for.inc ]
  %6 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  %7 = load volatile i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp3 = icmp sgt i32 %6, %7
  br i1 %cmp3, label %omp.loop.exit.split, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  %add4 = add nsw i32 %8, 1
  store i32 %add4, i32* %i, align 4, !tbaa !2
  %add5 = add nuw nsw i32 %LOCK_COUNT.0, 1
  %cmp6 = icmp eq i32 %add5, 4
  br i1 %cmp6, label %if.then, label %omp.inner.for.inc

if.then:                                          ; preds = %omp.inner.for.body
  %call = call i32 @omp_get_thread_num() #2
  %9 = load i32, i32* %i, align 4, !tbaa !2
  %call7 = call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([39 x i8], [39 x i8]* @.str, i64 0, i64 0), i32 %call, i32 %9) #2
  call void @omp_set_lock(%struct.omp_lock_t* nonnull %LCK) #2
  %call8 = call i32 @omp_get_thread_num() #2
  %10 = load i32, i32* %i, align 4, !tbaa !2
  %call9 = call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([38 x i8], [38 x i8]* @.str.1, i64 0, i64 0), i32 %call8, i32 %10) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.then, %omp.inner.for.body
  %11 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  %add10 = add nsw i32 %11, 1
  store volatile i32 %add10, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.loop.exit.split:                              ; preds = %omp.inner.for.cond
  br label %DIR.OMP.END.LOOP.3

DIR.OMP.END.LOOP.3:                               ; preds = %omp.loop.exit.split
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.LOOP.4

DIR.OMP.END.LOOP.4:                               ; preds = %DIR.OMP.END.LOOP.3
  %12 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %12) #2
  %13 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %13) #2
  %14 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %14) #2
  %call11 = call i32 @omp_get_thread_num() #2
  %call12 = call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([29 x i8], [29 x i8]* @.str.2, i64 0, i64 0), i32 %call11)
  call void @omp_unset_lock(%struct.omp_lock_t* nonnull %LCK)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #2
  ret void
}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = distinct !{!6, !7}
!7 = distinct !{!"intel.optreport.rootnode", !8}
!8 = distinct !{!"intel.optreport", !9}
!9 = !{!"intel.optreport.remarks", !10}
!10 = !{!"intel.optreport.remark", !"OpenMP: Worksharing loop"}

; Outlined Function for parallelized loop
; CHECK-LABEL: @_Z4SubrR10omp_lock_t(
; CHECK: dispatch.header:
; CHECK-NEXT: [[ADDED_PHI:%.*]] = phi i32 [ [[DEF:%.*]], %dispatch.inc ], [ 0, %omp.inner.for.body.lr.ph ]
;
; CHECK: [[LOOP_PREHEADER:omp.inner.for.body]]:
; CHECK-NEXT: [[ORIGINAL_PHI:%.*]] = phi i32 [ [[ADDED_PHI]], %dispatch.body ], [ [[DEF]], %[[LOOP_LATCH:omp.inner.for.inc]] ]
; CHECK: [[DEF]] = add nuw nsw i32 [[ORIGINAL_PHI]], 1
