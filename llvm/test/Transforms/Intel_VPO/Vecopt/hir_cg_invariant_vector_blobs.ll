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

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -vplan-force-vf=2 -hir-details -disable-output < %s 2>&1 | FileCheck %s

%class.complex = type { float, float }

; CHECK-LABEL: Function: main
; CHECK:        BEGIN REGION { modified }
; CHECK:              + DO i64 i1 = 0, 99, 2   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK:              |   %.replicated = shufflevector %inv.temp,  undef,  <i32 0, i32 1, i32 0, i32 1>;
; CHECK:              |   %.vec = <float 4.000000e+00, float 6.000000e+00, float 4.000000e+00, float 6.000000e+00>  +  %.replicated;
; CHECK:              |   (<4 x float>*)(%dst)[0][i1] = %.vec;
; CHECK:              + END LOOP
; CHECK:        END REGION

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

; Incoming HIR
;   BEGIN REGION { }
;         + DO i1 = 0, 99, 1   <DO_LOOP>
;         |   %inv.temp = (%inv.addr)[0];
;         |   %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;         |
;         |   + DO i2 = 0, 99, 1   <DO_LOOP>
;         |   |   %add = <float 4.000000e+00, float 6.000000e+00>  +  %inv.temp;
;         |   |   (<2 x float>*)(%dst)[0][i2] = %add;
;         |   + END LOOP
;         |
;         |   @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;         + END LOOP
;   END REGION

; CHECK-LABEL: Function: main2
; CHECK:        BEGIN REGION { modified }
; CHECK:              + DO i64 i1 = 0, 99, 1   <DO_LOOP>
; CHECK:              |   %inv.temp = (%inv.addr)[0];
; CHECK:              |
; CHECK:              |   + DO i64 i2 = 0, 99, 2   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK:              |   |   %.replicated = shufflevector %inv.temp,  undef,  <i32 0, i32 1, i32 0, i32 1>;
; CHECK:              |   |   <LVAL-REG> NON-LINEAR <4 x float> %.replicated
; CHECK:              |   |   <RVAL-REG> NON-LINEAR <2 x float> %inv.temp
; CHECK:              |   |
; CHECK:              |   |   %.vec = <float 4.000000e+00, float 6.000000e+00, float 4.000000e+00, float 6.000000e+00>  +  %.replicated;
; CHECK:              |   |   (<4 x float>*)(%dst)[0][i2] = %.vec;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:        END REGION

define dso_local void @main2(<2 x float>* %inv.addr) local_unnamed_addr #0 {
entry:
  %dst = alloca [100 x %class.complex], align 16
  br label %outer

outer:
  %out.iv = phi i64 [ 0, %entry ], [ %out.iv.next, %outer.exit ]
  %inv.temp = load <2 x float>, <2 x float>* %inv.addr, align 8
  br label %header

header:
  %iv = phi i64 [ 0, %outer ], [ %iv.next, %header ]
  %ptr = getelementptr inbounds [100 x %class.complex], [100 x %class.complex]* %dst, i64 0, i64 %iv
  %ptr.bc = bitcast %class.complex* %ptr to <2 x float>*
  %add = fadd <2 x float> <float 4.000000e+00, float 6.000000e+00>, %inv.temp
  store <2 x float> %add, <2 x float>* %ptr.bc, align 8
  %iv.next = add nuw nsw i64 %iv, 1
  %iv.cmp = icmp eq i64 %iv.next, 100
  br i1 %iv.cmp, label %outer.exit, label %header

outer.exit:
  %out.iv.next = add nuw nsw i64 %out.iv, 1
  %out.iv.cmp = icmp eq i64 %out.iv.next, 100
  br i1 %out.iv.cmp, label %exit, label %outer

exit:
  ret void
}
