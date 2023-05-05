; Check that we bail out from running SOA transformation on non-POD array private element type.
; See CMPLRLLVM-9193

; REQUIRES: asserts

; RUN: opt -S -passes=vplan-vec -vplan-force-vf=2 -vplan-enable-masked-variant=0 -vplan-enable-soa -vplan-dump-soa-info -disable-vplan-codegen -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec -vplan-force-vf=2 -vplan-enable-masked-variant=0 -vplan-enable-soa-hir -vplan-dump-soa-info\
; RUN: -disable-output  -disable-vplan-codegen -debug-only=LoopVectorizationPlannerHIR %s 2>&1 | FileCheck %s

; Also test opt-report remark when code gen is disabled.
; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -vplan-force-vf=2 -vplan-enable-masked-variant=0 -vplan-enable-soa -disable-vplan-codegen -disable-output -intel-opt-report=medium < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTMED
; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -vplan-force-vf=2 -vplan-enable-masked-variant=0 -vplan-enable-soa -disable-vplan-codegen -disable-output -intel-opt-report=high < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTHI
; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec,hir-optreport-emitter -vplan-force-vf=2 -vplan-enable-masked-variant=0 -vplan-enable-soa -disable-vplan-codegen -disable-output -intel-opt-report=high < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTHI-HIR

; CHECK: SOA profitability:
; CHECK:  SOAUnsafe = [[VP_Y3_LPRIV:%.*]] (y3.lpriv)
; CHECK:  SOASafe = [[VP_I_PRIV:%.*]] (i.priv) Profitable = 1

; OPTRPTMED: remark #15436: loop was not vectorized:
; OPTRPTHI: remark #15436: loop was not vectorized:
; OPTRPTHI: remark #15436: loop was not vectorized: Code generation is disabled.
; OPTRPTHI-HIR: remark #15436: loop was not vectorized: Code generation is disabled.


%struct.int_int = type { i32, i32 }
%struct.my_struct = type { [12 x %struct.int_int] }
%class.my_class = type { %struct.my_struct*, [12 x %struct.int_int]* }

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @_ZTS7int_int.omp.def_constr(%struct.int_int* %0)

declare void @_ZTS7int_int.omp.destr(%struct.int_int* %0)

declare dso_local nonnull align 4 dereferenceable(8) %struct.int_int* @_ZN7int_intaSERKS_(%struct.int_int* nonnull align 4 dereferenceable(8) %this, %struct.int_int* nonnull align 4 dereferenceable(8) %t1)

define void @test1(i32* %l.bnd, i32* %u.bnd, %class.my_class* %this1, [12 x %struct.int_int]* %y3) {
newFuncRoot:
  %y3.lpriv = alloca [12 x %struct.int_int], align 1
  %i.priv = alloca i32, align 4
  %lb.new = load i32, i32* %l.bnd, align 4
  %ub.new = load i32, i32* %u.bnd, align 4
  br label %omp.inner.for.body.preheader

omp.inner.for.body.preheader:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:NONPOD.TYPED"([12 x %struct.int_int]* %y3.lpriv, %struct.int_int zeroinitializer, i32 12, void (%struct.int_int*)* @_ZTS7int_int.omp.def_constr, void (%struct.int_int*)* @_ZTS7int_int.omp.destr), "QUAL.OMP.LINEAR:IV.TYPED"(i32* %i.priv, i32 0, i32 1, i32 1) ]
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
