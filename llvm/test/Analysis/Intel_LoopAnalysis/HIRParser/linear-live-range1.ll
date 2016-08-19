; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the linear livein copy occurs last in the loop.

; CHECK: + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK: |   %mul12.out = %mul12;
; CHECK: |   %mul13.out = %mul13;
; CHECK: |   %mul = %mul13.out  *  i1 + %mul12.out;
; CHECK: |   %mul13 = ((%mul12.out + %indvars.iv) * %mul13.out);
; CHECK: |   %mul12 = ((%mul12.out + %indvars.iv) * %mul13.out);
; CHECK: |   %indvars.iv = i1 + 1;
; CHECK: + END LOOP


source_filename = "bug.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@g_j = local_unnamed_addr global i64 1, align 8
@g_nctlk = local_unnamed_addr global i64 0, align 8
@main_v_n = local_unnamed_addr global i32 0, align 4


define i32 @main() local_unnamed_addr #2 {
entry:
  store i32 0, i32* @main_v_n, align 4
  %g_nctlk.promoted = load i64, i64* @g_nctlk, align 8
  %g_j.promoted = load i64, i64* @g_j, align 8
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %mul13 = phi i64 [ %g_j.promoted, %entry ], [ %mul, %for.body ]
  %mul12 = phi i64 [ %g_nctlk.promoted, %entry ], [ %mul, %for.body ]
  %add = add i64 %mul12, %indvars.iv
  %mul = mul i64 %mul13, %add
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %mul.lcssa = phi i64 [ %mul, %for.body ]
  ret i32 0
}

