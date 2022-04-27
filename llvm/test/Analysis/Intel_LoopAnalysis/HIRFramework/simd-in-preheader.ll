; RUN: opt < %s -analyze -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Check that SIMD intrinsic placed as a last node of the loop preheader makes the loop recognized as a SIMD loop.

; BEGIN REGION { }
;       + DO i1 = 0, sext.i32.i64(%ub.new) + -1 * sext.i32.i64(%lb.new), 1   <DO_LOOP>  <MAX_TC_EST = 101>
;       |      %3 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.LINEAR:IV(&((%j.priv.linear.iv)[0])1),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
;       |   + DO i2 = 0, zext.i32.i64(%gp12.addr.fpriv.v) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100> <simd>
;       |   |   (%lhsX)[0][0][0][1][0][i2 + 1][i1 + sext.i32.i64(%lb.new) + 1] = 1.000000e+00;
;       |   |   (%lhsX)[0][0][0][1][%isize.addr.fpriv.v][i2 + 1][i1 + sext.i32.i64(%lb.new) + 1] = 1.000000e+00;
;       |   |   (%lhsX)[0][1][1][1][0][i2 + 1][i1 + sext.i32.i64(%lb.new) + 1] = 1.000000e+00;
;       |   |   (%lhsX)[0][1][1][1][%isize.addr.fpriv.v][i2 + 1][i1 + sext.i32.i64(%lb.new) + 1] = 1.000000e+00;
;       |   |   (%lhsX)[0][2][2][1][0][i2 + 1][i1 + sext.i32.i64(%lb.new) + 1] = 1.000000e+00;
;       |   |   (%lhsX)[0][2][2][1][%isize.addr.fpriv.v][i2 + 1][i1 + sext.i32.i64(%lb.new) + 1] = 1.000000e+00;
;       |   |   (%lhsX)[0][3][3][1][0][i2 + 1][i1 + sext.i32.i64(%lb.new) + 1] = 1.000000e+00;
;       |   |   (%lhsX)[0][3][3][1][%isize.addr.fpriv.v][i2 + 1][i1 + sext.i32.i64(%lb.new) + 1] = 1.000000e+00;
;       |   |   (%lhsX)[0][4][4][1][0][i2 + 1][i1 + sext.i32.i64(%lb.new) + 1] = 1.000000e+00;
;       |   |   (%lhsX)[0][4][4][1][%isize.addr.fpriv.v][i2 + 1][i1 + sext.i32.i64(%lb.new) + 1] = 1.000000e+00;
;       |   + END LOOP
;       |      (%j.priv.linear.iv)[0] = %gp12.addr.fpriv.v + 1;
;       |      @llvm.directive.region.exit(%3); [ DIR.OMP.END.SIMD() ]
;       + END LOOP
; END REGION

; CHECK: DO i2
; CHECK-SAME: <simd>

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, i8* }

@.kmpc_loc.0.0 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.2 = external hidden unnamed_addr global %struct.ident_t

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

; Function Attrs: nounwind
declare void @__kmpc_dist_for_static_init_4(%struct.ident_t*, i32, i32, i32*, i32*, i32*, i32*, i32*, i32, i32) local_unnamed_addr #0

; Function Attrs: nofree nounwind
declare void @__kmpc_for_static_fini(%struct.ident_t* nocapture readonly, i32) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define hidden void @x_solve.DIR.OMP.DISTRIBUTE.PARLOOP.5.split168.split(i32* nocapture readonly %tid, i32* nocapture readnone %bid, i32* noalias nocapture readonly %gp12.addr.fpriv, i32* noalias nocapture readonly %isize.addr.fpriv, [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]]* noalias nocapture %lhsX, i64 %.omp.lb.priv.val.zext, i64 %.omp.ub.priv.val) #2 {
DIR.OMP.DISTRIBUTE.PARLOOP.8:
  %is.last = alloca i32, align 4
  %lower.bnd = alloca i32, align 4
  %upper.bnd = alloca i32, align 4
  %stride = alloca i32, align 4
  %upperD = alloca i32, align 4
  %j.priv.linear.iv = alloca i32, align 4
  store i32 0, i32* %is.last, align 4
  %gp12.addr.fpriv.v = load i32, i32* %gp12.addr.fpriv, align 4
  %isize.addr.fpriv.v = load i32, i32* %isize.addr.fpriv, align 4
  %0 = trunc i64 %.omp.ub.priv.val to i32
  %cmp2162 = icmp sgt i32 0, %0
  br i1 %cmp2162, label %omp.precond.end106.exitStub, label %omp.inner.for.body.lr.ph

omp.precond.end106.exitStub:                      ; preds = %loop.region.exit, %DIR.OMP.DISTRIBUTE.PARLOOP.8
  ret void

omp.inner.for.body.lr.ph:                         ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.8
  %my.tid = load i32, i32* %tid, align 4
  store i32 0, i32* %lower.bnd, align 4
  store i32 %0, i32* %upper.bnd, align 4
  store i32 1, i32* %stride, align 4
  store i32 %0, i32* %upperD, align 4
  call void @__kmpc_dist_for_static_init_4(%struct.ident_t* nonnull @.kmpc_loc.0.0, i32 %my.tid, i32 34, i32* nonnull %is.last, i32* nonnull %lower.bnd, i32* nonnull %upper.bnd, i32* nonnull %upperD, i32* nonnull %stride, i32 1, i32 1) #0
  %lb.new = load i32, i32* %lower.bnd, align 4
  %ub.new = load i32, i32* %upper.bnd, align 4
  %omp.ztt = icmp sgt i32 %lb.new, %ub.new
  br i1 %omp.ztt, label %loop.region.exit, label %omp.inner.for.body.preheader

omp.inner.for.body.preheader:                     ; preds = %omp.inner.for.body.lr.ph
  %cmp12165 = icmp slt i32 %gp12.addr.fpriv.v, 1
  %idxprom25 = sext i32 %isize.addr.fpriv.v to i64
  %1 = sext i32 %lb.new to i64
  %2 = sext i32 %ub.new to i64
  %wide.trip.count21 = zext i32 %gp12.addr.fpriv.v to i64
  %add100 = add i32 %gp12.addr.fpriv.v, 1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.precond.end, %omp.inner.for.body.preheader
  %indvars.iv19 = phi i64 [ %1, %omp.inner.for.body.preheader ], [ %indvars.iv.next20, %omp.precond.end ]
  %indvars.iv.next20 = add i64 %indvars.iv19, 1
  br i1 %cmp12165, label %omp.precond.end, label %omp.inner.for.body13.lr.ph

loop.region.exit.loopexit:                        ; preds = %omp.precond.end
  br label %loop.region.exit

loop.region.exit:                                 ; preds = %loop.region.exit.loopexit, %omp.inner.for.body.lr.ph
  call void @__kmpc_for_static_fini(%struct.ident_t* nonnull @.kmpc_loc.0.0.2, i32 %my.tid) #0
  br label %omp.precond.end106.exitStub

omp.precond.end:                                  ; preds = %omp.inner.for.cond11.omp.loop.exit.split.loopexit_crit_edge, %omp.inner.for.body
  %cmp2 = icmp sgt i64 %indvars.iv.next20, %2
  br i1 %cmp2, label %loop.region.exit.loopexit, label %omp.inner.for.body

omp.inner.for.body13.lr.ph:                       ; preds = %omp.inner.for.body
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %j.priv.linear.iv, i32 1), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %omp.inner.for.body13

omp.inner.for.body13:                             ; preds = %omp.inner.for.body13, %omp.inner.for.body13.lr.ph
  %indvars.iv = phi i64 [ 0, %omp.inner.for.body13.lr.ph ], [ %indvars.iv.next, %omp.inner.for.body13 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx21 = getelementptr inbounds [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]], [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]]* %lhsX, i64 0, i64 0, i64 0, i64 1, i64 0, i64 %indvars.iv.next, i64 %indvars.iv.next20
  store double 1.000000e+00, double* %arrayidx21, align 8
  %arrayidx30 = getelementptr inbounds [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]], [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]]* %lhsX, i64 0, i64 0, i64 0, i64 1, i64 %idxprom25, i64 %indvars.iv.next, i64 %indvars.iv.next20
  store double 1.000000e+00, double* %arrayidx30, align 8
  %arrayidx38 = getelementptr inbounds [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]], [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]]* %lhsX, i64 0, i64 1, i64 1, i64 1, i64 0, i64 %indvars.iv.next, i64 %indvars.iv.next20
  store double 1.000000e+00, double* %arrayidx38, align 8
  %arrayidx47 = getelementptr inbounds [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]], [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]]* %lhsX, i64 0, i64 1, i64 1, i64 1, i64 %idxprom25, i64 %indvars.iv.next, i64 %indvars.iv.next20
  store double 1.000000e+00, double* %arrayidx47, align 8
  %arrayidx55 = getelementptr inbounds [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]], [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]]* %lhsX, i64 0, i64 2, i64 2, i64 1, i64 0, i64 %indvars.iv.next, i64 %indvars.iv.next20
  store double 1.000000e+00, double* %arrayidx55, align 8
  %arrayidx64 = getelementptr inbounds [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]], [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]]* %lhsX, i64 0, i64 2, i64 2, i64 1, i64 %idxprom25, i64 %indvars.iv.next, i64 %indvars.iv.next20
  store double 1.000000e+00, double* %arrayidx64, align 8
  %arrayidx72 = getelementptr inbounds [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]], [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]]* %lhsX, i64 0, i64 3, i64 3, i64 1, i64 0, i64 %indvars.iv.next, i64 %indvars.iv.next20
  store double 1.000000e+00, double* %arrayidx72, align 8
  %arrayidx81 = getelementptr inbounds [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]], [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]]* %lhsX, i64 0, i64 3, i64 3, i64 1, i64 %idxprom25, i64 %indvars.iv.next, i64 %indvars.iv.next20
  store double 1.000000e+00, double* %arrayidx81, align 8
  %arrayidx89 = getelementptr inbounds [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]], [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]]* %lhsX, i64 0, i64 4, i64 4, i64 1, i64 0, i64 %indvars.iv.next, i64 %indvars.iv.next20
  store double 1.000000e+00, double* %arrayidx89, align 8
  %arrayidx98 = getelementptr inbounds [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]], [5 x [5 x [3 x [102 x [101 x [101 x double]]]]]]* %lhsX, i64 0, i64 4, i64 4, i64 1, i64 %idxprom25, i64 %indvars.iv.next, i64 %indvars.iv.next20
  store double 1.000000e+00, double* %arrayidx98, align 8
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count21
  br i1 %exitcond, label %omp.inner.for.cond11.omp.loop.exit.split.loopexit_crit_edge, label %omp.inner.for.body13

omp.inner.for.cond11.omp.loop.exit.split.loopexit_crit_edge: ; preds = %omp.inner.for.body13
  store i32 %add100, i32* %j.priv.linear.iv, align 4
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end
}

