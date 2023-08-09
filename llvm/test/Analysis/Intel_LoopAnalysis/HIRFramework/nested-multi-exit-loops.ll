; RUN: opt < %s -hir-cost-model-throttling=0 -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s


; This test was compfailing because of incorrect lexical link formation for
; nested multi-exit i2-i3 loops and a def-level bug in parser.
; I did not try to reduce the test case much because of complicated CFG
; required to trigger the bugs.

; Parser fix is checked using verification.

; CHECK: + DO i1
; CHECK: |
; CHECK: | + UNKNOWN LOOP i2
; CHECK: | |
; CHECK: | |  + UNKNOWN LOOP i3
; CHECK: | |  |    goto bb66;
; CHECK: | |  + END LOOP
; CHECK: | |
; CHECK: | |   goto bb79;
; CHECK: | |   bb66:
; CHECK: | |
; CHECK: | |   goto bb81;
; CHECK: | + END LOOP
; CHECK: |
; CHECK: | bb81:
; CHECK: | bb79:
; CHECK: | + UNKNOWN LOOP i2
; CHECK: | |
; CHECK: | |  + UNKNOWN LOOP i3
; CHECK: | |  + END LOOP
; CHECK: | |
; CHECK: | + END LOOP
; CHECK: + END LOOP

; ModuleID = 'nested-multi-exit-loops.ll'
source_filename = "nested-multi-exit-loops.ll"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.uExpressionValue = type { i32, %union.anon }
%union.anon = type { double }
%struct.T_SKTREE = type { ptr, ptr, ptr, ptr, ptr, ptr }
%struct.IMPLEMENTATION = type { i32, ptr, ptr, i32, ptr, i32, ptr }
%struct.PARAM = type { ptr, ptr, i32, ptr, i32, ptr, ptr }
%struct.PARAM_PROPS = type { ptr, ptr, i32, ptr, ptr, i32, ptr, i32, i32, i32, i32, ptr }

@implist = external hidden unnamed_addr global ptr, align 8
@.str.606 = external hidden unnamed_addr constant [21 x i8], align 1
@.str.47.608 = external hidden unnamed_addr constant [51 x i8], align 1
@.str.48.609 = external hidden unnamed_addr constant [45 x i8], align 1
@.str.49.610 = external hidden unnamed_addr constant [46 x i8], align 1
@.str.72 = external hidden unnamed_addr constant [51 x i8], align 1
@.str.73 = external hidden unnamed_addr constant [60 x i8], align 1
@.str.977 = external hidden unnamed_addr constant [3 x i8], align 1
@.str.2.1260 = external hidden unnamed_addr constant [3 x i8], align 1
@.str.1.10510 = external hidden unnamed_addr constant [7 x i8], align 1

declare dso_local noalias ptr @malloc(i64) local_unnamed_addr

declare dso_local void @free(ptr nocapture) local_unnamed_addr

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

declare dso_local nonnull ptr @__ctype_tolower_loc() local_unnamed_addr

declare dso_local i64 @strlen(ptr nocapture) local_unnamed_addr

declare dso_local i64 @strtol(ptr readonly, ptr nocapture, i32) local_unnamed_addr

declare dso_local ptr @strcpy(ptr noalias returned, ptr noalias nocapture readonly) local_unnamed_addr

define hidden i32 @IntParameterEvaluator(i32 %arg, ptr nocapture readonly %arg1, ptr nocapture %arg2, ptr %arg3) {
bb:
  %i = alloca ptr, align 8
  %i4 = alloca ptr, align 8
  %i5 = alloca ptr, align 8
  %i6 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i6)
  %i7 = bitcast ptr %i4 to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i7)
  %i8 = bitcast ptr %i5 to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i8)
  %i9 = icmp sgt i32 %arg, 0
  br i1 %i9, label %bb10, label %bb180

bb10:                                             ; preds = %bb
  %i11 = sext i32 %arg to i64
  br label %bb12

bb12:                                             ; preds = %bb176, %bb10
  %i13 = phi i64 [ 0, %bb10 ], [ %i177, %bb176 ]
  %i14 = getelementptr inbounds %struct.uExpressionValue, ptr %arg2, i64 %i13, i32 0
  store i32 1, ptr %i14, align 8
  %i15 = getelementptr inbounds ptr, ptr %arg1, i64 %i13
  %i16 = load ptr, ptr %i15, align 8
  %i17 = tail call i64 @strlen(ptr %i16)
  %i18 = add i64 %i17, 1
  %i19 = tail call noalias ptr @malloc(i64 %i18)
  %i20 = icmp eq ptr %i19, null
  br i1 %i20, label %bb23, label %bb21

bb21:                                             ; preds = %bb12
  %i22 = tail call ptr @strcpy(ptr nonnull %i19, ptr %i16)
  br label %bb23

bb23:                                             ; preds = %bb21, %bb12
  %i24 = call i64 @strtol(ptr %i19, ptr nonnull %i, i32 0)
  %i25 = trunc i64 %i24 to i32
  %i26 = getelementptr inbounds %struct.uExpressionValue, ptr %arg2, i64 %i13, i32 1
  %i27 = bitcast ptr %i26 to ptr
  store i32 %i25, ptr %i27, align 8
  %i28 = load ptr, ptr %i, align 8
  %i29 = icmp eq ptr %i28, %i19
  br i1 %i29, label %bb30, label %bb176

bb30:                                             ; preds = %bb23
  %i31 = call fastcc i32 @Util_SplitString(ptr nonnull %i4, ptr nonnull %i5, ptr %i19, ptr @.str.2.1260)
  switch i32 %i31, label %bb34 [
    i32 0, label %bb36
    i32 1, label %bb32
    i32 2, label %bb33
  ]

bb32:                                             ; preds = %bb30
  tail call void (i32, i32, ptr, ptr, ptr, ...) @CCTK_VWarn(i32 8, i32 1198, ptr @.str.606, ptr @.str.1.10510, ptr @.str.47.608, ptr %i19)
  br label %bb35

bb33:                                             ; preds = %bb30
  tail call void (i32, i32, ptr, ptr, ptr, ...) @CCTK_VWarn(i32 2, i32 1205, ptr @.str.606, ptr @.str.1.10510, ptr @.str.977, ptr @.str.48.609)
  br label %bb35

bb34:                                             ; preds = %bb30
  tail call void (i32, i32, ptr, ptr, ptr, ...) @CCTK_VWarn(i32 1, i32 1211, ptr @.str.606, ptr @.str.1.10510, ptr @.str.977, ptr @.str.49.610)
  br label %bb35

bb35:                                             ; preds = %bb34, %bb33, %bb32
  store ptr null, ptr %i4, align 8
  store ptr null, ptr %i5, align 8
  br label %bb152

bb36:                                             ; preds = %bb30
  %i37 = load ptr, ptr %i4, align 8
  %i38 = load ptr, ptr @implist, align 8
  %i39 = icmp eq ptr %i38, null
  br i1 %i39, label %bb149, label %bb40

bb40:                                             ; preds = %bb36
  %i41 = tail call ptr @__ctype_tolower_loc()
  %i42 = load ptr, ptr %i41, align 8
  br label %bb43

bb43:                                             ; preds = %bb75, %bb40
  %i44 = phi ptr [ %i77, %bb75 ], [ %i38, %bb40 ]
  %i45 = getelementptr inbounds %struct.T_SKTREE, ptr %i44, i64 0, i32 4
  %i46 = load ptr, ptr %i45, align 8
  br label %bb47

bb47:                                             ; preds = %bb62, %bb43
  %i48 = phi ptr [ %i46, %bb43 ], [ %i64, %bb62 ]
  %i49 = phi ptr [ %i37, %bb43 ], [ %i63, %bb62 ]
  %i50 = load i8, ptr %i49, align 1
  %i51 = sext i8 %i50 to i64
  %i52 = getelementptr inbounds i32, ptr %i42, i64 %i51
  %i53 = load i32, ptr %i52, align 4
  %i54 = load i8, ptr %i48, align 1
  %i55 = sext i8 %i54 to i64
  %i56 = getelementptr inbounds i32, ptr %i42, i64 %i55
  %i57 = load i32, ptr %i56, align 4
  %i58 = sub nsw i32 %i53, %i57
  %i59 = icmp ne i32 %i58, 0
  %i60 = icmp eq i8 %i50, 0
  %i61 = or i1 %i60, %i59
  br i1 %i61, label %bb66, label %bb62

bb62:                                             ; preds = %bb47
  %i63 = getelementptr inbounds i8, ptr %i49, i64 1
  %i64 = getelementptr inbounds i8, ptr %i48, i64 1
  %i65 = icmp eq i8 %i54, 0
  br i1 %i65, label %bb79, label %bb47

bb66:                                             ; preds = %bb47
  %i67 = phi i32 [ %i58, %bb47 ]
  %i68 = icmp slt i32 %i67, 0
  br i1 %i68, label %bb69, label %bb71

bb69:                                             ; preds = %bb66
  %i70 = getelementptr inbounds %struct.T_SKTREE, ptr %i44, i64 0, i32 0
  br label %bb75

bb71:                                             ; preds = %bb66
  %i72 = icmp eq i32 %i67, 0
  br i1 %i72, label %bb81, label %bb73

bb73:                                             ; preds = %bb71
  %i74 = getelementptr inbounds %struct.T_SKTREE, ptr %i44, i64 0, i32 1
  br label %bb75

bb75:                                             ; preds = %bb73, %bb69
  %i76 = phi ptr [ %i74, %bb73 ], [ %i70, %bb69 ]
  %i77 = load ptr, ptr %i76, align 8
  %i78 = icmp eq ptr %i77, null
  br i1 %i78, label %bb148, label %bb43

bb79:                                             ; preds = %bb62
  %i80 = phi ptr [ %i44, %bb62 ]
  br label %bb83

bb81:                                             ; preds = %bb71
  %i82 = phi ptr [ %i44, %bb71 ]
  br label %bb83

bb83:                                             ; preds = %bb81, %bb79
  %i84 = phi ptr [ %i80, %bb79 ], [ %i82, %bb81 ]
  %i85 = icmp eq ptr %i84, null
  br i1 %i85, label %bb149, label %bb86

bb86:                                             ; preds = %bb83
  %i87 = getelementptr inbounds %struct.T_SKTREE, ptr %i84, i64 0, i32 5
  %i88 = bitcast ptr %i87 to ptr
  %i89 = load ptr, ptr %i88, align 8
  %i90 = getelementptr inbounds %struct.IMPLEMENTATION, ptr %i89, i64 0, i32 0
  %i91 = load i32, ptr %i90, align 8
  %i92 = icmp eq i32 %i91, 0
  br i1 %i92, label %bb149, label %bb93

bb93:                                             ; preds = %bb86
  br label %bb94

bb94:                                             ; preds = %bb126, %bb93
  %i95 = phi ptr [ %i128, %bb126 ], [ %i38, %bb93 ]
  %i96 = getelementptr inbounds %struct.T_SKTREE, ptr %i95, i64 0, i32 4
  %i97 = load ptr, ptr %i96, align 8
  br label %bb98

bb98:                                             ; preds = %bb113, %bb94
  %i99 = phi ptr [ %i97, %bb94 ], [ %i115, %bb113 ]
  %i100 = phi ptr [ %i37, %bb94 ], [ %i114, %bb113 ]
  %i101 = load i8, ptr %i100, align 1
  %i102 = sext i8 %i101 to i64
  %i103 = getelementptr inbounds i32, ptr %i42, i64 %i102
  %i104 = load i32, ptr %i103, align 4
  %i105 = load i8, ptr %i99, align 1
  %i106 = sext i8 %i105 to i64
  %i107 = getelementptr inbounds i32, ptr %i42, i64 %i106
  %i108 = load i32, ptr %i107, align 4
  %i109 = sub nsw i32 %i104, %i108
  %i110 = icmp ne i32 %i109, 0
  %i111 = icmp eq i8 %i101, 0
  %i112 = or i1 %i111, %i110
  br i1 %i112, label %bb117, label %bb113

bb113:                                            ; preds = %bb98
  %i114 = getelementptr inbounds i8, ptr %i100, i64 1
  %i115 = getelementptr inbounds i8, ptr %i99, i64 1
  %i116 = icmp eq i8 %i105, 0
  br i1 %i116, label %bb130, label %bb98

bb117:                                            ; preds = %bb98
  %i118 = phi i32 [ %i109, %bb98 ]
  %i119 = icmp slt i32 %i118, 0
  br i1 %i119, label %bb120, label %bb122

bb120:                                            ; preds = %bb117
  %i121 = getelementptr inbounds %struct.T_SKTREE, ptr %i95, i64 0, i32 0
  br label %bb126

bb122:                                            ; preds = %bb117
  %i123 = icmp eq i32 %i118, 0
  br i1 %i123, label %bb132, label %bb124

bb124:                                            ; preds = %bb122
  %i125 = getelementptr inbounds %struct.T_SKTREE, ptr %i95, i64 0, i32 1
  br label %bb126

bb126:                                            ; preds = %bb124, %bb120
  %i127 = phi ptr [ %i125, %bb124 ], [ %i121, %bb120 ]
  %i128 = load ptr, ptr %i127, align 8
  %i129 = icmp eq ptr %i128, null
  br i1 %i129, label %bb147, label %bb94

bb130:                                            ; preds = %bb113
  %i131 = phi ptr [ %i95, %bb113 ]
  br label %bb134

bb132:                                            ; preds = %bb122
  %i133 = phi ptr [ %i95, %bb122 ]
  br label %bb134

bb134:                                            ; preds = %bb132, %bb130
  %i135 = phi ptr [ %i131, %bb130 ], [ %i133, %bb132 ]
  %i136 = icmp eq ptr %i135, null
  br i1 %i136, label %bb149, label %bb137

bb137:                                            ; preds = %bb134
  %i138 = getelementptr inbounds %struct.T_SKTREE, ptr %i135, i64 0, i32 5
  %i139 = bitcast ptr %i138 to ptr
  %i140 = load ptr, ptr %i139, align 8
  %i141 = getelementptr inbounds %struct.IMPLEMENTATION, ptr %i140, i64 0, i32 0
  %i142 = load i32, ptr %i141, align 8
  %i143 = icmp eq i32 %i142, 0
  br i1 %i143, label %bb149, label %bb144

bb144:                                            ; preds = %bb137
  %i145 = getelementptr inbounds %struct.IMPLEMENTATION, ptr %i140, i64 0, i32 2
  %i146 = load ptr, ptr %i145, align 8
  br label %bb149

bb147:                                            ; preds = %bb126
  br label %bb149

bb148:                                            ; preds = %bb75
  br label %bb149

bb149:                                            ; preds = %bb148, %bb147, %bb144, %bb137, %bb134, %bb86, %bb83, %bb36
  %i150 = phi ptr [ %i37, %bb86 ], [ %i146, %bb144 ], [ null, %bb137 ], [ null, %bb134 ], [ %i37, %bb83 ], [ %i37, %bb36 ], [ null, %bb147 ], [ %i37, %bb148 ]
  %i151 = load ptr, ptr %i5, align 8
  br label %bb152

bb152:                                            ; preds = %bb149, %bb35
  %i153 = phi ptr [ null, %bb35 ], [ %i151, %bb149 ]
  %i154 = phi ptr [ null, %bb35 ], [ %i37, %bb149 ]
  %i155 = phi ptr [ %arg3, %bb35 ], [ %i150, %bb149 ]
  %i156 = phi ptr [ %i19, %bb35 ], [ %i151, %bb149 ]
  %i157 = tail call fastcc ptr @ParameterFind(ptr %i156, ptr %i155, i32 905)
  %i158 = icmp eq ptr %i157, null
  br i1 %i158, label %bb173, label %bb159

bb159:                                            ; preds = %bb152
  %i160 = getelementptr inbounds %struct.PARAM, ptr %i157, i64 0, i32 0
  %i161 = load ptr, ptr %i160, align 8
  %i162 = getelementptr inbounds %struct.PARAM_PROPS, ptr %i161, i64 0, i32 5
  %i163 = load i32, ptr %i162, align 8
  %i164 = getelementptr inbounds %struct.PARAM, ptr %i157, i64 0, i32 1
  %i165 = load ptr, ptr %i164, align 8
  %i166 = bitcast ptr %i165 to ptr
  %i167 = icmp ne ptr %i165, null
  %i168 = icmp eq i32 %i163, 704
  %i169 = and i1 %i167, %i168
  br i1 %i169, label %bb170, label %bb172

bb170:                                            ; preds = %bb159
  %i171 = load i32, ptr %i166, align 4
  store i32 %i171, ptr %i27, align 8
  br label %bb175

bb172:                                            ; preds = %bb159
  br i1 %i167, label %bb174, label %bb173

bb173:                                            ; preds = %bb172, %bb152
  tail call void (i32, i32, ptr, ptr, ptr, ...) @CCTK_VWarn(i32 0, i32 2882, ptr @.str.606, ptr @.str.1.10510, ptr @.str.72, ptr %i155, ptr %i156)
  br label %bb175

bb174:                                            ; preds = %bb172
  tail call void (i32, i32, ptr, ptr, ptr, ...) @CCTK_VWarn(i32 0, i32 2888, ptr @.str.606, ptr @.str.1.10510, ptr @.str.73, ptr %i155, ptr %i156)
  br label %bb175

bb175:                                            ; preds = %bb174, %bb173, %bb170
  tail call void @free(ptr %i154)
  tail call void @free(ptr %i153)
  br label %bb176

bb176:                                            ; preds = %bb175, %bb23
  tail call void @free(ptr %i19)
  %i177 = add nuw nsw i64 %i13, 1
  %i178 = icmp eq i64 %i177, %i11
  br i1 %i178, label %bb179, label %bb12

bb179:                                            ; preds = %bb176
  br label %bb180

bb180:                                            ; preds = %bb179, %bb
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i8)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i7)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i6)
  ret i32 0
}

declare hidden fastcc ptr @ParameterFind(ptr, ptr readonly, i32) unnamed_addr

declare hidden void @CCTK_VWarn(i32, i32, ptr, ptr, ptr nocapture readonly, ...) unnamed_addr

declare hidden fastcc i32 @Util_SplitString(ptr nocapture, ptr nocapture, ptr, ptr nocapture readonly) unnamed_addr

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
