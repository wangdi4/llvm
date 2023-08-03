; HIR Idiom Rec: memset with negative IV where UB can be represented as a blob only.

; RUN: opt -passes="hir-ssa-deconstruction,hir-idiom,print<hir>,hir-cg" -disable-output 2>&1 < %s | FileCheck %s

; HIR:
;       BEGIN REGION { }
;             + DO i1 = 0, (%n)/u3, 1   <DO_LOOP>
;             |   (%p)[-1 * i1] = 21845;
;             + END LOOP
;       END REGION

; CHECK: BEGIN REGION { modified }
; CHECK: llvm.memset.p0.i32(&((i8*)(%p)[-1 * (%n /u 3)]),  85,  2 * ((3 + %n) /u 3),  0)

; ModuleID = 'memset-negative-blob.bc'
source_filename = "memset-negative-blob.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(ptr %p, i32 %n) #0 {
entry:
  %div = udiv i32 %n, 3
  %add = add i32 %div, 1
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %sub = sub nsw i32 0, %i.01
  %idxprom = sext i32 %sub to i64
  %arrayidx = getelementptr inbounds i16, ptr %p, i64 %idxprom
  store i16 21845, ptr %arrayidx, align 2
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp ult i32 %inc, %add
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20662) (llvm/branches/loopopt 20688)"}
