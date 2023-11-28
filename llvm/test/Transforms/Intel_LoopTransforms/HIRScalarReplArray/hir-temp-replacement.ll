; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam,hir-loop-independent-scalar-repl" -hir-unroll-and-jam-max-factor=2 -print-before=hir-loop-independent-scalar-repl -print-after=hir-loop-independent-scalar-repl < %s 2>&1 | FileCheck %s

; Verify that the replacement of %t3 and %t4 by %scalarepl is not incorrectly
; simplified by ScalarEvolution. When they are substituted in these two blobs:
; zext.i32.i64(smax(((-1 * %t3) + %temp), ((-1 * %temp) + %t3)))
; zext.i32.i64(smax(((-1 * %t4) + %temp2), ((-1 * %temp2) + %t4)))

; All the temps in the blob are temps created by HIR. These temps are created
; by the HIR framework using dummy LLVM insts of this form-
; %temp = @llvm.ssa.copy(undef)
; %scalarepl = @llvm.ssa.copy(undef)

; They were treated as undefs by ScalarEvolution and the resulting expression
; was optimized based on that. 

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 71, 1   <DO_LOOP> <nounroll and jam>
; CHECK: |   %temp = (@Xfile)[0][2 * i1];
; CHECK: |   %temp2 = (@Xrank)[0][2 * i1];
; CHECK: |   %t1 = (@Xfile)[0][2 * i1 + 1];
; CHECK: |   %t2 = (@Xrank)[0][2 * i1 + 1];
; CHECK: |
; CHECK: |   + DO i2 = 0, 143, 1   <DO_LOOP>
; CHECK: |   |   %t3 = (@Xrank)[0][i2];
; CHECK: |   |   %t4 = (@Xrank)[0][i2];
; CHECK: |   |   %t.phi1 = %t.phi1  +  (@pre_p_tropism)[0][umax(zext.i32.i64(smax(((-1 * %t3) + %temp), ((-1 * %temp) + %t3))), zext.i32.i64(smax(((-1 * %t4) + %temp2), ((-1 * %temp2) + %t4))))];
; CHECK: |   |   %t3 = (@Xrank)[0][i2];
; CHECK: |   |   %t4 = (@Xrank)[0][i2];
; CHECK: |   |   %t.phi1 = %t.phi1  +  (@pre_p_tropism)[0][umax(zext.i32.i64(smax(((-1 * %t3) + %t1), ((-1 * %t1) + %t3))), zext.i32.i64(smax(((-1 * %t4) + %t2), ((-1 * %t2) + %t4))))];
; CHECK: |   + END LOOP
; CHECK: + END LOOP
; CHECK: END REGION

; CHECK: Dump After

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 71, 1   <DO_LOOP> <nounroll and jam>
; CHECK: |   %temp = (@Xfile)[0][2 * i1];
; CHECK: |   %temp2 = (@Xrank)[0][2 * i1];
; CHECK: |   %t1 = (@Xfile)[0][2 * i1 + 1];
; CHECK: |   %t2 = (@Xrank)[0][2 * i1 + 1];
; CHECK: |
; CHECK: |   + DO i2 = 0, 143, 1   <DO_LOOP>
; CHECK: |   |   %scalarepl = (@Xrank)[0][i2];
; CHECK: |   |   %t.phi1 = %t.phi1  +   (@pre_p_tropism)[0][umax(zext.i32.i64(smax(((-1 * %scalarepl) + %temp), ((-1 * %temp) + %scalarepl))), zext.i32.i64(smax(((-1 * %scalarepl) + %temp2), ((-1 * %temp2) + %scalarepl))))]
; CHECK: |   |   %t.phi1 = %t.phi1  +  (@pre_p_tropism)[0][umax(zext.i32.i64(smax(((-1 * %scalarepl) + %t1), ((-1 * %t1) + %scalarepl))), zext.i32.i64(smax(((-1 * %scalarepl) + %t2), ((-1 * %t2) + %scalarepl))))];
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30146"

@Xfile = external dso_local local_unnamed_addr constant [144 x i32], align 16
@Xrank = external dso_local local_unnamed_addr constant [144 x i32], align 16
@pre_p_tropism = external dso_local local_unnamed_addr global [9 x i32], align 16

declare i32 @llvm.abs.i32(i32, i1 immarg) #0

declare i32 @llvm.umax.i32(i32, i32) #0

define dso_local void @initialize_eval.V() local_unnamed_addr {
entry:
  br label %for.cond28.preheader

for.cond28.preheader:                             ; preds = %for.inc197, %entry
  %t.phi1 = phi i32 [ %t.lcssa1, %for.inc197 ], [ 0, %entry ]
  %indvars.iv287 = phi i64 [ %indvars.iv.next288, %for.inc197 ], [ 0, %entry ]
  %arrayidx32 = getelementptr inbounds [144 x i32], ptr @Xfile, i64 0, i64 %indvars.iv287
  %t1 = load i32, ptr %arrayidx32, align 4
  %arrayidx38 = getelementptr inbounds [144 x i32], ptr @Xrank, i64 0, i64 %indvars.iv287
  %t2 = load i32, ptr %arrayidx38, align 4
  br label %for.body30

for.body30:                                       ; preds = %for.body30, %for.cond28.preheader
  %indvars.iv284 = phi i64 [ 0, %for.cond28.preheader ], [ %indvars.iv.next285, %for.body30 ]
  %t.phi2 = phi i32 [ %t.phi1, %for.cond28.preheader ], [ %t.upd, %for.body30 ]
  %arrayidx34 = getelementptr inbounds [144 x i32], ptr @Xrank, i64 0, i64 %indvars.iv284
  %t3 = load i32, ptr %arrayidx34, align 4
  %sub35 = sub nsw i32 %t1, %t3
  %t5 = tail call i32 @llvm.abs.i32(i32 %sub35, i1 true)
  %t4 = load i32, ptr %arrayidx34, align 4
  %sub40 = sub nsw i32 %t2, %t4
  %t12 = tail call i32 @llvm.abs.i32(i32 %sub40, i1 true)
  %spec.select = tail call i32 @llvm.umax.i32(i32 %t5, i32 %t12)
  %idxprom56 = zext i32 %spec.select to i64
  %arrayidx57 = getelementptr inbounds [9 x i32], ptr @pre_p_tropism, i64 0, i64 %idxprom56
  %ld = load i32, ptr %arrayidx57, align 4
  %t.upd = add i32 %t.phi2, %ld
  %indvars.iv.next285 = add nuw nsw i64 %indvars.iv284, 1
  %exitcond286.not = icmp eq i64 %indvars.iv.next285, 144
  br i1 %exitcond286.not, label %for.inc197, label %for.body30

for.inc197:                                       ; preds = %for.body30
  %t.lcssa1 = phi i32 [ %t.upd, %for.body30]
  %indvars.iv.next288 = add nuw nsw i64 %indvars.iv287, 1
  %exitcond289.not = icmp eq i64 %indvars.iv.next288, 144
  br i1 %exitcond289.not, label %for.end199, label %for.cond28.preheader

for.end199:                                       ; preds = %for.inc197
  %t.lcssa2 = phi i32 [ %t.lcssa1, %for.inc197]
  ret void
}

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

