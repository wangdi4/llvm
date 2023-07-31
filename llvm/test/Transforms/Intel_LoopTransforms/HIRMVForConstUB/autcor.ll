; Check the test case from telecom/autcore00_data1

; HIR Before
; <0>       BEGIN REGION { }
; <257>           + DO i1 = 0, %0 + -1, 1   <DO_LOOP>
; <258>           |   + DO i2 = 0, sext.i16.i32(%32) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1>
; <11>            |   |   %66 = 0;
; <12>            |   |   if (i2 < 16)
; <12>            |   |   {
; <16>            |   |      %49 = 0;
; <259>           |   |
; <259>           |   |      + DO i3 = 0, -1 * i2 + 15, 1   <DO_LOOP>  <MAX_TC_EST = 16>
; <21>            |   |      |   %52 = (@input_buf)[0][i3];
; <25>            |   |      |   %56 = (@input_buf)[0][i2 + i3];
; <28>            |   |      |   %59 = (sext.i16.i32(%56) * sext.i16.i32(%52))  >>  4;
; <29>            |   |      |   %49 = %59  +  %49;
; <259>           |   |      + END LOOP
; <259>           |   |
; <37>            |   |      %66 = %49;
; <12>            |   |   }
; <43>            |   |   (%19)[i2] = (%66)/u65536;
; <258>           |   + END LOOP
; <257>           + END LOOP
; <0>       END REGION


; HIR After
; <0>       BEGIN REGION { }
; <257>           + DO i1 = 0, %0 + -1, 1   <DO_LOOP>
; <264>           |   if (%32 == 8)
; <264>           |   {
; <258>           |      + DO i2 = 0, 7, 1   <DO_LOOP>
; <11>            |      |   %66 = 0;
; <12>            |      |   if (i2 < 16)
; <12>            |      |   {
; <16>            |      |      %49 = 0;
; <259>           |      |
; <259>           |      |      + DO i3 = 0, -1 * i2 + 15, 1   <DO_LOOP>  <MAX_TC_EST = 16>
; <21>            |      |      |   %52 = (@input_buf)[0][i3];
; <25>            |      |      |   %56 = (@input_buf)[0][i2 + i3];
; <28>            |      |      |   %59 = (sext.i16.i32(%56) * sext.i16.i32(%52))  >>  4;
; <29>            |      |      |   %49 = %59  +  %49;
; <259>           |      |      + END LOOP
; <259>           |      |
; <37>            |      |      %66 = %49;
; <12>            |      |   }
; <43>            |      |   (%19)[i2] = (%66)/u65536;
; <258>           |      + END LOOP
; <264>           |   }
; <264>           |   else
; <264>           |   {
; <265>           |      + DO i2 = 0, sext.i16.i32(%32) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1>
; <267>           |      |   %66 = 0;
; <268>           |      |   if (i2 < 16)
; <268>           |      |   {
; <269>           |      |      %49 = 0;
; <270>           |      |
; <270>           |      |      + DO i3 = 0, -1 * i2 + 15, 1   <DO_LOOP>  <MAX_TC_EST = 16>
; <271>           |      |      |   %52 = (@input_buf)[0][i3];
; <272>           |      |      |   %56 = (@input_buf)[0][i2 + i3];
; <273>           |      |      |   %59 = (sext.i16.i32(%56) * sext.i16.i32(%52))  >>  4;
; <274>           |      |      |   %49 = %59  +  %49;
; <270>           |      |      + END LOOP
; <270>           |      |
; <275>           |      |      %66 = %49;
; <268>           |      |   }
; <276>           |      |   (%19)[i2] = (%66)/u65536;
; <265>           |      + END LOOP
; <264>           |   }
; <257>           + END LOOP
; <0>       END REGION

; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-mv-const-ub,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-details < %s 2>&1 | FileCheck %s
;
; CHECK: Function
; CHECK: <0> BEGIN REGION
; CHECK: DO i32 i2 = 0
; CHECK-SAME: %tmp30

; CHECK: Function
; CHECK: <0> BEGIN REGION
; CHECK: if (%tmp30 == 8)
; CHECK-NEXT: LINEAR
; CHECK-SAME: %tmp30
; CHECK: DO i32 i2 = 0, 7

; ModuleID = 'autcor2.ll'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%struct.blam = type { [16 x i8], [64 x i8], [16 x i8], [16 x i8], [16 x i8], i16, %struct.wibble, %struct.wibble, ptr, i32, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%struct.wibble = type { i8, i8, i8, i8 }
%struct.barney = type { i32, i32, i16, i32, i32, i32, i32, ptr }
%struct.ham = type { [128 x i8], i32, ptr, i32, i32 }

@global = external hidden unnamed_addr constant [17 x i8], align 1
@global.1 = external hidden unnamed_addr global ptr, align 4
@global.2 = external hidden unnamed_addr constant [29 x i8], align 1
@global.3 = external hidden unnamed_addr constant [45 x i8], align 1
@global.4 = external hidden unnamed_addr constant [18 x i8], align 1
@global.5 = external hidden unnamed_addr constant [43 x i8], align 1
@global.6 = external hidden unnamed_addr constant [16 x i16], align 2
@global.7 = external hidden unnamed_addr constant [8 x double], align 8
@global.8 = external hidden global [64 x i8], align 1
@global.9 = external hidden unnamed_addr constant [23 x i8], align 1
@global.10 = external hidden unnamed_addr constant [30 x i8], align 1
@global.11 = external hidden global %struct.blam, align 4

; Function Attrs: nounwind
define hidden i32 @widget(i32 %arg, i32 %arg1, ptr nocapture readonly %arg2) #0 {
bb:
  %tmp = alloca %struct.barney, align 4
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %tmp) #4
  %tmp4 = load ptr, ptr getelementptr inbounds (%struct.blam, ptr @global.11, i32 0, i32 20), align 4, !tbaa !3
  %tmp5 = icmp eq ptr %tmp4, @widget.12
  br i1 %tmp5, label %bb6, label %bb8

bb6:                                              ; preds = %bb
  %tmp7 = call noalias ptr @malloc(i32 32) #4
  br label %bb10

bb8:                                              ; preds = %bb
  %tmp9 = call ptr %tmp4(i32 32, ptr @global, i32 243) #4
  br label %bb10

bb10:                                             ; preds = %bb8, %bb6
  %tmp11 = phi ptr [ %tmp7, %bb6 ], [ %tmp9, %bb8 ]
  store ptr %tmp11, ptr @global.1, align 4, !tbaa !27
  %tmp12 = icmp eq ptr %tmp11, null
  br i1 %tmp12, label %bb14, label %bb16

bb14:                                             ; preds = %bb10
  tail call void (i32, ptr, ...) @wibble(i32 8, ptr @global.2, ptr @global, i32 245) #4
  %tmp15 = load ptr, ptr @global.1, align 4, !tbaa !27
  br label %bb16

bb16:                                             ; preds = %bb14, %bb10
  %tmp17 = phi ptr [ %tmp15, %bb14 ], [ %tmp11, %bb10 ]
  %tmp18 = icmp slt i32 %arg1, 2
  br i1 %tmp18, label %bb19, label %bb20

bb19:                                             ; preds = %bb16
  tail call void (ptr, ...) @wobble.13(ptr @global.3, ptr @global.4) #4
  tail call void (ptr, ...) @wobble.13(ptr @global.5, i32 8) #4
  br label %bb29

bb20:                                             ; preds = %bb16
  %tmp21 = icmp eq i32 %arg1, 2
  br i1 %tmp21, label %bb28, label %bb22

bb22:                                             ; preds = %bb20
  %tmp23 = getelementptr inbounds ptr, ptr %arg2, i32 2
  %tmp24 = load ptr, ptr %tmp23, align 4, !tbaa !29
  %tmp25 = tail call i32 @strtol(ptr nocapture nonnull %tmp24, ptr null, i32 10) #4
  %tmp26 = trunc i32 %tmp25 to i16
  %tmp27 = icmp eq i16 %tmp26, 0
  br i1 %tmp27, label %bb28, label %bb29

bb28:                                             ; preds = %bb22, %bb20
  tail call void (ptr, ...) @wobble.13(ptr @global.5, i32 8) #4
  br label %bb29

bb29:                                             ; preds = %bb28, %bb22, %bb19
  %tmp30 = phi i16 [ 8, %bb19 ], [ 8, %bb28 ], [ %tmp26, %bb22 ]
  %tmp31 = load ptr, ptr getelementptr inbounds (%struct.blam, ptr @global.11, i32 0, i32 23), align 4, !tbaa !31
  tail call void %tmp31() #4
  %tmp32 = icmp eq i32 %arg, 0
  br i1 %tmp32, label %bb33, label %bb35

bb33:                                             ; preds = %bb29
  %tmp34 = sext i16 %tmp30 to i32
  br label %bb76

bb35:                                             ; preds = %bb29
  %tmp36 = sext i16 %tmp30 to i32
  %tmp37 = icmp sgt i16 %tmp30, 0
  br label %bb38

bb38:                                             ; preds = %bb72, %bb35
  %tmp39 = phi i32 [ %tmp73, %bb72 ], [ 0, %bb35 ]
  br i1 %tmp37, label %bb40, label %bb72

bb40:                                             ; preds = %bb38
  br label %bb41

bb41:                                             ; preds = %bb63, %bb40
  %tmp42 = phi i32 [ %tmp69, %bb63 ], [ 16, %bb40 ]
  %tmp43 = phi i32 [ %tmp68, %bb63 ], [ 0, %bb40 ]
  %tmp44 = icmp slt i32 %tmp43, 16
  br i1 %tmp44, label %bb45, label %bb63

bb45:                                             ; preds = %bb41
  br label %bb46

bb46:                                             ; preds = %bb46, %bb45
  %tmp47 = phi i32 [ %tmp58, %bb46 ], [ 0, %bb45 ]
  %tmp48 = phi i32 [ %tmp59, %bb46 ], [ 0, %bb45 ]
  %tmp49 = getelementptr inbounds [16 x i16], ptr @global.6, i32 0, i32 %tmp48
  %tmp50 = load i16, ptr %tmp49, align 2, !tbaa !32
  %tmp51 = sext i16 %tmp50 to i32
  %tmp52 = add nuw nsw i32 %tmp48, %tmp43
  %tmp53 = getelementptr inbounds [16 x i16], ptr @global.6, i32 0, i32 %tmp52
  %tmp54 = load i16, ptr %tmp53, align 2, !tbaa !32
  %tmp55 = sext i16 %tmp54 to i32
  %tmp56 = mul nsw i32 %tmp55, %tmp51
  %tmp57 = ashr i32 %tmp56, 4
  %tmp58 = add nsw i32 %tmp57, %tmp47
  %tmp59 = add nuw nsw i32 %tmp48, 1
  %tmp60 = icmp eq i32 %tmp59, %tmp42
  br i1 %tmp60, label %bb61, label %bb46

bb61:                                             ; preds = %bb46
  %tmp62 = phi i32 [ %tmp58, %bb46 ]
  br label %bb63

bb63:                                             ; preds = %bb61, %bb41
  %tmp64 = phi i32 [ 0, %bb41 ], [ %tmp62, %bb61 ]
  %tmp65 = lshr i32 %tmp64, 16
  %tmp66 = trunc i32 %tmp65 to i16
  %tmp67 = getelementptr inbounds i16, ptr %tmp17, i32 %tmp43
  store i16 %tmp66, ptr %tmp67, align 2, !tbaa !32
  %tmp68 = add nuw nsw i32 %tmp43, 1
  %tmp69 = add nsw i32 %tmp42, -1
  %tmp70 = icmp eq i32 %tmp68, %tmp36
  br i1 %tmp70, label %bb71, label %bb41

bb71:                                             ; preds = %bb63
  br label %bb72

bb72:                                             ; preds = %bb71, %bb38
  %tmp73 = add nuw i32 %tmp39, 1
  %tmp74 = icmp eq i32 %tmp73, %arg
  br i1 %tmp74, label %bb75, label %bb38

bb75:                                             ; preds = %bb72
  br label %bb76

bb76:                                             ; preds = %bb75, %bb33
  %tmp77 = phi i32 [ %tmp34, %bb33 ], [ %tmp36, %bb75 ]
  %tmp78 = load ptr, ptr getelementptr inbounds (%struct.blam, ptr @global.11, i32 0, i32 24), align 4, !tbaa !33
  %tmp79 = tail call i32 %tmp78() #4
  %tmp80 = getelementptr inbounds %struct.barney, ptr %tmp, i32 0, i32 1
  store i32 %tmp79, ptr %tmp80, align 4, !tbaa !34
  store i32 %arg, ptr %tmp, align 4, !tbaa !37
  %tmp82 = icmp eq ptr %tmp17, null
  br i1 %tmp82, label %bb169, label %bb83

bb83:                                             ; preds = %bb76
  ret i32 0

bb169:                                            ; preds = %bb161, %bb157, %bb127, %bb76
  ret i32 1
}

; Function Attrs: nounwind
declare i32 @strtol(ptr readonly, ptr nocapture, i32) local_unnamed_addr #1

; Function Attrs: nounwind readnone
declare double @__log10_finite(double) local_unnamed_addr #2

; Function Attrs: nounwind
declare noalias ptr @malloc(i32) local_unnamed_addr #1

; Function Attrs: nounwind
declare hidden noalias ptr @widget.12(i32, ptr nocapture readnone, i32) #0

; Function Attrs: nounwind
declare hidden i32 @wobble(ptr nocapture readonly, i16 zeroext) #0

; Function Attrs: nounwind
declare hidden void @wobble.13(ptr, ...) unnamed_addr #0

; Function Attrs: nounwind
declare hidden void @baz(ptr nocapture readnone, ptr nocapture readonly, ...) unnamed_addr #0

; Function Attrs: nounwind
declare hidden void @wibble(i32, ptr, ...) unnamed_addr #0

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #3

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="slm" "target-features"="+aes,+cx16,+fxsr,+mmx,+pclmul,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="slm" "target-features"="+aes,+cx16,+fxsr,+mmx,+pclmul,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="slm" "target-features"="+aes,+cx16,+fxsr,+mmx,+pclmul,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind }
attributes #4 = { nounwind }
attributes #5 = { nounwind readnone }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2}

!0 = !{!"clang version 4.0.0 (trunk 21010) (llvm/branches/loopopt 21057)"}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{!4, !20, i64 188}
!4 = !{!"struct@THDef", !5, i64 0, !8, i64 16, !5, i64 80, !5, i64 96, !5, i64 112, !9, i64 128, !10, i64 130, !10, i64 134, !11, i64 140, !12, i64 144, !12, i64 148, !13, i64 152, !14, i64 156, !15, i64 160, !16, i64 164, !17, i64 168, !18, i64 172, !19, i64 176, !19, i64 180, !19, i64 184, !20, i64 188, !21, i64 192, !22, i64 196, !22, i64 200, !19, i64 204, !23, i64 208, !24, i64 212, !25, i64 216, !24, i64 220, !24, i64 224, !26, i64 228}
!5 = !{!"array@_ZTSA16_c", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!"array@_ZTSA64_c", !6, i64 0}
!9 = !{!"short", !6, i64 0}
!10 = !{!"struct@", !6, i64 0, !6, i64 1, !6, i64 2, !6, i64 3}
!11 = !{!"pointer@_ZTSPv", !6, i64 0}
!12 = !{!"int", !6, i64 0}
!13 = !{!"pointer@_ZTSPFiPKcPcE", !6, i64 0}
!14 = !{!"pointer@_ZTSPFiPcPKcS_E", !6, i64 0}
!15 = !{!"pointer@_ZTSPFiPKcE", !6, i64 0}
!16 = !{!"pointer@_ZTSPFicE", !6, i64 0}
!17 = !{!"pointer@_ZTSPFiPKcjE", !6, i64 0}
!18 = !{!"pointer@_ZTSPFjPcjE", !6, i64 0}
!19 = !{!"pointer@_ZTSPFjvE", !6, i64 0}
!20 = !{!"pointer@_ZTSPFPvjPKciE", !6, i64 0}
!21 = !{!"pointer@_ZTSPFvPvPKciE", !6, i64 0}
!22 = !{!"pointer@_ZTSPFvvE", !6, i64 0}
!23 = !{!"pointer@_ZTSPFviPKcPcE", !6, i64 0}
!24 = !{!"unspecified pointer", !6, i64 0}
!25 = !{!"pointer@_ZTSPFivE", !6, i64 0}
!26 = !{!"pointer@_ZTSPFiPKcjS0_E", !6, i64 0}
!27 = !{!28, !28, i64 0}
!28 = !{!"pointer@_ZTSPc", !6, i64 0}
!29 = !{!30, !30, i64 0}
!30 = !{!"pointer@_ZTSPKc", !6, i64 0}
!31 = !{!4, !22, i64 200}
!32 = !{!9, !9, i64 0}
!33 = !{!4, !19, i64 204}
!34 = !{!35, !36, i64 4}
!35 = !{!"struct@THTestResults", !12, i64 0, !36, i64 4, !9, i64 8, !12, i64 12, !12, i64 16, !12, i64 20, !12, i64 24, !30, i64 28}
!36 = !{!"long", !6, i64 0}
!37 = !{!35, !12, i64 0}
!38 = !{!39, !39, i64 0}
!39 = !{!"double", !6, i64 0}
!40 = !{!35, !12, i64 12}
!41 = !{!35, !12, i64 16}
!42 = !{!35, !12, i64 20}
!43 = !{!35, !12, i64 24}
!44 = !{!35, !30, i64 28}
!45 = !{!35, !9, i64 8}
!46 = !{!4, !24, i64 212}
