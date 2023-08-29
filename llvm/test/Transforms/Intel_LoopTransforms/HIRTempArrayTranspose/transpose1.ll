; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-temp-array-transpose,print<hir>" -hir-temp-array-transpose-allow-unknown-sizes -disable-output 2>&1 | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-temp-array-transpose" -print-changed -hir-temp-array-transpose-allow-unknown-sizes -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

; Check that we successfully transpose the array for non-unit stride access
; (%arg4)[i2 + sext.i32.i64(%arg1) * i3];

;         BEGIN REGION { }
;               + DO i1 = 0, sext.i32.i64(%arg) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;               |   %load84 = (%phi41)[i1];
;               |
;               |   + DO i2 = 0, sext.i32.i64(%arg1) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>  <LEGAL_MAX_TC = 2147483647>
;               |   |   %load88 = (%arg6)[0].0[i2];
;               |   |   %load91 = (%arg5)[sext.i32.i64(%arg1) * i1 + i2];
;               |   |   %phi96 = %load91;
;               |   |
;               |   |      %phi108 = %load91;
;               |   |   + DO i3 = 0, sext.i32.i64(%arg2) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;               |   |   |   %load111 = (%arg3)[sext.i32.i64(%arg2) * i1 + i3];
;               |   |   |   %load115 = (%arg4)[i2 + sext.i32.i64(%arg1) * i3];
;               |   |   |   %phi108 = (%load115 * %load111)  +  %phi108;
;               |   |   + END LOOP
;               |   |      %phi96 = %phi108;
;               |   |
;               |   |   %load98 = (%phi)[i2];
;               |   |   (%arg5)[sext.i32.i64(%arg1) * i1 + i2] = %load91 + -1 * ((%load84 * %load88) + %load98) + (%arg2 * %load88) + %phi96;
;               |   + END LOOP
;               + END LOOP
;         END REGION

; CHECK:  BEGIN REGION { modified }
; CHECK:        %call6 = @llvm.stacksave.p0();
; CHECK:        %TranspTmpArr = alloca 4 * (sext.i32.i64(%arg1) * sext.i32.i64(%arg2));
; CHECK:        + DO i1 = 0, sext.i32.i64(%arg1) + -1, 1
; CHECK:        |   + DO i2 = 0, sext.i32.i64(%arg2) + -1, 1
; CHECK:        |   |   (%TranspTmpArr)[i1][i2] = (%arg4)[i2][i1];
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:        + DO i1 = 0, sext.i32.i64(%arg) + -1, 1
; CHECK:        |   %load84 = (%phi41)[i1];
; CHECK:        |
; CHECK:        |   + DO i2 = 0, sext.i32.i64(%arg1) + -1, 1
; CHECK:        |   |   %load88 = (%arg6)[0].0[i2];
; CHECK:        |   |   %load91 = (%arg5)[sext.i32.i64(%arg1) * i1 + i2];
; CHECK:        |   |   %phi96 = %load91;
; CHECK:        |   |
; CHECK:        |   |      %phi108 = %load91;
; CHECK:        |   |   + DO i3 = 0, sext.i32.i64(%arg2) + -1, 1
; CHECK:        |   |   |   %load111 = (%arg3)[sext.i32.i64(%arg2) * i1 + i3];
; CHECK:        |   |   |   %load115 = (%TranspTmpArr)[i2][i3];
; CHECK:        |   |   |   %phi108 = (%load115 * %load111)  +  %phi108;
; CHECK:        |   |   + END LOOP
; CHECK:        |   |      %phi96 = %phi108;
; CHECK:        |   |
; CHECK:        |   |   %load98 = (%phi)[i2];
; CHECK:        |   |   (%arg5)[sext.i32.i64(%arg1) * i1 + i2] = %load91 + -1 * ((%load84 * %load88) + %load98) + (%arg2 * %load88) + %phi96;
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:        @llvm.stackrestore.p0(&((%call6)[0]));
; CHECK:  END REGION


; Verify that pass is dumped with print-changed when it triggers.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED: Dump After HIRTempArrayTranspose

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.eggs.8 = type { [1024 x i32] }

@global.5 = external hidden unnamed_addr constant [49 x i8], align 1

; Function Attrs: norecurse uwtable
define hidden fastcc void @snork(i32 noundef %arg, i32 noundef %arg1, i32 noundef %arg2, ptr noalias nocapture noundef readonly %arg3, ptr noalias nocapture noundef readonly %arg4, ptr noalias nocapture noundef %arg5, %struct.eggs.8* noalias nocapture noundef readonly byval(%struct.eggs.8) align 8 %arg6) unnamed_addr #0 personality ptr bitcast (i32 (...)* @pluto to ptr) {
bb:
  %sext = sext i32 %arg1 to i64
  %icmp = icmp slt i32 %arg1, 0
  br i1 %icmp, label %bb7, label %bb8

bb7:                                              ; preds = %bb
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef getelementptr inbounds ([49 x i8], ptr @global.5, i64 0, i64 0)) #5
  unreachable

bb8:                                              ; preds = %bb
  %icmp9 = icmp eq i32 %arg1, 0
  br i1 %icmp9, label %bb11, label %bb10

bb10:                                             ; preds = %bb8
  %shl = shl nuw nsw i64 %sext, 2
  %call = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %shl) #6
  %bitcast = bitcast ptr %call to ptr
  tail call void @llvm.memset.p0.i64(ptr nonnull align 4 %call, i8 0, i64 %shl, i1 false), !tbaa !6
  br label %bb11

bb11:                                             ; preds = %bb10, %bb8
  %phi = phi ptr [ %bitcast, %bb10 ], [ null, %bb8 ]
  %sext12 = sext i32 %arg to i64
  %icmp13 = icmp slt i32 %arg, 0
  br i1 %icmp13, label %bb14, label %bb16

bb14:                                             ; preds = %bb11
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef getelementptr inbounds ([49 x i8], ptr @global.5, i64 0, i64 0)) #5
          to label %bb15 unwind label %bb65

bb15:                                             ; preds = %bb14
  unreachable

bb16:                                             ; preds = %bb11
  %icmp17 = icmp eq i32 %arg, 0
  br i1 %icmp17, label %bb40, label %bb18

bb18:                                             ; preds = %bb16
  %shl19 = shl nuw nsw i64 %sext12, 2
  %invoke = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %shl19) #6
          to label %bb20 unwind label %bb65

bb20:                                             ; preds = %bb18
  %bitcast21 = bitcast ptr %invoke to ptr
  tail call void @llvm.memset.p0.i64(ptr nonnull align 4 %invoke, i8 0, i64 %shl19, i1 false), !tbaa !6
  %icmp22 = icmp sgt i32 %arg2, 0
  br i1 %icmp22, label %bb24, label %bb23

bb23:                                             ; preds = %bb20
  br label %bb69

bb24:                                             ; preds = %bb20
  %zext = zext i32 %arg2 to i64
  br label %bb25

bb25:                                             ; preds = %bb34, %bb24
  %phi26 = phi i64 [ 0, %bb24 ], [ %add36, %bb34 ]
  %getelementptr = getelementptr inbounds i32, ptr %bitcast21, i64 %phi26, !intel-tbaa !6
  store i32 0, ptr %getelementptr, align 4, !tbaa !6
  %mul = mul nuw nsw i64 %phi26, %zext
  br label %bb27

bb27:                                             ; preds = %bb27, %bb25
  %phi28 = phi i32 [ 0, %bb25 ], [ %add31, %bb27 ]
  %phi29 = phi i64 [ 0, %bb25 ], [ %add32, %bb27 ]
  %add = add nuw nsw i64 %phi29, %mul
  %getelementptr30 = getelementptr inbounds i32, ptr %arg3, i64 %add
  %load = load i32, ptr %getelementptr30, align 4, !tbaa !6
  %add31 = add nsw i32 %phi28, %load
  %add32 = add nuw nsw i64 %phi29, 1
  %icmp33 = icmp eq i64 %add32, %zext
  br i1 %icmp33, label %bb34, label %bb27, !llvm.loop !10

bb34:                                             ; preds = %bb27
  %phi35 = phi i32 [ %add31, %bb27 ]
  store i32 %phi35, ptr %getelementptr, align 4, !tbaa !6
  %add36 = add nuw nsw i64 %phi26, 1
  %icmp37 = icmp eq i64 %add36, %sext12
  br i1 %icmp37, label %bb38, label %bb25, !llvm.loop !12

bb38:                                             ; preds = %bb34
  br label %bb40

bb39:                                             ; preds = %bb69
  br label %bb40

bb40:                                             ; preds = %bb39, %bb38, %bb16
  %phi41 = phi ptr [ null, %bb16 ], [ %bitcast21, %bb38 ], [ %bitcast21, %bb39 ]
  %icmp42 = icmp sgt i32 %arg1, 0
  br i1 %icmp42, label %bb43, label %bb128

bb43:                                             ; preds = %bb40
  %icmp44 = icmp sgt i32 %arg2, 0
  br i1 %icmp44, label %bb46, label %bb45

bb45:                                             ; preds = %bb43
  br label %bb123

bb46:                                             ; preds = %bb43
  %zext47 = zext i32 %arg2 to i64
  br label %bb48

bb48:                                             ; preds = %bb61, %bb46
  %phi49 = phi i64 [ 0, %bb46 ], [ %add63, %bb61 ]
  %getelementptr50 = getelementptr inbounds i32, ptr %phi, i64 %phi49, !intel-tbaa !6
  store i32 0, ptr %getelementptr50, align 4, !tbaa !6
  br label %bb51

bb51:                                             ; preds = %bb51, %bb48
  %phi52 = phi i32 [ 0, %bb48 ], [ %add58, %bb51 ]
  %phi53 = phi i64 [ 0, %bb48 ], [ %add59, %bb51 ]
  %mul54 = mul nsw i64 %phi53, %sext
  %add55 = add nsw i64 %mul54, %phi49
  %getelementptr56 = getelementptr inbounds i32, ptr %arg4, i64 %add55
  %load57 = load i32, ptr %getelementptr56, align 4, !tbaa !6
  %add58 = add nsw i32 %phi52, %load57
  %add59 = add nuw nsw i64 %phi53, 1
  %icmp60 = icmp eq i64 %add59, %zext47
  br i1 %icmp60, label %bb61, label %bb51, !llvm.loop !13

bb61:                                             ; preds = %bb51
  %phi62 = phi i32 [ %add58, %bb51 ]
  store i32 %phi62, ptr %getelementptr50, align 4, !tbaa !6
  %add63 = add nuw nsw i64 %phi49, 1
  %icmp64 = icmp eq i64 %add63, %sext
  br i1 %icmp64, label %bb74, label %bb48, !llvm.loop !14

bb65:                                             ; preds = %bb18, %bb14
  %landingpad = landingpad { ptr, i32 }
          cleanup
  %icmp66 = icmp eq ptr %phi, null
  br i1 %icmp66, label %bb138, label %bb67

bb67:                                             ; preds = %bb65
  %bitcast68 = bitcast ptr %phi to ptr
  tail call void @_ZdlPv(ptr noundef nonnull %bitcast68) #7
  br label %bb138

bb69:                                             ; preds = %bb69, %bb23
  %phi70 = phi i64 [ %add72, %bb69 ], [ 0, %bb23 ]
  %getelementptr71 = getelementptr inbounds i32, ptr %bitcast21, i64 %phi70, !intel-tbaa !6
  store i32 0, ptr %getelementptr71, align 4, !tbaa !6
  %add72 = add nuw nsw i64 %phi70, 1
  %icmp73 = icmp eq i64 %add72, %sext12
  br i1 %icmp73, label %bb39, label %bb69, !llvm.loop !12

bb74:                                             ; preds = %bb61
  br label %bb76

bb75:                                             ; preds = %bb123
  br label %bb76

bb76:                                             ; preds = %bb75, %bb74
  br i1 %icmp17, label %bb128, label %bb77

bb77:                                             ; preds = %bb76
  %sext78 = sext i32 %arg2 to i64
  br label %bb79

bb79:                                             ; preds = %bb120, %bb77
  %phi80 = phi i64 [ 0, %bb77 ], [ %add121, %bb120 ]
  %mul81 = mul nsw i64 %phi80, %sext
  %mul82 = mul nsw i64 %phi80, %sext78
  %getelementptr83 = getelementptr inbounds i32, ptr %phi41, i64 %phi80, !intel-tbaa !6
  %load84 = load i32, ptr %getelementptr83, align 4, !tbaa !6
  br label %bb85

bb85:                                             ; preds = %bb95, %bb79
  %phi86 = phi i64 [ 0, %bb79 ], [ %add104, %bb95 ]
  %getelementptr87 = getelementptr inbounds %struct.eggs.8, %struct.eggs.8* %arg6, i64 0, i32 0, i64 %phi86, !intel-tbaa !15
  %load88 = load i32, ptr %getelementptr87, align 4, !tbaa !15
  %add89 = add nsw i64 %phi86, %mul81
  %getelementptr90 = getelementptr inbounds i32, ptr %arg5, i64 %add89
  %load91 = load i32, ptr %getelementptr90, align 4, !tbaa !6
  br i1 %icmp44, label %bb92, label %bb95

bb92:                                             ; preds = %bb85
  br label %bb106

bb93:                                             ; preds = %bb106
  %phi94 = phi i32 [ %add117, %bb106 ]
  br label %bb95

bb95:                                             ; preds = %bb93, %bb85
  %phi96 = phi i32 [ %load91, %bb85 ], [ %phi94, %bb93 ]
  %getelementptr97 = getelementptr inbounds i32, ptr %phi, i64 %phi86, !intel-tbaa !6
  %load98 = load i32, ptr %getelementptr97, align 4, !tbaa !6
  %mul99 = mul i32 %load84, %load88
  %mul100 = mul i32 %load88, %arg2
  %add101 = add i32 %mul99, %load98
  %add102 = add i32 %mul100, %load91
  %add103 = add i32 %add102, %phi96
  %sub = sub i32 %add103, %add101
  store i32 %sub, ptr %getelementptr90, align 4, !tbaa !6
  %add104 = add nuw nsw i64 %phi86, 1
  %icmp105 = icmp eq i64 %add104, %sext
  br i1 %icmp105, label %bb120, label %bb85, !llvm.loop !18

bb106:                                            ; preds = %bb106, %bb92
  %phi107 = phi i64 [ %add118, %bb106 ], [ 0, %bb92 ]
  %phi108 = phi i32 [ %add117, %bb106 ], [ %load91, %bb92 ]
  %add109 = add nsw i64 %phi107, %mul82
  %getelementptr110 = getelementptr inbounds i32, ptr %arg3, i64 %add109
  %load111 = load i32, ptr %getelementptr110, align 4, !tbaa !6
  %mul112 = mul nsw i64 %phi107, %sext
  %add113 = add nsw i64 %mul112, %phi86
  %getelementptr114 = getelementptr inbounds i32, ptr %arg4, i64 %add113
  %load115 = load i32, ptr %getelementptr114, align 4, !tbaa !6
  %mul116 = mul nsw i32 %load115, %load111
  %add117 = add nsw i32 %mul116, %phi108
  %add118 = add nuw nsw i64 %phi107, 1
  %icmp119 = icmp eq i64 %add118, %sext78
  br i1 %icmp119, label %bb93, label %bb106, !llvm.loop !19

bb120:                                            ; preds = %bb95
  %add121 = add nuw nsw i64 %phi80, 1
  %icmp122 = icmp eq i64 %add121, %sext12
  br i1 %icmp122, label %bb130, label %bb79, !llvm.loop !20

bb123:                                            ; preds = %bb123, %bb45
  %phi124 = phi i64 [ %add126, %bb123 ], [ 0, %bb45 ]
  %getelementptr125 = getelementptr inbounds i32, ptr %phi, i64 %phi124, !intel-tbaa !6
  store i32 0, ptr %getelementptr125, align 4, !tbaa !6
  %add126 = add nuw nsw i64 %phi124, 1
  %icmp127 = icmp eq i64 %add126, %sext
  br i1 %icmp127, label %bb75, label %bb123, !llvm.loop !14

bb128:                                            ; preds = %bb76, %bb40
  %icmp129 = icmp eq ptr %phi41, null
  br i1 %icmp129, label %bb133, label %bb131

bb130:                                            ; preds = %bb120
  br label %bb131

bb131:                                            ; preds = %bb130, %bb128
  %bitcast132 = bitcast ptr %phi41 to ptr
  tail call void @_ZdlPv(ptr noundef nonnull %bitcast132) #7
  br label %bb133

bb133:                                            ; preds = %bb131, %bb128
  %icmp134 = icmp eq ptr %phi, null
  br i1 %icmp134, label %bb137, label %bb135

bb135:                                            ; preds = %bb133
  %bitcast136 = bitcast ptr %phi to ptr
  tail call void @_ZdlPv(ptr noundef nonnull %bitcast136) #7
  br label %bb137

bb137:                                            ; preds = %bb135, %bb133
  ret void

bb138:                                            ; preds = %bb67, %bb65
  resume { ptr, i32 } %landingpad
}

declare dso_local i32 @pluto(...)

; Function Attrs: nofree noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #1

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #2

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(ptr noundef) local_unnamed_addr #4

attributes #0 = { norecurse uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nofree noreturn "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nobuiltin allocsize(0) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #4 = { nobuiltin nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { noreturn }
attributes #6 = { allocsize(0) }
attributes #7 = { nounwind }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{i32 1, !"LTOPostLink", i32 1}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C++ TBAA"}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.mustprogress"}
!12 = distinct !{!12, !11}
!13 = distinct !{!13, !11}
!14 = distinct !{!14, !11}
!15 = !{!16, !7, i64 0}
!16 = !{!"struct@_ZTSSt5arrayIiLm1024EE", !17, i64 0}
!17 = !{!"array@_ZTSA1024_i", !7, i64 0}
!18 = distinct !{!18, !11}
!19 = distinct !{!19, !11}
!20 = distinct !{!20, !11}
