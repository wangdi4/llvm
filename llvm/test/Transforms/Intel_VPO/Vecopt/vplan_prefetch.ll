;
; LIT test to check that we do not widen/scalarize call to prefetch.
;
; RUN: opt -disable-output -passes='vplan-vec,simplifycfg,print' -vplan-print-after-call-vec-decisions -vplan-print-scalvec-results < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,LLVMCHECK
; RUN: opt -disable-output -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vplan-print-after-call-vec-decisions -vplan-print-scalvec-results < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,HIRCHECK
;
; CHECK-LABEL:   VPlan after CallVecDecisions analysis for merged CFG:
; CHECK:            [DA: Div] call ptr [[VPPTR:%.*]] i32 0 i32 3 i32 1 ptr @llvm.prefetch.p0
;
; CHECK-LABEL:   VPlan after ScalVec analysis:
; CHECK:            [DA: Div, SVA: (F  )] call ptr [[VPPTR]] i32 0 i32 3 i32 1 ptr @llvm.prefetch.p0 (SVAOpBits 0->F 1->F 2->F 3->F 4->F )
;
; LLVMCHECK-LABEL:   vector.body:
; LLVMCHECK-COUNT-1:       tail call void @llvm.prefetch.p0({{.*}})
;
; HIRCHECK:      DO i1 = 0, 1023, 4 <DO_LOOP>
; HIRCHECK-COUNT-1:      @llvm.prefetch.p0({{.*}})
;
define void @foo(ptr %arr, i64 %stride) local_unnamed_addr #0 {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:
  %l2.06 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %inc = add nuw nsw i64 %l2.06, 1
  %mul = mul nuw nsw i64 %stride, %inc
  %add.ptr = getelementptr inbounds i64, ptr %arr, i64 %mul
  tail call void @llvm.prefetch.p0(ptr nonnull %add.ptr, i32 0, i32 3, i32 1)
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.prefetch.p0(ptr, i32, i32, i32)
