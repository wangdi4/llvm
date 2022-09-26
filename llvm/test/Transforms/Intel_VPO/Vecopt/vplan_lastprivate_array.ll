; REQUIRES: asserts
; RUN: opt -vplan-vec -vplan-force-vf=2 -S -debug-only=vplan-vec -debug-only=vpo-ir-loop-vectorize-legality < %s 2>&1 | FileCheck %s
; RUN: opt -passes="vplan-vec" -vplan-force-vf=2 -S -debug-only=vplan-vec -debug-only=vpo-ir-loop-vectorize-legality < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=2 -debug-only=HIRLegality -debug-only=vplan-vec -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIR
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -debug-only=HIRLegality -debug-only=vplan-vec -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIR

; CHECK: VPlan LLVM-IR Driver for Function: test1
; CHECK: Cannot handle nonPOD array lastprivates.
; CHECK: VD: Not vectorizing: Cannot prove legality.

; CHECK: define void @test1
; CHECK: %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:NONPOD.TYPED"([12 x %struct.int_int]* %y3.lpriv, %struct.int_int zeroinitializer, i32 12, i8* null, void (%struct.int_int*, %struct.int_int*)* @_ZTS7int_int.omp.copy_assign, void (%struct.int_int*)* @_ZTS7int_int.omp.destr), "QUAL.OMP.LINEAR:IV.TYPED"(i32* %i.priv, i32 0, i32 1, i32 1) ]

; HIR: VPlan HIR Driver for Function: test1
; HIR: Cannot handle nonPOD array lastprivates.
; HIR: VD: Not vectorizing: Cannot prove legality.
; HIR: Function: test1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.int_int = type { i32, i32 }
%struct.my_struct = type { [12 x %struct.int_int] }
%class.my_class = type { %struct.my_struct*, [12 x %struct.int_int]* }

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: noinline uwtable
declare void @_ZTS7int_int.omp.copy_constr(%struct.int_int* %0, %struct.int_int* %1)

; Function Attrs: noinline uwtable
declare void @_ZTS7int_int.omp.destr(%struct.int_int* %0)

; Function Attrs: noinline uwtable
declare void @_ZTS7int_int.omp.copy_assign(%struct.int_int* %0, %struct.int_int* %1)

; Function Attrs: mustprogress noinline nounwind optnone uwtable
declare dso_local nonnull align 4 dereferenceable(8) %struct.int_int* @_ZN7int_intaSERKS_(%struct.int_int* nonnull align 4 dereferenceable(8) %this, %struct.int_int* nonnull align 4 dereferenceable(8) %t1)


; Function Attrs: mustprogress noinline nounwind optnone uwtable
define void @test1(i32* %l.bnd, i32* %u.bnd, %class.my_class* %this1, [12 x %struct.int_int]* %y3) {
newFuncRoot:
  %y3.lpriv = alloca [12 x %struct.int_int], align 1
  %i.priv = alloca i32, align 4
  %lb.new = load i32, i32* %l.bnd, align 4
  %ub.new = load i32, i32* %u.bnd, align 4
  br label %omp.inner.for.body.preheader

omp.inner.for.body.preheader:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:NONPOD.TYPED"([12 x %struct.int_int]* %y3.lpriv, %struct.int_int zeroinitializer, i32 12, i8* null, void (%struct.int_int*, %struct.int_int*)* @_ZTS7int_int.omp.copy_assign, void (%struct.int_int*)* @_ZTS7int_int.omp.destr), "QUAL.OMP.LINEAR:IV.TYPED"(i32* %i.priv, i32 0, i32 1, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.local.027 = phi i32 [ %add8, %omp.inner.for.inc ], [ %lb.new, %omp.inner.for.body.preheader ]
  store i32 %.omp.iv.local.027, i32* %i.priv, align 4
  br label %if.then

if.then:
  %1 = getelementptr inbounds %class.my_class, %class.my_class* %this1, i32 0, i32 1
  %2 = load [12 x %struct.int_int]*, [12 x %struct.int_int]** %1, align 8
  %3 = load i32, i32* %i.priv, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds [12 x %struct.int_int], [12 x %struct.int_int]* %2, i64 0, i64 %idxprom
  %4 = load i32, i32* %i.priv, align 4
  %idxprom6 = sext i32 %4 to i64
  %arrayidx7 = getelementptr inbounds [12 x %struct.int_int], [12 x %struct.int_int]* %y3.lpriv, i64 0, i64 %idxprom6
  %call = call nonnull align 4 dereferenceable(8) %struct.int_int* @_ZN7int_intaSERKS_(%struct.int_int* nonnull align 4 dereferenceable(8) %arrayidx7, %struct.int_int* nonnull align 4 dereferenceable(8) %arrayidx)
  br label %omp.inner.for.inc

omp.inner.for.inc:
  %add8 = add nsw i32 %.omp.iv.local.027, 1
  %cmp = icmp slt i32 %add8, %ub.new
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.exit

omp.inner.for.exit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}
