; LLVM IR generated from the following using icx -O1 -S -emit-llvm
; foo (float d, int n) {
;   float accum = d;
;   for (int i = 0; i < 1024; i++)
;     accum += d;
; 
;   return accum;
; }
; ModuleID = 't.c'
;RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -S %s -print-after-all 2>&1 | FileCheck %s
;RUN: opt --passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert" -S %s -print-after-all 2>&1 | FileCheck %s
; CHECK-NOT:           llvm.intel.directive
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind readnone uwtable
define i32 @foo(float %d, i32 %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %conv = fptosi float %add to i32
  ret i32 %conv

for.body:                                         ; preds = %for.body, %entry
  %i.07 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %accum.06 = phi float [ %d, %entry ], [ %add, %for.body ]
  %add = fadd float %accum.06, %d
  %inc = add nuw nsw i32 %i.07, 1
  %exitcond = icmp eq i32 %inc, 1024
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c7daf524579a31f63ea990d9c287238ba2f33038) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 89c768cf09d2ec76edef60db162613704cd896f0)"}
