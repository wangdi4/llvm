; This test checks bailout of non-const length pod array privates.
; REQUIRES: asserts
; RUN: opt -passes="vplan-vec" -vplan-force-vf=2 -S -debug-only=vplan-vec -debug-only=vpo-ir-loop-vectorize-legality < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -debug-only=HIRLegality -debug-only=vplan-vec -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIR

; CHECK: Cannot handle array privates yet.
; CHECK: VD: Not vectorizing: Cannot prove legality.

; CHECK: %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:NONPOD"(%struct.point2d* %vla.priv, %struct.point2d* (%struct.point2d*)* @_ZTS7point2d.omp.def_constr, void (%struct.point2d*)* @_ZTS7point2d.omp.destr), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1) ]

; HIR: VPlan HIR Driver for Function: _Z3fooi
; HIR: Cannot handle array privates yet.
; HIR: VD: Not vectorizing: Cannot prove legality.
; HIR: Function: _Z3fooi

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
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:NONPOD"(%struct.point2d* %vla.priv, %struct.point2d* (%struct.point2d*)* @_ZTS7point2d.omp.def_constr, void (%struct.point2d*)* @_ZTS7point2d.omp.destr), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %2 = bitcast i32* %i.linear.iv to i8*
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %s.020 = phi i32 [ 0, %DIR.OMP.SIMD.2 ], [ %add7, %omp.inner.for.body ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2)
  %3 = trunc i64 %indvars.iv to i32
  %x = getelementptr inbounds %struct.point2d, %struct.point2d* %vla.priv, i64 %indvars.iv, i32 0
  store i32 %3, i32* %x, align 8
  %add4 = add nsw i32 %s.020, %3
  %y = getelementptr inbounds %struct.point2d, %struct.point2d* %vla.priv, i64 %indvars.iv, i32 1
  %4 = load i32, i32* %y, align 4
  %add7 = add nsw i32 %add4, %4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn writeonly uwtable
declare %struct.point2d* @_ZTS7point2d.omp.def_constr(%struct.point2d* %0)

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
declare void @_ZTS7point2d.omp.destr(%struct.point2d* %0)

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)
