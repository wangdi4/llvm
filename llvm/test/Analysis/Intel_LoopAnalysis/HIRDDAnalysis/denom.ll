;  Test for A[(i +1)/2]. Treat this as non-linear for now
;
; RUN: opt < %s -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze -enable-new-pm=0  | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s

; CHECK: DD graph for function
; CHECK-DAG: OUTPUT (*)
;
;Module Before HIR; ModuleID = 'dd2.c'
source_filename = "dd2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(float* nocapture %p, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp7 = icmp eq i32 %n, 0
  br i1 %cmp7, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.08 = phi i32 [ %add, %for.body ], [ 0, %for.body.preheader ]
  %add = add nuw i32 %i.08, 1
  %div = lshr i32 %add, 1
  %idxprom = zext i32 %div to i64
  %arrayidx = getelementptr inbounds float, float* %p, i64 %idxprom
  store float 1.000000e+00, float* %arrayidx, align 4, !tbaa !1
  %exitcond = icmp eq i32 %add, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %phitmp = zext i32 %n to i64
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %i.0.lcssa = phi i64 [ 0, %entry ], [ %phitmp, %for.end.loopexit ]
  %arrayidx2 = getelementptr inbounds float, float* %p, i64 %i.0.lcssa
  %0 = load float, float* %arrayidx2, align 4, !tbaa !1
  %conv = fptosi float %0 to i32
  ret i32 %conv
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20398) (llvm/branches/loopopt 20464)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
