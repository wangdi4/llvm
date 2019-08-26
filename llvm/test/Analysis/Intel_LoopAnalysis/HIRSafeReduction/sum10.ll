; for (int i=0; i< n1; i++) {
;   s  +=  a[i];
;   c[i]  +=  s;
; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction | opt -analyze  -hir-safe-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="loop-simplify,hir-ssa-deconstruction,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s

; CHECK: No Safe Reduction
; ModuleID = 'sum10.c'
source_filename = "sum10.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common global [1000 x i32] zeroinitializer, align 16
@.str = private unnamed_addr constant [4 x i8] c" %f\00", align 1

; Function Attrs: nounwind uwtable
define i32 @sub(float* nocapture readonly %a, float* nocapture readnone %b, i32 %n1, i32 %n2, i32 %n3) #0 {
entry:
  %cmp13 = icmp sgt i32 %n1, 0
  br i1 %cmp13, label %for.body, label %for.cond.cleanup

for.cond.cleanup.loopexit:                        ; preds = %for.body
  %phitmp = fpext float %add to double
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %s.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %phitmp, %for.cond.cleanup.loopexit ]
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), double %s.0.lcssa)
  ret i32 0

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %s.014 = phi float [ %add, %for.body ], [ 0.000000e+00, %entry ]
  %arrayidx = getelementptr inbounds float, float* %a, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %add = fadd float %s.014, %0
  %arrayidx2 = getelementptr inbounds [1000 x i32], [1000 x i32]* @c, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4, !tbaa !5
  %conv = sitofp i32 %1 to float
  %add3 = fadd float %add, %conv
  %conv4 = fptosi float %add3 to i32
  store i32 %conv4, i32* %arrayidx2, align 4, !tbaa !5
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n1
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 11232) (llvm/branches/loopopt 12487)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
