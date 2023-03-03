; LIT test to check that we do not crash in VPlan unroller when we force an unroll factor for
; outer loops. Unroller needs to add support for outer loops.
;
; RUN: opt -disable-output -passes='vplan-vec,print' -vplan-force-uf=2 -vplan-force-vf=4 < %s 2>&1 | FileCheck %s --check-prefix=IRCHECK
; RUN: opt -disable-output -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vplan-force-uf=2 -vplan-force-vf=4 < %s 2>&1 | FileCheck %s --check-prefix=HIRCHECK


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = dso_local local_unnamed_addr global [1024 x [1024 x i64]] zeroinitializer, align 16

define void @foo() {
; IRCHECK:       vector.body:
; IRCHECK:         [[VEC_PHI:%.*]] = phi <4 x i64> [ <i64 0, i64 1, i64 2, i64 3>, [[VPLANNEDBB1:%.*]] ], [ [[TMP3:%.*]], [[VPLANNEDBB5:%.*]] ]
; IRCHECK:         [[TMP3]] = add nuw nsw <4 x i64> [[VEC_PHI]], <i64 4, i64 4, i64 4, i64 4>
;
; HIRCHECK:      DO i1 = 0, 1023, 4   <DO_LOOP> <simd-vectorized> <novectorize>
;
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc5
  %l1.015 = phi i64 [ 0, %entry ], [ %inc6, %for.inc5 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %l2.014 = phi i64 [ 0, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %add = add nuw nsw i64 %l2.014, %l1.015
  %arrayidx4 = getelementptr inbounds [1024 x [1024 x i64]], ptr @arr, i64 0, i64 %l2.014, i64 %l1.015
  store i64 %add, ptr %arrayidx4, align 8
  %inc = add nuw nsw i64 %l2.014, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.inc5, label %for.body3

for.inc5:                                         ; preds = %for.body3
  %inc6 = add nuw nsw i64 %l1.015, 1
  %exitcond16.not = icmp eq i64 %inc6, 1024
  br i1 %exitcond16.not, label %for.end7, label %for.cond1.preheader

for.end7:                                         ; preds = %for.inc5
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
