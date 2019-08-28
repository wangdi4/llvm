; RUN: opt < %s -xmain-opt-level=3 -analyze -hir-ssa-deconstruction -hir-framework 2>&1 | FileCheck %s
; RUN: opt < %s -xmain-opt-level=3 -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Verify that we build 4-level convolution loopnest even though the number of nested ifs (7) in i2 loop exceed threshold of 6.

; CHECK: + DO i1
; CHECK: |   + DO i2
; CHECK: |   |   + DO i3
; CHECK: |   |   |   + DO i4
; CHECK: |   |   |   |   %1 = (%pix1)[i3 + i4].0;
; CHECK: |   |   |   |   %conv.us = uitofp.i16.double(%1);
; CHECK: |   |   |   |   %mul10.us = %conv.us  *  %mul;
; CHECK: |   |   |   |   %t1.0214 = %t1.0214  +  %mul10.us;
; CHECK: |   |   |   + END LOOP
; CHECK: |   |   + END LOOP

; CHECK: |   |   if (
; CHECK: |   |      if (
; CHECK: |   |         if (
; CHECK: |   |            if (
; CHECK: |   |               if (
; CHECK: |   |                  if (
; CHECK: |   |                     if (
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Pixel = type { i16, i16, i16 }

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local double @foo(%struct.Pixel* nocapture readonly %pix1, %struct.Pixel* nocapture readonly %pix2, double %mul, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp212 = icmp sgt i32 %n, 0
  br i1 %cmp212, label %for.cond1.preheader.lr.ph, label %for.end105

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %wide.trip.count223 = sext i32 %n to i64
  %wide.trip.count232 = sext i32 %n to i64
  br label %for.cond4.preheader.preheader

for.cond4.preheader.preheader:                    ; preds = %for.cond1.preheader.lr.ph, %for.inc103
  %t3.0216 = phi double [ 0.000000e+00, %for.cond1.preheader.lr.ph ], [ %t3.6.lcssa, %for.inc103 ]
  %t2.0215 = phi double [ 0.000000e+00, %for.cond1.preheader.lr.ph ], [ %t2.6.lcssa, %for.inc103 ]
  %t1.0214 = phi double [ 0.000000e+00, %for.cond1.preheader.lr.ph ], [ %add41.us.lcssa.lcssa.lcssa, %for.inc103 ]
  %i.0213 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc104, %for.inc103 ]
  br label %for.cond7.preheader.us.preheader

for.cond7.preheader.us.preheader:                 ; preds = %for.cond4.preheader.preheader, %for.inc100
  %t3.1208 = phi double [ %t3.6, %for.inc100 ], [ %t3.0216, %for.cond4.preheader.preheader ]
  %t2.1207 = phi double [ %t2.6, %for.inc100 ], [ %t2.0215, %for.cond4.preheader.preheader ]
  %t1.1206 = phi double [ %add41.us.lcssa.lcssa, %for.inc100 ], [ %t1.0214, %for.cond4.preheader.preheader ]
  %j.0205 = phi i32 [ %inc101, %for.inc100 ], [ 0, %for.cond4.preheader.preheader ]
  br label %for.cond7.preheader.us

for.cond7.preheader.us:                           ; preds = %for.cond7.for.inc24_crit_edge.us, %for.cond7.preheader.us.preheader
  %indvars.iv221 = phi i64 [ 0, %for.cond7.preheader.us.preheader ], [ %indvars.iv.next222, %for.cond7.for.inc24_crit_edge.us ]
  %t3.2183.us = phi double [ %t3.1208, %for.cond7.preheader.us.preheader ], [ %add23.us.lcssa, %for.cond7.for.inc24_crit_edge.us ]
  %t2.2182.us = phi double [ %t2.1207, %for.cond7.preheader.us.preheader ], [ %add17.us.lcssa, %for.cond7.for.inc24_crit_edge.us ]
  %t1.2181.us = phi double [ %t1.1206, %for.cond7.preheader.us.preheader ], [ %add11.us.lcssa, %for.cond7.for.inc24_crit_edge.us ]
  br label %for.body9.us

for.body9.us:                                     ; preds = %for.body9.us, %for.cond7.preheader.us
  %indvars.iv = phi i64 [ 0, %for.cond7.preheader.us ], [ %indvars.iv.next, %for.body9.us ]
  %t3.3175.us = phi double [ %t3.2183.us, %for.cond7.preheader.us ], [ %add23.us, %for.body9.us ]
  %t2.3174.us = phi double [ %t2.2182.us, %for.cond7.preheader.us ], [ %add17.us, %for.body9.us ]
  %t1.3173.us = phi double [ %t1.2181.us, %for.cond7.preheader.us ], [ %add11.us, %for.body9.us ]
  %0 = add nuw nsw i64 %indvars.iv, %indvars.iv221
  %a.us = getelementptr inbounds %struct.Pixel, %struct.Pixel* %pix1, i64 %0, i32 0
  %1 = load i16, i16* %a.us, align 2, !tbaa !2
  %conv.us = uitofp i16 %1 to double
  %mul10.us = fmul double %conv.us, %mul
  %add11.us = fadd double %t1.3173.us, %mul10.us
  %b.us = getelementptr inbounds %struct.Pixel, %struct.Pixel* %pix1, i64 %0, i32 1
  %2 = load i16, i16* %b.us, align 2, !tbaa !7
  %conv15.us = uitofp i16 %2 to double
  %mul16.us = fmul double %conv15.us, %mul
  %add17.us = fadd double %t2.3174.us, %mul16.us
  %c.us = getelementptr inbounds %struct.Pixel, %struct.Pixel* %pix1, i64 %0, i32 2
  %3 = load i16, i16* %c.us, align 2, !tbaa !8
  %conv21.us = uitofp i16 %3 to double
  %mul22.us = fmul double %conv21.us, %mul
  %add23.us = fadd double %t3.3175.us, %mul22.us
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count223
  br i1 %exitcond, label %for.cond7.for.inc24_crit_edge.us, label %for.body9.us

for.cond7.for.inc24_crit_edge.us:                 ; preds = %for.body9.us
  %add11.us.lcssa = phi double [ %add11.us, %for.body9.us ]
  %add17.us.lcssa = phi double [ %add17.us, %for.body9.us ]
  %add23.us.lcssa = phi double [ %add23.us, %for.body9.us ]
  %indvars.iv.next222 = add nuw nsw i64 %indvars.iv221, 1
  %exitcond224 = icmp eq i64 %indvars.iv.next222, %wide.trip.count223
  br i1 %exitcond224, label %for.cond31.preheader.us.preheader, label %for.cond7.preheader.us

for.cond31.preheader.us.preheader:                ; preds = %for.cond7.for.inc24_crit_edge.us
  %add11.us.lcssa.lcssa = phi double [ %add11.us.lcssa, %for.cond7.for.inc24_crit_edge.us ]
  %add17.us.lcssa.lcssa = phi double [ %add17.us.lcssa, %for.cond7.for.inc24_crit_edge.us ]
  %add23.us.lcssa.lcssa = phi double [ %add23.us.lcssa, %for.cond7.for.inc24_crit_edge.us ]
  br label %for.cond31.preheader.us

for.cond31.preheader.us:                          ; preds = %for.cond31.preheader.us.preheader, %for.cond31.for.inc59_crit_edge.us
  %indvars.iv230 = phi i64 [ %indvars.iv.next231, %for.cond31.for.inc59_crit_edge.us ], [ 0, %for.cond31.preheader.us.preheader ]
  %t3.4200.us = phi double [ %add55.us.lcssa, %for.cond31.for.inc59_crit_edge.us ], [ %add23.us.lcssa.lcssa, %for.cond31.preheader.us.preheader ]
  %t2.4199.us = phi double [ %add48.us.lcssa, %for.cond31.for.inc59_crit_edge.us ], [ %add17.us.lcssa.lcssa, %for.cond31.preheader.us.preheader ]
  %t1.4198.us = phi double [ %add41.us.lcssa, %for.cond31.for.inc59_crit_edge.us ], [ %add11.us.lcssa.lcssa, %for.cond31.preheader.us.preheader ]
  br label %for.body34.us

for.body34.us:                                    ; preds = %for.body34.us, %for.cond31.preheader.us
  %indvars.iv225 = phi i64 [ 0, %for.cond31.preheader.us ], [ %indvars.iv.next226, %for.body34.us ]
  %t3.5191.us = phi double [ %t3.4200.us, %for.cond31.preheader.us ], [ %add55.us, %for.body34.us ]
  %t2.5190.us = phi double [ %t2.4199.us, %for.cond31.preheader.us ], [ %add48.us, %for.body34.us ]
  %t1.5189.us = phi double [ %t1.4198.us, %for.cond31.preheader.us ], [ %add41.us, %for.body34.us ]
  %4 = add nuw nsw i64 %indvars.iv225, %indvars.iv230
  %a38.us = getelementptr inbounds %struct.Pixel, %struct.Pixel* %pix2, i64 %4, i32 0
  %5 = load i16, i16* %a38.us, align 2, !tbaa !2
  %conv39.us = uitofp i16 %5 to double
  %mul40.us = fmul double %conv39.us, %mul
  %add41.us = fadd double %t1.5189.us, %mul40.us
  %b45.us = getelementptr inbounds %struct.Pixel, %struct.Pixel* %pix2, i64 %4, i32 1
  %6 = load i16, i16* %b45.us, align 2, !tbaa !7
  %conv46.us = uitofp i16 %6 to double
  %mul47.us = fmul double %conv46.us, %mul
  %add48.us = fadd double %t2.5190.us, %mul47.us
  %c52.us = getelementptr inbounds %struct.Pixel, %struct.Pixel* %pix2, i64 %4, i32 2
  %7 = load i16, i16* %c52.us, align 2, !tbaa !8
  %conv53.us = uitofp i16 %7 to double
  %mul54.us = fmul double %conv53.us, %mul
  %add55.us = fadd double %t3.5191.us, %mul54.us
  %indvars.iv.next226 = add nuw nsw i64 %indvars.iv225, 1
  %exitcond229 = icmp eq i64 %indvars.iv.next226, %wide.trip.count232
  br i1 %exitcond229, label %for.cond31.for.inc59_crit_edge.us, label %for.body34.us

for.cond31.for.inc59_crit_edge.us:                ; preds = %for.body34.us
  %add41.us.lcssa = phi double [ %add41.us, %for.body34.us ]
  %add48.us.lcssa = phi double [ %add48.us, %for.body34.us ]
  %add55.us.lcssa = phi double [ %add55.us, %for.body34.us ]
  %indvars.iv.next231 = add nuw nsw i64 %indvars.iv230, 1
  %exitcond233 = icmp eq i64 %indvars.iv.next231, %wide.trip.count232
  br i1 %exitcond233, label %for.end61, label %for.cond31.preheader.us

for.end61:                                        ; preds = %for.cond31.for.inc59_crit_edge.us
  %add41.us.lcssa.lcssa = phi double [ %add41.us.lcssa, %for.cond31.for.inc59_crit_edge.us ]
  %add48.us.lcssa.lcssa = phi double [ %add48.us.lcssa, %for.cond31.for.inc59_crit_edge.us ]
  %add55.us.lcssa.lcssa = phi double [ %add55.us.lcssa, %for.cond31.for.inc59_crit_edge.us ]
  %cmp62 = fcmp ogt double %add41.us.lcssa.lcssa, 1.000000e+00
  br i1 %cmp62, label %if.then, label %for.inc100

if.then:                                          ; preds = %for.end61
  %inc64 = fadd double %add48.us.lcssa.lcssa, 1.000000e+00
  %cmp65 = fcmp ogt double %add41.us.lcssa.lcssa, 2.000000e+00
  br i1 %cmp65, label %if.then67, label %for.inc100

if.then67:                                        ; preds = %if.then
  %inc68 = fadd double %add55.us.lcssa.lcssa, 1.000000e+00
  %cmp69 = fcmp ogt double %add41.us.lcssa.lcssa, 3.000000e+00
  br i1 %cmp69, label %if.then71, label %for.inc100

if.then71:                                        ; preds = %if.then67
  %inc72 = fadd double %inc64, 1.000000e+00
  %cmp73 = fcmp ogt double %add41.us.lcssa.lcssa, 4.000000e+00
  br i1 %cmp73, label %if.then75, label %for.inc100

if.then75:                                        ; preds = %if.then71
  %inc76 = fadd double %inc68, 1.000000e+00
  %cmp77 = fcmp ogt double %add41.us.lcssa.lcssa, 5.000000e+00
  br i1 %cmp77, label %if.then79, label %for.inc100

if.then79:                                        ; preds = %if.then75
  %inc80 = fadd double %inc72, 1.000000e+00
  %cmp81 = fcmp ogt double %add41.us.lcssa.lcssa, 6.000000e+00
  br i1 %cmp81, label %if.then83, label %for.inc100

if.then83:                                        ; preds = %if.then79
  %inc84 = fadd double %inc76, 1.000000e+00
  %cmp85 = fcmp ogt double %add41.us.lcssa.lcssa, 7.000000e+00
  br i1 %cmp85, label %if.then87, label %for.inc100

if.then87:                                        ; preds = %if.then83
  %inc88 = fadd double %inc80, 1.000000e+00
  %cmp89 = fcmp ogt double %add41.us.lcssa.lcssa, 8.000000e+00
  %inc92 = fadd double %inc84, 1.000000e+00
  %8 = select i1 %cmp89, double %inc92, double %inc84
  br label %for.inc100

for.inc100:                                       ; preds = %if.then87, %for.end61, %if.then67, %if.then75, %if.then83, %if.then79, %if.then71, %if.then
  %t2.6 = phi double [ %inc88, %if.then87 ], [ %inc80, %if.then83 ], [ %inc80, %if.then79 ], [ %inc72, %if.then75 ], [ %inc72, %if.then71 ], [ %inc64, %if.then67 ], [ %inc64, %if.then ], [ %add48.us.lcssa.lcssa, %for.end61 ]
  %t3.6 = phi double [ %8, %if.then87 ], [ %inc84, %if.then83 ], [ %inc76, %if.then79 ], [ %inc76, %if.then75 ], [ %inc68, %if.then71 ], [ %inc68, %if.then67 ], [ %add55.us.lcssa.lcssa, %if.then ], [ %add55.us.lcssa.lcssa, %for.end61 ]
  %inc101 = add nuw nsw i32 %j.0205, 1
  %exitcond234 = icmp eq i32 %inc101, %n
  br i1 %exitcond234, label %for.inc103, label %for.cond7.preheader.us.preheader

for.inc103:                                       ; preds = %for.inc100
  %t2.6.lcssa = phi double [ %t2.6, %for.inc100 ]
  %t3.6.lcssa = phi double [ %t3.6, %for.inc100 ]
  %add41.us.lcssa.lcssa.lcssa = phi double [ %add41.us.lcssa.lcssa, %for.inc100 ]
  %inc104 = add nuw nsw i32 %i.0213, 1
  %exitcond235 = icmp eq i32 %inc104, %n
  br i1 %exitcond235, label %for.end105.loopexit, label %for.cond4.preheader.preheader

for.end105.loopexit:                              ; preds = %for.inc103
  %t2.6.lcssa.lcssa = phi double [ %t2.6.lcssa, %for.inc103 ]
  %t3.6.lcssa.lcssa = phi double [ %t3.6.lcssa, %for.inc103 ]
  %add41.us.lcssa.lcssa.lcssa.lcssa = phi double [ %add41.us.lcssa.lcssa.lcssa, %for.inc103 ]
  br label %for.end105

for.end105:                                       ; preds = %for.end105.loopexit, %entry
  %t1.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %add41.us.lcssa.lcssa.lcssa.lcssa, %for.end105.loopexit ]
  %t2.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %t2.6.lcssa.lcssa, %for.end105.loopexit ]
  %t3.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %t3.6.lcssa.lcssa, %for.end105.loopexit ]
  %add106 = fadd double %t1.0.lcssa, %t2.0.lcssa
  %add107 = fadd double %add106, %t3.0.lcssa
  ret double %add107
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"struct@Pixel", !4, i64 0, !4, i64 2, !4, i64 4}
!4 = !{!"short", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!3, !4, i64 2}
!8 = !{!3, !4, i64 4}
