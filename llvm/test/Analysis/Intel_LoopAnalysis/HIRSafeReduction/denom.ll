; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-safe-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s
;
; CHECK: No Safe Reduction
;
; Check safe reduction is not identified because of non-1 denominator.
; 
; Source looks like:
;
; void myred(unsigned a[], unsigned k, unsigned* out, int N)
; {
;   unsigned t = 1;
;   for(int i = 0; i < 40; i++) {
;     t = t/3 + k + a[i];
;   }
; 
;   *out = t;
; }
; 
; HIR looks like
; <14>      + DO i1 = 0, 39, 1   <DO_LOOP>
; <2>       |   %t.07.out = %t.07;
; <6>       |   %0 = (%a)[i1];
; <7>       |   %t.07 = (%t.07.out /u 3) + %k  +  %0;
; <14>      + END LOOP
;
;Module Before HIR; ModuleID = 'denom.c'
source_filename = "denom.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @myred(i32* nocapture readonly %a, i32 %k, i32* nocapture %out, i32 %N) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %add1.lcssa = phi i32 [ %add1, %for.body ]
  store i32 %add1.lcssa, i32* %out, align 4, !tbaa !2
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %t.07 = phi i32 [ 1, %entry ], [ %add1, %for.body ]
  %div = udiv i32 %t.07, 3
  %add = add i32 %div, %k
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %add1 = add i32 %add, %0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 40
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 756622f1dabd3965f80d20e0d52127a138802cbd) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 650bc000535d5e6ced9f1fbd9494baca1be77ec0)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
