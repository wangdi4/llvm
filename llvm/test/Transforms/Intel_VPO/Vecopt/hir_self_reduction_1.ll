; Test for safe self reduction - loop should be marked as vectorizable.
;
; LLVM IR generated from following testcase using clang -O1 -S -emit-llvm
; int foo(int *arr)
; {
;   int index, sum;
; 
;   sum = 0;
;   for (index = 0; index < 1024; index++)
;     sum += arr[index];
; 
;   return sum;
; }
; 
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -print-after=hir-vec-dir-insert -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>" -S < %s 2>&1 | FileCheck %s
; HIR Test.
; CHECK: @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; CHECK: DO i1 = 0, 1023, 1   <DO_LOOP>
; ModuleID = 'r1.c'
source_filename = "r1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @foo(i32* nocapture readonly %arr) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %sum.07 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %0, %sum.07
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 %add
}

attributes #0 = { norecurse nounwind readonly uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 12506) (llvm/branches/loopopt 12534)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
