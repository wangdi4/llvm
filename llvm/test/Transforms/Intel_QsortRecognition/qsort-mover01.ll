; REQUIRES: asserts
; RUN: opt < %s -qsortrecognizer -debug-only=qsortrecognizer -qsort-test-insert=false -qsort-test-pivot=false -qsort-test-pivot-movers=true -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(qsortrecognizer)' -debug-only=qsortrecognizer -qsort-test-insert=false -qsort-test-pivot=false -qsort-test-pivot-movers=true -disable-output 2>&1 | FileCheck %s

; Check that the up and down pivot mover loops are recognized.

; CHECK: QsortRec: Checking Pivot Mover Candidate in qsort_mover
; CHECK: QsortRec: Pivot Mover Candidate in qsort_mover PASSED Test (UP)
; CHECK: QsortRec: Checking Pivot Mover Candidate in qsort_mover
; CHECK: QsortRec: Pivot Mover Candidate in qsort_mover PASSED Test (DOWN)

; ModuleID = 'qsort-mover01.ll'
source_filename = "qsort-mover01.ll"

%struct.arc = type { i32, i64, %struct.node*, %struct.node*, i16, %struct.arc*, %struct.arc*, i64, i64 }
%struct.node = type { i64, i32, %struct.node*, %struct.node*, %struct.node*, %struct.node*, %struct.arc*, %struct.arc*, %struct.arc*, %struct.arc*, i64, i64, i32, i32 }
%struct.basket = type { %struct.arc*, i64, i64, i64 }

@myglobal1 = dso_local global i64 32, align 4
@myglobal2 = dso_local global i64 32, align 4
@buffer = dso_local global i8* null, align 8

declare i32 @arc_compare(%struct.arc** nocapture readonly, %struct.arc** nocapture readonly)

define internal fastcc void @qsort_mover(i8* %arg, i64 %arg1) unnamed_addr {
bb:
  %tmp = ptrtoint i8* %arg to i64
  %tmp2 = icmp ult i64 %arg1, 7
  br i1 %tmp2, label %bb319, label %bb30

bb30:                                             ; preds = %bb318, %bb
  %tmp31 = phi i64 [ %tmp400, %bb318 ], [ %tmp, %bb ]
  %tmp32 = phi i8* [ %tmp401, %bb318 ], [ %arg, %bb ]
  %tmp33 = phi i64 [ %tmp402, %bb318 ], [ %arg1, %bb ]
  %tmp34 = lshr i64 %tmp33, 1
  %tmp35 = shl i64 %tmp34, 3
  %tmp36 = getelementptr inbounds i8, i8* %tmp32, i64 %tmp35
  %tmp37 = icmp eq i64 %tmp33, 7
  br i1 %tmp37, label %bb154, label %bb38

bb38:                                             ; preds = %bb30
  %tmp39 = shl i64 %tmp33, 3
  %tmp40 = add i64 %tmp39, -8
  %tmp41 = getelementptr inbounds i8, i8* %tmp32, i64 %tmp40
  %tmp42 = icmp ugt i64 %tmp33, 40
  br i1 %tmp42, label %bb43, label %bb127

bb43:                                             ; preds = %bb38
  %tmp44 = and i64 %tmp33, -8
  %tmp45 = getelementptr inbounds i8, i8* %tmp32, i64 %tmp44
  %tmp46 = shl i64 %tmp44, 1
  %tmp47 = getelementptr inbounds i8, i8* %tmp32, i64 %tmp46
  %tmp48 = bitcast i8* %tmp32 to %struct.arc**
  %tmp49 = bitcast i8* %tmp45 to %struct.arc**
  %tmp50 = tail call i32 @arc_compare(%struct.arc** %tmp48, %struct.arc** %tmp49)
  %tmp51 = icmp slt i32 %tmp50, 0
  %tmp52 = bitcast i8* %tmp45 to %struct.arc**
  %tmp53 = bitcast i8* %tmp47 to %struct.arc**
  %tmp54 = tail call i32 @arc_compare(%struct.arc** %tmp52, %struct.arc** %tmp53)
  br i1 %tmp51, label %bb55, label %bb63

bb55:                                             ; preds = %bb43
  %tmp56 = icmp slt i32 %tmp54, 0
  br i1 %tmp56, label %bb71, label %bb57

bb57:                                             ; preds = %bb55
  %tmp58 = bitcast i8* %tmp32 to %struct.arc**
  %tmp59 = bitcast i8* %tmp47 to %struct.arc**
  %tmp60 = tail call i32 @arc_compare(%struct.arc** %tmp58, %struct.arc** %tmp59)
  %tmp61 = icmp slt i32 %tmp60, 0
  %tmp62 = select i1 %tmp61, i8* %tmp47, i8* %tmp32
  br label %bb71

bb63:                                             ; preds = %bb43
  %tmp64 = icmp sgt i32 %tmp54, 0
  br i1 %tmp64, label %bb71, label %bb65

bb65:                                             ; preds = %bb63
  %tmp66 = bitcast i8* %tmp32 to %struct.arc**
  %tmp67 = bitcast i8* %tmp47 to %struct.arc**
  %tmp68 = tail call i32 @arc_compare(%struct.arc** %tmp66, %struct.arc** %tmp67)
  %tmp69 = icmp slt i32 %tmp68, 0
  %tmp70 = select i1 %tmp69, i8* %tmp32, i8* %tmp47
  br label %bb71

bb71:                                             ; preds = %bb65, %bb63, %bb57, %bb55
  %tmp72 = phi i8* [ %tmp62, %bb57 ], [ %tmp70, %bb65 ], [ %tmp45, %bb55 ], [ %tmp45, %bb63 ]
  %tmp73 = sub i64 0, %tmp44
  %tmp74 = getelementptr inbounds i8, i8* %tmp36, i64 %tmp73
  %tmp75 = getelementptr inbounds i8, i8* %tmp36, i64 %tmp44
  %tmp76 = bitcast i8* %tmp74 to %struct.arc**
  %tmp77 = bitcast i8* %tmp36 to %struct.arc**
  %tmp78 = tail call i32 @arc_compare(%struct.arc** %tmp76, %struct.arc** %tmp77)
  %tmp79 = icmp slt i32 %tmp78, 0
  %tmp80 = bitcast i8* %tmp36 to %struct.arc**
  %tmp81 = bitcast i8* %tmp75 to %struct.arc**
  %tmp82 = tail call i32 @arc_compare(%struct.arc** %tmp80, %struct.arc** %tmp81)
  br i1 %tmp79, label %bb83, label %bb91

bb83:                                             ; preds = %bb71
  %tmp84 = icmp slt i32 %tmp82, 0
  br i1 %tmp84, label %bb99, label %bb85

bb85:                                             ; preds = %bb83
  %tmp86 = bitcast i8* %tmp74 to %struct.arc**
  %tmp87 = bitcast i8* %tmp75 to %struct.arc**
  %tmp88 = tail call i32 @arc_compare(%struct.arc** %tmp86, %struct.arc** %tmp87)
  %tmp89 = icmp slt i32 %tmp88, 0
  %tmp90 = select i1 %tmp89, i8* %tmp75, i8* %tmp74
  br label %bb99

bb91:                                             ; preds = %bb71
  %tmp92 = icmp sgt i32 %tmp82, 0
  br i1 %tmp92, label %bb99, label %bb93

bb93:                                             ; preds = %bb91
  %tmp94 = bitcast i8* %tmp74 to %struct.arc**
  %tmp95 = bitcast i8* %tmp75 to %struct.arc**
  %tmp96 = tail call i32 @arc_compare(%struct.arc** %tmp94, %struct.arc** %tmp95)
  %tmp97 = icmp slt i32 %tmp96, 0
  %tmp98 = select i1 %tmp97, i8* %tmp74, i8* %tmp75
  br label %bb99

bb99:                                             ; preds = %bb93, %bb91, %bb85, %bb83
  %tmp100 = phi i8* [ %tmp90, %bb85 ], [ %tmp98, %bb93 ], [ %tmp36, %bb83 ], [ %tmp36, %bb91 ]
  %tmp101 = sub i64 0, %tmp46
  %tmp102 = getelementptr inbounds i8, i8* %tmp41, i64 %tmp101
  %tmp103 = getelementptr inbounds i8, i8* %tmp41, i64 %tmp73
  %tmp104 = bitcast i8* %tmp102 to %struct.arc**
  %tmp105 = bitcast i8* %tmp103 to %struct.arc**
  %tmp106 = tail call i32 @arc_compare(%struct.arc** %tmp104, %struct.arc** %tmp105)
  %tmp107 = icmp slt i32 %tmp106, 0
  %tmp108 = bitcast i8* %tmp103 to %struct.arc**
  %tmp109 = bitcast i8* %tmp41 to %struct.arc**
  %tmp110 = tail call i32 @arc_compare(%struct.arc** %tmp108, %struct.arc** %tmp109)
  br i1 %tmp107, label %bb111, label %bb119

bb111:                                            ; preds = %bb99
  %tmp112 = icmp slt i32 %tmp110, 0
  br i1 %tmp112, label %bb127, label %bb113

bb113:                                            ; preds = %bb111
  %tmp114 = bitcast i8* %tmp102 to %struct.arc**
  %tmp115 = bitcast i8* %tmp41 to %struct.arc**
  %tmp116 = tail call i32 @arc_compare(%struct.arc** %tmp114, %struct.arc** %tmp115)
  %tmp117 = icmp slt i32 %tmp116, 0
  %tmp118 = select i1 %tmp117, i8* %tmp41, i8* %tmp102
  br label %bb127

bb119:                                            ; preds = %bb99
  %tmp120 = icmp sgt i32 %tmp110, 0
  br i1 %tmp120, label %bb127, label %bb121

bb121:                                            ; preds = %bb119
  %tmp122 = bitcast i8* %tmp102 to %struct.arc**
  %tmp123 = bitcast i8* %tmp41 to %struct.arc**
  %tmp124 = tail call i32 @arc_compare(%struct.arc** %tmp122, %struct.arc** %tmp123)
  %tmp125 = icmp slt i32 %tmp124, 0
  %tmp126 = select i1 %tmp125, i8* %tmp102, i8* %tmp41
  br label %bb127

bb127:                                            ; preds = %bb121, %bb119, %bb113, %bb111, %bb38
  %tmp128 = phi i8* [ %tmp41, %bb38 ], [ %tmp118, %bb113 ], [ %tmp126, %bb121 ], [ %tmp103, %bb111 ], [ %tmp103, %bb119 ]
  %tmp129 = phi i8* [ %tmp36, %bb38 ], [ %tmp100, %bb113 ], [ %tmp100, %bb121 ], [ %tmp100, %bb111 ], [ %tmp100, %bb119 ]
  %tmp130 = phi i8* [ %tmp32, %bb38 ], [ %tmp72, %bb113 ], [ %tmp72, %bb121 ], [ %tmp72, %bb111 ], [ %tmp72, %bb119 ]
  %tmp131 = bitcast i8* %tmp130 to %struct.arc**
  %tmp132 = bitcast i8* %tmp129 to %struct.arc**
  %tmp133 = tail call i32 @arc_compare(%struct.arc** %tmp131, %struct.arc** %tmp132)
  %tmp134 = icmp slt i32 %tmp133, 0
  %tmp135 = bitcast i8* %tmp129 to %struct.arc**
  %tmp136 = bitcast i8* %tmp128 to %struct.arc**
  %tmp137 = tail call i32 @arc_compare(%struct.arc** %tmp135, %struct.arc** %tmp136)
  br i1 %tmp134, label %bb138, label %bb146

bb138:                                            ; preds = %bb127
  %tmp139 = icmp slt i32 %tmp137, 0
  br i1 %tmp139, label %bb154, label %bb140

bb140:                                            ; preds = %bb138
  %tmp141 = bitcast i8* %tmp130 to %struct.arc**
  %tmp142 = bitcast i8* %tmp128 to %struct.arc**
  %tmp143 = tail call i32 @arc_compare(%struct.arc** %tmp141, %struct.arc** %tmp142)
  %tmp144 = icmp slt i32 %tmp143, 0
  %tmp145 = select i1 %tmp144, i8* %tmp128, i8* %tmp130
  br label %bb154

bb146:                                            ; preds = %bb127
  %tmp147 = icmp sgt i32 %tmp137, 0
  br i1 %tmp147, label %bb154, label %bb148

bb148:                                            ; preds = %bb146
  %tmp149 = bitcast i8* %tmp130 to %struct.arc**
  %tmp150 = bitcast i8* %tmp128 to %struct.arc**
  %tmp151 = tail call i32 @arc_compare(%struct.arc** %tmp149, %struct.arc** %tmp150)
  %tmp152 = icmp slt i32 %tmp151, 0
  %tmp153 = select i1 %tmp152, i8* %tmp130, i8* %tmp128
  br label %bb154

bb154:                                            ; preds = %bb148, %bb146, %bb140, %bb138, %bb30
  %tmp155 = phi i8* [ %tmp36, %bb30 ], [ %tmp145, %bb140 ], [ %tmp153, %bb148 ], [ %tmp129, %bb138 ], [ %tmp129, %bb146 ]
  %tmp156 = bitcast i8* %tmp32 to i64*
  %tmp157 = load i64, i64* %tmp156, align 8
  %tmp158 = bitcast i8* %tmp155 to i64*
  %tmp159 = load i64, i64* %tmp158, align 8
  store i64 %tmp159, i64* %tmp156, align 8
  store i64 %tmp157, i64* %tmp158, align 8
  %tmp160 = getelementptr inbounds i8, i8* %tmp32, i64 8
  %tmp161 = shl i64 %tmp33, 3
  %tmp162 = add i64 %tmp161, -8
  %tmp163 = getelementptr inbounds i8, i8* %tmp32, i64 %tmp162
  br label %bb164

bb164:                                            ; preds = %bb218, %bb154
  %tmp165 = phi i32 [ 0, %bb154 ], [ 1, %bb218 ]
  %tmp166 = phi i8* [ %tmp163, %bb154 ], [ %tmp199, %bb218 ]
  %tmp167 = phi i8* [ %tmp163, %bb154 ], [ %tmp224, %bb218 ]
  %tmp168 = phi i8* [ %tmp160, %bb154 ], [ %tmp223, %bb218 ]
  %tmp169 = phi i8* [ %tmp160, %bb154 ], [ %tmp195, %bb218 ]
  %tmp170 = icmp ugt i8* %tmp168, %tmp167
  br i1 %tmp170, label %bb192, label %bb171

bb171:                                            ; preds = %bb187, %bb164
  %tmp172 = phi i8* [ %tmp189, %bb187 ], [ %tmp169, %bb164 ]
  %tmp173 = phi i8* [ %tmp190, %bb187 ], [ %tmp168, %bb164 ]
  %tmp174 = phi i32 [ %tmp188, %bb187 ], [ %tmp165, %bb164 ]
  %tmp175 = bitcast i8* %tmp173 to %struct.arc**
  %tmp176 = bitcast i8* %tmp32 to %struct.arc**
  %tmp177 = tail call i32 @arc_compare(%struct.arc** %tmp175, %struct.arc** %tmp176)
  %tmp178 = icmp slt i32 %tmp177, 1
  br i1 %tmp178, label %bb179, label %bb192

bb179:                                            ; preds = %bb171
  %tmp180 = icmp eq i32 %tmp177, 0
  br i1 %tmp180, label %bb181, label %bb187

bb181:                                            ; preds = %bb179
  %tmp182 = bitcast i8* %tmp172 to i64*
  %tmp183 = load i64, i64* %tmp182, align 8
  %tmp184 = bitcast i8* %tmp173 to i64*
  %tmp185 = load i64, i64* %tmp184, align 8
  store i64 %tmp185, i64* %tmp182, align 8
  store i64 %tmp183, i64* %tmp184, align 8
  %tmp186 = getelementptr inbounds i8, i8* %tmp172, i64 8
  br label %bb187

bb187:                                            ; preds = %bb181, %bb179
  %tmp188 = phi i32 [ 1, %bb181 ], [ %tmp174, %bb179 ]
  %tmp189 = phi i8* [ %tmp186, %bb181 ], [ %tmp172, %bb179 ]
  %tmp190 = getelementptr inbounds i8, i8* %tmp173, i64 8
  %tmp191 = icmp ugt i8* %tmp190, %tmp167
  br i1 %tmp191, label %bb192, label %bb171

bb192:                                            ; preds = %bb187, %bb171, %bb164
  %tmp193 = phi i32 [ %tmp165, %bb164 ], [ %tmp174, %bb171 ], [ %tmp188, %bb187 ]
  %tmp194 = phi i8* [ %tmp168, %bb164 ], [ %tmp173, %bb171 ], [ %tmp190, %bb187 ]
  %tmp195 = phi i8* [ %tmp169, %bb164 ], [ %tmp172, %bb171 ], [ %tmp189, %bb187 ]
  %tmp196 = icmp ugt i8* %tmp194, %tmp167
  br i1 %tmp196, label %bb319, label %bb197

bb197:                                            ; preds = %bb213, %bb192
  %tmp198 = phi i8* [ %tmp216, %bb213 ], [ %tmp167, %bb192 ]
  %tmp199 = phi i8* [ %tmp215, %bb213 ], [ %tmp166, %bb192 ]
  %tmp200 = phi i32 [ %tmp214, %bb213 ], [ %tmp193, %bb192 ]
  %tmp201 = bitcast i8* %tmp198 to %struct.arc**
  %tmp202 = bitcast i8* %tmp32 to %struct.arc**
  %tmp203 = tail call i32 @arc_compare(%struct.arc** %tmp201, %struct.arc** %tmp202)
  %tmp204 = icmp sgt i32 %tmp203, -1
  br i1 %tmp204, label %bb205, label %bb218

bb205:                                            ; preds = %bb197
  %tmp206 = icmp eq i32 %tmp203, 0
  br i1 %tmp206, label %bb207, label %bb213

bb207:                                            ; preds = %bb205
  %tmp208 = bitcast i8* %tmp198 to i64*
  %tmp209 = load i64, i64* %tmp208, align 8
  %tmp210 = bitcast i8* %tmp199 to i64*
  %tmp211 = load i64, i64* %tmp210, align 8
  store i64 %tmp211, i64* %tmp208, align 8
  store i64 %tmp209, i64* %tmp210, align 8
  %tmp212 = getelementptr inbounds i8, i8* %tmp199, i64 -8
  br label %bb213

bb213:                                            ; preds = %bb207, %bb205
  %tmp214 = phi i32 [ 1, %bb207 ], [ %tmp200, %bb205 ]
  %tmp215 = phi i8* [ %tmp212, %bb207 ], [ %tmp199, %bb205 ]
  %tmp216 = getelementptr inbounds i8, i8* %tmp198, i64 -8
  %tmp217 = icmp ugt i8* %tmp194, %tmp216
  br i1 %tmp217, label %bb319, label %bb197

bb218:                                            ; preds = %bb197
  %tmp219 = bitcast i8* %tmp194 to i64*
  %tmp220 = load i64, i64* %tmp219, align 8
  %tmp221 = bitcast i8* %tmp198 to i64*
  %tmp222 = load i64, i64* %tmp221, align 8
  store i64 %tmp222, i64* %tmp219, align 8
  store i64 %tmp220, i64* %tmp221, align 8
  %tmp223 = getelementptr inbounds i8, i8* %tmp194, i64 8
  %tmp224 = getelementptr inbounds i8, i8* %tmp198, i64 -8
  br label %bb164

bb318:
  %tmp400 = load i64, i64* @myglobal1, align 8
  %tmp401 = load i8*, i8** @buffer, align 8
  %tmp402 = load i64, i64* @myglobal2, align 8
  %tmp403 = icmp ult i64 %tmp400, 7
  br i1 %tmp403, label %bb30, label %bb319

bb319:                                            ; preds = %bb311, %bb250, %bb232, %bb27, %bb3

  ret void
}
