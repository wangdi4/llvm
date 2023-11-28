;
; Test for registerized last private of a vector type. Bailout for now.
;
; RUN: opt %s -disable-output -passes="vplan-vec" -debug-only=VPlanLegality 2>&1 | FileCheck %s
; RUN: opt %s -disable-output -passes=vplan-vec,intel-ir-optreport-emitter -intel-opt-report=medium 2>&1 | FileCheck %s --check-prefix=OPTRPTMED
; TODO: Enable test for HIR when vectors are supported by loopopt
; R_UN: opt %s -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec" -debug-only=LoopVectorizationPlanner 2>&1 | FileCheck %s

; CHECK: LV: Found an unidentified PHI.  %priv_phi = phi <2 x i64>
; CHECK-NOT: <16 x i8>
; OPTRPTMED: remark #15571: simd loop was not vectorized: loop contains a recurrent computation that could not be identified as an induction or reduction. Try using #pragma omp simd reduction/linear/private to clarify recurrence.
;
define <2 x i64> @foo(ptr nocapture %larr, ptr %mm) {
entry:
  %m1 = load <2 x i64>, ptr %mm
  br label %b1

b1:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i64 2) ]
  br label %for.body

for.body:
  %l1.010 = phi i64 [ 0, %b1 ], [ %inc, %else ]
  %priv_phi = phi <2 x i64> [ %m1, %b1 ], [ %merge, %else ]
  %arrayidx = getelementptr inbounds <2 x i64>, ptr %larr, i64 %l1.010
  %cmp = icmp eq i64 %l1.010, 100
  br i1 %cmp, label %then, label %else

then:
  %0 = load <2 x i64>, ptr %arrayidx, align 8
  br label %else

else:
  %merge = phi <2 x i64> [ %priv_phi, %for.body ], [ %0, %then ]
  %inc = add nuw nsw i64 %l1.010, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  %lcssa.merge =  phi <2 x i64> [%merge, %else]
  store <2 x i64> %lcssa.merge, ptr %mm
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret <2 x i64> %lcssa.merge
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
