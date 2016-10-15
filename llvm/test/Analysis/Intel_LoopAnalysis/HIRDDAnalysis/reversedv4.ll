;  DV of form (<> *) will generate a forward and backward edge 
;  with DV  (< *)
; RUN:  opt < %s   -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s 
; CHECK: 'HIR Data Dependence Analysis' 
; CHECK-NOT: <>
;
;Module Before HIR; ModuleID = 'x.cpp'
source_filename = "x.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @_Z3subPfi(float* nocapture %a, i32 %n) local_unnamed_addr #0 {
entry:
  %conv = sext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc21, %entry
  %indvars.iv = phi i64 [ %conv, %entry ], [ %indvars.iv.next, %for.inc21 ]
  %i1.041 = phi i64 [ 0, %entry ], [ %inc22, %for.inc21 ]
  %add = add nsw i64 %i1.041, %conv
  %cmp239 = icmp sgt i64 %add, 0
  br i1 %cmp239, label %for.body3.preheader, label %for.inc21

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %i2.040 = phi i64 [ %inc, %for.body3 ], [ 0, %for.body3.preheader ]
  %mul = mul nuw nsw i64 %i2.040, 7
  %add4 = sub nsw i64 %mul, %i1.041
  %add5 = add nsw i64 %add4, 110
  %arrayidx = getelementptr inbounds float, float* %a, i64 %add5
  store float 0.000000e+00, float* %arrayidx, align 4, !tbaa !1
  %add9 = add nsw i64 %add4, 112
  %arrayidx10 = getelementptr inbounds float, float* %a, i64 %add9
  store float 0.000000e+00, float* %arrayidx10, align 4, !tbaa !1
  %add14 = add nsw i64 %add4, 114
  %arrayidx15 = getelementptr inbounds float, float* %a, i64 %add14
  store float 0.000000e+00, float* %arrayidx15, align 4, !tbaa !1
  %add19 = add nsw i64 %add4, 116
  %arrayidx20 = getelementptr inbounds float, float* %a, i64 %add19
  store float 0.000000e+00, float* %arrayidx20, align 4, !tbaa !1
  %inc = add nuw nsw i64 %i2.040, 1
  %exitcond = icmp eq i64 %inc, %indvars.iv
  br i1 %exitcond, label %for.inc21.loopexit, label %for.body3

for.inc21.loopexit:                               ; preds = %for.body3
  br label %for.inc21

for.inc21:                                        ; preds = %for.inc21.loopexit, %for.cond1.preheader
  %inc22 = add nuw nsw i64 %i1.041, 1
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %exitcond43 = icmp eq i64 %inc22, 50
  br i1 %exitcond43, label %for.end23, label %for.cond1.preheader

for.end23:                                        ; preds = %for.inc21
  ret i32 undef
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 17939) (llvm/branches/loopopt 17953)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}

