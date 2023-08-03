; RUN: opt -intel-libirc-allowed -hir-non-perfect-nest-loop-blocking-stripmine-size=2 --passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-non-perfect-nest-loop-blocking,print<hir>" -disable-hir-non-perfect-nest-loop-blocking=false -disable-output 2>&1 < %s | FileCheck %s

; Verify that inter loop blocking works with switch and goto, where goto's target bb is unreachable.

; Before transformation

; CHECK: Function: ham
;
; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, %zext1130 + -1 * smin(0, %zext1130), 1   <DO_MULTI_EXIT_LOOP>
; CHECK:               |   %shl1134 = 1  <<  -1 * i1 + %zext1130;
; CHECK:               |   %and1135 = %shl1134  &  %arg;
; CHECK:               |   %select1137 = (%and1135 == 0) ? 0 : 2;
; CHECK:               |   %lshr1138 = %arg1  >>  -1 * i1 + %zext1130;
; CHECK:               |   switch(zext.i1.i32(trunc.i32.i1(%lshr1138)) + %select1137)
; CHECK:               |   {
; CHECK:               |   case 0:
; CHECK:               |         %shl1145 = 1  <<  -1 * i1 + %zext1130;
; CHECK:               |         %shl1147 = 1  <<  -1 * i1 + trunc.i64.i32(%zext1130) + %arg2;
; CHECK:               |      + DO i2 = 0, zext.i32.i64(%load963) + -1, 1   <DO_LOOP>
; CHECK:               |      |   %load1151 = (%load1124)[i2].1;
; CHECK:               |      |   %and1152 = %load1151  &  %shl1145;
; CHECK:               |      |   if (%and1152 != 0)
; CHECK:               |      |   {
; CHECK:               |      |      %xor1155 = %load1151  ^  %shl1147;
; CHECK:               |      |      (%load1124)[i2].1 = %xor1155;
; CHECK:               |      |   }
; CHECK:               |      + END LOOP
;                      |
;                      |
; CHECK:               |         %shl1161 = 1  <<  -1 * i1 + trunc.i64.i32(%zext1130) + %arg2 + 1;
; CHECK:               |         %freeze1162 = freeze(%shl1147);
; CHECK:               |         %or1163 = %shl1145  |  %freeze1162;
; CHECK:               |      + DO i2 = 0, zext.i32.i64(%load963) + -1, 1   <DO_LOOP>
; CHECK:               |      |   %load1167 = (%load1124)[i2].1;
; CHECK:               |      |   %and1168 = %load1167  &  %or1163;
; CHECK:               |      |   if (%and1168 == %or1163)
; CHECK:               |      |   {
; CHECK:               |      |      %xor1171 = %load1167  ^  %shl1161;
; CHECK:               |      |      (%load1124)[i2].1 = %xor1171;
; CHECK:               |      |   }
; CHECK:               |      + END LOOP
; CHECK:               |      break;
; CHECK:               |   case 3:
; CHECK:               |         %shl1177 = 1  <<  -1 * i1 + %zext1130;
; CHECK:               |         %shl1179 = 1  <<  -1 * i1 + trunc.i64.i32(%zext1130) + %arg2;
; CHECK:               |      + DO i2 = 0, zext.i32.i64(%load963) + -1, 1   <DO_LOOP>
; CHECK:               |      |   %load1183 = (%load1124)[i2].1;
; CHECK:               |      |   %and1184 = %load1183  &  %shl1177;
; CHECK:               |      |   if (%and1184 != 0)
; CHECK:               |      |   {
; CHECK:               |      |      %xor1187 = %load1183  ^  %shl1179;
; CHECK:               |      |      (%load1124)[i2].1 = %xor1187;
; CHECK:               |      |   }
; CHECK:               |      + END LOOP
; CHECK:               |      break;
; CHECK:               |   default:
; CHECK:               |      goto bb1432;
; CHECK:               |   }
; CHECK:               + END LOOP
; CHECK:         END REGION

; After transformation

; CHECK: Function: ham
;
; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, (-1 + zext.i32.i64(%load963)), 2   <DO_LOOP>
; CHECK:       |   %tile_e_min = (i1 + 1 <= (-1 + zext.i32.i64(%load963))) ? i1 + 1 : (-1 + zext.i32.i64(%load963));
;              |
; CHECK:       |   + DO i2 = 0, %zext1130 + -1 * smin(0, %zext1130), 1   <DO_MULTI_EXIT_LOOP>
;              |   |   %shl1134 = 1  <<  -1 * i2 + %zext1130;
;              |   |   %and1135 = %shl1134  &  %arg;
;              |   |   %select1137 = (%and1135 == 0) ? 0 : 2;
;              |   |   %lshr1138 = %arg1  >>  -1 * i2 + %zext1130;
;              |   |   switch(zext.i1.i32(trunc.i32.i1(%lshr1138)) + %select1137)
;              |   |   {
;              |   |   case 0:
; CHECK:       |   |      %lb_max = (0 <= i1) ? i1 : 0;
; CHECK:       |   |      %ub_min = (zext.i32.i64(%load963) + -1 <= %tile_e_min) ? zext.i32.i64(%load963) + -1 : %tile_e_min;
;              |   |
;              |   |         %shl1145 = 1  <<  -1 * i2 + %zext1130;
;              |   |         %shl1147 = 1  <<  -1 * i2 + trunc.i64.i32(%zext1130) + %arg2;
; CHECK:       |   |      + DO i3 = 0, -1 * %lb_max + %ub_min, 1   <DO_LOOP>
; CHECK:       |   |      |   %load1151 = (%load1124)[i3 + %lb_max].1;
;              |   |      |   %and1152 = %load1151  &  %shl1145;
;              |   |      |   if (%and1152 != 0)
;              |   |      |   {
;              |   |      |      %xor1155 = %load1151  ^  %shl1147;
; CHECK:       |   |      |      (%load1124)[i3 + %lb_max].1 = %xor1155;
;              |   |      |   }
;              |   |      + END LOOP
;              |   |
; CHECK:       |   |      %lb_max[[V4:[0-9]*]] = (0 <= i1) ? i1 : 0;
; CHECK:       |   |      %ub_min[[V5:[0-9]*]] = (zext.i32.i64(%load963) + -1 <= %tile_e_min) ? zext.i32.i64(%load963) + -1 : %tile_e_min;
;              |   |
;              |   |         %shl1161 = 1  <<  -1 * i2 + trunc.i64.i32(%zext1130) + %arg2 + 1;
;              |   |         %freeze1162 = freeze(%shl1147);
;              |   |         %or1163 = %shl1145  |  %freeze1162;
;              |   |      + DO i3 = 0, -1 * %lb_max[[V4]] + %ub_min[[V5]], 1   <DO_LOOP>
; CHECK:       |   |      |   %load1167 = (%load1124)[i3 + %lb_max[[V4]]].1;
;              |   |      |   %and1168 = %load1167  &  %or1163;
;              |   |      |   if (%and1168 == %or1163)
;              |   |      |   {
;              |   |      |      %xor1171 = %load1167  ^  %shl1161;
; CHECK:       |   |      |      (%load1124)[i3 + %lb_max[[V4]]].1 = %xor1171;
;              |   |      |   }
;              |   |      + END LOOP
;              |   |      break;
;              |   |   case 3:
; CHECK:       |   |      %lb_max[[V6:[0-9]*]] = (0 <= i1) ? i1 : 0;
; CHECK:       |   |      %ub_min[[V7:[0-9]*]] = (zext.i32.i64(%load963) + -1 <= %tile_e_min) ? zext.i32.i64(%load963) + -1 : %tile_e_min;
;              |   |
;              |   |         %shl1177 = 1  <<  -1 * i2 + %zext1130;
;              |   |         %shl1179 = 1  <<  -1 * i2 + trunc.i64.i32(%zext1130) + %arg2;
; CHECK:       |   |      + DO i3 = 0, -1 * %lb_max[[V6]] + %ub_min[[V7]], 1   <DO_LOOP>
; CHECK:       |   |      |   %load1183 = (%load1124)[i3 + %lb_max[[V6]]].1;
;              |   |      |   %and1184 = %load1183  &  %shl1177;
;              |   |      |   if (%and1184 != 0)
;              |   |      |   {
;              |   |      |      %xor1187 = %load1183  ^  %shl1179;
; CHECK:       |   |      |      (%load1124)[i3 + %lb_max[[V6]]].1 = %xor1187;
;              |   |      |   }
;              |   |      + END LOOP
;              |   |      break;
;              |   |   default:
;              |   |      goto bb1432;
;              |   |   }
;              |   + END LOOP
;              + END LOOP
;        END REGION

;
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.blam = type { i32, i32, i32, ptr, ptr }
%struct.wombat = type { { float, float }, i64 }

@global = external hidden unnamed_addr global i1, align 4
@global.1 = external hidden unnamed_addr global i1, align 4

; Function Attrs: noinline nounwind uwtable
define hidden fastcc void @ham(i32 noundef %arg, i32 noundef %arg1, i32 noundef %arg2, i64 noundef %shl1127, i64 noundef %zext1130, ptr %load1124, i64 noundef %or1129, i64 noundef %shl1122, i32 noundef %load963) unnamed_addr {
entry:
  br label %bb1131

bb1131:                                           ; preds = %bb1437, %bb1118
  %phi1132 = phi i64 [ %add1439, %bb1437 ], [ %zext1130, %entry]
  %trunc1133 = trunc i64 %phi1132 to i32
  %shl1134 = shl nuw i32 1, %trunc1133
  %and1135 = and i32 %shl1134, %arg
  %icmp1136 = icmp eq i32 %and1135, 0
  %select1137 = select i1 %icmp1136, i32 0, i32 2
  %lshr1138 = lshr i32 %arg1, %trunc1133
  %and1139 = and i32 %lshr1138, 1
  %or1140 = or i32 %select1137, %and1139
  %add1141 = add nsw i32 %trunc1133, %arg2
  %add1120 = add nuw nsw i32 %arg2, 1
  %add1142 = add nsw i32 %add1120, %trunc1133
  %icmp964 = icmp sgt i32 %load963, 0
  %zext1125 = zext i32 %load963 to i64
  switch i32 %or1140, label %bb1432 [
    i32 0, label %bb1143
    i32 3, label %bb1175
  ]

bb1143:                                           ; preds = %bb1131
  br i1 %icmp964, label %bb1144, label %bb1437

bb1144:                                           ; preds = %bb1143
  %shl1145 = shl nuw i64 1, %phi1132
  %zext1146 = zext i32 %add1141 to i64
  %shl1147 = shl nuw i64 1, %zext1146
  br label %bb1148

bb1148:                                           ; preds = %bb1156, %bb1144
  %phi1149 = phi i64 [ 0, %bb1144 ], [ %add1157, %bb1156 ]
  %getelementptr1150 = getelementptr inbounds %struct.wombat, ptr %load1124, i64 %phi1149, i32 1
  %load1151 = load i64, ptr %getelementptr1150, align 8
  %and1152 = and i64 %load1151, %shl1145
  %icmp1153 = icmp eq i64 %and1152, 0
  br i1 %icmp1153, label %bb1156, label %bb1154

bb1154:                                           ; preds = %bb1148
  %xor1155 = xor i64 %load1151, %shl1147
  store i64 %xor1155, ptr %getelementptr1150, align 8
  br label %bb1156

bb1156:                                           ; preds = %bb1154, %bb1148
  %add1157 = add nuw nsw i64 %phi1149, 1
  %icmp1158 = icmp eq i64 %add1157, %zext1125
  br i1 %icmp1158, label %bb1159, label %bb1148

bb1159:                                           ; preds = %bb1156
  %zext1160 = zext i32 %add1142 to i64
  %shl1161 = shl nuw i64 1, %zext1160
  %freeze1162 = freeze i64 %shl1147
  %or1163 = or i64 %shl1145, %freeze1162
  br label %bb1164

bb1164:                                           ; preds = %bb1172, %bb1159
  %phi1165 = phi i64 [ 0, %bb1159 ], [ %add1173, %bb1172 ]
  %getelementptr1166 = getelementptr inbounds %struct.wombat, ptr %load1124, i64 %phi1165, i32 1
  %load1167 = load i64, ptr %getelementptr1166, align 8
  %and1168 = and i64 %load1167, %or1163
  %icmp1169 = icmp eq i64 %and1168, %or1163
  br i1 %icmp1169, label %bb1170, label %bb1172

bb1170:                                           ; preds = %bb1164
  %xor1171 = xor i64 %load1167, %shl1161
  store i64 %xor1171, ptr %getelementptr1166, align 8
  br label %bb1172

bb1172:                                           ; preds = %bb1170, %bb1164
  %add1173 = add nuw nsw i64 %phi1165, 1
  %icmp1174 = icmp eq i64 %add1173, %zext1125
  br i1 %icmp1174, label %bb1433, label %bb1164

bb1175:                                           ; preds = %bb1131
  br i1 %icmp964, label %bb1176, label %bb1437

bb1176:                                           ; preds = %bb1175
  %shl1177 = shl nuw i64 1, %phi1132
  %zext1178 = zext i32 %add1141 to i64
  %shl1179 = shl nuw i64 1, %zext1178
  br label %bb1180

bb1180:                                           ; preds = %bb1188, %bb1176
  %phi1181 = phi i64 [ 0, %bb1176 ], [ %add1189, %bb1188 ]
  %getelementptr1182 = getelementptr inbounds %struct.wombat, ptr %load1124, i64 %phi1181, i32 1
  %load1183 = load i64, ptr %getelementptr1182, align 8
  %and1184 = and i64 %load1183, %shl1177
  %icmp1185 = icmp eq i64 %and1184, 0
  br i1 %icmp1185, label %bb1188, label %bb1186

bb1186:                                           ; preds = %bb1180
  %xor1187 = xor i64 %load1183, %shl1179
  store i64 %xor1187, ptr %getelementptr1182, align 8
  br label %bb1188

bb1188:                                           ; preds = %bb1186, %bb1180
  %add1189 = add nuw nsw i64 %phi1181, 1
  %icmp1190 = icmp eq i64 %add1189, %zext1125
  br i1 %icmp1190, label %bb1434, label %bb1180

bb1432:                                           ; preds = %bb1131
  unreachable

bb1433:                                           ; preds = %bb1172
  br label %bb1437

bb1434:                                           ; preds = %bb1229
  br label %bb1437

bb1435:                                           ; preds = %bb1321
  br label %bb1437

bb1436:                                           ; preds = %bb1425
  br label %bb1437

bb1437:                                           ; preds = %bb1436, %bb1435, %bb1434, %bb1433, %bb1324, %bb1232, %bb1175, %bb1143
  %icmp1438 = icmp sgt i64 %phi1132, 0
  %add1439 = add nsw i64 %phi1132, -1
  br i1 %icmp1438, label %bb1131, label %bb958

bb958:
  ret void
}
