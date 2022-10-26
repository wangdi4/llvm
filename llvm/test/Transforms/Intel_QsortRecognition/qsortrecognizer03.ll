; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; REQUIRES: asserts
; RUN: opt < %s -passes='module(qsortrecognizer)' -debug-only=qsortrecognizer -S 2>&1 | FileCheck %s

; Check that the qsortrecognizer recognizes @spec_qsort.40 as a spec qsort,
; by marking it with the 'is-qsort' attribute.

; This test is the same as qsortrecognizer01.ll, but has replaced some
; sequences of a compare instruction following by a select instruction
; with use of the @llvm.smin intrinsic.

; CHECK: FOUND QSORT
; CHECK: define{{.*}}@spec_qsort.40{{.*}}#1
; CHECK: attributes #1 = { "is-qsort" "is-qsort-spec_qsort" }

%__SOADT___DFR___DFT_struct.arc = type { i64, i32, i32, i64, i64, i32, i16 }

define internal i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** nocapture readonly %arg, %__SOADT___DFR___DFT_struct.arc** nocapture readonly %arg1) #0 {
bb:
  call void @llvm.dbg.value(metadata %__SOADT___DFR___DFT_struct.arc** %arg, metadata !8, metadata !DIExpression()), !dbg !14
  %i = load %__SOADT___DFR___DFT_struct.arc*, %__SOADT___DFR___DFT_struct.arc** %arg, align 8
  %i2 = getelementptr %__SOADT___DFR___DFT_struct.arc, %__SOADT___DFR___DFT_struct.arc* %i, i64 0, i32 0
  %i3 = load i64, i64* %i2, align 8
  %i4 = load %__SOADT___DFR___DFT_struct.arc*, %__SOADT___DFR___DFT_struct.arc** %arg1, align 8
  %i5 = getelementptr %__SOADT___DFR___DFT_struct.arc, %__SOADT___DFR___DFT_struct.arc* %i4, i64 0, i32 0
  %i6 = load i64, i64* %i5, align 8
  %i7 = icmp sgt i64 %i3, %i6
  br i1 %i7, label %bb17, label %bb8

bb8:                                              ; preds = %bb
  %i9 = icmp slt i64 %i3, %i6
  br i1 %i9, label %bb17, label %bb10

bb10:                                             ; preds = %bb8
  %i11 = getelementptr inbounds %__SOADT___DFR___DFT_struct.arc, %__SOADT___DFR___DFT_struct.arc* %i, i64 0, i32 1
  %i12 = load i32, i32* %i11, align 8
  %i13 = getelementptr inbounds %__SOADT___DFR___DFT_struct.arc, %__SOADT___DFR___DFT_struct.arc* %i4, i64 0, i32 1
  %i14 = load i32, i32* %i13, align 8
  %i15 = icmp slt i32 %i12, %i14
  %i16 = select i1 %i15, i32 -1, i32 1
  ret i32 %i16

bb17:                                             ; preds = %bb8, %bb
  %i18 = phi i32 [ 1, %bb ], [ -1, %bb8 ]
  ret i32 %i18
}

define internal fastcc void @spec_qsort.40(i8* %arg, i64 %arg1) unnamed_addr #1 {
bb:
  %i = ptrtoint i8* %arg to i64
  %i2 = icmp ult i64 %arg1, 7
  br i1 %i2, label %bb3, label %bb30

bb3:                                              ; preds = %bb313, %bb
  %i4 = phi i64 [ %arg1, %bb ], [ %i316, %bb313 ]
  %i5 = phi i8* [ %arg, %bb ], [ %i315, %bb313 ]
  %i6 = shl i64 %i4, 3
  %i7 = getelementptr inbounds i8, i8* %i5, i64 %i6
  %i8 = icmp sgt i64 %i6, 8
  br i1 %i8, label %bb9, label %bb319

bb9:                                              ; preds = %bb3
  %i10 = getelementptr inbounds i8, i8* %i5, i64 8
  br label %bb11

bb11:                                             ; preds = %bb27, %bb9
  %i12 = phi i8* [ %i10, %bb9 ], [ %i28, %bb27 ]
  %i13 = icmp ugt i8* %i12, %i5
  br i1 %i13, label %bb14, label %bb27

bb14:                                             ; preds = %bb21, %bb11
  %i15 = phi i8* [ %i16, %bb21 ], [ %i12, %bb11 ]
  %i16 = getelementptr inbounds i8, i8* %i15, i64 -8
  %i17 = bitcast i8* %i16 to %__SOADT___DFR___DFT_struct.arc**
  %i18 = bitcast i8* %i15 to %__SOADT___DFR___DFT_struct.arc**
  %i19 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** nonnull %i17, %__SOADT___DFR___DFT_struct.arc** %i18) #3
  %i20 = icmp sgt i32 %i19, 0
  br i1 %i20, label %bb21, label %bb27

bb21:                                             ; preds = %bb14
  %i22 = bitcast i8* %i15 to i64*
  %i23 = load i64, i64* %i22, align 8
  %i24 = bitcast i8* %i16 to i64*
  %i25 = load i64, i64* %i24, align 8
  store i64 %i25, i64* %i22, align 8
  store i64 %i23, i64* %i24, align 8
  %i26 = icmp ugt i8* %i16, %i5
  br i1 %i26, label %bb14, label %bb27

bb27:                                             ; preds = %bb21, %bb14, %bb11
  %i28 = getelementptr inbounds i8, i8* %i12, i64 8
  %i29 = icmp ult i8* %i28, %i7
  br i1 %i29, label %bb11, label %bb319

bb30:                                             ; preds = %bb313, %bb
  %i31 = phi i64 [ %i317, %bb313 ], [ %i, %bb ]
  %i32 = phi i8* [ %i315, %bb313 ], [ %arg, %bb ]
  %i33 = phi i64 [ %i316, %bb313 ], [ %arg1, %bb ]
  %i34 = lshr i64 %i33, 1
  %i35 = shl i64 %i34, 3
  %i36 = getelementptr inbounds i8, i8* %i32, i64 %i35
  %i37 = icmp eq i64 %i33, 7
  br i1 %i37, label %bb154, label %bb38

bb38:                                             ; preds = %bb30
  %i39 = shl i64 %i33, 3
  %i40 = add i64 %i39, -8
  %i41 = getelementptr inbounds i8, i8* %i32, i64 %i40
  %i42 = icmp ugt i64 %i33, 40
  br i1 %i42, label %bb43, label %bb127

bb43:                                             ; preds = %bb38
  %i44 = and i64 %i33, -8
  %i45 = getelementptr inbounds i8, i8* %i32, i64 %i44
  %i46 = shl i64 %i44, 1
  %i47 = getelementptr inbounds i8, i8* %i32, i64 %i46
  %i48 = bitcast i8* %i32 to %__SOADT___DFR___DFT_struct.arc**
  %i49 = bitcast i8* %i45 to %__SOADT___DFR___DFT_struct.arc**
  %i50 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i48, %__SOADT___DFR___DFT_struct.arc** %i49) #3
  %i51 = icmp slt i32 %i50, 0
  %i52 = bitcast i8* %i45 to %__SOADT___DFR___DFT_struct.arc**
  %i53 = bitcast i8* %i47 to %__SOADT___DFR___DFT_struct.arc**
  %i54 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i52, %__SOADT___DFR___DFT_struct.arc** %i53) #3
  br i1 %i51, label %bb55, label %bb63

bb55:                                             ; preds = %bb43
  %i56 = icmp slt i32 %i54, 0
  br i1 %i56, label %bb71, label %bb57

bb57:                                             ; preds = %bb55
  %i58 = bitcast i8* %i32 to %__SOADT___DFR___DFT_struct.arc**
  %i59 = bitcast i8* %i47 to %__SOADT___DFR___DFT_struct.arc**
  %i60 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i58, %__SOADT___DFR___DFT_struct.arc** %i59) #3
  %i61 = icmp slt i32 %i60, 0
  %i62 = select i1 %i61, i8* %i47, i8* %i32
  br label %bb71

bb63:                                             ; preds = %bb43
  %i64 = icmp sgt i32 %i54, 0
  br i1 %i64, label %bb71, label %bb65

bb65:                                             ; preds = %bb63
  %i66 = bitcast i8* %i32 to %__SOADT___DFR___DFT_struct.arc**
  %i67 = bitcast i8* %i47 to %__SOADT___DFR___DFT_struct.arc**
  %i68 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i66, %__SOADT___DFR___DFT_struct.arc** %i67) #3
  %i69 = icmp slt i32 %i68, 0
  %i70 = select i1 %i69, i8* %i32, i8* %i47
  br label %bb71

bb71:                                             ; preds = %bb65, %bb63, %bb57, %bb55
  %i72 = phi i8* [ %i62, %bb57 ], [ %i70, %bb65 ], [ %i45, %bb55 ], [ %i45, %bb63 ]
  %i73 = sub i64 0, %i44
  %i74 = getelementptr inbounds i8, i8* %i36, i64 %i73
  %i75 = getelementptr inbounds i8, i8* %i36, i64 %i44
  %i76 = bitcast i8* %i74 to %__SOADT___DFR___DFT_struct.arc**
  %i77 = bitcast i8* %i36 to %__SOADT___DFR___DFT_struct.arc**
  %i78 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i76, %__SOADT___DFR___DFT_struct.arc** %i77) #3
  %i79 = icmp slt i32 %i78, 0
  %i80 = bitcast i8* %i36 to %__SOADT___DFR___DFT_struct.arc**
  %i81 = bitcast i8* %i75 to %__SOADT___DFR___DFT_struct.arc**
  %i82 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i80, %__SOADT___DFR___DFT_struct.arc** %i81) #3
  br i1 %i79, label %bb83, label %bb91

bb83:                                             ; preds = %bb71
  %i84 = icmp slt i32 %i82, 0
  br i1 %i84, label %bb99, label %bb85

bb85:                                             ; preds = %bb83
  %i86 = bitcast i8* %i74 to %__SOADT___DFR___DFT_struct.arc**
  %i87 = bitcast i8* %i75 to %__SOADT___DFR___DFT_struct.arc**
  %i88 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i86, %__SOADT___DFR___DFT_struct.arc** %i87) #3
  %i89 = icmp slt i32 %i88, 0
  %i90 = select i1 %i89, i8* %i75, i8* %i74
  br label %bb99

bb91:                                             ; preds = %bb71
  %i92 = icmp sgt i32 %i82, 0
  br i1 %i92, label %bb99, label %bb93

bb93:                                             ; preds = %bb91
  %i94 = bitcast i8* %i74 to %__SOADT___DFR___DFT_struct.arc**
  %i95 = bitcast i8* %i75 to %__SOADT___DFR___DFT_struct.arc**
  %i96 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i94, %__SOADT___DFR___DFT_struct.arc** %i95) #3
  %i97 = icmp slt i32 %i96, 0
  %i98 = select i1 %i97, i8* %i74, i8* %i75
  br label %bb99

bb99:                                             ; preds = %bb93, %bb91, %bb85, %bb83
  %i100 = phi i8* [ %i90, %bb85 ], [ %i98, %bb93 ], [ %i36, %bb83 ], [ %i36, %bb91 ]
  %i101 = sub i64 0, %i46
  %i102 = getelementptr inbounds i8, i8* %i41, i64 %i101
  %i103 = getelementptr inbounds i8, i8* %i41, i64 %i73
  %i104 = bitcast i8* %i102 to %__SOADT___DFR___DFT_struct.arc**
  %i105 = bitcast i8* %i103 to %__SOADT___DFR___DFT_struct.arc**
  %i106 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i104, %__SOADT___DFR___DFT_struct.arc** %i105) #3
  %i107 = icmp slt i32 %i106, 0
  %i108 = bitcast i8* %i103 to %__SOADT___DFR___DFT_struct.arc**
  %i109 = bitcast i8* %i41 to %__SOADT___DFR___DFT_struct.arc**
  %i110 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i108, %__SOADT___DFR___DFT_struct.arc** %i109) #3
  br i1 %i107, label %bb111, label %bb119

bb111:                                            ; preds = %bb99
  %i112 = icmp slt i32 %i110, 0
  br i1 %i112, label %bb127, label %bb113

bb113:                                            ; preds = %bb111
  %i114 = bitcast i8* %i102 to %__SOADT___DFR___DFT_struct.arc**
  %i115 = bitcast i8* %i41 to %__SOADT___DFR___DFT_struct.arc**
  %i116 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i114, %__SOADT___DFR___DFT_struct.arc** %i115) #3
  %i117 = icmp slt i32 %i116, 0
  %i118 = select i1 %i117, i8* %i41, i8* %i102
  br label %bb127

bb119:                                            ; preds = %bb99
  %i120 = icmp sgt i32 %i110, 0
  br i1 %i120, label %bb127, label %bb121

bb121:                                            ; preds = %bb119
  %i122 = bitcast i8* %i102 to %__SOADT___DFR___DFT_struct.arc**
  %i123 = bitcast i8* %i41 to %__SOADT___DFR___DFT_struct.arc**
  %i124 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i122, %__SOADT___DFR___DFT_struct.arc** %i123) #3
  %i125 = icmp slt i32 %i124, 0
  %i126 = select i1 %i125, i8* %i102, i8* %i41
  br label %bb127

bb127:                                            ; preds = %bb121, %bb119, %bb113, %bb111, %bb38
  %i128 = phi i8* [ %i41, %bb38 ], [ %i118, %bb113 ], [ %i126, %bb121 ], [ %i103, %bb111 ], [ %i103, %bb119 ]
  %i129 = phi i8* [ %i36, %bb38 ], [ %i100, %bb113 ], [ %i100, %bb121 ], [ %i100, %bb111 ], [ %i100, %bb119 ]
  %i130 = phi i8* [ %i32, %bb38 ], [ %i72, %bb113 ], [ %i72, %bb121 ], [ %i72, %bb111 ], [ %i72, %bb119 ]
  %i131 = bitcast i8* %i130 to %__SOADT___DFR___DFT_struct.arc**
  %i132 = bitcast i8* %i129 to %__SOADT___DFR___DFT_struct.arc**
  %i133 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i131, %__SOADT___DFR___DFT_struct.arc** %i132) #3
  %i134 = icmp slt i32 %i133, 0
  %i135 = bitcast i8* %i129 to %__SOADT___DFR___DFT_struct.arc**
  %i136 = bitcast i8* %i128 to %__SOADT___DFR___DFT_struct.arc**
  %i137 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i135, %__SOADT___DFR___DFT_struct.arc** %i136) #3
  br i1 %i134, label %bb138, label %bb146

bb138:                                            ; preds = %bb127
  %i139 = icmp slt i32 %i137, 0
  br i1 %i139, label %bb154, label %bb140

bb140:                                            ; preds = %bb138
  %i141 = bitcast i8* %i130 to %__SOADT___DFR___DFT_struct.arc**
  %i142 = bitcast i8* %i128 to %__SOADT___DFR___DFT_struct.arc**
  %i143 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i141, %__SOADT___DFR___DFT_struct.arc** %i142) #3
  %i144 = icmp slt i32 %i143, 0
  %i145 = select i1 %i144, i8* %i128, i8* %i130
  br label %bb154

bb146:                                            ; preds = %bb127
  %i147 = icmp sgt i32 %i137, 0
  br i1 %i147, label %bb154, label %bb148

bb148:                                            ; preds = %bb146
  %i149 = bitcast i8* %i130 to %__SOADT___DFR___DFT_struct.arc**
  %i150 = bitcast i8* %i128 to %__SOADT___DFR___DFT_struct.arc**
  %i151 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i149, %__SOADT___DFR___DFT_struct.arc** %i150) #3
  %i152 = icmp slt i32 %i151, 0
  %i153 = select i1 %i152, i8* %i130, i8* %i128
  br label %bb154

bb154:                                            ; preds = %bb148, %bb146, %bb140, %bb138, %bb30
  %i155 = phi i8* [ %i36, %bb30 ], [ %i145, %bb140 ], [ %i153, %bb148 ], [ %i129, %bb138 ], [ %i129, %bb146 ]
  %i156 = bitcast i8* %i32 to i64*
  %i157 = load i64, i64* %i156, align 8
  %i158 = bitcast i8* %i155 to i64*
  %i159 = load i64, i64* %i158, align 8
  store i64 %i159, i64* %i156, align 8
  store i64 %i157, i64* %i158, align 8
  %i160 = getelementptr inbounds i8, i8* %i32, i64 8
  %i161 = shl i64 %i33, 3
  %i162 = add i64 %i161, -8
  %i163 = getelementptr inbounds i8, i8* %i32, i64 %i162
  br label %bb164

bb164:                                            ; preds = %bb218, %bb154
  %i165 = phi i32 [ 0, %bb154 ], [ 1, %bb218 ]
  %i166 = phi i8* [ %i163, %bb154 ], [ %i199, %bb218 ]
  %i167 = phi i8* [ %i163, %bb154 ], [ %i224, %bb218 ]
  %i168 = phi i8* [ %i160, %bb154 ], [ %i223, %bb218 ]
  %i169 = phi i8* [ %i160, %bb154 ], [ %i195, %bb218 ]
  %i170 = icmp ugt i8* %i168, %i167
  br i1 %i170, label %bb192, label %bb171

bb171:                                            ; preds = %bb187, %bb164
  %i172 = phi i8* [ %i189, %bb187 ], [ %i169, %bb164 ]
  %i173 = phi i8* [ %i190, %bb187 ], [ %i168, %bb164 ]
  %i174 = phi i32 [ %i188, %bb187 ], [ %i165, %bb164 ]
  %i175 = bitcast i8* %i173 to %__SOADT___DFR___DFT_struct.arc**
  %i176 = bitcast i8* %i32 to %__SOADT___DFR___DFT_struct.arc**
  %i177 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i175, %__SOADT___DFR___DFT_struct.arc** %i176) #3
  %i178 = icmp slt i32 %i177, 1
  br i1 %i178, label %bb179, label %bb192

bb179:                                            ; preds = %bb171
  %i180 = icmp eq i32 %i177, 0
  br i1 %i180, label %bb181, label %bb187

bb181:                                            ; preds = %bb179
  %i182 = bitcast i8* %i172 to i64*
  %i183 = load i64, i64* %i182, align 8
  %i184 = bitcast i8* %i173 to i64*
  %i185 = load i64, i64* %i184, align 8
  store i64 %i185, i64* %i182, align 8
  store i64 %i183, i64* %i184, align 8
  %i186 = getelementptr inbounds i8, i8* %i172, i64 8
  br label %bb187

bb187:                                            ; preds = %bb181, %bb179
  %i188 = phi i32 [ 1, %bb181 ], [ %i174, %bb179 ]
  %i189 = phi i8* [ %i186, %bb181 ], [ %i172, %bb179 ]
  %i190 = getelementptr inbounds i8, i8* %i173, i64 8
  %i191 = icmp ugt i8* %i190, %i167
  br i1 %i191, label %bb192, label %bb171

bb192:                                            ; preds = %bb187, %bb171, %bb164
  %i193 = phi i32 [ %i165, %bb164 ], [ %i174, %bb171 ], [ %i188, %bb187 ]
  %i194 = phi i8* [ %i168, %bb164 ], [ %i173, %bb171 ], [ %i190, %bb187 ]
  %i195 = phi i8* [ %i169, %bb164 ], [ %i172, %bb171 ], [ %i189, %bb187 ]
  %i196 = icmp ugt i8* %i194, %i167
  br i1 %i196, label %bb225, label %bb197

bb197:                                            ; preds = %bb213, %bb192
  %i198 = phi i8* [ %i216, %bb213 ], [ %i167, %bb192 ]
  %i199 = phi i8* [ %i215, %bb213 ], [ %i166, %bb192 ]
  %i200 = phi i32 [ %i214, %bb213 ], [ %i193, %bb192 ]
  %i201 = bitcast i8* %i198 to %__SOADT___DFR___DFT_struct.arc**
  %i202 = bitcast i8* %i32 to %__SOADT___DFR___DFT_struct.arc**
  %i203 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** %i201, %__SOADT___DFR___DFT_struct.arc** %i202) #3
  %i204 = icmp sgt i32 %i203, -1
  br i1 %i204, label %bb205, label %bb218

bb205:                                            ; preds = %bb197
  %i206 = icmp eq i32 %i203, 0
  br i1 %i206, label %bb207, label %bb213

bb207:                                            ; preds = %bb205
  %i208 = bitcast i8* %i198 to i64*
  %i209 = load i64, i64* %i208, align 8
  %i210 = bitcast i8* %i199 to i64*
  %i211 = load i64, i64* %i210, align 8
  store i64 %i211, i64* %i208, align 8
  store i64 %i209, i64* %i210, align 8
  %i212 = getelementptr inbounds i8, i8* %i199, i64 -8
  br label %bb213

bb213:                                            ; preds = %bb207, %bb205
  %i214 = phi i32 [ 1, %bb207 ], [ %i200, %bb205 ]
  %i215 = phi i8* [ %i212, %bb207 ], [ %i199, %bb205 ]
  %i216 = getelementptr inbounds i8, i8* %i198, i64 -8
  %i217 = icmp ugt i8* %i194, %i216
  br i1 %i217, label %bb225, label %bb197

bb218:                                            ; preds = %bb197
  %i219 = bitcast i8* %i194 to i64*
  %i220 = load i64, i64* %i219, align 8
  %i221 = bitcast i8* %i198 to i64*
  %i222 = load i64, i64* %i221, align 8
  store i64 %i222, i64* %i219, align 8
  store i64 %i220, i64* %i221, align 8
  %i223 = getelementptr inbounds i8, i8* %i194, i64 8
  %i224 = getelementptr inbounds i8, i8* %i198, i64 -8
  br label %bb164

bb225:                                            ; preds = %bb213, %bb192
  %i226 = phi i32 [ %i214, %bb213 ], [ %i193, %bb192 ]
  %i227 = phi i8* [ %i215, %bb213 ], [ %i166, %bb192 ]
  %i228 = phi i8* [ %i216, %bb213 ], [ %i167, %bb192 ]
  %i229 = icmp eq i32 %i226, 0
  %i230 = shl i64 %i33, 3
  %i231 = getelementptr inbounds i8, i8* %i32, i64 %i230
  br i1 %i229, label %bb232, label %bb253

bb232:                                            ; preds = %bb225
  %i233 = icmp sgt i64 %i230, 8
  br i1 %i233, label %bb234, label %bb319

bb234:                                            ; preds = %bb250, %bb232
  %i235 = phi i8* [ %i251, %bb250 ], [ %i160, %bb232 ]
  %i236 = icmp ugt i8* %i235, %i32
  br i1 %i236, label %bb237, label %bb250

bb237:                                            ; preds = %bb244, %bb234
  %i238 = phi i8* [ %i239, %bb244 ], [ %i235, %bb234 ]
  %i239 = getelementptr inbounds i8, i8* %i238, i64 -8
  %i240 = bitcast i8* %i239 to %__SOADT___DFR___DFT_struct.arc**
  %i241 = bitcast i8* %i238 to %__SOADT___DFR___DFT_struct.arc**
  %i242 = tail call i32 @arc_compare.55.85.115(%__SOADT___DFR___DFT_struct.arc** nonnull %i240, %__SOADT___DFR___DFT_struct.arc** %i241) #3
  %i243 = icmp sgt i32 %i242, 0
  br i1 %i243, label %bb244, label %bb250

bb244:                                            ; preds = %bb237
  %i245 = bitcast i8* %i238 to i64*
  %i246 = load i64, i64* %i245, align 8
  %i247 = bitcast i8* %i239 to i64*
  %i248 = load i64, i64* %i247, align 8
  store i64 %i248, i64* %i245, align 8
  store i64 %i246, i64* %i247, align 8
  %i249 = icmp ugt i8* %i239, %i32
  br i1 %i249, label %bb237, label %bb250

bb250:                                            ; preds = %bb244, %bb237, %bb234
  %i251 = getelementptr inbounds i8, i8* %i235, i64 8
  %i252 = icmp ult i8* %i251, %i231
  br i1 %i252, label %bb234, label %bb319

bb253:                                            ; preds = %bb225
  %i254 = ptrtoint i8* %i195 to i64
  %i255 = sub i64 %i254, %i31
  %i256 = ptrtoint i8* %i194 to i64
  %i257 = sub i64 %i256, %i254
  %i259 = tail call i64 @llvm.smin.i64(i64 %i255, i64 %i257)
  %i260 = icmp eq i64 %i259, 0
  br i1 %i260, label %bb279, label %bb261

bb261:                                            ; preds = %bb253
  %i262 = sub i64 0, %i259
  %i263 = getelementptr inbounds i8, i8* %i194, i64 %i262
  %i264 = shl i64 %i259, 32
  %i265 = ashr exact i64 %i264, 32
  %i266 = lshr i64 %i265, 3
  %i267 = bitcast i8* %i32 to i64*
  %i268 = bitcast i8* %i263 to i64*
  br label %bb269

bb269:                                            ; preds = %bb269, %bb261
  %i270 = phi i64* [ %i268, %bb261 ], [ %i276, %bb269 ]
  %i271 = phi i64* [ %i267, %bb261 ], [ %i275, %bb269 ]
  %i272 = phi i64 [ %i266, %bb261 ], [ %i277, %bb269 ]
  %i273 = load i64, i64* %i271, align 8
  %i274 = load i64, i64* %i270, align 8
  %i275 = getelementptr inbounds i64, i64* %i271, i64 1
  store i64 %i274, i64* %i271, align 8
  %i276 = getelementptr inbounds i64, i64* %i270, i64 1
  store i64 %i273, i64* %i270, align 8
  %i277 = add nsw i64 %i272, -1
  %i278 = icmp sgt i64 %i272, 1
  br i1 %i278, label %bb269, label %bb279

bb279:                                            ; preds = %bb269, %bb253
  %i280 = ptrtoint i8* %i227 to i64
  %i281 = ptrtoint i8* %i228 to i64
  %i282 = sub i64 %i280, %i281
  %i283 = ptrtoint i8* %i231 to i64
  %i284 = sub i64 %i283, %i280
  %i285 = add i64 %i284, -8
  %i287 = tail call i64 @llvm.smin.i64(i64 %i282, i64 %i285)
  %i288 = icmp eq i64 %i287, 0
  br i1 %i288, label %bb307, label %bb289

bb289:                                            ; preds = %bb279
  %i290 = sub i64 0, %i287
  %i291 = getelementptr inbounds i8, i8* %i231, i64 %i290
  %i292 = shl i64 %i287, 32
  %i293 = ashr exact i64 %i292, 32
  %i294 = lshr i64 %i293, 3
  %i295 = bitcast i8* %i194 to i64*
  %i296 = bitcast i8* %i291 to i64*
  br label %bb297

bb297:                                            ; preds = %bb297, %bb289
  %i298 = phi i64* [ %i296, %bb289 ], [ %i304, %bb297 ]
  %i299 = phi i64* [ %i295, %bb289 ], [ %i303, %bb297 ]
  %i300 = phi i64 [ %i294, %bb289 ], [ %i305, %bb297 ]
  %i301 = load i64, i64* %i299, align 8
  %i302 = load i64, i64* %i298, align 8
  %i303 = getelementptr inbounds i64, i64* %i299, i64 1
  store i64 %i302, i64* %i299, align 8
  %i304 = getelementptr inbounds i64, i64* %i298, i64 1
  store i64 %i301, i64* %i298, align 8
  %i305 = add nsw i64 %i300, -1
  %i306 = icmp sgt i64 %i300, 1
  br i1 %i306, label %bb297, label %bb307

bb307:                                            ; preds = %bb297, %bb279
  %i308 = icmp ugt i64 %i257, 8
  br i1 %i308, label %bb309, label %bb311

bb309:                                            ; preds = %bb307
  %i310 = lshr i64 %i257, 3
  tail call fastcc void @spec_qsort.40(i8* %i32, i64 %i310)
  br label %bb311

bb311:                                            ; preds = %bb309, %bb307
  %i312 = icmp ugt i64 %i282, 8
  br i1 %i312, label %bb313, label %bb319

bb313:                                            ; preds = %bb311
  %i314 = sub i64 0, %i282
  %i315 = getelementptr inbounds i8, i8* %i231, i64 %i314
  %i316 = lshr i64 %i282, 3
  %i317 = ptrtoint i8* %i315 to i64
  %i318 = icmp ult i64 %i316, 7
  br i1 %i318, label %bb3, label %bb30

bb319:                                            ; preds = %bb311, %bb250, %bb232, %bb27, %bb3
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #2
declare i64 @llvm.smin.i64(i64, i64)

attributes #0 = { "is-qsort-compare" }
attributes #1 = { "is-qsort-spec_qsort" }
attributes #2 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { "must-be-qsort-compare" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "test", directory: ".")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!""}
!8 = !DILocalVariable(name: "na", arg: 1, scope: !9, file: !1, line: 1, type: !13)
!9 = distinct !DISubprogram(name: "na", linkageName: "na", scope: !1, file: !1, line: 1, type: !10, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!10 = !DISubroutineType(types: !11)
!11 = !{!12, !13}
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!14 = !DILocation(line: 1, column: 1, scope: !9)

; end INTEL_FEATURE_SW_ADVANCED
