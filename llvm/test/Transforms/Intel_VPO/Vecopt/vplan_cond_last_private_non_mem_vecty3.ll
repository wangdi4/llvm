;
; Test for registerized last private of a vector type. Bailout for now.
;
; RUN: opt %s -disable-output -passes="vplan-vec" -debug-only=LoopVectorizationPlanner 2>&1 | FileCheck %s
; TODO: Enable test for HIR when vectors are supported by loopopt
; R_UN: opt %s -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec" -debug-only=LoopVectorizationPlanner 2>&1 | FileCheck %s

; CHECK: LVP: Unrecognized phi found.
; CHECK: LVP: VPlan is not legal to process, bailing out.
; CHECK-NOT: <16 x i8>
;
define <2 x i64> @foo(<2 x i64>* nocapture %larr, <2 x i64>* %mm) {
entry:
  %m1 = load <2 x i64>, <2 x i64>* %mm
  br label %b1

b1:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:TYPED"(<2 x i64>* %mm, <2 x i64> zeroinitializer, i32 1), "QUAL.OMP.SIMDLEN"(i64 2) ]
  br label %for.body

for.body:
  %l1.010 = phi i64 [ 0, %b1 ], [ %inc, %else ]
  %priv_phi = phi <2 x i64> [ %m1, %b1 ], [ %merge, %else ]
  %arrayidx = getelementptr inbounds <2 x i64>, <2 x i64>* %larr, i64 %l1.010
  %cmp = icmp eq i64 %l1.010, 100
  br i1 %cmp, label %then, label %else

then:
  %0 = load <2 x i64>, <2 x i64>* %arrayidx, align 8
  br label %else

else:
  %merge = phi <2 x i64> [ %priv_phi, %for.body ], [ %0, %then ]
  %inc = add nuw nsw i64 %l1.010, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  %lcssa.merge =  phi <2 x i64> [%merge, %else]
  store <2 x i64> %lcssa.merge, <2 x i64>* %mm
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret <2 x i64> %lcssa.merge
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
