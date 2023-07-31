; RUN: opt -passes="hir-ssa-deconstruction,hir-unroll-and-jam" -print-before=hir-unroll-and-jam -print-after=hir-unroll-and-jam -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we can unroll & jam i1 loop when the inner loop simd directives
; are present in preheader/postexit.

; CHECK: Dump Before

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |      %t7 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
; CHECK: |   + DO i2 = 0, %m + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   |   %0 = (@B)[0][i2];
; CHECK: |   |   %1 = (@A)[0][i1];
; CHECK: |   |   (@A)[0][i1] = %0 + %1;
; CHECK: |   + END LOOP
; CHECK: |      @llvm.directive.region.exit(%t7); [ DIR.OMP.END.SIMD() ]
; CHECK: + END LOOP
; CHECK: END REGION


; CHECK: Dump After

; CHECK: BEGIN REGION { modified }
; CHECK: %tgu = (%n)/u8;

; CHECK: + DO i1 = 0, %tgu + -1, 1   <DO_LOOP>  <MAX_TC_EST = 12>
; CHECK: |      %t7 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
; CHECK: |   + DO i2 = 0, %m + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   |   %0 = (@B)[0][i2];
; CHECK: |   |   %1 = (@A)[0][8 * i1];
; CHECK: |   |   (@A)[0][8 * i1] = %0 + %1;
; CHECK: |   |   %0 = (@B)[0][i2];
; CHECK: |   |   %1 = (@A)[0][8 * i1 + 1];
; CHECK: |   |   (@A)[0][8 * i1 + 1] = %0 + %1;
; CHECK: |   |   %0 = (@B)[0][i2];
; CHECK: |   |   %1 = (@A)[0][8 * i1 + 2];
; CHECK: |   |   (@A)[0][8 * i1 + 2] = %0 + %1;
; CHECK: |   |   %0 = (@B)[0][i2];
; CHECK: |   |   %1 = (@A)[0][8 * i1 + 3];
; CHECK: |   |   (@A)[0][8 * i1 + 3] = %0 + %1;
; CHECK: |   |   %0 = (@B)[0][i2];
; CHECK: |   |   %1 = (@A)[0][8 * i1 + 4];
; CHECK: |   |   (@A)[0][8 * i1 + 4] = %0 + %1;
; CHECK: |   |   %0 = (@B)[0][i2];
; CHECK: |   |   %1 = (@A)[0][8 * i1 + 5];
; CHECK: |   |   (@A)[0][8 * i1 + 5] = %0 + %1;
; CHECK: |   |   %0 = (@B)[0][i2];
; CHECK: |   |   %1 = (@A)[0][8 * i1 + 6];
; CHECK: |   |   (@A)[0][8 * i1 + 6] = %0 + %1;
; CHECK: |   |   %0 = (@B)[0][i2];
; CHECK: |   |   %1 = (@A)[0][8 * i1 + 7];
; CHECK: |   |   (@A)[0][8 * i1 + 7] = %0 + %1;
; CHECK: |   + END LOOP
; CHECK: |      @llvm.directive.region.exit(%t7); [ DIR.OMP.END.SIMD() ]
; CHECK: + END LOOP

; CHECK: + DO i1 = 8 * %tgu, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 7>
; CHECK: |      %t7 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
; CHECK: |   + DO i2 = 0, %m + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   |   %0 = (@B)[0][i2];
; CHECK: |   |   %1 = (@A)[0][i1];
; CHECK: |   |   (@A)[0][i1] = %0 + %1;
; CHECK: |   + END LOOP
; CHECK: |      @llvm.directive.region.exit(%t7); [ DIR.OMP.END.SIMD() ]
; CHECK: + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@A = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define void @foo(i32 %n, i32 %m) local_unnamed_addr {
entry:
  %cmp3 = icmp slt i32 0, %n
  br i1 %cmp3, label %for.cond1.preheader.lr.ph, label %for.end8

for.cond1.preheader.lr.ph:                        ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc6
  %i.04 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc7, %for.inc6 ]
  %cmp21 = icmp slt i32 0, %m
  br i1 %cmp21, label %for.body3.lr.ph, label %for.inc6

for.body3.lr.ph:                                  ; preds = %for.cond1.preheader
  %t7 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.body3
  %j.02 = phi i32 [ 0, %for.body3.lr.ph ], [ %inc, %for.body3 ]
  %idxprom = sext i32 %j.02 to i64
  %arrayidx = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %idxprom
  %0 = load i32, ptr %arrayidx, align 4
  %idxprom4 = sext i32 %i.04 to i64
  %arrayidx5 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %idxprom4
  %1 = load i32, ptr %arrayidx5, align 4
  %add = add nsw i32 %1, %0
  store i32 %add, ptr %arrayidx5, align 4
  %inc = add nsw i32 %j.02, 1
  %cmp2 = icmp slt i32 %inc, %m
  br i1 %cmp2, label %for.body3, label %for.cond1.for.inc6_crit_edge

for.cond1.for.inc6_crit_edge:                     ; preds = %for.body3
  call void @llvm.directive.region.exit(token %t7) [ "DIR.OMP.END.SIMD"() ]
  br label %for.inc6

for.inc6:                                         ; preds = %for.cond1.for.inc6_crit_edge, %for.cond1.preheader
  %inc7 = add nsw i32 %i.04, 1
  %cmp = icmp slt i32 %inc7, %n
  br i1 %cmp, label %for.cond1.preheader, label %for.cond.for.end8_crit_edge

for.cond.for.end8_crit_edge:                      ; preds = %for.inc6
  br label %for.end8

for.end8:                                         ; preds = %for.cond.for.end8_crit_edge, %entry
  ret void
}

declare token @llvm.directive.region.entry() #1

declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }
