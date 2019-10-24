; REQUIRES: asserts
; RUN: opt < %s -qsortrecognizer -debug-only=qsortrecognizer -qsort-test-insert=false -qsort-test-pivot=true -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(qsortrecognizer)' -debug-only=qsortrecognizer -qsort-test-insert=false -qsort-test-pivot=true -disable-output 2>&1 | FileCheck %s

; Test that the pivot values are recognized for qsort, but that they are not all
; provably within range.

; CHECK: QsortRec: Pivot:   %tmp33 = getelementptr inbounds i8, i8* %tmp29, i64 100
; CHECK: QsortRec: Pivot:   %tmp38 = getelementptr inbounds i8, i8* %tmp29, i64 %tmp37
; CHECK: QsortRec: Pivot:   %tmp99 = getelementptr inbounds i8, i8* %tmp38, i64 %tmp98
; CHECK: QsortRec: Pivot:   %tmp100 = getelementptr inbounds i8, i8* %tmp38, i64 %tmp70
; CHECK: QsortRec: Pivot:   %tmp44 = getelementptr inbounds i8, i8* %tmp29, i64 %tmp43
; CHECK: QsortRec: Pivot:   %tmp42 = getelementptr inbounds i8, i8* %tmp29, i64 %tmp41
; CHECK: QsortRec: Pivot:   %tmp72 = getelementptr inbounds i8, i8* %tmp33, i64 %tmp41
; CHECK: QsortRec: Pivot:   %tmp71 = getelementptr inbounds i8, i8* %tmp33, i64 %tmp70
; CHECK: QsortRec: Check:  %tmp33 = getelementptr inbounds i8, i8* %tmp29, i64 100
; CHECK: QsortRec: Bad Pivot:   %tmp33 = getelementptr inbounds i8, i8* %tmp29, i64 100
; CHECK-NOT: QsortRec:   %tmp38 = getelementptr inbounds i8, i8* %tmp29, i64 %tmp37
; CHECK-NOT: QsortRec:   %tmp99 = getelementptr inbounds i8, i8* %tmp38, i64 %tmp98
; CHECK-NOT: QsortRec:   %tmp100 = getelementptr inbounds i8, i8* %tmp38, i64 %tmp70
; CHECK-NOT: QsortRec:   %tmp44 = getelementptr inbounds i8, i8* %tmp29, i64 %tmp43
; CHECK-NOT: QsortRec:   %tmp42 = getelementptr inbounds i8, i8* %tmp29, i64 %tmp41
; CHECK-NOT: QsortRec:   %tmp72 = getelementptr inbounds i8, i8* %tmp33, i64 %tmp41
; CHECK-NOT: QsortRec:   %tmp71 = getelementptr inbounds i8, i8* %tmp33, i64 %tmp70
; CHECK-NOT: QsortRec: qsort_pivot passed pivot test

%struct.arc = type { i32, i64, %struct.node*, %struct.node*, i16, %struct.arc*, %struct.arc*, i64, i64 }
%struct.node = type { i64, i32, %struct.node*, %struct.node*, %struct.node*, %struct.node*, %struct.arc*, %struct.arc*, %struct.arc*, %struct.arc*, i64, i64, i32, i32 }

@myglobal = dso_local global i64 32, align 4
@buffer = dso_local global i8* null, align 8

define internal i32 @arc_compare(%struct.arc** nocapture readonly %arg, %struct.arc** nocapture readonly %arg1) {
bb:
  %tmp = load %struct.arc*, %struct.arc** %arg, align 8
  %tmp2 = getelementptr inbounds %struct.arc, %struct.arc* %tmp, i64 0, i32 7
  %tmp3 = load i64, i64* %tmp2, align 8
  %tmp4 = load %struct.arc*, %struct.arc** %arg1, align 8
  %tmp5 = getelementptr inbounds %struct.arc, %struct.arc* %tmp4, i64 0, i32 7
  %tmp6 = load i64, i64* %tmp5, align 8
  %tmp7 = icmp sgt i64 %tmp3, %tmp6
  br i1 %tmp7, label %bb17, label %bb8

bb8:                                              ; preds = %bb
  %tmp9 = icmp slt i64 %tmp3, %tmp6
  br i1 %tmp9, label %bb17, label %bb10

bb10:                                             ; preds = %bb8
  %tmp11 = getelementptr inbounds %struct.arc, %struct.arc* %tmp, i64 0, i32 0
  %tmp12 = load i32, i32* %tmp11, align 8
  %tmp13 = getelementptr inbounds %struct.arc, %struct.arc* %tmp4, i64 0, i32 0
  %tmp14 = load i32, i32* %tmp13, align 8
  %tmp15 = icmp slt i32 %tmp12, %tmp14
  %tmp16 = select i1 %tmp15, i32 -1, i32 1
  br label %bb17

bb17:                                             ; preds = %bb10, %bb8, %bb
  %tmp18 = phi i32 [ 1, %bb ], [ -1, %bb8 ], [ %tmp16, %bb10 ]
  ret i32 %tmp18
}

define internal fastcc void @qsort_pivot(i8* %arg, i64 %arg1) unnamed_addr {
bb:
  %tmp = icmp ult i64 %arg1, 7
  br i1 %tmp, label %bb2, label %bb28

bb2:                                              ; preds = %bb
  ret void

bb28:                                             ; preds = %bb151, %bb
  %tmp29 = phi i8* [ %tmp154, %bb151 ], [ %arg, %bb ]
  %tmp30 = phi i64 [ %tmp153, %bb151 ], [ %arg1, %bb ]
  %tmp31 = lshr i64 %tmp30, 1
  %tmp32 = shl i64 %tmp31, 3
  %tmp33 = getelementptr inbounds i8, i8* %tmp29, i64 100
  %tmp34 = icmp eq i64 %tmp30, 7
  br i1 %tmp34, label %bb151, label %bb35

bb35:                                             ; preds = %bb28
  %tmp36 = shl i64 %tmp30, 3
  %tmp37 = add i64 %tmp36, -8
  %tmp38 = getelementptr inbounds i8, i8* %tmp29, i64 %tmp37
  %tmp39 = icmp ugt i64 %tmp30, 40
  br i1 %tmp39, label %bb40, label %bb124

bb40:                                             ; preds = %bb35
  %tmp41 = and i64 %tmp30, -8
  %tmp42 = getelementptr inbounds i8, i8* %tmp29, i64 %tmp41
  %tmp43 = shl i64 %tmp41, 1
  %tmp44 = getelementptr inbounds i8, i8* %tmp29, i64 %tmp43
  %tmp45 = bitcast i8* %tmp29 to %struct.arc**
  %tmp46 = bitcast i8* %tmp42 to %struct.arc**
  %tmp47 = tail call i32 @arc_compare(%struct.arc** %tmp45, %struct.arc** %tmp46)
  %tmp48 = icmp slt i32 %tmp47, 0
  %tmp49 = bitcast i8* %tmp42 to %struct.arc**
  %tmp50 = bitcast i8* %tmp44 to %struct.arc**
  %tmp51 = tail call i32 @arc_compare(%struct.arc** %tmp49, %struct.arc** %tmp50)
  br i1 %tmp48, label %bb52, label %bb60

bb52:                                             ; preds = %bb40
  %tmp53 = icmp slt i32 %tmp51, 0
  br i1 %tmp53, label %bb68, label %bb54

bb54:                                             ; preds = %bb52
  %tmp55 = bitcast i8* %tmp29 to %struct.arc**
  %tmp56 = bitcast i8* %tmp44 to %struct.arc**
  %tmp57 = tail call i32 @arc_compare(%struct.arc** %tmp55, %struct.arc** %tmp56)
  %tmp58 = icmp slt i32 %tmp57, 0
  %tmp59 = select i1 %tmp58, i8* %tmp44, i8* %tmp29
  br label %bb68

bb60:                                             ; preds = %bb40
  %tmp61 = icmp sgt i32 %tmp51, 0
  br i1 %tmp61, label %bb68, label %bb62

bb62:                                             ; preds = %bb60
  %tmp63 = bitcast i8* %tmp29 to %struct.arc**
  %tmp64 = bitcast i8* %tmp44 to %struct.arc**
  %tmp65 = tail call i32 @arc_compare(%struct.arc** %tmp63, %struct.arc** %tmp64)
  %tmp66 = icmp slt i32 %tmp65, 0
  %tmp67 = select i1 %tmp66, i8* %tmp29, i8* %tmp44
  br label %bb68

bb68:                                             ; preds = %bb62, %bb60, %bb54, %bb52
  %tmp69 = phi i8* [ %tmp59, %bb54 ], [ %tmp67, %bb62 ], [ %tmp42, %bb52 ], [ %tmp42, %bb60 ]
  %tmp70 = sub i64 0, %tmp41
  %tmp71 = getelementptr inbounds i8, i8* %tmp33, i64 %tmp70
  %tmp72 = getelementptr inbounds i8, i8* %tmp33, i64 %tmp41
  %tmp73 = bitcast i8* %tmp71 to %struct.arc**
  %tmp74 = bitcast i8* %tmp33 to %struct.arc**
  %tmp75 = tail call i32 @arc_compare(%struct.arc** %tmp73, %struct.arc** %tmp74)
  %tmp76 = icmp slt i32 %tmp75, 0
  %tmp77 = bitcast i8* %tmp33 to %struct.arc**
  %tmp78 = bitcast i8* %tmp72 to %struct.arc**
  %tmp79 = tail call i32 @arc_compare(%struct.arc** %tmp77, %struct.arc** %tmp78)
  br i1 %tmp76, label %bb80, label %bb88

bb80:                                             ; preds = %bb68
  %tmp81 = icmp slt i32 %tmp79, 0
  br i1 %tmp81, label %bb96, label %bb82

bb82:                                             ; preds = %bb80
  %tmp83 = bitcast i8* %tmp71 to %struct.arc**
  %tmp84 = bitcast i8* %tmp72 to %struct.arc**
  %tmp85 = tail call i32 @arc_compare(%struct.arc** %tmp83, %struct.arc** %tmp84)
  %tmp86 = icmp slt i32 %tmp85, 0
  %tmp87 = select i1 %tmp86, i8* %tmp72, i8* %tmp71
  br label %bb96

bb88:                                             ; preds = %bb68
  %tmp89 = icmp sgt i32 %tmp79, 0
  br i1 %tmp89, label %bb96, label %bb90

bb90:                                             ; preds = %bb88
  %tmp91 = bitcast i8* %tmp71 to %struct.arc**
  %tmp92 = bitcast i8* %tmp72 to %struct.arc**
  %tmp93 = tail call i32 @arc_compare(%struct.arc** %tmp91, %struct.arc** %tmp92)
  %tmp94 = icmp slt i32 %tmp93, 0
  %tmp95 = select i1 %tmp94, i8* %tmp71, i8* %tmp72
  br label %bb96

bb96:                                             ; preds = %bb90, %bb88, %bb82, %bb80
  %tmp97 = phi i8* [ %tmp87, %bb82 ], [ %tmp95, %bb90 ], [ %tmp33, %bb80 ], [ %tmp33, %bb88 ]
  %tmp98 = sub i64 0, %tmp43
  %tmp99 = getelementptr inbounds i8, i8* %tmp38, i64 %tmp98
  %tmp100 = getelementptr inbounds i8, i8* %tmp38, i64 %tmp70
  %tmp101 = bitcast i8* %tmp99 to %struct.arc**
  %tmp102 = bitcast i8* %tmp100 to %struct.arc**
  %tmp103 = tail call i32 @arc_compare(%struct.arc** %tmp101, %struct.arc** %tmp102)
  %tmp104 = icmp slt i32 %tmp103, 0
  %tmp105 = bitcast i8* %tmp100 to %struct.arc**
  %tmp106 = bitcast i8* %tmp38 to %struct.arc**
  %tmp107 = tail call i32 @arc_compare(%struct.arc** %tmp105, %struct.arc** %tmp106)
  br i1 %tmp104, label %bb108, label %bb116

bb108:                                            ; preds = %bb96
  %tmp109 = icmp slt i32 %tmp107, 0
  br i1 %tmp109, label %bb124, label %bb110

bb110:                                            ; preds = %bb108
  %tmp111 = bitcast i8* %tmp99 to %struct.arc**
  %tmp112 = bitcast i8* %tmp38 to %struct.arc**
  %tmp113 = tail call i32 @arc_compare(%struct.arc** %tmp111, %struct.arc** %tmp112)
  %tmp114 = icmp slt i32 %tmp113, 0
  %tmp115 = select i1 %tmp114, i8* %tmp38, i8* %tmp99
  br label %bb124

bb116:                                            ; preds = %bb96
  %tmp117 = icmp sgt i32 %tmp107, 0
  br i1 %tmp117, label %bb124, label %bb118

bb118:                                            ; preds = %bb116
  %tmp119 = bitcast i8* %tmp99 to %struct.arc**
  %tmp120 = bitcast i8* %tmp38 to %struct.arc**
  %tmp121 = tail call i32 @arc_compare(%struct.arc** %tmp119, %struct.arc** %tmp120)
  %tmp122 = icmp slt i32 %tmp121, 0
  %tmp123 = select i1 %tmp122, i8* %tmp99, i8* %tmp38
  br label %bb124

bb124:                                            ; preds = %bb118, %bb116, %bb110, %bb108, %bb35
  %tmp125 = phi i8* [ %tmp38, %bb35 ], [ %tmp115, %bb110 ], [ %tmp123, %bb118 ], [ %tmp100, %bb108 ], [ %tmp100, %bb116 ]
  %tmp126 = phi i8* [ %tmp33, %bb35 ], [ %tmp97, %bb110 ], [ %tmp97, %bb118 ], [ %tmp97, %bb108 ], [ %tmp97, %bb116 ]
  %tmp127 = phi i8* [ %tmp29, %bb35 ], [ %tmp69, %bb110 ], [ %tmp69, %bb118 ], [ %tmp69, %bb108 ], [ %tmp69, %bb116 ]
  %tmp128 = bitcast i8* %tmp127 to %struct.arc**
  %tmp129 = bitcast i8* %tmp126 to %struct.arc**
  %tmp130 = tail call i32 @arc_compare(%struct.arc** %tmp128, %struct.arc** %tmp129)
  %tmp131 = icmp slt i32 %tmp130, 0
  %tmp132 = bitcast i8* %tmp126 to %struct.arc**
  %tmp133 = bitcast i8* %tmp125 to %struct.arc**
  %tmp134 = tail call i32 @arc_compare(%struct.arc** %tmp132, %struct.arc** %tmp133)
  br i1 %tmp131, label %bb135, label %bb143

bb135:                                            ; preds = %bb124
  %tmp136 = icmp slt i32 %tmp134, 0
  br i1 %tmp136, label %bb151, label %bb137

bb137:                                            ; preds = %bb135
  %tmp138 = bitcast i8* %tmp127 to %struct.arc**
  %tmp139 = bitcast i8* %tmp125 to %struct.arc**
  %tmp140 = tail call i32 @arc_compare(%struct.arc** %tmp138, %struct.arc** %tmp139)
  %tmp141 = icmp slt i32 %tmp140, 0
  %tmp142 = select i1 %tmp141, i8* %tmp125, i8* %tmp127
  br label %bb151

bb143:                                            ; preds = %bb124
  %tmp144 = icmp sgt i32 %tmp134, 0
  br i1 %tmp144, label %bb151, label %bb145

bb145:                                            ; preds = %bb143
  %tmp146 = bitcast i8* %tmp127 to %struct.arc**
  %tmp147 = bitcast i8* %tmp125 to %struct.arc**
  %tmp148 = tail call i32 @arc_compare(%struct.arc** %tmp146, %struct.arc** %tmp147)
  %tmp149 = icmp slt i32 %tmp148, 0
  %tmp150 = select i1 %tmp149, i8* %tmp127, i8* %tmp125
  br label %bb151

bb151:                                            ; preds = %bb145, %bb143, %bb137, %bb135, %bb28
  %tmp152 = phi i8* [ %tmp33, %bb28 ], [ %tmp142, %bb137 ], [ %tmp150, %bb145 ], [ %tmp126, %bb135 ], [ %tmp126, %bb143 ]
  %tmp153 = load i64, i64* @myglobal, align 8
  %tmp154 = load i8*, i8** @buffer, align 8
  %tmp155 = icmp ult i64 %tmp153, 7
  br i1 %tmp155, label %bb2, label %bb28

bb156:                                            ; preds = %bb12, %bb2
  ret void
}
