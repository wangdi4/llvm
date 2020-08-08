; Check HIR parsing of cases with undefined values in GEP instruction
; |   (undef)[undef][sext.i32.i64(undef)] = 5 * i1;
; |   <LVAL-REG> (LINEAR [5 x i32]* undef)[LINEAR i32 undef][LINEAR i64 sext.i32.i64(undef)] {sb:0}
; |   <RVAL-REG> LINEAR i32 5 * i1 {sb:4}

; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser -hir-details | FileCheck %s

; CHECK: HasSignedIV: Yes

; CHECK: (undef)[undef][{{.*}}undef{{.*}}]
; CHECK-NEXT: <LVAL-REG>{{.*}}
; undef is assumed as a constant so we do not create blob ddrefs for it.
; CHECK-NOT: <BLOB> {{.*}} undef

; ModuleID = 'gep-undef.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = common global i32 0, align 4

define i32 @main() {
entry:
  %A = alloca [5 x i32], align 16
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %mul = mul nsw i32 5, %i.01
  %0 = load i32, i32* @x, align 4
  %idxprom = sext i32 undef to i64
  %arrayidx = getelementptr inbounds [5 x i32], [5 x i32]* undef, i32 undef, i64 %idxprom
  store i32 %mul, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 5
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  %arrayidx3 = getelementptr inbounds [5 x i32], [5 x i32]* %A, i32 0, i64 0
  %1 = load i32, i32* %arrayidx3, align 4
  ret i32 %1
}
