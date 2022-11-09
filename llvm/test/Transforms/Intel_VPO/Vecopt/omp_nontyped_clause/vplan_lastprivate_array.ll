; REQUIRES: asserts
; RUN: opt -vplan-vec -vplan-force-vf=2 -S -debug-only=vplan-vec -debug-only=vpo-ir-loop-vectorize-legality < %s 2>&1 | FileCheck %s
; RUN: opt -passes="vplan-vec" -vplan-force-vf=2 -S -debug-only=vplan-vec -debug-only=vpo-ir-loop-vectorize-legality < %s 2>&1 | FileCheck %s

; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=2 -debug-only=HIRLegality -debug-only=vplan-vec -debug-only=LoopVectorizationPlannerHIR -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIR
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -debug-only=HIRLegality -debug-only=vplan-vec -debug-only=LoopVectorizationPlannerHIR -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIR

; CHECK:   [12 x %struct.int_int]* [[ALLPRIV:%.*]] = allocate-priv [12 x %struct.int_int]*, OrigAlign = 8
; CHECK:   private-last-value-nonpod-array [12 x %struct.int_int]* [[ALLPRIV]] [12 x %struct.int_int]* [[TMP1:%.*]]
; CHECK:   private-nonpod-array-dtor [12 x %struct.int_int]* [[ALLPRIV]]
; CHECK-NOT: private-nonpod-array-ctor [12 x %struct.int_int]* [[TMP1]]

; CHECK:      array.nonpod.last.private.loop:                   ; preds = %array.nonpod.last.private.loop, %VPlannedBB11
; CHECK-NEXT:   %27 = phi i64 [ 0, %VPlannedBB11 ], [ %30, %array.nonpod.last.private.loop ]
; CHECK-NEXT:   %28 = getelementptr [12 x %struct.int_int], [12 x %struct.int_int]* %y3.lpriv.vec.base.addr.extract.1., i64 0, i64 %27
; CHECK-NEXT:   %29 = getelementptr [12 x %struct.int_int], [12 x %struct.int_int]* %y3.lpriv, i64 0, i64 %27
; CHECK-NEXT:   call void @_ZTS7int_int.omp.copy_assign(%struct.int_int* %29, %struct.int_int* %28)
; CHECK-NEXT:   %30 = add i64 %27, 1
; CHECK-NEXT:   %31 = icmp ult i64 %30, 12
; CHECK-NEXT:   br i1 %31, label %array.nonpod.last.private.loop, label %array.nonpod.last.private.loop.exit
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.last.private.loop.exit:              ; preds = %array.nonpod.last.private.loop
; CHECK-NEXT:   br label %array.nonpod.private.outer.loop
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.outer.loop:                  ; preds = %array.nonpod.private.outer.loop.inc, %array.nonpod.last.private.loop.exit
; CHECK-NEXT:   %32 = phi i64 [ 0, %array.nonpod.last.private.loop.exit ], [ %37, %array.nonpod.private.outer.loop.inc ]
; CHECK-NEXT:   %priv.extract = extractelement <2 x [12 x %struct.int_int]*> %y3.lpriv.vec.base.addr, i64 %32
; CHECK-NEXT:   br label %array.nonpod.private.inner.loop
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.inner.loop:                  ; preds = %array.nonpod.private.inner.loop, %array.nonpod.private.outer.loop
; CHECK-NEXT:   %33 = phi i64 [ 0, %array.nonpod.private.outer.loop ], [ %35, %array.nonpod.private.inner.loop ]
; CHECK-NEXT:   %34 = getelementptr [12 x %struct.int_int], [12 x %struct.int_int]* %priv.extract, i64 0, i64 %33
; CHECK-NEXT:   call void @_ZTS7int_int.omp.destr(%struct.int_int* %34)
; CHECK-NEXT:   %35 = add i64 %33, 1
; CHECK-NEXT:   %36 = icmp ult i64 %35, 12
; CHECK-NEXT:   br i1 %36, label %array.nonpod.private.inner.loop, label %array.nonpod.private.outer.loop.inc
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.outer.loop.inc:              ; preds = %array.nonpod.private.inner.loop
; CHECK-NEXT:   %37 = add i64 %32, 1
; CHECK-NEXT:   %38 = icmp ult i64 %32, 2
; CHECK-NEXT:   br i1 %38, label %array.nonpod.private.outer.loop, label %array.nonpod.private.loop.exit
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.loop.exit:                   ; preds = %array.nonpod.private.outer.loop.inc
; CHECK:        br label %VPlannedBB13

; Incomming HIR
;    BEGIN REGION { }
;          %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.LASTPRIVATE:NONPOD.TYPED(&((%y3.lpriv)[0])zeroinitializer12null&((@_ZTS7int_int.omp.copy_assign)[0])&((@_ZTS7int_int.omp.destr)[0])),  QUAL.OMP.LINEAR:IV.TYPED(&((%i.priv)[0])011) ]
;
;          + DO i1 = 0, -1 * %lb.new + smax((1 + %lb.new), %ub.new) + -1, 1   <DO_LOOP> <simd>
;          |   (%i.priv)[0] = i1 + %lb.new;
;          |   %2 = (%this1)[0].1;
;          |   %3 = (%i.priv)[0];
;          |   %4 = (%i.priv)[0];
;          |   %call = @_ZN7int_intaSERKS_(&((%y3.lpriv)[0][%4]),  &((%2)[0][%3]));
;          + END LOOP
;
;          @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
;          ret ;
;    END REGION

; *** IR Dump After vpo::VPlanDriverHIRPass ***
; HIR:           + DO i1 = 0, 12, 1   <DO_LOOP>
; HIR-NEXT:      |   @_ZTS7int_int.omp.copy_assign(&((%struct.int_int*)(%y3.lpriv)[i1]),  &((%struct.int_int*)(%extract.1.22)[i1]));
; HIR-NEXT:      + END LOOP
; HIR:           + DO i1 = 0, 2, 1   <DO_LOOP>
; HIR-NEXT:      |   %priv.extract = extractelement &((<2 x [12 x %struct.int_int]*>)(%priv.mem.bc)[<i32 0, i32 1>]),  i1;
; HIR-NEXT:      |
; HIR-NEXT:      |   + DO i2 = 0, 12, 1   <DO_LOOP>
; HIR-NEXT:      |   |   @_ZTS7int_int.omp.destr(&((%struct.int_int*)(%priv.extract)[i2]));
; HIR-NEXT:      |   + END LOOP
; HIR-NEXT:      + END LOOP

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
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:NONPOD"([12 x %struct.int_int]* %y3.lpriv, i8* null, void (%struct.int_int*, %struct.int_int*)* @_ZTS7int_int.omp.copy_assign, void (%struct.int_int*)* @_ZTS7int_int.omp.destr), "QUAL.OMP.LINEAR:IV"(i32* %i.priv, i32 1) ]
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
