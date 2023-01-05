; Test to verify that VPlan's LiveInOutCreator framework doesn't crash
; for loops containing FP IV that was also recognized as a SafeReduction.
; In such cases, the FP IV is ignored during import process.

; HIR before vectorizer
;  BEGIN REGION { }
;        %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;        + DO i1 = 0, 999, 1   <DO_LOOP>
;        |   %expected_result.087 = %expected_result.087  +  1.000000e+00; <Safe Reduction>
;        + END LOOP
;
;        @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;  END REGION

; RUN: opt -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -disable-output < %s 2>&1 | FileCheck %s

; Function Attrs: nounwind uwtable
define dso_local float @test() local_unnamed_addr #3 {
; CHECK-LABEL: Function: test
; CHECK-EMPTY:
; CHECK-NEXT:  BEGIN REGION { modified }
; CHECK-NEXT:        %red.init = 0.000000e+00;
; CHECK-NEXT:        %phi.temp = %red.init;

; CHECK:             + DO i1 = 0, 999, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:        |   %.vec = %phi.temp  +  1.000000e+00;
; CHECK-NEXT:        |   %phi.temp = %.vec;
; CHECK-NEXT:        + END LOOP

; CHECK:             %expected_result.087 = @llvm.vector.reduce.fadd.v4f32(%expected_result.087,  %.vec);
; CHECK-NEXT:  END REGION
;
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %expected_result.087 = phi float [ 0.000000e+00, %entry ], [ %add10, %for.body ]
  %storemerge86 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %add10 = fadd fast float %expected_result.087, 1.000000e+00
  %inc = add nuw nsw i32 %storemerge86, 1
  %exitcond.not = icmp eq i32 %inc, 1000
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  %add10.lcssa = phi float [ %add10, %for.body ]
  ret float %add10.lcssa
}
