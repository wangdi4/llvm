; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,hir-lmm,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
;
; C Source Code:
;
;int A[100];
;int B[100];
;void foo() {
;
;  int n, i, j, k;
;  for(n = 0; n < 10; n++){
;    B[5]++;
;    for (i=0; i< 10; i++) {
;      for (j=0; j< 20; j++) {
;        #pragma unroll 5
;        for(k=0;k<5; k++) {
;          A[i+j+k] += B[k];
;        }
;      }
;    }
;  }
;}
;*** IR Dump Before HIR Loop Memory Motion ***
;Function: foo
;
;<0>          BEGIN REGION { modified }
;<49>               + DO i1 = 0, 9, 1   <DO_LOOP>
;<3>                |   (@B)[0][5] = i1 + %.pre + 1;
;<50>               |
;<50>               |   + DO i2 = 0, 9, 1   <DO_LOOP>
;<51>               |   |   + DO i3 = 0, 19, 1   <DO_LOOP>
;<54>               |   |   |   %2 = (@B)[0][0];
;<55>               |   |   |   %4 = (@A)[0][i2 + i3];
;<56>               |   |   |   (@A)[0][i2 + i3] = %2 + %4;
;<57>               |   |   |   %2 = (@B)[0][1];
;<58>               |   |   |   %4 = (@A)[0][i2 + i3 + 1];
;<59>               |   |   |   (@A)[0][i2 + i3 + 1] = %2 + %4;
;<60>               |   |   |   %2 = (@B)[0][2];
;<61>               |   |   |   %4 = (@A)[0][i2 + i3 + 2];
;<62>               |   |   |   (@A)[0][i2 + i3 + 2] = %2 + %4;
;<63>               |   |   |   %2 = (@B)[0][3];
;<64>               |   |   |   %4 = (@A)[0][i2 + i3 + 3];
;<65>               |   |   |   (@A)[0][i2 + i3 + 3] = %2 + %4;
;<15>               |   |   |   %2 = (@B)[0][4];
;<18>               |   |   |   %4 = (@A)[0][i2 + i3 + 4];
;<20>               |   |   |   (@A)[0][i2 + i3 + 4] = %2 + %4;
;<51>               |   |   + END LOOP
;<50>               |   + END LOOP
;<49>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Loop Memory Motion ***
;Function: foo
;
;CHECK:        BEGIN REGION { modified }
;CHECK:               %[[LIMM:limm[0-9]*]] = (@B)[0][0];
;CHECK:               %[[LIMM2:limm[0-9]*]] = (@B)[0][1];
;CHECK:               %[[LIMM3:limm[0-9]*]] = (@B)[0][2];
;CHECK:               %[[LIMM4:limm[0-9]*]] = (@B)[0][3];
;CHECK:               %[[LIMM5:limm[0-9]*]] = (@B)[0][4];
;CHECK:            + DO i1 = 0, 9, 1   <DO_LOOP>
;CHECK:            |   (@B)[0][5] = i1 + %.pre + 1;
;CHECK:            |
;CHECK:            |   + DO i2 = 0, 9, 1   <DO_LOOP>
;CHECK:            |   |   + DO i3 = 0, 19, 1   <DO_LOOP>
;CHECK:            |   |   |   %2 = %[[LIMM]];
;CHECK:            |   |   |   %4 = (@A)[0][i2 + i3];
;CHECK:            |   |   |   (@A)[0][i2 + i3] = %2 + %4;
;CHECK:            |   |   |   %2 = %[[LIMM2]];
;CHECK:            |   |   |   %4 = (@A)[0][i2 + i3 + 1];
;CHECK:            |   |   |   (@A)[0][i2 + i3 + 1] = %2 + %4;
;CHECK:            |   |   |   %2 = %[[LIMM3]];
;CHECK:            |   |   |   %4 = (@A)[0][i2 + i3 + 2];
;CHECK:            |   |   |   (@A)[0][i2 + i3 + 2] = %2 + %4;
;CHECK:            |   |   |   %2 = %[[LIMM4]];
;CHECK:            |   |   |   %4 = (@A)[0][i2 + i3 + 3];
;CHECK:            |   |   |   (@A)[0][i2 + i3 + 3] = %2 + %4;
;CHECK:            |   |   |   %2 = %[[LIMM5]];
;CHECK:            |   |   |   %4 = (@A)[0][i2 + i3 + 4];
;CHECK:            |   |   |   (@A)[0][i2 + i3 + 4] = %2 + %4;
;CHECK:            |   |   + END LOOP
;CHECK:            |   + END LOOP
;CHECK:            + END LOOP
;CHECK:      END REGION
;
;Module Before HIR
; ModuleID = 't2.c'
source_filename = "t2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  %.pre = load i32, ptr getelementptr inbounds ([100 x i32], ptr @B, i64 0, i64 5), align 4, !tbaa !2
  br label %for.body

for.body:                                         ; preds = %for.inc21, %entry
  %0 = phi i32 [ %.pre, %entry ], [ %inc, %for.inc21 ]
  %n.039 = phi i32 [ 0, %entry ], [ %inc22, %for.inc21 ]
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr getelementptr inbounds ([100 x i32], ptr @B, i64 0, i64 5), align 4, !tbaa !2
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc18, %for.body
  %indvars.iv45 = phi i64 [ 0, %for.body ], [ %indvars.iv.next46, %for.inc18 ]
  br label %for.cond7.preheader

for.cond7.preheader:                              ; preds = %for.inc15, %for.cond4.preheader
  %indvars.iv41 = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next42, %for.inc15 ]
  %1 = add nuw nsw i64 %indvars.iv41, %indvars.iv45
  br label %for.body9

for.body9:                                        ; preds = %for.body9, %for.cond7.preheader
  %indvars.iv = phi i64 [ 0, %for.cond7.preheader ], [ %indvars.iv.next, %for.body9 ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %2 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %3 = add nuw nsw i64 %1, %indvars.iv
  %arrayidx12 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %3, !intel-tbaa !2
  %4 = load i32, ptr %arrayidx12, align 4, !tbaa !2
  %add13 = add nsw i32 %4, %2
  store i32 %add13, ptr %arrayidx12, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.inc15, label %for.body9, !llvm.loop !7

for.inc15:                                        ; preds = %for.body9
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond44 = icmp eq i64 %indvars.iv.next42, 20
  br i1 %exitcond44, label %for.inc18, label %for.cond7.preheader

for.inc18:                                        ; preds = %for.inc15
  %indvars.iv.next46 = add nuw nsw i64 %indvars.iv45, 1
  %exitcond47 = icmp eq i64 %indvars.iv.next46, 10
  br i1 %exitcond47, label %for.inc21, label %for.cond4.preheader

for.inc21:                                        ; preds = %for.inc18
  %inc22 = add nuw nsw i32 %n.039, 1
  %exitcond48 = icmp eq i32 %inc22, 10
  br i1 %exitcond48, label %for.end23, label %for.body

for.end23:                                        ; preds = %for.inc21
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
