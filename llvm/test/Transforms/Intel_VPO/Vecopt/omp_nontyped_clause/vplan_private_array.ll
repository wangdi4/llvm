; REQUIRES: asserts
; RUN: opt -vplan-vec -vplan-force-vf=2 -S -debug-only=vplan-vec -debug-only=vpo-ir-loop-vectorize-legality < %s 2>&1 | FileCheck %s
; RUN: opt -passes="vplan-vec" -vplan-force-vf=2 -S -debug-only=vplan-vec -debug-only=vpo-ir-loop-vectorize-legality < %s 2>&1 | FileCheck %s

; FIXME: Updated checks once HIR support for private array type is added
; COM: RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=2 -debug-only=HIRLegality -debug-only=vplan-vec -debug-only=LoopVectorizationPlannerHIR -print-after=hir-vplan-vec -disable-output < %s 2>&1
; COM: RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -debug-only=HIRLegality -debug-only=vplan-vec -debug-only=LoopVectorizationPlannerHIR -print-after=hir-vplan-vec -disable-output < %s 2>&1

; CHECK:   [12 x %struct.int_int]* [[ALLPRIV:%.*]] = allocate-priv [12 x %struct.int_int]*, OrigAlign = 8
; CHECK:   private-nonpod-array-dtor [12 x %struct.int_int]* [[ALLPRIV]]
; CHECK-NOT: private-nonpod-array-ctor [12 x %struct.int_int]* [[ALLPRIV]]

; CHECK:      array.nonpod.private.outer.loop:                  ; preds = %array.nonpod.private.outer.loop.inc, %VPlannedBB11
; CHECK-NEXT:   %27 = phi i64 [ 0, %VPlannedBB11 ], [ %32, %array.nonpod.private.outer.loop.inc ]
; CHECK-NEXT:   %priv.extract = extractelement <2 x [12 x %struct.int_int]*> %y3.priv.vec.base.addr, i64 %27
; CHECK-NEXT:   br label %array.nonpod.private.inner.loop
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.inner.loop:                  ; preds = %array.nonpod.private.inner.loop, %array.nonpod.private.outer.loop
; CHECK-NEXT:   %28 = phi i64 [ 0, %array.nonpod.private.outer.loop ], [ %30, %array.nonpod.private.inner.loop ]
; CHECK-NEXT:   %29 = getelementptr [12 x %struct.int_int], [12 x %struct.int_int]* %priv.extract, i64 0, i64 %28
; CHECK-NEXT:   call void @_ZTS7int_int.omp.destr(%struct.int_int* %29)
; CHECK-NEXT:   %30 = add i64 %28, 1
; CHECK-NEXT:   %31 = icmp ult i64 %30, 12
; CHECK-NEXT:   br i1 %31, label %array.nonpod.private.inner.loop, label %array.nonpod.private.outer.loop.inc
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.outer.loop.inc:              ; preds = %array.nonpod.private.inner.loop
; CHECK-NEXT:   %32 = add i64 %27, 1
; CHECK-NEXT:   %33 = icmp ult i64 %27, 2
; CHECK-NEXT:   br i1 %33, label %array.nonpod.private.outer.loop, label %array.nonpod.private.loop.exit
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.loop.exit:                   ; preds = %array.nonpod.private.outer.loop.inc
; CHECK:        br label %VPlannedBB13

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

; Function Attrs: mustprogress noinline nounwind optnone uwtable
declare dso_local nonnull align 4 dereferenceable(8) %struct.int_int* @_ZN7int_intaSERKS_(%struct.int_int* nonnull align 4 dereferenceable(8) %this, %struct.int_int* nonnull align 4 dereferenceable(8) %t1)


; Function Attrs: mustprogress noinline nounwind optnone uwtable
define void @test1(i32* %l.bnd, i32* %u.bnd, %class.my_class* %this1, [12 x %struct.int_int]* %y3) {
newFuncRoot:
  %y3.priv = alloca [12 x %struct.int_int], align 1
  %i.priv = alloca i32, align 4
  %lb.new = load i32, i32* %l.bnd, align 4
  %ub.new = load i32, i32* %u.bnd, align 4
  br label %omp.inner.for.body.preheader

omp.inner.for.body.preheader:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:NONPOD"([12 x %struct.int_int]* %y3.priv, i8* null, void (%struct.int_int*)* @_ZTS7int_int.omp.destr), "QUAL.OMP.LINEAR:IV"(i32* %i.priv, i32 1) ]
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
  %arrayidx7 = getelementptr inbounds [12 x %struct.int_int], [12 x %struct.int_int]* %y3.priv, i64 0, i64 %idxprom6
  %call = call nonnull align 4 dereferenceable(8) %struct.int_int* @_ZN7int_intaSERKS_(%struct.int_int* nonnull align 4 dereferenceable(8) %arrayidx7, %struct.int_int* nonnull align 4 dereferenceable(8) %arrayidx) #0
  br label %omp.inner.for.inc

omp.inner.for.inc:
  %add8 = add nsw i32 %.omp.iv.local.027, 1
  %cmp = icmp slt i32 %add8, %ub.new
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.exit

omp.inner.for.exit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

attributes #0 = { nounwind }
