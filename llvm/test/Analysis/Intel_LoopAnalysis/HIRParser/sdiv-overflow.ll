; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser | FileCheck %s
; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-framework>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; During parsing of store rval, the division -2 / -1 (i2 type) overflowed and
; caused assertion. We now have a check for that.

; CHECK: + DO i1 = 0, 43, 1   <DO_LOOP>
; CHECK: |   %ld = (@a1_zalp)[0][-1 * i1 + 63];
; CHECK: |   (@a1_m)[0][-1 * i1 + 63] = -1 * trunc.i32.i2(%ld) + -2 * (trunc.i64.i2(%v_dmzu.7607) * trunc.i64.i2(%indvars.iv621));
; CHECK: |   %or332 = 2 * zext.i31.i64((-27 * trunc.i64.i31(%v_dmzu.7607)))  |  97;
; CHECK: |   %v_dmzu.7607 = %or332;
; CHECK: |   %indvars.iv621 = -1 * i1 + 62;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a1_m = external dso_local local_unnamed_addr global [192 x i64], align 16
@a1_zalp = external dso_local local_unnamed_addr global [192 x i32], align 16
@a1_fypfsoz = external dso_local local_unnamed_addr global [192 x i32], align 16
@a3_qveneciv = external dso_local global [192 x [192 x [192 x i32]]], align 16

define void @foo() {
entry:
  br label %for.end335

for.end335:                                       ; preds = %for.end335, %entry
  %indvars.iv621 = phi i64 [ %indvars.iv.next622, %for.end335 ], [ 63, %entry ]
  %v_dmzu.7607 = phi i64 [ %or332, %for.end335 ], [ 103, %entry ]
  %conv311 = mul i64 %v_dmzu.7607, 4294967242
  %conv312 = and i64 %conv311, 4294967294
  %mul319 = mul i64 %conv312, %indvars.iv621
  %arrayidx320 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_zalp, i64 0, i64 %indvars.iv621
  %ld = load i32, i32* %arrayidx320, align 4
  %mul321 = mul i32 %ld, 71
  %conv322 = zext i32 %mul321 to i64
  %add323 = add i64 %mul319, %conv322
  %arrayidx324 = getelementptr inbounds [192 x i64], [192 x i64]* @a1_m, i64 0, i64 %indvars.iv621
  %and331 = and i64 %add323, 3
  store i64 %and331, i64* %arrayidx324, align 8
  %or332 = or i64 %conv312, 97
  %indvars.iv.next622 = add nsw i64 %indvars.iv621, -1
  %cmp306 = icmp ugt i64 %indvars.iv.next622, 19
  br i1 %cmp306, label %for.end335, label %for.end338

for.end338:                                       ; preds = %for.end335
  %or332.lcssa = phi i64 [ %or332, %for.end335 ]
  ret void
}

