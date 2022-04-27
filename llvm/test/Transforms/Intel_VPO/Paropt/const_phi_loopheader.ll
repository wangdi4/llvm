; RUN: opt -vpo-paropt -S %s | FileCheck %s
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

; ModuleID = 't2.c'
source_filename = "t2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, i8* }

@.str = private unnamed_addr constant [24 x i8] c"thread=%d flag=%d i=%d\0A\00", align 1
@.str.1 = private unnamed_addr constant [20 x i8] c"*** TEST PASSED ***\00", align 1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nofree nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #3

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: noreturn nounwind
declare dso_local void @exit(i32) #5

; Function Attrs: noreturn nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #4 {

entry:
  %thread = alloca i32, align 4
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.NUM_THREADS"(i32 2), "QUAL.OMP.PRIVATE"(i32* %thread) ]
  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.2
  %1 = bitcast i32* %thread to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  %call = call i32 @omp_get_thread_num() #2
  store i32 %call, i32* %thread, align 4, !tbaa !2
  %i.i = alloca i32, align 4
  %.omp.iv.i = alloca i32, align 4
  %.omp.lb.i = alloca i32, align 4
  %.omp.ub.i = alloca i32, align 4
  %2 = bitcast i32* %i.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #2
  %3 = bitcast i32* %.omp.iv.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3) #2
  %4 = bitcast i32* %.omp.lb.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %4) #2
  store i32 0, i32* %.omp.lb.i, align 4, !tbaa !2
  %5 = bitcast i32* %.omp.ub.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %5) #2
  store volatile i32 5, i32* %.omp.ub.i, align 4, !tbaa !2
  br label %DIR.OMP.LOOP.4

DIR.OMP.LOOP.4:                                   ; preds = %DIR.OMP.PARALLEL.3
  %6 = call token @llvm.directive.region.entry() #2 [ "DIR.OMP.LOOP"(), "QUAL.OMP.SCHEDULE.STATIC"(i32 2), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb.i), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv.i), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub.i), "QUAL.OMP.PRIVATE"(i32* %i.i) ]
  br label %DIR.OMP.LOOP.2.i

DIR.OMP.LOOP.2.i:                                 ; preds = %DIR.OMP.LOOP.4
  %7 = load i32, i32* %.omp.lb.i, align 4, !tbaa !2
  store volatile i32 %7, i32* %.omp.iv.i, align 4, !tbaa !2
  br label %omp.inner.for.cond.i

omp.inner.for.cond.i:                             ; preds = %omp.inner.for.inc.i, %DIR.OMP.LOOP.2.i
  %flag.0.i = phi i32 [ 0, %DIR.OMP.LOOP.2.i ], [ 1, %omp.inner.for.inc.i ]
  %8 = load volatile i32, i32* %.omp.iv.i, align 4, !tbaa !2
  %9 = load volatile i32, i32* %.omp.ub.i, align 4, !tbaa !2
  %cmp.i = icmp sgt i32 %8, %9
  br i1 %cmp.i, label %enclosed.exit, label %omp.inner.for.body.i

omp.inner.for.body.i:                             ; preds = %omp.inner.for.cond.i
  %10 = load volatile i32, i32* %.omp.iv.i, align 4, !tbaa !2
  store i32 %10, i32* %i.i, align 4, !tbaa !2
  %cmp1.i = icmp eq i32 %flag.0.i, 0
  br i1 %cmp1.i, label %if.then.i, label %omp.inner.for.inc.i

if.then.i:                                        ; preds = %omp.inner.for.body.i
  %call.i = call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i64 0, i64 0), i32 %call, i32 1, i32 %10) #2
  br label %omp.inner.for.inc.i

omp.inner.for.inc.i:                              ; preds = %if.then.i, %omp.inner.for.body.i
  %11 = load volatile i32, i32* %.omp.iv.i, align 4, !tbaa !2
  %add2.i = add nsw i32 %11, 1
  store volatile i32 %add2.i, i32* %.omp.iv.i, align 4, !tbaa !2
  br label %omp.inner.for.cond.i

enclosed.exit:                                    ; preds = %omp.inner.for.cond.i
  br label %DIR.OMP.END.LOOP.6

DIR.OMP.END.LOOP.6:                               ; preds = %enclosed.exit
  call void @llvm.directive.region.exit(token %6) #2 [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.LOOP.7

DIR.OMP.END.LOOP.7:                               ; preds = %DIR.OMP.END.LOOP.6
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %5) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %4) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %3) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #2
  br label %DIR.OMP.END.PARALLEL.8

DIR.OMP.END.PARALLEL.8:                           ; preds = %DIR.OMP.END.LOOP.7
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.9

DIR.OMP.END.PARALLEL.9:                           ; preds = %DIR.OMP.END.PARALLEL.8
  %call1 = call i32 @puts(i8* nonnull dereferenceable(1) getelementptr inbounds ([20 x i8], [20 x i8]* @.str.1, i64 0, i64 0))
  call void @exit(i32 0) #7
  unreachable
}

; Function Attrs: nofree nounwind readonly
declare dso_local i32 @omp_get_thread_num() local_unnamed_addr #5

; Function Attrs: nofree nounwind
declare dso_local i32 @puts(i8* nocapture readonly) local_unnamed_addr #3

!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7}
!7 = distinct !{!"intel.optreport.rootnode", !8}
!8 = distinct !{!"intel.optreport", !9}
!9 = !{!"intel.optreport.remarks", !10}
!10 = !{!"intel.optreport.remark", !"OpenMP: Worksharing loop"}

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

