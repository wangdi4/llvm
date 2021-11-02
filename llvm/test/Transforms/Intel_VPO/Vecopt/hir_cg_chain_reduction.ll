; Test to check correctness of generated vector code for chain reductions.

; HIR incoming to vectorizer:
; <0>     BEGIN REGION { }
; <22>          %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; <21>
; <21>          + DO i1 = 0, 1023, 1   <DO_LOOP>
; <5>           |   %conv = sitofp.i32.float(i1);
; <8>           |   %mul = (@b)[0][i1]  *  %conv;
; <11>          |   %add = %add4  +  %sum.021; <Safe Reduction>
; <12>          |   %add5 = %add  +  (@a)[0][i1]; <Safe Reduction>
; <13>          |   %sub = %add5  +  (@c)[0][i1]; <Safe Reduction>
; <14>          |   %sum.021 = %sub  +  %mul; <Safe Reduction>
; <21>          + END LOOP
; <21>
; <23>          @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; <0>     END REGION

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -vplan-print-after-initial-transforms -vplan-entities-dump -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-print-after-initial-transforms -vplan-entities-dump -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s


; Check that reduction is imported as VPReduction.
; CHECK:      Reduction list
; CHECK-NEXT:  (+) Start: float %sum.021 Exit: float [[LAST_UPDATE:%vp.*]]

; TODO: Update test to check that other instructions of reduction chain are in LinkedVPValues.

; Check generated vector code.
; CHECK:                     %red.init = 0.000000e+00;
; CHECK:                     %phi.temp = %red.init;
; CHECK:                     + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:                |   %.vec = sitofp.<4 x i32>.<4 x float>(i1 + <i64 0, i64 1, i64 2, i64 3>);
; CHECK-NEXT:                |   %.vec1 = (<4 x float>*)(@b)[0][i1];
; CHECK-NEXT:                |   %.vec2 = %.vec1  *  %.vec;
; CHECK-NEXT:                |   %.vec3 = %add4  +  %phi.temp;
; CHECK-NEXT:                |   %.vec4 = (<4 x float>*)(@a)[0][i1];
; CHECK-NEXT:                |   %.vec5 = %.vec3  +  %.vec4;
; CHECK-NEXT:                |   %.vec6 = (<4 x float>*)(@c)[0][i1];
; CHECK-NEXT:                |   %.vec7 = %.vec5  +  %.vec6;
; CHECK-NEXT:                |   %.vec8 = %.vec7  +  %.vec2;
; CHECK-NEXT:                |   %phi.temp = %.vec8;
; CHECK-NEXT:                + END LOOP
; CHECK:                     %sum.021 = @llvm.vector.reduce.fadd.v4f32(%sum.021,  %.vec8);


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16
@c = common dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local float @foo(i32 %m, i32 %n) local_unnamed_addr {
entry:
  %conv3 = sitofp i32 %m to float
  %conv8 = sitofp i32 %n to float
  %add4 = fsub fast float %conv3, %conv8
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %sum.021 = phi float [ 0.000000e+00, %entry ], [ %add9, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @a, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load float, float* %arrayidx, align 4, !tbaa !2
  %1 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %1 to float
  %arrayidx2 = getelementptr inbounds [1024 x float], [1024 x float]* @b, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %2 = load float, float* %arrayidx2, align 4, !tbaa !2
  %mul = fmul fast float %2, %conv
  %arrayidx7 = getelementptr inbounds [1024 x float], [1024 x float]* @c, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %3 = load float, float* %arrayidx7, align 4, !tbaa !2
  %add = fadd fast float %add4, %sum.021
  %add5 = fadd fast float %add, %0
  %sub = fadd fast float %add5, %3
  %add9 = fadd fast float %sub, %mul
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add9.lcssa = phi float [ %add9, %for.body ]
  ret float %add9.lcssa
}

!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
