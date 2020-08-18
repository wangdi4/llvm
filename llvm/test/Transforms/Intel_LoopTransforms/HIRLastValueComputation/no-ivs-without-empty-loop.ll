; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -print-after=hir-last-value-computation < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
; If the loop is not an empty node after the transformation and the candidate insts are without iv, the code generation will be suppressed
;*** IR Dump Before HIR Last Value Computation ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<15>               + DO i1 = 0, 99, 1   <DO_LOOP>
;<2>                |   %add = %t.02  +  i1;
;<3>                |   %and = %b  &  127;
;<9>                |   %t.02 = %add;
;<15>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Last Value Computation ***
;Function: foo
;
; CHECK:    BEGIN REGION { }
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   %add = %t.02  +  i1;
; CHECK:           |   %t.02 = %add;
; CHECK:           + END LOOP
; CHECK:              %and = %b  &  127;
; CHECK:     END REGION
;
; ModuleID = 't.ll'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @foo(i32 %b) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %t.02 = phi i32 [ undef, %entry ], [ %add, %for.inc ], !in.de.ssa !2
  %j.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ], !in.de.ssa !3
  %add = add nsw i32 %t.02, %j.01
  %and = and i32 %b, 127
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %j.01, 1
  %cmp = icmp slt i32 %inc, 100
  %j.01.in = call i32 @llvm.ssa.copy.i32(i32 %inc), !in.de.ssa !3
  %t.02.in = call i32 @llvm.ssa.copy.i32(i32 %add), !in.de.ssa !2
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  %t.0.lcssa = phi i32 [ %add, %for.inc ]
  %k.0.lcssa = phi i32 [ %and, %for.inc ]
  %mul = mul nsw i32 %t.0.lcssa, %k.0.lcssa
  ret i32 %mul
}

; Function Attrs: nounwind readnone
declare i32 @llvm.ssa.copy.i32(i32 returned) #1

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!"t.02.de.ssa"}
!3 = !{!"j.01.de.ssa"}
