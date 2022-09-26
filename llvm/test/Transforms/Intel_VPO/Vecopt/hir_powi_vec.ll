; LIT test for fix in HIR vector codegen support for llvm.powi intrinsic call
; where the exponent argument is the result of a convert operation. The source
; and destination types of the canon expression corresponding to the exponent
; argument were being incorrectly forced to canon expression destination type's
; scalar type causing HIR verification errors.
;
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -print-after=hir-vplan-vec -hir-details -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -hir-details -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s


; CHECK:          @llvm.powi.v4f64.i16(%.vec,  %P)
; CHECK:          <RVAL-REG> LINEAR trunc.i32.i16(%P)
; CHECK-NEXT:        <BLOB> LINEAR i32 %P

declare double @llvm.powi.f64.i16(double %Val, i16 %power) nounwind readnone

define void @powi_f64(double* noalias nocapture readonly %y, i32 %P) local_unnamed_addr #2 {
entry:
  %P.CVT = trunc i32 %P to i16
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds double, double* %y, i64 %indvars.iv
  %0 = load double, double* %arrayidx, align 8
  %call = tail call double @llvm.powi.f64.i16(double %0, i16 %P.CVT) #4
  store double %call, double* %arrayidx, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
