; RUN: opt -passes=slp-vectorizer -enable-intel-advanced-opts -slp-multinode -mtriple=x86_64 -mcpu=skylake-avx512 -S < %s | FileCheck %s

define i32 @x264_pixel_satd_8x4(ptr nocapture readonly %arg, i32 %arg1, ptr nocapture readonly %arg2, i32 %arg3) {
bb:
  %i = alloca [4 x [4 x i32]], align 16
  %i5 = sext i32 %arg1 to i64
  %i6 = sext i32 %arg3 to i64
  %i7 = load i8, ptr %arg, align 1
  %i8 = zext i8 %i7 to i32
  %i9 = load i8, ptr %arg2, align 1
  %i10 = zext i8 %i9 to i32
  %i11 = sub nsw i32 %i8, %i10
  %i12 = getelementptr inbounds i8, ptr %arg, i64 4
  %i13 = load i8, ptr %i12, align 1
  %i14 = zext i8 %i13 to i32
  %i15 = getelementptr inbounds i8, ptr %arg2, i64 4
  %i16 = load i8, ptr %i15, align 1
  %i17 = zext i8 %i16 to i32
  %i18 = sub nsw i32 %i14, %i17
  %i19 = shl nsw i32 %i18, 16
  %i20 = add nsw i32 %i19, %i11
  %i21 = getelementptr inbounds i8, ptr %arg, i64 1
  %i22 = load i8, ptr %i21, align 1
  %i23 = zext i8 %i22 to i32
  %i24 = getelementptr inbounds i8, ptr %arg2, i64 1
  %i25 = load i8, ptr %i24, align 1
  %i26 = zext i8 %i25 to i32
  %i27 = sub nsw i32 %i23, %i26
  %i28 = getelementptr inbounds i8, ptr %arg, i64 5
  %i29 = load i8, ptr %i28, align 1
  %i30 = zext i8 %i29 to i32
  %i31 = getelementptr inbounds i8, ptr %arg2, i64 5
  %i32 = load i8, ptr %i31, align 1
  %i33 = zext i8 %i32 to i32
  %i34 = sub nsw i32 %i30, %i33
  %i35 = shl nsw i32 %i34, 16
  %i36 = add nsw i32 %i35, %i27
  %i37 = getelementptr inbounds i8, ptr %arg, i64 2
  %i38 = load i8, ptr %i37, align 1
  %i39 = zext i8 %i38 to i32
  %i40 = getelementptr inbounds i8, ptr %arg2, i64 2
  %i41 = load i8, ptr %i40, align 1
  %i42 = zext i8 %i41 to i32
  %i43 = sub nsw i32 %i39, %i42
  %i44 = getelementptr inbounds i8, ptr %arg, i64 6
  %i45 = load i8, ptr %i44, align 1
  %i46 = zext i8 %i45 to i32
  %i47 = getelementptr inbounds i8, ptr %arg2, i64 6
  %i48 = load i8, ptr %i47, align 1
  %i49 = zext i8 %i48 to i32
  %i50 = sub nsw i32 %i46, %i49
  %i51 = shl nsw i32 %i50, 16
  %i52 = add nsw i32 %i51, %i43
  %i53 = getelementptr inbounds i8, ptr %arg, i64 3
  %i54 = load i8, ptr %i53, align 1
  %i55 = zext i8 %i54 to i32
  %i56 = getelementptr inbounds i8, ptr %arg2, i64 3
  %i57 = load i8, ptr %i56, align 1
  %i58 = zext i8 %i57 to i32
  %i59 = sub nsw i32 %i55, %i58
  %i60 = getelementptr inbounds i8, ptr %arg, i64 7
  %i61 = load i8, ptr %i60, align 1
  %i62 = zext i8 %i61 to i32
  %i63 = getelementptr inbounds i8, ptr %arg2, i64 7
  %i64 = load i8, ptr %i63, align 1
  %i65 = zext i8 %i64 to i32
  %i66 = sub nsw i32 %i62, %i65
  %i67 = shl nsw i32 %i66, 16
  %i68 = add nsw i32 %i67, %i59
  %i69 = add nsw i32 %i36, %i20
  %i70 = sub nsw i32 %i20, %i36
  %i71 = add nsw i32 %i68, %i52
  %i72 = sub nsw i32 %i52, %i68
  %i73 = add nsw i32 %i71, %i69
  store i32 %i73, ptr %i, align 16
  %i75 = sub nsw i32 %i69, %i71
  %i76 = getelementptr inbounds [4 x [4 x i32]], ptr %i, i64 0, i64 0, i64 2
  store i32 %i75, ptr %i76, align 8
  %i77 = add nsw i32 %i72, %i70
  %i78 = getelementptr inbounds [4 x [4 x i32]], ptr %i, i64 0, i64 0, i64 1
  store i32 %i77, ptr %i78, align 4
  %i79 = sub nsw i32 %i70, %i72
  %i80 = getelementptr inbounds [4 x [4 x i32]], ptr %i, i64 0, i64 0, i64 3
  store i32 %i79, ptr %i80, align 4
  %i81 = getelementptr inbounds i8, ptr %arg, i64 %i5
  %i82 = getelementptr inbounds i8, ptr %arg2, i64 %i6
  %i83 = load i8, ptr %i81, align 1
  %i84 = zext i8 %i83 to i32
  %i85 = load i8, ptr %i82, align 1
  %i86 = zext i8 %i85 to i32
  %i87 = sub nsw i32 %i84, %i86
  %i88 = getelementptr inbounds i8, ptr %i81, i64 4
  %i89 = load i8, ptr %i88, align 1
  %i90 = zext i8 %i89 to i32
  %i91 = getelementptr inbounds i8, ptr %i82, i64 4
  %i92 = load i8, ptr %i91, align 1
  %i93 = zext i8 %i92 to i32
  %i94 = sub nsw i32 %i90, %i93
  %i95 = shl nsw i32 %i94, 16
  %i96 = add nsw i32 %i95, %i87
  %i97 = getelementptr inbounds i8, ptr %i81, i64 1
  %i98 = load i8, ptr %i97, align 1
  %i99 = zext i8 %i98 to i32
  %i100 = getelementptr inbounds i8, ptr %i82, i64 1
  %i101 = load i8, ptr %i100, align 1
  %i102 = zext i8 %i101 to i32
  %i103 = sub nsw i32 %i99, %i102
  %i104 = getelementptr inbounds i8, ptr %i81, i64 5
  %i105 = load i8, ptr %i104, align 1
  %i106 = zext i8 %i105 to i32
  %i107 = getelementptr inbounds i8, ptr %i82, i64 5
  %i108 = load i8, ptr %i107, align 1
  %i109 = zext i8 %i108 to i32
  %i110 = sub nsw i32 %i106, %i109
  %i111 = shl nsw i32 %i110, 16
  %i112 = add nsw i32 %i111, %i103
  %i113 = getelementptr inbounds i8, ptr %i81, i64 2
  %i114 = load i8, ptr %i113, align 1
  %i115 = zext i8 %i114 to i32
  %i116 = getelementptr inbounds i8, ptr %i82, i64 2
  %i117 = load i8, ptr %i116, align 1
  %i118 = zext i8 %i117 to i32
  %i119 = sub nsw i32 %i115, %i118
  %i120 = getelementptr inbounds i8, ptr %i81, i64 6
  %i121 = load i8, ptr %i120, align 1
  %i122 = zext i8 %i121 to i32
  %i123 = getelementptr inbounds i8, ptr %i82, i64 6
  %i124 = load i8, ptr %i123, align 1
  %i125 = zext i8 %i124 to i32
  %i126 = sub nsw i32 %i122, %i125
  %i127 = shl nsw i32 %i126, 16
  %i128 = add nsw i32 %i127, %i119
  %i129 = getelementptr inbounds i8, ptr %i81, i64 3
  %i130 = load i8, ptr %i129, align 1
  %i131 = zext i8 %i130 to i32
  %i132 = getelementptr inbounds i8, ptr %i82, i64 3
  %i133 = load i8, ptr %i132, align 1
  %i134 = zext i8 %i133 to i32
  %i135 = sub nsw i32 %i131, %i134
  %i136 = getelementptr inbounds i8, ptr %i81, i64 7
  %i137 = load i8, ptr %i136, align 1
  %i138 = zext i8 %i137 to i32
  %i139 = getelementptr inbounds i8, ptr %i82, i64 7
  %i140 = load i8, ptr %i139, align 1
  %i141 = zext i8 %i140 to i32
  %i142 = sub nsw i32 %i138, %i141
  %i143 = shl nsw i32 %i142, 16
  %i144 = add nsw i32 %i143, %i135
  %i145 = add nsw i32 %i112, %i96
  %i146 = sub nsw i32 %i96, %i112
  %i147 = add nsw i32 %i144, %i128
  %i148 = sub nsw i32 %i128, %i144
  %i149 = add nsw i32 %i147, %i145
  %i150 = getelementptr inbounds [4 x [4 x i32]], ptr %i, i64 0, i64 1, i64 0
  store i32 %i149, ptr %i150, align 16
  %i151 = sub nsw i32 %i145, %i147
  %i152 = getelementptr inbounds [4 x [4 x i32]], ptr %i, i64 0, i64 1, i64 2
  store i32 %i151, ptr %i152, align 8
  %i153 = add nsw i32 %i148, %i146
  %i154 = getelementptr inbounds [4 x [4 x i32]], ptr %i, i64 0, i64 1, i64 1
  store i32 %i153, ptr %i154, align 4
  %i155 = sub nsw i32 %i146, %i148
  %i156 = getelementptr inbounds [4 x [4 x i32]], ptr %i, i64 0, i64 1, i64 3
  store i32 %i155, ptr %i156, align 4
  ret i32 0
}

; CHECK: [[L1:%.*]] = load <4 x i8>
; CHECK: [[L2:%.*]] = load <4 x i8>
; CHECK: [[L3:%.*]] = load <4 x i8>
; CHECK: [[L4:%.*]] = load <4 x i8>

; CHECK: [[L5:%.*]] = load <4 x i8>
; CHECK: [[L6:%.*]] = load <4 x i8>
; CHECK: [[L7:%.*]] = load <4 x i8>
; CHECK: [[L8:%.*]] = load <4 x i8>
; CHECK: store <8 x i32>
