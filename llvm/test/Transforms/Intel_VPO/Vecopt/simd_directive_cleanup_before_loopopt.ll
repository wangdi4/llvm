; Verify that SIMD directives are cleaned up before loop-opt passes even if VPlan fails to vectorize the SIMD loop.
; HIR VPlan now works before LLVM VPlan by default so we need -pre-loopopt-vpo-passes option to make this test work

; RUN: opt -O3 -enable-new-pm=0 -pre-loopopt-vpo-passes -print-after=vplan-vec -print-before=hir-ssa-deconstruction -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes='default<O3>' -pre-loopopt-vpo-passes -print-after=hir-vplan-vec -print-after=vplan-vec -print-after=vpo-directive-cleanup -disable-output < %s 2>&1 | FileCheck %s

; Check that there is no HIR VPlan pass triggered in new pass manager run case
; CHECK-NOT: IR Dump After vpo::VPlanDriverHIRPass

; Check that VPlan didn't vectorize loop (SIMD directive will be present)
; CHECK-LABEL: IR Dump After {{VPlan Vectorizer|vpo::VPlanDriverPass on main}}
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]

; CHECK-NOT: IR Dump After vpo::VPlanDriverHIRPass

; Check that SIMD driectives are removed before loop-opt
; CHECK-LABEL: IR Dump {{Before HIR SSA Deconstruction|After VPODirectiveCleanupPass on main}}
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]

; CHECK-NOT: IR Dump After vpo::VPlanDriverHIRPass

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x double] zeroinitializer, align 16
@.source.0.0 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private unnamed_addr global { i32, i32, i32, i32, i8* } { i32 0, i32 2, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0, i32 0, i32 0) }
@.source.0.0.1 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.2 = private unnamed_addr global { i32, i32, i32, i32, i8* } { i32 0, i32 2, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.1, i32 0, i32 0) }
@.source.0.0.3 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.4 = private unnamed_addr global { i32, i32, i32, i32, i8* } { i32 0, i32 66, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.3, i32 0, i32 0) }
@.source.0.0.5 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.6 = private unnamed_addr global { i32, i32, i32, i32, i8* } { i32 0, i32 2, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.5, i32 0, i32 0) }

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr "loopopt-pipeline"="full" {
omp.inner.for.body.lr.ph:
  %tid.val = tail call i32 @__kmpc_global_thread_num({ i32, i32, i32, i32, i8* }* nonnull @.kmpc_loc.0.0.6) #1
  %is.last = alloca i32, align 4
  store i32 0, i32* %is.last, align 4
  %lower.bnd = alloca i32, align 4
  %upper.bnd = alloca i32, align 4
  %stride = alloca i32, align 4
  store i32 0, i32* %lower.bnd, align 4
  store i32 99, i32* %upper.bnd, align 4
  store i32 1, i32* %stride, align 4
  call void @__kmpc_for_static_init_4u({ i32, i32, i32, i32, i8* }* nonnull @.kmpc_loc.0.0, i32 %tid.val, i32 34, i32* nonnull %is.last, i32* nonnull %lower.bnd, i32* nonnull %upper.bnd, i32* nonnull %stride, i32 1, i32 1) #1
  %lb.new = load i32, i32* %lower.bnd, align 4
  %ub.new = load i32, i32* %upper.bnd, align 4
  %omp.ztt = icmp ugt i32 %lb.new, %ub.new
  br i1 %omp.ztt, label %DIR.OMP.END.LOOP.2, label %omp.inner.for.body.preheader

omp.inner.for.body.preheader:                     ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.preheader
  %.omp.iv.0 = phi i32 [ %add2, %omp.inner.for.body ], [ %lb.new, %omp.inner.for.body.preheader ]
  %idxprom = zext i32 %.omp.iv.0 to i64
  %arrayidx = getelementptr inbounds [100 x double], [100 x double]* @A, i64 0, i64 %idxprom, !intel-tbaa !2
  store double 1.234000e+00, double* %arrayidx, align 8, !tbaa !2
  %add2 = add i32 %.omp.iv.0, 1
  %cmp = icmp ugt i32 %add2, %ub.new
  br i1 %cmp, label %omp.inner.for.cond.omp.loop.exit_crit_edge, label %omp.inner.for.body, !llvm.loop !7

omp.inner.for.cond.omp.loop.exit_crit_edge:       ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.LOOP.2

DIR.OMP.END.LOOP.2:                               ; preds = %omp.inner.for.body.lr.ph, %omp.inner.for.cond.omp.loop.exit_crit_edge
  call void @__kmpc_for_static_fini({ i32, i32, i32, i32, i8* }* nonnull @.kmpc_loc.0.0.2, i32 %tid.val) #1
  call void @__kmpc_barrier({ i32, i32, i32, i32, i8* }* nonnull @.kmpc_loc.0.0.4, i32 %tid.val) #1
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare void @__kmpc_for_static_init_4u({ i32, i32, i32, i32, i8* }*, i32, i32, i32*, i32*, i32*, i32*, i32, i32) local_unnamed_addr

declare void @__kmpc_for_static_fini({ i32, i32, i32, i32, i8* }*, i32) local_unnamed_addr

declare void @__kmpc_barrier({ i32, i32, i32, i32, i8* }*, i32) local_unnamed_addr

declare i32 @__kmpc_global_thread_num({ i32, i32, i32, i32, i8* }*) local_unnamed_addr

attributes #1 = { nounwind }


!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_d", !4, i64 0}
!4 = !{!"double", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = distinct !{!"intel.optreport.rootnode", !9}
!9 = distinct !{!"intel.optreport", !10}
!10 = !{!"intel.optreport.remarks", !11}
!11 = !{!"intel.optreport.remark", !"OpenMP: Worksharing loop"}

