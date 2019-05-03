; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-distribute-memrec | opt -analyze -force-hir-sparse-array-reduction-analysis -hir-sparse-array-reduction-analysis 
; RUN: opt < %s -aa-pipeline=basic-aa -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,print<hir-sparse-array-reduction-analysis>" -hir-cost-model-throttling=0 -force-hir-sparse-array-reduction-analysis -disable-output 2>&1 | FileCheck %s

; None of the following loop has sparse array reduction of our interest and should pass sparse array reduction silently. First loop used to cause a stability issue.

;          BEGIN REGION { }
;                + DO i1 = 0, %0 + -1, 1   <DO_LOOP>
;                |   %mul21 = (%I13248)[0]  *  (%I137650)[%I100604.110 + -1];
;                |   %add23 = (%I07578)[%I235886.111 + -1]  +  %mul21;
;                |   (%I07578)[%I235886.111 + -1] = %add23;
;                |   %I100604.110 = (%I228867)[0]  +  %I100604.110;
;                |   %I235886.111 = (%I228868)[0]  +  %I235886.111;
;                + END LOOP
;          END REGION

; CHECK:               No Sparse Array Reduction

; ModuleID = 'vah.ll'
source_filename = "vah_dmqxsco.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i64 %I235886.0, i64 %I100604.0, i64* nocapture readonly %I01541, double* nocapture readonly %I13248, double* nocapture readonly %I137650, i64* nocapture readonly %I228867, double* nocapture %I07578, i64* nocapture readonly %I228868) {
entry:
  %incdec.ptr = getelementptr inbounds double, double* %I07578, i64 -1
  %incdec.ptr1 = getelementptr inbounds double, double* %I137650, i64 -1
  %t0 = load i64, i64* %I01541, align 8
  br label %for.body;

for.body:                                         
  %I235886.111 = phi i64 [ %add25, %for.body ], [ %I235886.0, %entry ]
  %I100604.110 = phi i64 [ %add24, %for.body ], [ %I100604.0, %entry ]
  %I235885.09 = phi i64 [ %inc, %for.body ], [ 1, %entry ]
  %t5 = load double, double* %I13248, align 8
  %arrayidx = getelementptr inbounds double, double* %incdec.ptr1, i64 %I100604.110
  %t6 = load double, double* %arrayidx, align 8
  %mul21 = fmul double %t5, %t6
  %arrayidx22 = getelementptr inbounds double, double* %incdec.ptr, i64 %I235886.111
  %t7 = load double, double* %arrayidx22, align 8
  %add23 = fadd double %t7, %mul21
  store double %add23, double* %arrayidx22, align 8
  %t8 = load i64, i64* %I228867, align 8
  %add24 = add nsw i64 %t8, %I100604.110
  %t9 = load i64, i64* %I228868, align 8
  %add25 = add nsw i64 %t9, %I235886.111
  %inc = add nuw nsw i64 %I235885.09, 1
  %exitcond13 = icmp eq i64 %I235885.09, %t0
  br i1 %exitcond13, label %cleanup.loopexit15, label %for.body

cleanup.loopexit15:                                          
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
