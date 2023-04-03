; RUN: opt -S < %s -passes='vplan-vec' -vplan-enable-nested-simd 2>&1 | FileCheck %s
; RUN: opt -S < %s -passes=vplan-vec,intel-ir-optreport-emitter -vplan-enable-nested-simd -disable-output -intel-opt-report=medium 2>&1 | FileCheck %s --check-prefix=OPTRPTMED
; RUN: opt -S < %s -passes=vplan-vec,intel-ir-optreport-emitter -vplan-enable-nested-simd -disable-output -intel-opt-report=high 2>&1 | FileCheck %s --check-prefix=OPTRPTHI

; OPTRPTMED: remark #15436: loop was not vectorized:
; OPTRPTHI: remark #15436: loop was not vectorized:
; OPTRPTHI: remark #15436: loop was not vectorized: An illegal OpenMP construct was found inside this SIMD loop.

%struct.ident_t = type { i32, i32, i32, i32, ptr }

@.gomp_critical_user_.AS0.var = common global [8 x i32] zeroinitializer
@.source.0.0 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr @.source.0.0 }
@.source.0.0.1 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.2 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr @.source.0.0.1 }
@.source.0.0.3 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.4 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, ptr @.source.0.0.3 }

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @critical_in_simd(ptr nocapture noundef writeonly %a, ptr nocapture noundef readonly %b) local_unnamed_addr #0 {
; CHECK-LABEL: void @critical_in_simd
; CHECK-NOT: <4 x
DIR.OMP.SIMD.27:
  %tid.val = tail call i32 @__kmpc_global_thread_num(ptr nonnull @.kmpc_loc.0.0.4)
  br label %omp.inner.for.body.lr.ph
omp.inner.for.body.lr.ph:                         ; preds = %DIR.OMP.SIMD.27
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0) ]
  br label %omp.inner.for.body
omp.inner.for.body:                               ; preds = %omp.inner.for.body.lr.ph, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %omp.inner.for.body.lr.ph ], [ %indvars.iv.next, %omp.inner.for.body ]
  call void @__kmpc_critical(ptr nonnull @.kmpc_loc.0.0, i32 %tid.val, ptr nonnull @.gomp_critical_user_.AS0.var) #2
  fence acquire
  %1 = shl nsw i64 %indvars.iv, 2
  %arrayidx = getelementptr inbounds i8, ptr %b, i64 %1
  %2 = load i8, ptr %arrayidx, align 1
  %arrayidx3 = getelementptr inbounds i8, ptr %a, i64 %indvars.iv
  store i8 %2, ptr %arrayidx3, align 1
  fence release
  call void @__kmpc_end_critical(ptr nonnull @.kmpc_loc.0.0.2, i32 %tid.val, ptr nonnull @.gomp_critical_user_.AS0.var) #2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge, label %omp.inner.for.body
omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge: ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.1
DIR.OMP.END.SIMD.1:                               ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge
  ret void
}


define dso_local void @ordered_in_simd(ptr nocapture noundef writeonly %a, ptr nocapture noundef readonly %b) local_unnamed_addr #0 {
; CHECK-LABEL: void @ordered_in_simd
; CHECK-NOT: <4 x
DIR.OMP.SIMD.27:
  br label %omp.inner.for.body.lr.ph
omp.inner.for.body.lr.ph:                         ; preds = %DIR.OMP.SIMD.27
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0) ]
  br label %omp.inner.for.body
omp.inner.for.body:                               ; preds = %omp.inner.for.body.lr.ph, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %omp.inner.for.body.lr.ph ], [ %indvars.iv.next, %omp.inner.for.body.afterOrd ]
  br label %beginOrdered
beginOrdered:
  %ordTok = call token @llvm.directive.region.entry() ["DIR.OMP.ORDERED"()]
  br label %forBody
forBody:
  %1 = shl nsw i64 %indvars.iv, 2
  %arrayidx = getelementptr inbounds i8, ptr %b, i64 %1
  %2 = load i8, ptr %arrayidx, align 1
  %arrayidx3 = getelementptr inbounds i8, ptr %a, i64 %indvars.iv
  store i8 %2, ptr %arrayidx3, align 1
  br label %endOrdered
endOrdered:
  call void @llvm.directive.region.exit(token %ordTok) [ "DIR.OMP.END.ORDERED"() ]
  br label %omp.inner.for.body.afterOrd
omp.inner.for.body.afterOrd:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge, label %omp.inner.for.body
omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge: ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.1
DIR.OMP.END.SIMD.1:                               ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2
; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2
; Function Attrs: convergent nounwind
declare void @__kmpc_critical(ptr, i32, ptr) local_unnamed_addr #3
; Function Attrs: convergent nounwind
declare void @__kmpc_end_critical(ptr, i32, ptr) local_unnamed_addr #3
; Function Attrs: nounwind
declare i32 @__kmpc_global_thread_num(ptr nocapture readonly) local_unnamed_addr #2
