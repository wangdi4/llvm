; This test checks bailout of non-const length pod array privates.
; REQUIRES: asserts
; RUN: opt -passes="vplan-vec" -vplan-force-vf=2 -S -debug-only=VPlanDriver -debug-only=VPlanLegality < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -debug-only=VPlanLegality -debug-only=VPlanDriver -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIRVEC
; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -vplan-force-vf=2 -disable-output -intel-opt-report=medium < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTMED
; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -vplan-force-vf=2 -disable-output -intel-opt-report=high < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTHI
; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -vplan-force-vf=2 -disable-output -intel-opt-report=high < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTHI-HIR

; CHECK: Cannot handle array privates yet.
; CHECK: VD: Not vectorizing: Cannot prove legality.

; CHECK: %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr %vla.priv, %struct.point2d zeroinitializer, i64 %0, ptr @_ZTS7point2d.omp.def_constr, ptr @_ZTS7point2d.omp.destr), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null), "QUAL.OMP.LINEAR:IV"(ptr %i.linear.iv, i32 1) ]

; HIRVEC: VPlan HIR Driver for Function: _Z3fooi
; HIRVEC: Cannot handle array privates yet.
; HIRVEC: VD: Not vectorizing: Cannot prove legality.
; HIRVEC: Function: _Z3fooi

; OPTRPTMED: remark #15436: loop was not vectorized:
; OPTRPTHI: remark #15436: loop was not vectorized:
; OPTRPTHI: remark #15436: loop was not vectorized: Cannot handle array privates yet.
; OPTRPTHI-HIR: remark #15436: loop was not vectorized: HIR: Cannot handle array privates yet.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.point2d = type { i32, i32 }

; Function Attrs: nounwind uwtable
define void @_Z3fooi(i32 %n) {
entry:
  %i.linear.iv = alloca i32, align 4
  %0 = zext i32 %n to i64
  %vla.priv = alloca %struct.point2d, i64 %0, align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr %vla.priv, %struct.point2d zeroinitializer, i64 %0, ptr @_ZTS7point2d.omp.def_constr, ptr @_ZTS7point2d.omp.destr), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null), "QUAL.OMP.LINEAR:IV"(ptr %i.linear.iv, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %s.020 = phi i32 [ 0, %DIR.OMP.SIMD.2 ], [ %add7, %omp.inner.for.body ]
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i.linear.iv)
  %2 = trunc i64 %indvars.iv to i32
  %x = getelementptr inbounds %struct.point2d, ptr %vla.priv, i64 %indvars.iv, i32 0
  store i32 %2, ptr %x, align 8
  %add4 = add nsw i32 %s.020, %2
  %y = getelementptr inbounds %struct.point2d, ptr %vla.priv, i64 %indvars.iv, i32 1
  %3 = load i32, ptr %y, align 4
  %add7 = add nsw i32 %add4, %3
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i.linear.iv)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn writeonly uwtable
declare ptr @_ZTS7point2d.omp.def_constr(ptr %0)

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
declare void @_ZTS7point2d.omp.destr(ptr %0)

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg)
