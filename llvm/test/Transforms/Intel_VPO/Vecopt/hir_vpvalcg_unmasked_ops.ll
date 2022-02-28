; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s

;
; Test to check that safe operations such as multiply remain unmasked in VPValue
; code generation. This is important for stability as well as performance of
; VPValue CG. In mixed CG mode, we can have multiple HLInsts that write to the
; same temp under different mask. Temps in HIR correspond to memory locations
; and an HLInst such as 't1 = val @Mask = m1' is lowered as:
;
;   ldval = load from t1 memory location
;   blendval = select m1, val, ldval
;   store blendval, t1 memory location
;
; This sequence is unnecessary in VPValue CG and can actually cause stability
; issues. Consider the following scalar HIR:
;
;    DO i1 = 0, 99, 1   <DO_LOOP> <simd>
;      %1 = (%unif)[0];
;      %2 = (@larr)[0][i1];
;      if (%2 != 0)
;      {
;          (@larr2)[0][(%n2 * %1)][i1] = i1;
;      }
;    END LOOP
;
; The value %n2 * %1 is uniform and the store to larr2 is unit strided. When
; trying to form the unit strided ref, we use scalar value for lane 0 for this
; uniform value. If the multiply remains masked, the value in this lane may
; be garbage if the mask for lane 0 is false. For operations other than calls,
; loads/stores, we need to generate unmasked operations for VPValue CG. This
; is essentially the same as what we do for LLVM IR path.
;
@larr = global [100 x i64] zeroinitializer, align 16
@larr2 = global [100 x [100 x i64]] zeroinitializer, align 16

define void @foo(i64* nocapture readonly %unif, i64 %n2) local_unnamed_addr #0 {
; CHECK:       DO i1 = 0, 99, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:    %.unifload = (%unif)[0];
; CHECK-NEXT:    %.vec = (<4 x i64>*)(@larr)[0][i1];
; CHECK-NEXT:    %.vec2 = %.vec != 0;
; CHECK-NEXT:    %.vec3 = %.unifload  *  %n2;
; CHECK-NEXT:    %.scal = %.unifload  *  %n2;
; CHECK-NEXT:    (<4 x i64>*)(@larr2)[0][%.scal][i1] = i1 + <i64 0, i64 1, i64 2, i64 3>, Mask = @{%.vec2};
; CHECK-NEXT:  END LOOP
;
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %if.end, %omp.inner.for.body.lr.ph
  %.omp.iv.local.010 = phi i64 [ 0, %omp.inner.for.body.lr.ph ], [ %add4, %if.end ]
  %1 = load i64, i64* %unif, align 8
  %arrayidx = getelementptr inbounds [100 x i64], [100 x i64]* @larr, i64 0, i64 %.omp.iv.local.010
  %2 = load i64, i64* %arrayidx, align 8
  %tobool = icmp eq i64 %2, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %mul1 = mul nsw i64 %1, %n2
  %arrayidx3 = getelementptr inbounds [100 x [100 x i64]], [100 x [100 x i64]]* @larr2, i64 0, i64 %mul1, i64 %.omp.iv.local.010
  store i64 %.omp.iv.local.010, i64* %arrayidx3, align 8
  br label %if.end

if.end:                                           ; preds = %omp.inner.for.body, %if.then
  %add4 = add nuw nsw i64 %.omp.iv.local.010, 1
  %exitcond = icmp eq i64 %add4, 100
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %if.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
