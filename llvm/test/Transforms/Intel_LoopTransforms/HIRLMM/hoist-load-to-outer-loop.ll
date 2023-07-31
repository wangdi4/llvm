; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,hir-lmm,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
;
; C Source Code:
;
;int A[100];
;int B[100];
;void foo() {
;
;  int i, j, k;
;
;  for (i=0; i< 10; i++) {
;    for (j=0; j< 20; j++) {
;      #pragma unroll 5
;      for(k=0;k<5; k++) {
;        A[i+j+k] += B[k];
;      }
;    }
;  }
;}
;
;*** IR Dump Before HIR Loop Memory Motion ***
;Function: foo
;
;<0>          BEGIN REGION { modified }
;<36>               + DO i1 = 0, 9, 1   <DO_LOOP>
;<37>               |   + DO i2 = 0, 19, 1   <DO_LOOP>
;<40>               |   |   %1 = (@B)[0][0];
;<41>               |   |   %3 = (@A)[0][i1 + i2];
;<42>               |   |   (@A)[0][i1 + i2] = %1 + %3;
;<43>               |   |   %1 = (@B)[0][1];
;<44>               |   |   %3 = (@A)[0][i1 + i2 + 1];
;<45>               |   |   (@A)[0][i1 + i2 + 1] = %1 + %3;
;<46>               |   |   %1 = (@B)[0][2];
;<47>               |   |   %3 = (@A)[0][i1 + i2 + 2];
;<48>               |   |   (@A)[0][i1 + i2 + 2] = %1 + %3;
;<49>               |   |   %1 = (@B)[0][3];
;<50>               |   |   %3 = (@A)[0][i1 + i2 + 3];
;<51>               |   |   (@A)[0][i1 + i2 + 3] = %1 + %3;
;<10>               |   |   %1 = (@B)[0][4];
;<13>               |   |   %3 = (@A)[0][i1 + i2 + 4];
;<15>               |   |   (@A)[0][i1 + i2 + 4] = %1 + %3;
;<37>               |   + END LOOP
;<36>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Loop Memory Motion ***
;Function: foo
;
;CHECK:      BEGIN REGION { modified }
;CHECK:               %[[LIMM:limm[0-9]*]] = (@B)[0][0];
;CHECK:               %[[LIMM2:limm[0-9]*]] = (@B)[0][1];
;CHECK:               %[[LIMM3:limm[0-9]*]] = (@B)[0][2];
;CHECK:               %[[LIMM4:limm[0-9]*]] = (@B)[0][3];
;CHECK:               %[[LIMM5:limm[0-9]*]] = (@B)[0][4];
;CHECK:            + DO i1 = 0, 9, 1   <DO_LOOP>
;CHECK:            |   + DO i2 = 0, 19, 1   <DO_LOOP>
;CHECK:            |   |   %1 = %[[LIMM]];
;CHECK:            |   |   %3 = (@A)[0][i1 + i2];
;CHECK:            |   |   (@A)[0][i1 + i2] = %1 + %3;
;CHECK:            |   |   %1 = %[[LIMM2]];
;CHECK:            |   |   %3 = (@A)[0][i1 + i2 + 1];
;CHECK:            |   |   (@A)[0][i1 + i2 + 1] = %1 + %3;
;CHECK:            |   |   %1 = %[[LIMM3]];
;CHECK:            |   |   %3 = (@A)[0][i1 + i2 + 2];
;CHECK:            |   |   (@A)[0][i1 + i2 + 2] = %1 + %3;
;CHECK:            |   |   %1 = %[[LIMM4]];
;CHECK:            |   |   %3 = (@A)[0][i1 + i2 + 3];
;CHECK:            |   |   (@A)[0][i1 + i2 + 3] = %1 + %3;
;CHECK:            |   |   %1 = %[[LIMM5]];
;CHECK:            |   |   %3 = (@A)[0][i1 + i2 + 4];
;CHECK:            |   |   (@A)[0][i1 + i2 + 4] = %1 + %3;
;CHECK:            |   + END LOOP
;CHECK:            + END LOOP
;CHECK:      END REGION

;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc14, %entry
  %indvars.iv35 = phi i64 [ 0, %entry ], [ %indvars.iv.next36, %for.inc14 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc11, %for.cond1.preheader
  %indvars.iv31 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next32, %for.inc11 ]
  %0 = add nuw nsw i64 %indvars.iv31, %indvars.iv35
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %2 = add nuw nsw i64 %0, %indvars.iv
  %arrayidx9 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %2, !intel-tbaa !2
  %3 = load i32, ptr %arrayidx9, align 4, !tbaa !2
  %add10 = add nsw i32 %3, %1
  store i32 %add10, ptr %arrayidx9, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.inc11, label %for.body6, !llvm.loop !7

for.inc11:                                        ; preds = %for.body6
  %indvars.iv.next32 = add nuw nsw i64 %indvars.iv31, 1
  %exitcond34 = icmp eq i64 %indvars.iv.next32, 20
  br i1 %exitcond34, label %for.inc14, label %for.cond4.preheader

for.inc14:                                        ; preds = %for.inc11
  %indvars.iv.next36 = add nuw nsw i64 %indvars.iv35, 1
  %exitcond37 = icmp eq i64 %indvars.iv.next36, 10
  br i1 %exitcond37, label %for.end16, label %for.cond1.preheader

for.end16:                                        ; preds = %for.inc14
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.unroll.count", i32 5}
