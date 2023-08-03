; RUN: opt -aa-pipeline="tbaa" -passes="gvn" -S < %s | FileCheck %s

; When a null pointer value is initialized by a non-ANSI compliant pointer
; store, TBAA reports the corresponding load and store as 'no alias'. When
; PRE removes that non aliasing load, it leads to segmentation fault.


; CHECK-NOT: phi ptr [ null, %for.body20.us.preheader ], [ %.pre, %for.cond24.for.cond.cleanup28_crit_edge.us.for.body20.us_crit_edge ]
; CHECK: load ptr, ptr %arrayidx36.us, align 8, !tbaa !12


%struct.S = type { i32, i32, ptr }

; Function Attrs: nounwind uwtable
define dso_local i32 @func(ptr nocapture %s) local_unnamed_addr #0 {
entry:
  %0 = load i32, ptr %s, align 8, !tbaa !2
  %d2 = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 1
  %1 = load i32, ptr %d2, align 4, !tbaa !8
  %mul = mul nsw i32 %1, %0
  %conv = sext i32 %mul to i64
  %call = tail call noalias ptr @calloc(i64 %conv, i64 4) #3
  %conv2 = sext i32 %0 to i64
  %call3 = tail call noalias ptr @calloc(i64 %conv2, i64 8) #3
  %cmp33 = icmp sgt i32 %0, 0
  br i1 %cmp33, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %2 = sext i32 %1 to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %ptr = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 2
  store ptr %call3, ptr %ptr, align 8, !tbaa !9
  %tobool = icmp eq ptr %call3, null
  br i1 %tobool, label %cleanup, label %if.end

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv44 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next45, %for.body ]
  %indvars.iv42 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next43, %for.body ]
  %mul7 = shl nsw i64 %indvars.iv44, 2
  %arrayidx = getelementptr inbounds i8, ptr %call, i64 %mul7
  %arrayidx8 = getelementptr inbounds ptr, ptr %call3, i64 %indvars.iv42
  store ptr %arrayidx, ptr %arrayidx8, align 8, !tbaa !10
  %indvars.iv.next43 = add nuw nsw i64 %indvars.iv42, 1
  %indvars.iv.next45 = add nsw i64 %indvars.iv44, %2
  %wide.trip.count46 = sext i32 %0 to i64
  %exitcond47 = icmp ne i64 %indvars.iv.next43, %wide.trip.count46
  br i1 %exitcond47, label %for.body, label %for.cond.cleanup.loopexit

if.end:                                           ; preds = %for.cond.cleanup
  %cmp1731 = icmp sgt i32 %0, 0
  br i1 %cmp1731, label %for.body20.lr.ph, label %cleanup

for.body20.lr.ph:                                 ; preds = %if.end
  %conv12 = sitofp i32 %1 to double
  %3 = fdiv fast double 5.000000e-01, %conv12
  %cmp2629 = icmp sgt i32 %1, 0
  br i1 %cmp2629, label %for.body20.us.preheader, label %for.body20.preheader

for.body20.preheader:                             ; preds = %for.body20.lr.ph
  br label %cleanup.loopexit36

for.body20.us.preheader:                          ; preds = %for.body20.lr.ph
  br label %for.body20.us

for.body20.us:                                    ; preds = %for.body20.us.preheader, %for.cond24.for.cond.cleanup28_crit_edge.us
  %indvars.iv37 = phi i64 [ 0, %for.body20.us.preheader ], [ %indvars.iv.next38, %for.cond24.for.cond.cleanup28_crit_edge.us ]
  %4 = trunc i64 %indvars.iv37 to i32
  %conv21.us = sitofp i32 %4 to double
  %mul22.us = fmul fast double %conv21.us, 0x401921FB54442D18
  %5 = fmul fast double %mul22.us, %3
  %6 = load ptr, ptr %ptr, align 8, !tbaa !9
  %arrayidx36.us = getelementptr inbounds ptr, ptr %6, i64 %indvars.iv37
  %7 = load ptr, ptr %arrayidx36.us, align 8, !tbaa !12
  br label %for.body29.us

for.body29.us:                                    ; preds = %for.body20.us, %for.body29.us
  %indvars.iv = phi i64 [ 0, %for.body20.us ], [ %indvars.iv.next, %for.body29.us ]
  %8 = trunc i64 %indvars.iv to i32
  %conv30.us = sitofp i32 %8 to double
  %add31.us = fadd fast double %conv30.us, 5.000000e-01
  %mul32.us = fmul fast double %add31.us, %5
  %9 = tail call fast double @llvm.cos.f64(double %mul32.us)
  %conv33.us = fptrunc double %9 to float
  %arrayidx38.us = getelementptr inbounds float, ptr %7, i64 %indvars.iv
  store float %conv33.us, ptr %arrayidx38.us, align 4, !tbaa !14
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %wide.trip.count = sext i32 %1 to i64
  %exitcond = icmp ne i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.body29.us, label %for.cond24.for.cond.cleanup28_crit_edge.us

for.cond24.for.cond.cleanup28_crit_edge.us:       ; preds = %for.body29.us
  %indvars.iv.next38 = add nuw nsw i64 %indvars.iv37, 1
  %wide.trip.count39 = sext i32 %0 to i64
  %exitcond40 = icmp ne i64 %indvars.iv.next38, %wide.trip.count39
  br i1 %exitcond40, label %for.body20.us, label %cleanup.loopexit

cleanup.loopexit:                                 ; preds = %for.cond24.for.cond.cleanup28_crit_edge.us
  br label %cleanup

cleanup.loopexit36:                               ; preds = %for.body20.preheader
  br label %cleanup

cleanup:                                          ; preds = %cleanup.loopexit36, %cleanup.loopexit, %if.end, %for.cond.cleanup
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias ptr @calloc(i64, i64) local_unnamed_addr #1

; Function Attrs: nounwind readnone speculatable
declare double @llvm.cos.f64(double) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind readnone speculatable }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 3753d548e10919884603c0cda836d5da2b19ebe5) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 4473d8a6b5ee267b938499977e5a62b051ba190c)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"struct@", !4, i64 0, !4, i64 4, !7, i64 8}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!"pointer@_ZTSPPf", !5, i64 0}
!8 = !{!3, !4, i64 4}
!9 = !{!3, !7, i64 8}
!10 = !{!11, !11, i64 0}
!11 = !{!"pointer@_ZTSPv", !5, i64 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"pointer@_ZTSPf", !5, i64 0}
!14 = !{!15, !15, i64 0}
!15 = !{!"float", !5, i64 0}
