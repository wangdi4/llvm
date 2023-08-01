; Check that edges are not accumulated after getGraph().
;
;  for  (i=0; i< 1000; i++) {
;    a[2 *n * i] =   b[2 *n * i+ 1] +1;
;  }
; (Note: test revised so it will not be skipped due to Distribution cost model)

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,print<hir-dd-analysis>" -disable-output < %s 2>&1 | FileCheck %s

; Check that there are only two edges (one OUTPUT edge for (@a)[0][2 * %n * i1] and one FLOW edge for %add2).

; CHECK: DD graph for function sub8:
; CHECK: -->
; CHECK: -->
; CHECK-NOT: -->
;
;Module Before HIR; ModuleID = 'duplicates.c'
source_filename = "duplicates.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = common local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@a = common local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @sub8(i64 %n) local_unnamed_addr #0 {
entry:
  %mul = shl nsw i64 %n, 1
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.011 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %mul1 = mul nsw i64 %i.011, %mul
  %add = or i64 %mul1, 1
  %arrayidx = getelementptr inbounds [1000 x float], ptr @b, i64 0, i64 %add
  %0 = load float, ptr %arrayidx, align 4, !tbaa !2
  %add2 = fadd float %0, 1.000000e+00
  %arrayidx5 = getelementptr inbounds [1000 x float], ptr @a, i64 0, i64 %mul1
  store float %add2, ptr %arrayidx5, align 8, !tbaa !2
  %inc = add nuw nsw i64 %i.011, 1
  %exitcond = icmp eq i64 %inc, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d42e2277040a36be58770319d834272c2c076e9e)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
