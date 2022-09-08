; Check that opt-report is consistent with outgoing code emitted by vectorizer
; when vector loop is completely unrolled.

; RUN: opt %s -enable-new-pm=0 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-optreport-emitter -intel-opt-report=medium -vplan-force-vf=2 -print-after=hir-vplan-vec -disable-output 2>&1 | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-optreport-emitter" -intel-opt-report=medium -vplan-force-vf=2 -disable-output 2>&1 | FileCheck %s

; CHECK-LABEL: Function: full_unroll_vec_report
; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        %red.init = 0;
; CHECK-NEXT:        %red.init.insert = insertelement %red.init,  %hits.010,  0;
; CHECK-NEXT:        %phi.temp = %red.init.insert;
; CHECK-NEXT:        %.vec = (<2 x i32>*)(%points)[0];
; CHECK-NEXT:        %.vec1 = (<2 x i32>*)(%stars)[0];
; CHECK-NEXT:        %.vec2 = %phi.temp  +  %.vec + %.vec1;
; CHECK-NEXT:        %phi.temp = %.vec2;
; CHECK-NEXT:        %hits.010 = @llvm.vector.reduce.add.v2i32(%.vec2);
; CHECK-NEXT:  END REGION

; CHECK-LABEL: Report from: HIR Loop optimizations framework for : full_unroll_vec_report
; CHECK:       LOOP BEGIN
; CHECK-NEXT:      remark #15300: LOOP WAS VECTORIZED
; CHECK-NEXT:      remark #15305: vectorization support: vector length 2
; CHECK-NEXT:      remark #25436: Loop completely unrolled by 1
; CHECK-NEXT:  LOOP END

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree norecurse nosync nounwind readonly uwtable
define dso_local i32 @full_unroll_vec_report(i32* nocapture noundef readonly %points, i32* nocapture noundef readonly %stars) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %add3.lcssa = phi i32 [ %add3, %for.body ]
  ret i32 %add3.lcssa

for.body:                                         ; preds = %entry, %for.body
  %cmp = phi i1 [ true, %entry ], [ false, %for.body ]
  %indvars.iv = phi i64 [ 0, %entry ], [ 1, %for.body ]
  %hits.010 = phi i32 [ 0, %entry ], [ %add3, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %points, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %stars, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4
  %add = add nsw i32 %0, %1
  %add3 = add nsw i32 %hits.010, %add
  br i1 %cmp, label %for.body, label %for.cond.cleanup
}

attributes #0 = { "target-features"="+avx" }
