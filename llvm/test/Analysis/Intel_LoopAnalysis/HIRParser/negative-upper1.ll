; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that loop's upper was converted to positive value by parser.

; CHECK: + DO i1 = 0, 4294967294, 1   <DO_LOOP>
; CHECK: |   %and = i1  &&  1023;
; CHECK: |   (@A)[0][%and] = i1;
; CHECK: + END LOOP


;Module Before HIR; ModuleID = 'unroll_05_stab.c'
source_filename = "unroll_05_stab.c"
target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

@A = common global [1024 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo() #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %i.05 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %and = and i32 %i.05, 1023
  %idxprom = zext i32 %and to i64
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @A, i64 0, i64 %idxprom
  store i32 %i.05, i32* %arrayidx, align 4
  %inc = add nuw i32 %i.05, 1
  %cmp = icmp eq i32 %inc, -1
  br i1 %cmp, label %for.end, label %for.body

for.end:
  ret void
}
