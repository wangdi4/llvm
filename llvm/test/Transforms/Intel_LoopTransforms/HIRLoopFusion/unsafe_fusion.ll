; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verifies that that the i2 loops are not fused because fusion requires
; peeling of the last iteration of the first loop and moving it after
; the fused loops, which is illegal.

; HIR before Fusion
;            BEGIN REGION { }
;                  + DO i1 = 0, %iters.0 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;                  |   %hir.de.ssa.copy1.out = %7;
;                  |   if (%7 >u 311)
;                  |   {
;                  |      %9 = %6;
;                  |
;                  |      + DO i2 = 0, 155, 1   <DO_LOOP>
;                  |      |   %10 = (%4)[0].0.0[i2 + 1];
;                  |      |   %xor.i.i.i37 = zext.i30.i64(trunc.i64.i30((%10 /u 2))) + 1073741824 * (%9 /u 2147483648)  ^  (%4)[0].0.0[i2 + 156];
;                  |      |   %cond.i.i.i = (trunc.i64.i1(%10) == 0) ? 0 : -5403634167711393303;
;                  |      |   %xor9.i.i.i = %xor.i.i.i37  ^  %cond.i.i.i;
;                  |      |   (%4)[0].0.0[i2] = %xor9.i.i.i;
;                  |      |   %9 = %10;
;                  |      + END LOOP
;                  |
;                  |
;                  |      + DO i2 = 0, 154, 1   <DO_LOOP>
;                  |      |   %12 = (%4)[0].0.0[i2 + 156];
;                  |      |   %13 = (%4)[0].0.0[i2 + 157];
;                  |      |   %xor30.i.i.i = zext.i30.i64(trunc.i64.i30((%13 /u 2))) + 1073741824 * (%12 /u 2147483648)  ^  (%4)[0].0.0[i2];
;                  |      |   %cond33.i.i.i = (trunc.i64.i1(%13) == 0) ? 0 : -5403634167711393303;
;                  |      |   %xor34.i.i.i = %xor30.i.i.i  ^  %cond33.i.i.i;
;                  |      |   (%4)[0].0.0[i2 + 156] = %xor34.i.i.i;
;                  |      + END LOOP
;                  |
;                  |      %15 = (%4)[0].0.0[311];
;                  |      %16 = (%4)[0].0.0[0];
;                  |      %xor51.i.i.i = zext.i30.i64(trunc.i64.i30((%16 /u 2))) + 1073741824 * (%15 /u 2147483648)  ^  (%4)[0].0.0[155];
;                  |      %cond54.i.i.i = (trunc.i64.i1(%16) == 0) ? 0 : -5403634167711393303;
;                  |      %xor55.i.i.i = %xor51.i.i.i  ^  %cond54.i.i.i;
;                  |      (%4)[0].0.0[311] = %xor55.i.i.i;
;                  |      %6 = %16;
;                  |      %7 = 1;
;                  |   }
;                  |   else
;                  |   {
;                  |      %7 = %7  +  1;
;                  |   }
;                  + END LOOP
;            END REGION

; CHECK: BEGIN REGION { }
; CHECK-NOT: modified

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%class.Random64 = type { %"class.std::mersenne_twister_engine" }
%"class.std::mersenne_twister_engine" = type { [312 x i64], i64 }

; Function Attrs: norecurse uwtable
define dso_local noundef i32 @main(i32 noundef %argc, ptr nocapture noundef readonly %argv) local_unnamed_addr #3 personality ptr @__gxx_personality_v0 {
entry:
  %0 = add i32 %argc, -4
  %1 = icmp ult i32 %0, -2
  br i1 %1, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %cleanup

if.end:                                           ; preds = %entry
  %cmp2 = icmp eq i32 %argc, 3
  br i1 %cmp2, label %if.then3, label %if.end5

if.then3:                                         ; preds = %if.end
  %arrayidx = getelementptr inbounds ptr, ptr %argv, i64 2
  %2 = load ptr, ptr %arrayidx, align 8, !tbaa !3
  %call.i = tail call i64 @strtol(ptr nocapture noundef nonnull %2, ptr noundef null, i32 noundef 10) #7
  %conv.i = trunc i64 %call.i to i32
  br label %if.end5

if.end5:                                          ; preds = %if.then3, %if.end
  %iters.0 = phi i32 [ %conv.i, %if.then3 ], [ 100, %if.end ]
  %arrayidx6 = getelementptr inbounds ptr, ptr %argv, i64 1
  %3 = load ptr, ptr %arrayidx6, align 8, !tbaa !3
  %call9 = tail call noalias noundef nonnull dereferenceable(2504) ptr @_Znwm(i64 noundef 2504) #8
  %4 = bitcast ptr %call9 to ptr
  %arrayidx.i.i.i = getelementptr inbounds %class.Random64, ptr %4, i64 0, i32 0, i32 0, i64 0, !intel-tbaa !7
  store i64 7, ptr %arrayidx.i.i.i, align 8, !tbaa !7
  br label %_ZN8Random64C2Em.exit

for.body.i.i.i:                                   ; preds = %for.body.i.i.i
  %5 = phi i64 [ %add.i.i.i, %for.body.i.i.i ]
  %__i.019.i.i.i = phi i64 [ %inc.i.i.i, %for.body.i.i.i ]
  %shr.i.i.i = lshr i64 %5, 62
  %xor.i.i.i = xor i64 %shr.i.i.i, %5
  %mul.i.i.i = mul i64 %xor.i.i.i, 6364136223846793005
  %rem.i.i.lhs.trunc.i.i.i = trunc i64 %__i.019.i.i.i to i16
  %rem.i.i18.i.i.i = urem i16 %rem.i.i.lhs.trunc.i.i.i, 312
  %rem.i.i.zext.i.i.i = zext i16 %rem.i.i18.i.i.i to i64
  %add.i.i.i = add i64 %mul.i.i.i, %rem.i.i.zext.i.i.i
  %arrayidx7.i.i.i = getelementptr inbounds %class.Random64, ptr %4, i64 0, i32 0, i32 0, i64 %__i.019.i.i.i, !intel-tbaa !7
  store i64 %add.i.i.i, ptr %arrayidx7.i.i.i, align 8, !tbaa !7
  %inc.i.i.i = add nuw nsw i64 %__i.019.i.i.i, 1
  %exitcond.not.i.i.i = icmp eq i64 %inc.i.i.i, 312
  br i1 %exitcond.not.i.i.i, label %_ZN8Random64C2Em.exit, label %for.body.i.i.i, !llvm.loop !12

_ZN8Random64C2Em.exit:                            ; preds = %_ZN8Random64C2Em.exit
  %cmp1040 = icmp sgt i32 %iters.0, 0
  br i1 %cmp1040, label %for.body.lr.ph, label %cleanup

for.body.lr.ph:                                   ; preds = %_ZN8Random64C2Em.exit
  %arrayidx42.i.i.i = getelementptr inbounds %class.Random64, ptr %4, i64 0, i32 0, i32 0, i64 311
  %arrayidx49.i.i.i = getelementptr inbounds %class.Random64, ptr %4, i64 0, i32 0, i32 0, i64 155
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %_ZN8Random644NextEv.exit
  %6 = phi i64 [ 7, %for.body.lr.ph ], [ %18, %_ZN8Random644NextEv.exit ]
  %7 = phi i64 [ 312, %for.body.lr.ph ], [ %20, %_ZN8Random644NextEv.exit ]
  %i.041 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %_ZN8Random644NextEv.exit ]
  %cmp.i.i = icmp ugt i64 %7, 311
  br i1 %cmp.i.i, label %for.body.i.i.i39.preheader, label %entry.if.end_crit_edge.i.i

for.body.i.i.i39.preheader:                       ; preds = %for.body
  br label %for.body.i.i.i39

entry.if.end_crit_edge.i.i:                       ; preds = %for.body
  %arrayidx.phi.trans.insert.i.i = getelementptr inbounds %class.Random64, ptr %4, i64 0, i32 0, i32 0, i64 %7
  %.pre.i.i = load i64, ptr %arrayidx.phi.trans.insert.i.i, align 8, !tbaa !14
  %8 = add nuw nsw i64 %7, 1
  br label %_ZN8Random644NextEv.exit

for.body.i.i.i39:                                 ; preds = %for.body.i.i.i39.preheader, %for.body.i.i.i39
  %9 = phi i64 [ %10, %for.body.i.i.i39 ], [ %6, %for.body.i.i.i39.preheader ]
  %__k.078.i.i.i = phi i64 [ %add.i.i.i34, %for.body.i.i.i39 ], [ 0, %for.body.i.i.i39.preheader ]
  %arrayidx.i.i.i33 = getelementptr inbounds %class.Random64, ptr %4, i64 0, i32 0, i32 0, i64 %__k.078.i.i.i, !intel-tbaa !7
  %and.i.i.i = and i64 %9, -2147483648
  %add.i.i.i34 = add nuw nsw i64 %__k.078.i.i.i, 1
  %arrayidx3.i.i.i = getelementptr inbounds %class.Random64, ptr %4, i64 0, i32 0, i32 0, i64 %add.i.i.i34, !intel-tbaa !7
  %10 = load i64, ptr %arrayidx3.i.i.i, align 8, !tbaa !7
  %and4.i.i.i = and i64 %10, 2147483646
  %or.i.i.i = or i64 %and4.i.i.i, %and.i.i.i
  %add6.i.i.i = add nuw nsw i64 %__k.078.i.i.i, 156
  %arrayidx7.i.i.i35 = getelementptr inbounds %class.Random64, ptr %4, i64 0, i32 0, i32 0, i64 %add6.i.i.i, !intel-tbaa !7
  %11 = load i64, ptr %arrayidx7.i.i.i35, align 8, !tbaa !7
  %shr.i.i.i36 = lshr exact i64 %or.i.i.i, 1
  %xor.i.i.i37 = xor i64 %shr.i.i.i36, %11
  %and8.i.i.i = and i64 %10, 1
  %tobool.not.i.i.i = icmp eq i64 %and8.i.i.i, 0
  %cond.i.i.i = select i1 %tobool.not.i.i.i, i64 0, i64 -5403634167711393303
  %xor9.i.i.i = xor i64 %xor.i.i.i37, %cond.i.i.i
  store i64 %xor9.i.i.i, ptr %arrayidx.i.i.i33, align 8, !tbaa !7
  %exitcond.not.i.i.i38 = icmp eq i64 %add.i.i.i34, 156
  br i1 %exitcond.not.i.i.i38, label %for.body16.i.i.i.preheader, label %for.body.i.i.i39, !llvm.loop !15

for.body16.i.i.i.preheader:                       ; preds = %for.body.i.i.i39
  br label %for.body16.i.i.i

for.body16.i.i.i:                                 ; preds = %for.body16.i.i.i.preheader, %for.body16.i.i.i
  %__k12.079.i.i.i = phi i64 [ %add22.i.i.i, %for.body16.i.i.i ], [ 156, %for.body16.i.i.i.preheader ]
  %arrayidx19.i.i.i = getelementptr inbounds %class.Random64, ptr %4, i64 0, i32 0, i32 0, i64 %__k12.079.i.i.i, !intel-tbaa !7
  %12 = load i64, ptr %arrayidx19.i.i.i, align 8, !tbaa !7
  %and20.i.i.i = and i64 %12, -2147483648
  %add22.i.i.i = add nuw nsw i64 %__k12.079.i.i.i, 1
  %arrayidx23.i.i.i = getelementptr inbounds %class.Random64, ptr %4, i64 0, i32 0, i32 0, i64 %add22.i.i.i, !intel-tbaa !7
  %13 = load i64, ptr %arrayidx23.i.i.i, align 8, !tbaa !7
  %and24.i.i.i = and i64 %13, 2147483646
  %or25.i.i.i = or i64 %and24.i.i.i, %and20.i.i.i
  %add27.i.i.i = add nsw i64 %__k12.079.i.i.i, -156
  %arrayidx28.i.i.i = getelementptr inbounds %class.Random64, ptr %4, i64 0, i32 0, i32 0, i64 %add27.i.i.i, !intel-tbaa !7
  %14 = load i64, ptr %arrayidx28.i.i.i, align 8, !tbaa !7
  %shr29.i.i.i = lshr exact i64 %or25.i.i.i, 1
  %xor30.i.i.i = xor i64 %shr29.i.i.i, %14
  %and31.i.i.i = and i64 %13, 1
  %tobool32.not.i.i.i = icmp eq i64 %and31.i.i.i, 0
  %cond33.i.i.i = select i1 %tobool32.not.i.i.i, i64 0, i64 -5403634167711393303
  %xor34.i.i.i = xor i64 %xor30.i.i.i, %cond33.i.i.i
  store i64 %xor34.i.i.i, ptr %arrayidx19.i.i.i, align 8, !tbaa !7
  %exitcond80.not.i.i.i = icmp eq i64 %add22.i.i.i, 311
  br i1 %exitcond80.not.i.i.i, label %_ZNSt23mersenne_twister_engine.exit.i.i, label %for.body16.i.i.i, !llvm.loop !16

_ZNSt23mersenne_twister_engine.exit.i.i: ; preds = %for.body16.i.i.i
  %15 = load i64, ptr %arrayidx42.i.i.i, align 8, !tbaa !7
  %and43.i.i.i = and i64 %15, -2147483648
  %16 = load i64, ptr %arrayidx.i.i.i, align 8, !tbaa !7
  %and46.i.i.i = and i64 %16, 2147483646
  %or47.i.i.i = or i64 %and46.i.i.i, %and43.i.i.i
  %17 = load i64, ptr %arrayidx49.i.i.i, align 8, !tbaa !7
  %shr50.i.i.i = lshr exact i64 %or47.i.i.i, 1
  %xor51.i.i.i = xor i64 %shr50.i.i.i, %17
  %and52.i.i.i = and i64 %16, 1
  %tobool53.not.i.i.i = icmp eq i64 %and52.i.i.i, 0
  %cond54.i.i.i = select i1 %tobool53.not.i.i.i, i64 0, i64 -5403634167711393303
  %xor55.i.i.i = xor i64 %xor51.i.i.i, %cond54.i.i.i
  store i64 %xor55.i.i.i, ptr %arrayidx42.i.i.i, align 8, !tbaa !7
  br label %_ZN8Random644NextEv.exit

_ZN8Random644NextEv.exit:                         ; preds = %entry.if.end_crit_edge.i.i, %_ZNSt23mersenne_twister_engine.exit.i.i
  %18 = phi i64 [ %16, %_ZNSt23mersenne_twister_engine.exit.i.i ], [ %6, %entry.if.end_crit_edge.i.i ]
  %19 = phi i64 [ %16, %_ZNSt23mersenne_twister_engine.exit.i.i ], [ %.pre.i.i, %entry.if.end_crit_edge.i.i ]
  %20 = phi i64 [ 1, %_ZNSt23mersenne_twister_engine.exit.i.i ], [ %8, %entry.if.end_crit_edge.i.i ]
  %inc = add nuw nsw i32 %i.041, 1
  %exitcond.not = icmp eq i32 %inc, %iters.0
  br i1 %exitcond.not, label %cleanup.loopexit, label %for.body, !llvm.loop !17

cleanup.loopexit:                                 ; preds = %_ZN8Random644NextEv.exit
  br label %cleanup

cleanup:                                          ; preds = %cleanup.loopexit, %_ZN8Random64C2Em.exit, %if.then
  %retval.0 = phi i32 [ 1, %if.then ], [ 0, %_ZN8Random64C2Em.exit ], [ 0, %cleanup.loopexit ]
  ret i32 %retval.0
}

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #5

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: mustprogress nofree nounwind willreturn
declare dso_local i64 @strtol(ptr noundef readonly, ptr nocapture noundef, i32 noundef) local_unnamed_addr #4

attributes #0 = { nofree "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nofree nounwind }
attributes #3 = { norecurse uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { mustprogress nofree nounwind willreturn "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #5 = { nobuiltin allocsize(0) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #6 = { nofree uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #7 = { nounwind }
attributes #8 = { builtin allocsize(0) }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"pointer@_ZTSPc", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !11, i64 0}
!8 = !{!"struct@_ZTS8Random64", !9, i64 0}
!9 = !{!"struct@_ZTSSt23mersenne_twister_engine", !10, i64 0, !11, i64 2496}
!10 = !{!"array@_ZTSA312_m", !11, i64 0}
!11 = !{!"long", !5, i64 0}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.mustprogress"}
!14 = !{!9, !11, i64 0}
!15 = distinct !{!15, !13}
!16 = distinct !{!16, !13}
!17 = distinct !{!17, !13}
