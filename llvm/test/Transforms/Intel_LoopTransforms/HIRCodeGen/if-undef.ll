; RUN: opt < %s -loop-rotate -hir-cg -force-hir-cg -S | FileCheck %s
;          BEGIN REGION { }
;<18>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<2>          |   if (0 #UNDEF# 0)
;<2>          |   {
;<8>          |      (%A)[0][i1] = i1;
;<2>          |   }
;<18>         + END LOOP
;          END REGION
; HLIfs may have undefined predicate, which should be CG as a br with undef
; as first operand
;CHECK: region.0:
;CHECK: loop.{{[0-9]+}}:
;CHECK-NEXT: br i1 undef, 

; ModuleID = '2.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  %A = alloca [5 x i32], align 16
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp slt i32 %i.0, 5
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br i1 undef, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds [5 x i32], [5 x i32]* %A, i32 0, i64 %idxprom
  store i32 %i.0, i32* %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %arrayidx2 = getelementptr inbounds [5 x i32], [5 x i32]* %A, i32 0, i64 0
  %0 = load i32, i32* %arrayidx2, align 4
  ret i32 %0
}

