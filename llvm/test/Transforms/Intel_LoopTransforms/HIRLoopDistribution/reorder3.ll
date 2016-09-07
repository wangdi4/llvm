;  This is not legal to distribute 
;     for(int  i = 0; i <= 50; i++) {
;        A[100 - 2*i] = B[i] +1;
;        A[50 - i] =  B[i+n] +1; }
;RUN: opt -hir-ssa-deconstruction -hir-loop-distribute -print-after=hir-loop-distribute -hir-loop-distribute-heuristics=mem-rec -hir-cost-model-throttling=0  < %s 2>&1 | FileCheck %s
;  CHECK: DO i1    
;  CHECK-NOT: DO i1    
;Module Before HIR; ModuleID = 'reorder3.cpp'
source_filename = "reorder3.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@B = local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define float @_Z3fool(i64 %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %0 = load float, float* getelementptr inbounds ([1000 x float], [1000 x float]* @A, i64 0, i64 2), align 8, !tbaa !1
  ret float %0

for.body:                                         ; preds = %for.body, %entry
  %i.014 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %i.014
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !6
  %add = add nsw i32 %1, 1
  %conv = sitofp i32 %add to float
  %mul = shl nsw i64 %i.014, 1
  %sub = sub nuw nsw i64 100, %mul
  %arrayidx1 = getelementptr inbounds [1000 x float], [1000 x float]* @A, i64 0, i64 %sub
  store float %conv, float* %arrayidx1, align 8, !tbaa !1
  %add2 = add nsw i64 %i.014, %n
  %arrayidx3 = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %add2
  %2 = load i32, i32* %arrayidx3, align 4, !tbaa !6
  %add4 = add nsw i32 %2, 1
  %conv5 = sitofp i32 %add4 to float
  %sub6 = sub nuw nsw i64 50, %i.014
  %arrayidx7 = getelementptr inbounds [1000 x float], [1000 x float]* @A, i64 0, i64 %sub6
  store float %conv5, float* %arrayidx7, align 4, !tbaa !1
  %inc = add nuw nsw i64 %i.014, 1
  %exitcond = icmp eq i64 %inc, 51
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 17939) (llvm/branches/loopopt 17975)"}
!1 = !{!2, !3, i64 0}
!2 = !{!"array@_ZTSA1000_f", !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !8, i64 0}
!7 = !{!"array@_ZTSA1000_i", !8, i64 0}
!8 = !{!"int", !4, i64 0}
