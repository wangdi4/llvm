; Test to verify that VPlan HIR vector codegen correctly replaces
; scalar complex type function with SVML variant in remainder loop.

; Incoming HIR:
;  BEGIN REGION { }
;        %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;        + DO i1 = 0, %n, 1   <DO_LOOP>
;        |   %exp = @cexpf((%A)[i1]);
;        |   (%A)[i1] = %exp;
;        + END LOOP
;        @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;  END REGION


; CHECK:         BEGIN REGION { modified }
; Main vector loop
; CHECK:               + DO i1 = 0, %loop.ub, 4   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:          |   %.vec3 = (<8 x float>*)(%A)[i1];
; CHECK-NEXT:          |   %__svml_cexpf4 = @__svml_cexpf4(%.vec3);
; CHECK-NEXT:          |   (<8 x float>*)(%A)[i1] = %__svml_cexpf4;
; CHECK-NEXT:          + END LOOP

; Scalar remainder loop
; CHECK:               + DO i1 = %lb.tmp, %n, 1   <DO_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 3> <vector-remainder> <nounroll> <novectorize> <max_trip_count = 3>
; CHECK-NEXT:          |   %load = (%A)[i1];
; CHECK-NEXT:          |   %__svml_cexpf1 = @__svml_cexpf1(%load);
; CHECK-NEXT:          |   %exp = shufflevector %__svml_cexpf1,  undef,  <i32 0, i32 1>;
; CHECK-NEXT:          |   (%A)[i1] = %exp;
; CHECK-NEXT:          + END LOOP

; CHECK:         END REGION

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vector-library=SVML -vplan-force-vf=4 -disable-output %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture noalias %A, i64 %n) {
entry:
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %loop ]
  %A.idx = getelementptr inbounds <2 x float>, ptr %A, i64 %iv
  %A.ld = load <2 x float>, ptr %A.idx, align 1
  %exp = call fast nofpclass(nan inf) <2 x float> @cexpf(<2 x float> noundef nofpclass(nan inf) %A.ld)
  store <2 x float> %exp, ptr %A.idx, align 1
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv, %n
  br i1 %exitcond, label %exit, label %loop

exit:
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x float> @cexpf(<2 x float>) local_unnamed_addr #0

attributes #0 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
