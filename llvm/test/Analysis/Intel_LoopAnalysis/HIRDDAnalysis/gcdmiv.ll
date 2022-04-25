;
;  for (i1 =0; i1 < 100; i1++) {
;    for (i2 = 0; i2< 200; i2++)
;        A[ 3*i1 +  2*i2 ] =  5;
;    for (i2 = 0; i2< 100; i2++)
;        B[ i1 +  i2 ] =  A[ i1 +  2*i2 -1 ] ;
;  }
;   Between the 2 refs for A,
;   gcdmiv test should detect (<>) as the DV: 1 level for common level nesting.
;   It is interpreted as no dependency for same iteration of i, but
;   has loop-carried dep
;
; RUN: opt < %s -hir-ssa-deconstruction | opt -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 < %s | FileCheck %s
;
; CHECK: DD graph for function gcdmiv:
; CHECK-DAG:  (%A)[i1 + 2 * i2 + -1] --> (%A)[3 * i1 + 2 * i2] ANTI (<)
; CHECK-DAG:  (%A)[3 * i1 + 2 * i2] --> (i32*)(%A)[i1 + 2 * i2 + -1] FLOW (<)

;Module Before HIR; ModuleID = 'gcdmiv.c'
source_filename = "gcdmiv.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @gcdmiv(float* nocapture %A, i64 %n1, i64 %n2, i64 %n3) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc16, %entry
  %i1.035 = phi i64 [ 0, %entry ], [ %inc17, %for.inc16 ]
  %mul = mul nuw nsw i64 %i1.035, 3
  br label %for.body3

for.cond5.preheader:                              ; preds = %for.body3
  %add9 = add nsw i64 %i1.035, -1
  br label %for.body7

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %i2.033 = phi i64 [ 0, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %mul4 = shl nuw nsw i64 %i2.033, 1
  %add = add nuw nsw i64 %mul4, %mul
  %arrayidx = getelementptr inbounds float, float* %A, i64 %add
  store float 5.000000e+00, float* %arrayidx, align 4, !tbaa !2
  %inc = add nuw nsw i64 %i2.033, 1
  %exitcond = icmp eq i64 %inc, 200
  br i1 %exitcond, label %for.cond5.preheader, label %for.body3

for.body7:                                        ; preds = %for.body7, %for.cond5.preheader
  %i2.134 = phi i64 [ 0, %for.cond5.preheader ], [ %inc14, %for.body7 ]
  %mul8 = shl nuw nsw i64 %i2.134, 1
  %sub = add nsw i64 %add9, %mul8
  %arrayidx10 = getelementptr inbounds float, float* %A, i64 %sub
  %0 = bitcast float* %arrayidx10 to i32*
  %1 = load i32, i32* %0, align 4, !tbaa !2
  %add11 = add nuw nsw i64 %i2.134, %i1.035
  %arrayidx12 = getelementptr inbounds [1000 x float], [1000 x float]* @B, i64 0, i64 %add11
  %2 = bitcast float* %arrayidx12 to i32*
  store i32 %1, i32* %2, align 4, !tbaa !6
  %inc14 = add nuw nsw i64 %i2.134, 1
  %exitcond36 = icmp eq i64 %inc14, 100
  br i1 %exitcond36, label %for.inc16, label %for.body7

for.inc16:                                        ; preds = %for.body7
  %inc17 = add nuw nsw i64 %i1.035, 1
  %exitcond37 = icmp eq i64 %inc17, 100
  br i1 %exitcond37, label %for.end18, label %for.cond1.preheader

for.end18:                                        ; preds = %for.inc16
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c95d67b22c5ea6ea67afdc54154ea9648f91208c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 6f62ac9ad0ca583cb8b48929e9fcaf38f46b6513)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA1000_f", !3, i64 0}

