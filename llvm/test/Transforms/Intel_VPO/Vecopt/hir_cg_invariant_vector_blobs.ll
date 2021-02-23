; Check that VPlan HIR vectorizer codegen can replicate loop-invariant vector
; blobs during re-vectorization of incoming HIR.

; Incoming HIR
;   BEGIN REGION { }
;         %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;         + DO i1 = 0, 99, 1   <DO_LOOP>
;         |   %add = <float 4.000000e+00, float 6.000000e+00>  +  %inv.temp;
;         |   (<2 x float>*)(%dst)[0][i1] = %add;
;         + END LOOP
;
;         @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;   END REGION

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -print-after=VPlanDriverHIR -vplan-force-vf=2 -disable-output < %s 2>&1 | FileCheck %s

; CHECK-LABEL: Function: main
; CHECK:        BEGIN REGION { modified }
; CHECK-NEXT:            %.replicated = shufflevector %inv.temp,  undef,  <i32 0, i32 1, i32 0, i32 1>;
; CHECK-NEXT:         + DO i1 = 0, 99, 2   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:         |   %.vec = <float 4.000000e+00, float 6.000000e+00, float 4.000000e+00, float 6.000000e+00>  +  %.replicated;
; CHECK-NEXT:         |   (<4 x float>*)(%dst)[0][i1] = %.vec;
; CHECK-NEXT:         + END LOOP
; CHECK-NEXT:   END REGION

%class.complex = type { float, float }

define dso_local void @main(<2 x float> %inv.temp) local_unnamed_addr #0 {
entry:
  %dst = alloca [100 x %class.complex], align 16
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]
  %ptr = getelementptr inbounds [100 x %class.complex], [100 x %class.complex]* %dst, i64 0, i64 %iv
  %ptr.bc = bitcast %class.complex* %ptr to <2 x float>*
  %add = fadd <2 x float> <float 4.000000e+00, float 6.000000e+00>, %inv.temp
  store <2 x float> %add, <2 x float>* %ptr.bc, align 8
  %iv.next = add nuw nsw i64 %iv, 1
  %iv.cmp = icmp eq i64 %iv.next, 100
  br i1 %iv.cmp, label %exit, label %header

exit:
  ret void
}
