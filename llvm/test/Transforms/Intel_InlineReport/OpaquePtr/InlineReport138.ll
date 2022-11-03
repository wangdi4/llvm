; RUN: opt -passes='function(instcombine),print<inline-report>' -disable-output -inline-report=0xe807 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='function(instcombine)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; Check that various calls to fputc are deleted as dead code.

; CHECK-LABEL: COMPILE FUNC: i_send_buf_as_file
; CHECK: EXTERN: t_printf
; CHECK: DELETE: fputc {{.*}}Dead code{{.*}}
; CHECK: EXTERN: fputc
; CHECK: DELETE: fputc {{.*}}Dead code{{.*}}
; CHECK: EXTERN: fputc
; CHECK: DELETE: fputc {{.*}}Dead code{{.*}}
; CHECK: EXTERN: fputc
; CHECK: DELETE: fputc {{.*}}Dead code{{.*}}
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc
; CHECK: DELETE: fputc {{.*}}Dead code{{.*}}
; CHECK: EXTERN: fputc
; CHECK: DELETE: fputc {{.*}}Dead code{{.*}}
; CHECK: EXTERN: fputc
; CHECK: DELETE: fputc {{.*}}Dead code{{.*}}
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc
; CHECK: EXTERN: t_printf

@.str.1.89 = private unnamed_addr constant [14 x i8] c"begin %lo %s\0A\00", align 1
@.str.86 = private unnamed_addr constant [35 x i8] c"Uuencode buffer parameters error.\0A\00", align 1
@.str.2.90 = private unnamed_addr constant [6 x i8] c"end\0A\0A\00", align 1
@uu_std = internal unnamed_addr constant [64 x i8] c"`!\22#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_", align 16
@stdout = external dso_local local_unnamed_addr global ptr, align 8

declare void @t_printf(ptr nocapture noundef readonly, ...)

declare noundef i32 @fputc(i32 noundef, ptr nocapture noundef)

define internal i32 @i_send_buf_as_file(ptr noundef %arg, i64 noundef %arg1, ptr noundef %arg2) {
bb:
  %i = alloca i8, align 1
  %i3 = alloca i8, align 1
  %i4 = alloca i8, align 1
  %i5 = alloca i8, align 1
  %i6 = alloca i8, align 1
  %i7 = alloca i8, align 1
  %i8 = alloca i8, align 1
  %i9 = alloca i8, align 1
  %i10 = alloca i8, align 1
  %i11 = alloca i8, align 1
  %i12 = alloca i8, align 1
  %i13 = alloca i8, align 1
  %i14 = alloca i8, align 1
  %i15 = alloca i8, align 1
  %i16 = alloca i8, align 1
  %i17 = alloca i8, align 1
  %i18 = alloca [80 x i8], align 16
  %i19 = trunc i64 %arg1 to i32
  tail call void (ptr, ...) @t_printf(ptr noundef @.str.1.89, i64 noundef 384, ptr noundef %arg2)
  %i20 = getelementptr inbounds [80 x i8], ptr %i18, i64 0, i64 0
  call void @llvm.lifetime.start.p0(i64 80, ptr nonnull %i20)
  %i21 = icmp sgt i32 %i19, 0
  %i22 = icmp ne ptr %arg, null
  %i23 = and i1 %i21, %i22
  br i1 %i23, label %bb25, label %bb24

bb24:                                             ; preds = %bb
  tail call void (ptr, ...) @t_printf(ptr noundef @.str.86)
  br label %bb219

bb25:                                             ; preds = %bb
  %i26 = and i64 %arg1, 4294967295
  br label %bb27

bb27:                                             ; preds = %bb142, %bb25
  %i28 = phi i64 [ %i44, %bb142 ], [ 0, %bb25 ]
  br label %bb29

bb29:                                             ; preds = %bb33, %bb27
  %i30 = phi i64 [ 0, %bb27 ], [ %i37, %bb33 ]
  %i31 = add nuw nsw i64 %i30, %i28
  %i32 = icmp slt i64 %i31, %i26
  br i1 %i32, label %bb33, label %bb39

bb33:                                             ; preds = %bb29
  %i34 = getelementptr inbounds i8, ptr %arg, i64 %i31
  %i35 = load i8, ptr %i34, align 1
  %i36 = getelementptr inbounds [80 x i8], ptr %i18, i64 0, i64 %i30
  store i8 %i35, ptr %i36, align 1
  %i37 = add nuw nsw i64 %i30, 1
  %i38 = icmp eq i64 %i37, 45
  br i1 %i38, label %bb42, label %bb29

bb39:                                             ; preds = %bb29
  %i40 = trunc i64 %i30 to i32
  %i41 = icmp eq i32 %i40, 0
  br i1 %i41, label %bb212, label %bb42

bb42:                                             ; preds = %bb39, %bb33
  %i43 = phi i32 [ %i40, %bb39 ], [ 45, %bb33 ]
  %i44 = add nuw i64 %i28, 45
  %i45 = and i32 %i43, 63
  %i46 = zext i32 %i45 to i64
  %i47 = getelementptr inbounds [64 x i8], ptr @uu_std, i64 0, i64 %i46
  %i48 = load i8, ptr %i47, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i16)
  store i8 %i48, ptr %i16, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i17)
  store i8 13, ptr %i17, align 1
  br i1 false, label %bb49, label %bb54

bb49:                                             ; preds = %bb42
  %i50 = load ptr, ptr @stdout, align 8
  %i51 = load i8, ptr %i17, align 1
  %i52 = sext i8 %i51 to i32
  %i53 = call i32 @fputc(i32 %i52, ptr %i50)
  br label %bb54

bb54:                                             ; preds = %bb49, %bb42
  %i55 = load ptr, ptr @stdout, align 8
  %i56 = load i8, ptr %i16, align 1
  %i57 = sext i8 %i56 to i32
  %i58 = call i32 @fputc(i32 %i57, ptr %i55)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i17)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i16)
  %i59 = icmp sgt i32 %i43, 2
  br i1 %i59, label %bb60, label %bb138

bb60:                                             ; preds = %bb130, %bb54
  %i61 = phi i32 [ %i135, %bb130 ], [ %i43, %bb54 ]
  %i62 = phi ptr [ %i136, %bb130 ], [ %i20, %bb54 ]
  %i63 = load i8, ptr %i62, align 1
  %i64 = lshr i8 %i63, 2
  %i65 = zext i8 %i64 to i64
  %i66 = getelementptr inbounds [64 x i8], ptr @uu_std, i64 0, i64 %i65
  %i67 = load i8, ptr %i66, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i14)
  store i8 %i67, ptr %i14, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i15)
  store i8 13, ptr %i15, align 1
  br i1 false, label %bb68, label %bb73

bb68:                                             ; preds = %bb60
  %i69 = load ptr, ptr @stdout, align 8
  %i70 = load i8, ptr %i15, align 1
  %i71 = sext i8 %i70 to i32
  %i72 = call i32 @fputc(i32 %i71, ptr %i69)
  br label %bb73

bb73:                                             ; preds = %bb68, %bb60
  %i74 = load ptr, ptr @stdout, align 8
  %i75 = load i8, ptr %i14, align 1
  %i76 = sext i8 %i75 to i32
  %i77 = call i32 @fputc(i32 %i76, ptr %i74)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i15)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i14)
  %i78 = load i8, ptr %i62, align 1
  %i79 = sext i8 %i78 to i64
  %i80 = shl nsw i64 %i79, 4
  %i81 = and i64 %i80, 48
  %i82 = getelementptr inbounds i8, ptr %i62, i64 1
  %i83 = load i8, ptr %i82, align 1
  %i84 = lshr i8 %i83, 4
  %i85 = zext i8 %i84 to i64
  %i86 = or i64 %i81, %i85
  %i87 = getelementptr inbounds [64 x i8], ptr @uu_std, i64 0, i64 %i86
  %i88 = load i8, ptr %i87, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i12)
  store i8 %i88, ptr %i12, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i13)
  store i8 13, ptr %i13, align 1
  br i1 false, label %bb89, label %bb94

bb89:                                             ; preds = %bb73
  %i90 = load ptr, ptr @stdout, align 8
  %i91 = load i8, ptr %i13, align 1
  %i92 = sext i8 %i91 to i32
  %i93 = call i32 @fputc(i32 %i92, ptr %i90)
  br label %bb94

bb94:                                             ; preds = %bb89, %bb73
  %i95 = load ptr, ptr @stdout, align 8
  %i96 = load i8, ptr %i12, align 1
  %i97 = sext i8 %i96 to i32
  %i98 = call i32 @fputc(i32 %i97, ptr %i95)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i13)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i12)
  %i99 = load i8, ptr %i82, align 1
  %i100 = sext i8 %i99 to i64
  %i101 = shl nsw i64 %i100, 2
  %i102 = and i64 %i101, 60
  %i103 = getelementptr inbounds i8, ptr %i62, i64 2
  %i104 = load i8, ptr %i103, align 1
  %i105 = lshr i8 %i104, 6
  %i106 = zext i8 %i105 to i64
  %i107 = or i64 %i102, %i106
  %i108 = getelementptr inbounds [64 x i8], ptr @uu_std, i64 0, i64 %i107
  %i109 = load i8, ptr %i108, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i10)
  store i8 %i109, ptr %i10, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i11)
  store i8 13, ptr %i11, align 1
  br i1 false, label %bb110, label %bb115

bb110:                                            ; preds = %bb94
  %i111 = load ptr, ptr @stdout, align 8
  %i112 = load i8, ptr %i11, align 1
  %i113 = sext i8 %i112 to i32
  %i114 = call i32 @fputc(i32 %i113, ptr %i111)
  br label %bb115

bb115:                                            ; preds = %bb110, %bb94
  %i116 = load ptr, ptr @stdout, align 8
  %i117 = load i8, ptr %i10, align 1
  %i118 = sext i8 %i117 to i32
  %i119 = call i32 @fputc(i32 %i118, ptr %i116)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i11)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i10)
  %i120 = load i8, ptr %i103, align 1
  %i121 = and i8 %i120, 63
  %i122 = zext i8 %i121 to i64
  %i123 = getelementptr inbounds [64 x i8], ptr @uu_std, i64 0, i64 %i122
  %i124 = load i8, ptr %i123, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i8)
  store i8 %i124, ptr %i8, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i9)
  store i8 13, ptr %i9, align 1
  br i1 false, label %bb125, label %bb130

bb125:                                            ; preds = %bb115
  %i126 = load ptr, ptr @stdout, align 8
  %i127 = load i8, ptr %i9, align 1
  %i128 = sext i8 %i127 to i32
  %i129 = call i32 @fputc(i32 %i128, ptr %i126)
  br label %bb130

bb130:                                            ; preds = %bb125, %bb115
  %i131 = load ptr, ptr @stdout, align 8
  %i132 = load i8, ptr %i8, align 1
  %i133 = sext i8 %i132 to i32
  %i134 = call i32 @fputc(i32 %i133, ptr %i131)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i9)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i8)
  %i135 = add nsw i32 %i61, -3
  %i136 = getelementptr inbounds i8, ptr %i62, i64 3
  %i137 = icmp sgt i32 %i61, 5
  br i1 %i137, label %bb60, label %bb138

bb138:                                            ; preds = %bb130, %bb54
  %i139 = phi ptr [ %i20, %bb54 ], [ %i136, %bb130 ]
  %i140 = phi i32 [ %i43, %bb54 ], [ %i135, %bb130 ]
  %i141 = icmp eq i32 %i140, 0
  br i1 %i141, label %bb142, label %bb147

bb142:                                            ; preds = %bb138
  %i143 = load ptr, ptr @stdout, align 8
  %i144 = call i32 @fputc(i32 13, ptr %i143)
  %i145 = load ptr, ptr @stdout, align 8
  %i146 = call i32 @fputc(i32 10, ptr %i145)
  br label %bb27

bb147:                                            ; preds = %bb138
  %i148 = load i8, ptr %i139, align 1
  %i149 = icmp eq i32 %i140, 1
  br i1 %i149, label %bb154, label %bb150

bb150:                                            ; preds = %bb147
  %i151 = getelementptr inbounds i8, ptr %i139, i64 1
  %i152 = load i8, ptr %i151, align 1
  %i153 = sext i8 %i152 to i32
  br label %bb154

bb154:                                            ; preds = %bb150, %bb147
  %i155 = phi i32 [ %i153, %bb150 ], [ 0, %bb147 ]
  %i156 = sext i8 %i148 to i32
  %i157 = lshr i32 %i156, 2
  %i158 = and i32 %i157, 63
  %i159 = zext i32 %i158 to i64
  %i160 = getelementptr inbounds [64 x i8], ptr @uu_std, i64 0, i64 %i159
  %i161 = load i8, ptr %i160, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i6)
  store i8 %i161, ptr %i6, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i7)
  store i8 13, ptr %i7, align 1
  br i1 false, label %bb162, label %bb167

bb162:                                            ; preds = %bb154
  %i163 = load ptr, ptr @stdout, align 8
  %i164 = load i8, ptr %i7, align 1
  %i165 = sext i8 %i164 to i32
  %i166 = call i32 @fputc(i32 %i165, ptr %i163)
  br label %bb167

bb167:                                            ; preds = %bb162, %bb154
  %i168 = load ptr, ptr @stdout, align 8
  %i169 = load i8, ptr %i6, align 1
  %i170 = sext i8 %i169 to i32
  %i171 = call i32 @fputc(i32 %i170, ptr %i168)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i7)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i6)
  %i172 = shl nsw i32 %i156, 4
  %i173 = and i32 %i172, 48
  %i174 = lshr i32 %i155, 4
  %i175 = and i32 %i174, 15
  %i176 = or i32 %i173, %i175
  %i177 = zext i32 %i176 to i64
  %i178 = getelementptr inbounds [64 x i8], ptr @uu_std, i64 0, i64 %i177
  %i179 = load i8, ptr %i178, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i4)
  store i8 %i179, ptr %i4, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i5)
  store i8 13, ptr %i5, align 1
  br i1 false, label %bb180, label %bb185

bb180:                                            ; preds = %bb167
  %i181 = load ptr, ptr @stdout, align 8
  %i182 = load i8, ptr %i5, align 1
  %i183 = sext i8 %i182 to i32
  %i184 = call i32 @fputc(i32 %i183, ptr %i181)
  br label %bb185

bb185:                                            ; preds = %bb180, %bb167
  %i186 = load ptr, ptr @stdout, align 8
  %i187 = load i8, ptr %i4, align 1
  %i188 = sext i8 %i187 to i32
  %i189 = call i32 @fputc(i32 %i188, ptr %i186)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i5)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i4)
  %i190 = shl nsw i32 %i155, 2
  %i191 = and i32 %i190, 60
  %i192 = select i1 %i149, i32 0, i32 %i191
  %i193 = zext i32 %i192 to i64
  %i194 = getelementptr [64 x i8], ptr @uu_std, i64 0, i64 %i193
  %i195 = load i8, ptr %i194, align 4
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i)
  store i8 %i195, ptr %i, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i3)
  store i8 13, ptr %i3, align 1
  br i1 false, label %bb196, label %bb201

bb196:                                            ; preds = %bb185
  %i197 = load ptr, ptr @stdout, align 8
  %i198 = load i8, ptr %i3, align 1
  %i199 = sext i8 %i198 to i32
  %i200 = call i32 @fputc(i32 %i199, ptr %i197)
  br label %bb201

bb201:                                            ; preds = %bb196, %bb185
  %i202 = load ptr, ptr @stdout, align 8
  %i203 = load i8, ptr %i, align 1
  %i204 = sext i8 %i203 to i32
  %i205 = call i32 @fputc(i32 %i204, ptr %i202)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i3)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i)
  %i206 = load ptr, ptr @stdout, align 8
  %i207 = call i32 @fputc(i32 96, ptr %i206)
  %i208 = load ptr, ptr @stdout, align 8
  %i209 = call i32 @fputc(i32 13, ptr %i208)
  %i210 = load ptr, ptr @stdout, align 8
  %i211 = call i32 @fputc(i32 10, ptr %i210)
  br label %bb212

bb212:                                            ; preds = %bb201, %bb39
  %i213 = load ptr, ptr @stdout, align 8
  %i214 = call i32 @fputc(i32 96, ptr %i213)
  %i215 = load ptr, ptr @stdout, align 8
  %i216 = call i32 @fputc(i32 13, ptr %i215)
  %i217 = load ptr, ptr @stdout, align 8
  %i218 = call i32 @fputc(i32 10, ptr %i217)
  br label %bb219

bb219:                                            ; preds = %bb212, %bb24
  call void @llvm.lifetime.end.p0(i64 80, ptr nonnull %i20)
  tail call void (ptr, ...) @t_printf(ptr noundef @.str.2.90)
  ret i32 0
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

attributes #0 = { argmemonly nocallback nofree nosync nounwind willreturn }
