; REQUIRES: asserts
; RUN: opt -passes="vplan-vec" -vplan-force-vf=2 -S -debug-only=VPlanDriver -debug-only=vpo-ir-loop-vectorize-legality < %s 2>&1 | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -debug-only=HIRLegality -debug-only=VPlanDriver -debug-only=LoopVectorizationPlannerHIR -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIR

; CHECK:   ptr [[ALLPRIV:%.*]] = allocate-priv [12 x %struct.int_int], OrigAlign = 8
; CHECK:   private-last-value-nonpod-array ptr [[ALLPRIV]] ptr [[TMP1:%.*]]
; CHECK:   private-nonpod-array-dtor ptr [[ALLPRIV]]
; CHECK-NOT: private-nonpod-array-ctor ptr [[TMP1]]

; CHECK:      array.nonpod.last.private.loop:                   ; preds = %array.nonpod.last.private.loop, %VPlannedBB11
; CHECK-NEXT:   [[TMP1:%.*]] = phi i64 [ 0, %VPlannedBB11 ], [ [[TMP4:%.*]], %array.nonpod.last.private.loop ]
; CHECK-NEXT:   [[TMP2:%.*]] = getelementptr [12 x %struct.int_int], ptr %y3.lpriv.vec.base.addr.extract.1., i64 0, i64 [[TMP1]]
; CHECK-NEXT:   [[TMP3:%.*]] = getelementptr [12 x %struct.int_int], ptr %y3.lpriv, i64 0, i64 [[TMP1]]
; CHECK-NEXT:   call void @_ZTS7int_int.omp.copy_assign(ptr [[TMP3]], ptr [[TMP2]])
; CHECK-NEXT:   [[TMP4]] = add i64 [[TMP1]], 1
; CHECK-NEXT:   [[TMP5:%.*]] = icmp ult i64 [[TMP4]], 12
; CHECK-NEXT:   br i1 [[TMP5]], label %array.nonpod.last.private.loop, label %array.nonpod.last.private.loop.exit
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.last.private.loop.exit:              ; preds = %array.nonpod.last.private.loop
; CHECK-NEXT:   br label %array.nonpod.private.outer.loop
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.outer.loop:                  ; preds = %array.nonpod.private.outer.loop.inc, %array.nonpod.last.private.loop.exit
; CHECK-NEXT:   [[TMP6:%.*]] = phi i64 [ 0, %array.nonpod.last.private.loop.exit ], [ [[TMP11:%.*]], %array.nonpod.private.outer.loop.inc ]
; CHECK-NEXT:   %priv.extract = extractelement <2 x ptr> %y3.lpriv.vec.base.addr, i64 [[TMP6]]
; CHECK-NEXT:   br label %array.nonpod.private.inner.loop
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.inner.loop:                  ; preds = %array.nonpod.private.inner.loop, %array.nonpod.private.outer.loop
; CHECK-NEXT:   [[TMP7:%.*]] = phi i64 [ 0, %array.nonpod.private.outer.loop ], [ [[TMP9:%.*]], %array.nonpod.private.inner.loop ]
; CHECK-NEXT:   [[TMP8:%.*]] = getelementptr [12 x %struct.int_int], ptr %priv.extract, i64 0, i64 [[TMP7]]
; CHECK-NEXT:   call void @_ZTS7int_int.omp.destr(ptr [[TMP8]])
; CHECK-NEXT:   [[TMP9]] = add i64 [[TMP7]], 1
; CHECK-NEXT:   [[TMP10:%.*]] = icmp ult i64 [[TMP9]], 12
; CHECK-NEXT:   br i1 [[TMP10]], label %array.nonpod.private.inner.loop, label %array.nonpod.private.outer.loop.inc
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.outer.loop.inc:              ; preds = %array.nonpod.private.inner.loop
; CHECK-NEXT:   [[TMP11]] = add i64 [[TMP6]], 1
; CHECK-NEXT:   [[TMP12:%.*]] = icmp ult i64 [[TMP6]], 2
; CHECK-NEXT:   br i1 [[TMP12]], label %array.nonpod.private.outer.loop, label %array.nonpod.private.loop.exit
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
; HIR:           + DO i1 = 0, 11, 1   <DO_LOOP>
; HIR-NEXT:      |   @_ZTS7int_int.omp.copy_assign(&((%struct.int_int*)(%y3.lpriv)[i1]),  &((%struct.int_int*)(%extract.1.22)[i1]));
; HIR-NEXT:      + END LOOP
; HIR:           + DO i1 = 0, 1, 1   <DO_LOOP>
; HIR-NEXT:      |   %priv.extract = extractelement &((<2 x ptr>)(%priv.mem.bc)[<i32 0, i32 1>]),  i1;
; HIR-NEXT:      |
; HIR-NEXT:      |   + DO i2 = 0, 11, 1   <DO_LOOP>
; HIR-NEXT:      |   |   @_ZTS7int_int.omp.destr(&((%struct.int_int*)(%priv.extract)[i2]));
; HIR-NEXT:      |   + END LOOP
; HIR-NEXT:      + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.int_int = type { i32, i32 }
%struct.my_struct = type { [12 x %struct.int_int] }
%class.my_class = type { ptr, ptr }

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: noinline uwtable
declare void @_ZTS7int_int.omp.copy_constr(ptr %0, ptr %1)

; Function Attrs: noinline uwtable
declare void @_ZTS7int_int.omp.destr(ptr %0)

; Function Attrs: noinline uwtable
declare void @_ZTS7int_int.omp.copy_assign(ptr %0, ptr %1)

; Function Attrs: mustprogress noinline nounwind optnone uwtable
declare dso_local nonnull align 4 dereferenceable(8) ptr @_ZN7int_intaSERKS_(ptr nonnull align 4 dereferenceable(8) %this, ptr nonnull align 4 dereferenceable(8) %t1)


; Function Attrs: mustprogress noinline nounwind optnone uwtable
define void @test1(ptr %l.bnd, ptr %u.bnd, ptr %this1, ptr %y3) {
newFuncRoot:
  %y3.lpriv = alloca [12 x %struct.int_int], align 1
  %i.priv = alloca i32, align 4
  %lb.new = load i32, ptr %l.bnd, align 4
  %ub.new = load i32, ptr %u.bnd, align 4
  br label %omp.inner.for.body.preheader

omp.inner.for.body.preheader:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:NONPOD.TYPED"(ptr %y3.lpriv, %struct.int_int zeroinitializer, i32 12, ptr null, ptr @_ZTS7int_int.omp.copy_assign, ptr @_ZTS7int_int.omp.destr), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.priv, i32 0, i32 1, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.local.027 = phi i32 [ %add8, %omp.inner.for.inc ], [ %lb.new, %omp.inner.for.body.preheader ]
  store i32 %.omp.iv.local.027, ptr %i.priv, align 4
  br label %if.then

if.then:
  %1 = getelementptr inbounds %class.my_class, ptr %this1, i32 0, i32 1
  %2 = load ptr, ptr %1, align 8
  %3 = load i32, ptr %i.priv, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds [12 x %struct.int_int], ptr %2, i64 0, i64 %idxprom
  %4 = load i32, ptr %i.priv, align 4
  %idxprom6 = sext i32 %4 to i64
  %arrayidx7 = getelementptr inbounds [12 x %struct.int_int], ptr %y3.lpriv, i64 0, i64 %idxprom6
  %call = call nonnull align 4 dereferenceable(8) ptr @_ZN7int_intaSERKS_(ptr nonnull align 4 dereferenceable(8) %arrayidx7, ptr nonnull align 4 dereferenceable(8) %arrayidx) #0
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
