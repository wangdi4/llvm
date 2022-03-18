; RUN: opt -hir-ssa-deconstruction -hir-dead-store-elimination -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir-framework>,hir-dead-store-elimination,print<hir-framework>" 2>&1 < %s | FileCheck %s
;
; RUN: opt -opaque-pointers -hir-ssa-deconstruction -hir-dead-store-elimination -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir-framework>,hir-dead-store-elimination,print<hir-framework>" 2>&1 < %s | FileCheck %s
;
; C Source Code:
;int A[50];
;int B[50];
;int foo(int N){
;  int i, j;
;  for(i = 0; i < N; i++){
;    A[i] = 0;
;    int t = 0;
;    for(j = 0; j < N; j++){
;      t = t + B[j];
;    }
;    A[i] = t;
;  }
;  return A[0];
;}
;
;*** IR Dump Before HIR Dead Store Elimination ***
;Function: foo
;
; CHECK:      BEGIN REGION { }
; CHECK:         + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50>
; CHECK:         |   (@A)[0][i1] = 0;
; CHECK:         |   %t.022 = 0;
; CHECK:         |
; CHECK:         |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50>
; CHECK:         |   |   %0 = (@B)[0][i2];
; CHECK:         |   |   %t.022 = %0  +  %t.022;
; CHECK:         |   + END LOOP
; CHECK:         |
; CHECK:         |   (@A)[0][i1] = %t.022;
; CHECK:         + END LOOP
; CHECK:   END REGION

;*** IR Dump After HIR Dead Store Elimination ***
;Function: foo

; CHECK:   BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50>
; CHECK-NOT:    |   (@A)[0][i1] = 0;
; CHECK:        |   %t.022 = 0;
; CHECK:        |
; CHECK:        |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50>
; CHECK:        |   |   %0 = (@B)[0][i2];
; CHECK:        |   |   %t.022 = %0  +  %t.022;
; CHECK:        |   + END LOOP
; CHECK:        |
; CHECK:        |   (@A)[0][i1] = %t.022;
; CHECK:        + END LOOP
; CHECK:  END REGION

; RUN: opt -hir-ssa-deconstruction -hir-dead-store-elimination -hir-cg -force-hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter < %s 2>&1 | FileCheck %s -check-prefix=OPTREPORT
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-dead-store-elimination,hir-cg,simplifycfg,intel-ir-optreport-emitter" -intel-opt-report=low -force-hir-cg 2>&1 < %s | FileCheck %s -check-prefix=OPTREPORT
;
; OPTREPORT:  LOOP BEGIN
; OPTREPORT:    remark #25529: Dead stores eliminated in loop
; OPTREPORT:    LOOP BEGIN
; OPTREPORT:    LOOP END
; OPTREPORT:  LOOP END
;
;Module Before HIR; ModuleID = 'simple.c'
source_filename = "simple.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [50 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [50 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo(i32 %N) local_unnamed_addr #0 {
entry:
  %cmp23 = icmp sgt i32 %N, 0
  br i1 %cmp23, label %for.body.lr.ph, label %for.end10

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body3.preheader

for.body3.preheader:                              ; preds = %for.body.lr.ph, %for.end
  %indvars.iv25 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next26, %for.end ]
  %arrayidx = getelementptr inbounds [50 x i32], [50 x i32]* @A, i64 0, i64 %indvars.iv25
  store i32 0, i32* %arrayidx, align 4, !tbaa !2
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.preheader
  %indvars.iv = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next, %for.body3 ]
  %t.022 = phi i32 [ 0, %for.body3.preheader ], [ %add, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [50 x i32], [50 x i32]* @B, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx5, align 4, !tbaa !2
  %add = add nsw i32 %0, %t.022
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body3

for.end:                                          ; preds = %for.body3
  %add.lcssa = phi i32 [ %add, %for.body3 ]
  store i32 %add.lcssa, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next26 = add nuw nsw i64 %indvars.iv25, 1
  %exitcond28 = icmp eq i64 %indvars.iv.next26, %wide.trip.count
  br i1 %exitcond28, label %for.end10.loopexit, label %for.body3.preheader

for.end10.loopexit:                               ; preds = %for.end
  br label %for.end10

for.end10:                                        ; preds = %for.end10.loopexit, %entry
  %1 = load i32, i32* getelementptr inbounds ([50 x i32], [50 x i32]* @A, i64 0, i64 0), align 16, !tbaa !2
  ret i32 %1
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 229c5f66f37f029e03e0da7a8a5111061e7a487b) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 8b4658f1b8470fdcc3c43c4efd901ee6399c748b)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA50_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
