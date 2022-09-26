; Check that VPlan HIR vectorizer codegen generates correct sequence of instructions
; to revectorize bitcasts and handle uniform store optimization for VectorType stores
; seen in incoming HIR.

; Incoming HIR
;   BEGIN REGION { }
;         %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;         + DO i1 = 0, 99, 1   <DO_LOOP> <ivdep>
;         |   %ld = (%src)[i1];
;         |   %insert = insertelement poison,  %ld,  0;
;         |   %bc = bitcast.<2 x i64>.<2 x double>(%insert);
;         |   (%dest)[0] = %bc;
;         + END LOOP
;
;         @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;   END REGION

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s

; CHECK-LABEL: Function: foo
; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        + DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize> <ivdep>
; CHECK-NEXT:        |   %.vec = (<4 x i64>*)(%src)[i1];
; CHECK-NEXT:        |   %wide.insert = shufflevector %.vec,  undef,  <i32 0, i32 undef, i32 1, i32 undef, i32 2, i32 undef, i32 3, i32 undef>;
; CHECK-NEXT:        |   %.vec1 = bitcast.<8 x i64>.<8 x double>(%wide.insert);
; CHECK-NEXT:        |   %extractsubvec. = shufflevector %.vec1,  undef,  <i32 6, i32 7>;
; CHECK-NEXT:        |   (%dest)[0] = %extractsubvec.;
; CHECK-NEXT:        + END LOOP
; CHECK:       END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @foo(<2 x double>* nocapture %dest, i64* nocapture readonly %src) local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.010 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %gep = getelementptr inbounds i64, i64* %src, i64 %l1.010
  %ld = load i64, i64* %gep, align 8
  %insert = insertelement <2 x i64> poison, i64 %ld, i32 0
  %bc = bitcast <2 x i64> %insert to <2 x double>
  store <2 x double> %bc, <2 x double>* %dest, align 8
  %inc = add nuw nsw i64 %l1.010, 1
  %exitcond.not = icmp eq i64 %inc, 100
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !6

for.end:                                          ; preds = %for.body
  ret void
}

!6 = distinct !{!6, !7, !8}
!7 = !{!"llvm.loop.mustprogress"}
!8 = !{!"llvm.loop.vectorize.ivdep_back"}
