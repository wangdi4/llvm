; Check that VPlan HIR codegen can handle VPBasicBlocks with "and" instruction
; that has a single non-VPInstruction user.

; Incoming HIR
; <0>          BEGIN REGION { }
; <2>                %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
; <14>
; <14>               + DO i1 = 0, %n + -1, 1   <DO_LOOP> <simd>
; <5>                |   %and = %uni1  &  127;
; <14>               + END LOOP
; <14>
; <12>               @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; <0>          END REGION

; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=2 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -disable-output < %s 2>&1 | FileCheck %s


; CHECK:         BEGIN REGION { modified }
; CHECK:                 + DO i1 = 0, {{.*}}, 2   <DO_LOOP> <simd-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:             |   [[VEC:%.*]] = %uni1  &  127;
; CHECK-NEXT:             + END LOOP
; CHECK:                  %and = extractelement [[VEC]],  1;
; CHECK:         END REGION


define i32 @foo(i32 %uni1, i64 %n) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %if.end, %omp.inner.for.body.lr.ph
  %.omp.iv.local.010 = phi i64 [ 0, %omp.inner.for.body.lr.ph ], [ %add4, %omp.inner.for.body ]
  %and = and i32 %uni1, 127
  %add4 = add nuw nsw i64 %.omp.iv.local.010, 1
  %exitcond = icmp eq i64 %add4, %n
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %if.end
  %live.out = phi i32 [ %and, %omp.inner.for.body ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:
  ret i32 %live.out
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
