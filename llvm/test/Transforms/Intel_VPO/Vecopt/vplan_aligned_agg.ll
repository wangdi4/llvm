; Checks that vectorizer will not perform vector peeling for "#pragma vector aligned"
; And will generate aligned stores/loads for "#pragma vector aligned" for aggregate type.
; The main purpose of the test is to basically trigger execution of one line in
; llvm/lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanAlignmentAnalysis.cpp
; VPlanAlignmentAnalysis::getAlignmentUnitStride
;   ...
;   return Memref.getAlignment();
;   ...
; to increase coverage for CMPLRLLVM-10525 feature.

; RUN: opt < %s -S -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-cg' | FileCheck %s

; CHECK: %pR_fetch = load %complex_64bit, ptr %pR, align 4
; CHECK: store %complex_64bit %pR_fetch, ptr %pS, align 4
; CHECK-NOT: vector-peel

%complex_64bit = type { float, float }

@pR = internal unnamed_addr global [100 x %complex_64bit] zeroinitializer, align 16
@pS = internal unnamed_addr global [100 x %complex_64bit] zeroinitializer, align 16

define void @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:
  %p0 = phi i32 [ 0, %entry ], [ %add21, %for.body ]
  %int_sext17 = sext i32 %p0 to i64
  %pR = getelementptr inbounds [100 x %complex_64bit], ptr @pR, i32 0, i64 %int_sext17
  %pR_fetch = load %complex_64bit, ptr %pR
  %pS = getelementptr inbounds [100 x %complex_64bit], ptr @pS, i32 0, i64 %int_sext17
  store %complex_64bit %pR_fetch, ptr %pS
  %add21 = add nsw i32 %p0, 1
  %rel = icmp sle i32 %add21, 5
  br i1 %rel, label %for.body, label %for.end, !llvm.loop !0

for.end:
  ret void
}

!0 = distinct !{!0, !1, !2}
!1 = !{!"llvm.loop.mustprogress"}
!2 = !{!"llvm.loop.intel.vector.aligned"}
