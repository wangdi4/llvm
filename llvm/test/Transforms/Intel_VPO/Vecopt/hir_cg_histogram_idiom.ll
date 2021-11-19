; Test to check HIR vectorizer CG support for histogram idiom.

; RUN: opt -mattr=+avx512vl,+avx512cd -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s

; CHECK-LABEL:   BEGIN REGION { modified }
; CHECK-NEXT:          + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:          |   %.vec = (<4 x i32>*)(%B)[i1];
; CHECK-NEXT:          |   %.vec1 = (<4 x float>*)(%A)[%.vec];
; CHECK-NEXT:          |   %conflicts = @llvm.x86.avx512.conflict.q.256(%.vec);
; CHECK-NEXT:          |   %llvm.ctpop.v4i64 = @llvm.ctpop.v4i64(%conflicts);
; CHECK-NEXT:          |   %.vec2 = sitofp.<4 x i64>.<4 x float>(%llvm.ctpop.v4i64);
; CHECK-NEXT:          |   %.vec3 = %.vec2  +  1.000000e+00;
; CHECK-NEXT:          |   %.vec4 = %.vec3  *  2.000000e+00;
; CHECK-NEXT:          |   %.vec5 = %.vec1  +  %.vec4;
; CHECK-NEXT:          |   (<4 x float>*)(%A)[%.vec] = %.vec5;
; CHECK-NEXT:          + END LOOP

; CHECK:               + DO i1 = 1024, 1026, 1   <DO_LOOP> <novectorize>
; CHECK-NEXT:          |   %0 = (%B)[i1];
; CHECK-NEXT:          |   %add = (%A)[%0]  +  2.000000e+00;
; CHECK-NEXT:          |   (%A)[%0] = %add;
; CHECK-NEXT:          + END LOOP
; CHECK-NEXT:    END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @foo1(float* noalias nocapture %A, i32* noalias nocapture readonly %B) local_unnamed_addr #0 {
entry:
;   for (int i=0; i<N; i++){
;     index = B[i];
;     A[index] = A[index] + 2.0;
;   }
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %0 = load i32, i32* %ptridx, align 4
  %idxprom1 = sext i32 %0 to i64
  %ptridx2 = getelementptr inbounds float, float* %A, i64 %idxprom1
  %1 = load float, float* %ptridx2, align 4
  %add = fadd fast float %1, 2.000000e+00
  store float %add, float* %ptridx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1027
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

