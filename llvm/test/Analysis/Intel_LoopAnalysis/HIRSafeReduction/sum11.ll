; for (int j=0; j< n1; j++) {
;     for (int i=0; i< n1; i++) {
;         s  +=  a[i];
;     }
;     for (int i=0; i< n1; i++) {
;         a[i] += d[j] + 2;
;     }
;     for (int i=0; i< n1; i++) {
;         s  +=  a[i];
;     }
;     d[j] = d[j] +s ;
; RUN: opt < %s -loop-simplify  -hir-ssa-deconstruction | opt -hir-temp-cleanup -analyze  -hir-safe-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="loop-simplify,hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>" 2>&1 | FileCheck %s

; CHECK: <Safe Reduction>
; CHECK: No Safe Reduction
; CHECK: <Safe Reduction>
;
; ModuleID = 'sum11.c'
source_filename = "sum11.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@d = common global [1000 x float] zeroinitializer, align 16
@.str = private unnamed_addr constant [4 x i8] c" %f\00", align 1
@c = common global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @sub(float* nocapture %a, float* nocapture readnone %b, i32 %n1, i32 %n2, i32 %n3) #0 {
entry:
  %cmp70 = icmp sgt i32 %n1, 0
  br i1 %cmp70, label %for.cond1.preheader, label %for.cond.cleanup

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup22
  %indvars.iv82 = phi i64 [ %indvars.iv.next83, %for.cond.cleanup22 ], [ 0, %entry ]
  %s.071 = phi float [ %add26, %for.cond.cleanup22 ], [ 0.000000e+00, %entry ]
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup22
  %phitmp = fpext float %add26 to double
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %s.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %phitmp, %for.cond.cleanup.loopexit ]
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), double %s.0.lcssa)
  ret i32 0

for.body9.lr.ph:                                  ; preds = %for.body4
  %arrayidx11 = getelementptr inbounds [1000 x float], [1000 x float]* @d, i64 0, i64 %indvars.iv82
  br label %for.body9

for.body4:                                        ; preds = %for.cond1.preheader, %for.body4
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body4 ], [ 0, %for.cond1.preheader ]
  %s.162 = phi float [ %add, %for.body4 ], [ %s.071, %for.cond1.preheader ]
  %arrayidx = getelementptr inbounds float, float* %a, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %add = fadd float %s.162, %0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n1
  br i1 %exitcond, label %for.body9.lr.ph, label %for.body4

for.body9:                                        ; preds = %for.body9, %for.body9.lr.ph
  %indvars.iv74 = phi i64 [ 0, %for.body9.lr.ph ], [ %indvars.iv.next75, %for.body9 ]
  %1 = load float, float* %arrayidx11, align 4, !tbaa !1
  %add12 = fadd float %1, 2.000000e+00
  %arrayidx14 = getelementptr inbounds float, float* %a, i64 %indvars.iv74
  %2 = load float, float* %arrayidx14, align 4, !tbaa !1
  %add15 = fadd float %2, %add12
  store float %add15, float* %arrayidx14, align 4, !tbaa !1
  %indvars.iv.next75 = add nuw nsw i64 %indvars.iv74, 1
  %lftr.wideiv76 = trunc i64 %indvars.iv.next75 to i32
  %exitcond77 = icmp eq i32 %lftr.wideiv76, %n1
  br i1 %exitcond77, label %for.body23, label %for.body9

for.cond.cleanup22:                               ; preds = %for.body23
  %arrayidx31 = getelementptr inbounds [1000 x float], [1000 x float]* @d, i64 0, i64 %indvars.iv82
  %3 = load float, float* %arrayidx31, align 4, !tbaa !1
  %add32 = fadd float %add26, %3
  store float %add32, float* %arrayidx31, align 4, !tbaa !1
  %indvars.iv.next83 = add nuw nsw i64 %indvars.iv82, 1
  %lftr.wideiv84 = trunc i64 %indvars.iv.next83 to i32
  %exitcond85 = icmp eq i32 %lftr.wideiv84, %n1
  br i1 %exitcond85, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.body23:                                       ; preds = %for.body9, %for.body23
  %indvars.iv78 = phi i64 [ %indvars.iv.next79, %for.body23 ], [ 0, %for.body9 ]
  %s.267 = phi float [ %add26, %for.body23 ], [ %add, %for.body9 ]
  %arrayidx25 = getelementptr inbounds float, float* %a, i64 %indvars.iv78
  %4 = load float, float* %arrayidx25, align 4, !tbaa !1
  %add26 = fadd float %s.267, %4
  %indvars.iv.next79 = add nuw nsw i64 %indvars.iv78, 1
  %lftr.wideiv80 = trunc i64 %indvars.iv.next79 to i32
  %exitcond81 = icmp eq i32 %lftr.wideiv80, %n1
  br i1 %exitcond81, label %for.cond.cleanup22, label %for.body23
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
