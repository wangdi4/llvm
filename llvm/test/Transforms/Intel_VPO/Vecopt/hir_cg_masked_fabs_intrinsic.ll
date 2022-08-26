; Test to check that masked fabs intrinsic calls are vectorized or not vectorized in HIR vector CG,
; based  on the target features being compiled for.

; HIR incoming to vectorizer
; <0>     BEGIN REGION { }
; <22>          %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; <21>
; <21>          + DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; <3>           |   %0 = (%y)[i1];
; <5>           |   if (%0 == %key)
; <5>           |   {
; <9>           |      %call = @llvm.fabs.f64(%0);
; <11>          |      (%x)[i1] = %call;
; <5>           |   }
; <21>          + END LOOP
; <21>
; <23>          @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; <0>     END REGION

; RUN: opt -S -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -hir-cg -print-after=hir-vplan-vec -mtriple=x86_64-unknown-unknown -mattr=+avx2 -enable-intel-advanced-opts < %s 2>&1 | FileCheck %s --check-prefix=VEC
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -S -vplan-force-vf=4 -mtriple=x86_64-unknown-unknown -mattr=+avx2 -enable-intel-advanced-opts < %s 2>&1 | FileCheck %s --check-prefix=VEC

; RUN: opt -S -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -hir-cg -print-after=hir-vplan-vec -mtriple=x86_64-unknown-unknown -mattr=+avx2 -enable-intel-advanced-opts -mcpu=alderlake < %s 2>&1 | FileCheck %s --check-prefix=NOVEC
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -S -vplan-force-vf=4 -mtriple=x86_64-unknown-unknown -mattr=+avx2 -enable-intel-advanced-opts -mcpu=alderlake < %s 2>&1 | FileCheck %s --check-prefix=NOVEC


; Check that loop is vectorized if compiled for AVX2 target.
; VEC:            + DO i1 = 0, {{.*}}, 4   <DO_LOOP>
; VEC-NEXT:       |   [[LD:%.*]] = (<4 x double>*)(%y)[i1];
; VEC-NEXT:       |   [[CMP:%.*]] = [[LD]] == %key;
; VEC-NEXT:       |   %llvm.fabs.v4f64 = @llvm.fabs.v4f64([[LD]])
; VEC-NEXT:       |   (<4 x double>*)(%x)[i1] = %llvm.fabs.v4f64, Mask = @{[[CMP]]}
; VEC-NEXT:       + END LOOP

; Check generated LLVM call after HIR-CG
; VEC: [[VEC_CALL:%.*]] = call fast <4 x double> @llvm.fabs.v4f64(<4 x double> {{%.*}})


; Check that loop was not vectorized if compiled for Alderlake target.
; NOVEC:          + DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; NOVEC-NEXT:     |   %0 = (%y)[i1];
; NOVEC-NEXT:     |   if (%0 == %key)
; NOVEC-NEXT:     |   {
; NOVEC-NEXT:     |      %call = @llvm.fabs.f64(%0);
; NOVEC-NEXT:     |      (%x)[i1] = %call;
; NOVEC-NEXT:     |   }
; NOVEC-NEXT:     + END LOOP

declare double @llvm.fabs.f64(double %Val) nounwind readnone

define void @powi_f64(i32 %n, double* noalias nocapture readonly %y, double* noalias nocapture %x, i32 %P, double %key) local_unnamed_addr #2 {
entry:
  %cmp9 = icmp sgt i32 %n, 0
  br i1 %cmp9, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %call.continue ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds double, double* %y, i64 %indvars.iv
  %0 = load double, double* %arrayidx, align 8
  %cmp = fcmp fast oeq double %0, %key
  br i1 %cmp, label %masked.call, label %call.continue

masked.call:
  %call = tail call fast double @llvm.fabs.f64(double %0) #4
  %arrayidx4 = getelementptr inbounds double, double* %x, i64 %indvars.iv
  store double %call, double* %arrayidx4, align 8
  br label %call.continue

call.continue:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
