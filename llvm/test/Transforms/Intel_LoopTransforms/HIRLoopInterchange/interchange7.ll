;
; A[i1][i2][i4][i3] = 1;   only  i3 and i4 will be interchanged
; Testing Loop Bounds to make sure the values are updated correctly 
;  
; REQUIRES: asserts
; RUN: opt -hir-ssa-deconstruction -debug -hir-loop-interchange  -print-after=hir-loop-interchange < %s 2>&1 | FileCheck %s

; CHECK: After HIR Loop Interchange
; CHECK:  DO i1 = 0, %n1 + -1, 1  
; CHECK:  DO i2 = 0, %n2 + -1, 1  
; CHECK:  DO i3 = 0, %n4 + -1, 1  
; CHECK:  DO i4 = 0, %n3 + -1, 1  

;Module Before HIR; ModuleID = 'interchange11.c'
source_filename = "interchange11.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [100 x [100 x [100 x [100 x float]]]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @sub1(i64 %n1, i64 %n2, i64 %n3, i64 %n4) local_unnamed_addr #0 {
entry:
  %cmp44 = icmp sgt i64 %n1, 0
  br i1 %cmp44, label %for.cond1.preheader.lr.ph, label %for.end21

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp241 = icmp sgt i64 %n2, 0
  %cmp538 = icmp sgt i64 %n3, 0
  %cmp836 = icmp sgt i64 %n4, 0
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc19, %for.cond1.preheader.lr.ph
  %i1.045 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %inc20, %for.inc19 ]
  br i1 %cmp241, label %for.cond4.preheader.preheader, label %for.inc19

for.cond4.preheader.preheader:                    ; preds = %for.cond1.preheader
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond4.preheader.preheader, %for.inc16
  %i2.042 = phi i64 [ %inc17, %for.inc16 ], [ 0, %for.cond4.preheader.preheader ]
  br i1 %cmp538, label %for.cond7.preheader.preheader, label %for.inc16

for.cond7.preheader.preheader:                    ; preds = %for.cond4.preheader
  br label %for.cond7.preheader

for.cond7.preheader:                              ; preds = %for.cond7.preheader.preheader, %for.inc13
  %i3.039 = phi i64 [ %inc14, %for.inc13 ], [ 0, %for.cond7.preheader.preheader ]
  br i1 %cmp836, label %for.body9.preheader, label %for.inc13

for.body9.preheader:                              ; preds = %for.cond7.preheader
  br label %for.body9

for.body9:                                        ; preds = %for.body9.preheader, %for.body9
  %i4.037 = phi i64 [ %inc, %for.body9 ], [ 0, %for.body9.preheader ]
  %conv = sitofp i64 %i4.037 to float
  %arrayidx12 = getelementptr inbounds [100 x [100 x [100 x [100 x float]]]], [100 x [100 x [100 x [100 x float]]]]* @A, i64 0, i64 %i1.045, i64 %i2.042, i64 %i4.037, i64 %i3.039
  store float %conv, float* %arrayidx12, align 4, !tbaa !1
  %inc = add nuw nsw i64 %i4.037, 1
  %exitcond = icmp eq i64 %inc, %n4
  br i1 %exitcond, label %for.inc13.loopexit, label %for.body9

for.inc13.loopexit:                               ; preds = %for.body9
  br label %for.inc13

for.inc13:                                        ; preds = %for.inc13.loopexit, %for.cond7.preheader
  %inc14 = add nuw nsw i64 %i3.039, 1
  %exitcond47 = icmp eq i64 %inc14, %n3
  br i1 %exitcond47, label %for.inc16.loopexit, label %for.cond7.preheader

for.inc16.loopexit:                               ; preds = %for.inc13
  br label %for.inc16

for.inc16:                                        ; preds = %for.inc16.loopexit, %for.cond4.preheader
  %inc17 = add nuw nsw i64 %i2.042, 1
  %exitcond48 = icmp eq i64 %inc17, %n2
  br i1 %exitcond48, label %for.inc19.loopexit, label %for.cond4.preheader

for.inc19.loopexit:                               ; preds = %for.inc16
  br label %for.inc19

for.inc19:                                        ; preds = %for.inc19.loopexit, %for.cond1.preheader
  %inc20 = add nuw nsw i64 %i1.045, 1
  %exitcond49 = icmp eq i64 %inc20, %n1
  br i1 %exitcond49, label %for.end21.loopexit, label %for.cond1.preheader

for.end21.loopexit:                               ; preds = %for.inc19
  br label %for.end21

for.end21:                                        ; preds = %for.end21.loopexit, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 17939) (llvm/branches/loopopt 17975)"}
!1 = !{!2, !6, i64 0}
!2 = !{!"array@_ZTSA100_A100_A100_A100_f", !3, i64 0}
!3 = !{!"array@_ZTSA100_A100_A100_f", !4, i64 0}
!4 = !{!"array@_ZTSA100_A100_f", !5, i64 0}
!5 = !{!"array@_ZTSA100_f", !6, i64 0}
!6 = !{!"float", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
