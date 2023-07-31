; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -aa-pipeline="basic-aa" -xmain-opt-level=3 -disable-output <%s 2>&1 | FileCheck %s

; Test checks that HIR Opt Var Predicate cleans up %i1154 def and use after hoisting out 'if (i1 + -4 >= 1)'.
; We expect <66>-<69> nodes to be removed.

; <0>          BEGIN REGION { }
; <88>               + DO i1 = 0, 14, 1   <DO_LOOP>
; <4>                |   if (i1 + -4 > %i90)
; <4>                |   {
; <10>               |      %i1178 = (%arg19)[i1][%i83 + -1];
; <14>               |      %i1182 = (%arg21)[i1][%i83 + -1];
; <17>               |      if (%i1178 > 2.731600e+02 && %i1182 > 0.000000e+00)
; <17>               |      {
; <21>               |         %i1128 = %i1178  +  -2.731600e+02;
; <25>               |         %i1134 = %i1182  +  (%i133)[i1][%i83 + -1];
; <27>               |         %i1161 = %i1104;
; <28>               |         %i1162 = %i1134;
; <29>               |         if (i1 + -4 >= 1)
; <29>               |         {
; <31>               |            goto bb1157;
; <29>               |         }
; <35>               |         (%arg25)[i1][%i83 + -1] = 1;
; <36>               |         %i1154 = %i1134;
; <17>               |      }
; <17>               |      else
; <17>               |      {
; <45>               |         if (%i1178 < 2.731600e+02 && (%arg20)[i1][%i83 + -1] > 0.000000e+00)
; <45>               |         {
; <49>               |            %i1114 = %i1178  +  -2.731600e+02;
; <52>               |            %i1117 = (%i133)[i1][%i83 + -1];
; <54>               |            %i1161 = %i1104;
; <55>               |            %i1162 = %i1117;
; <56>               |            if (i1 + -4 >= 1)
; <56>               |            {
; <58>               |               goto bb1157;
; <56>               |            }
; <62>               |            (%arg25)[i1][%i83 + -1] = 2;
; <63>               |            %i1154 = %i1117;
; <45>               |         }
; <45>               |         else
; <45>               |         {
; <47>               |            goto bb1185;
; <45>               |         }
; <17>               |      }
; <66>               |      %i1156 = %i1104  +  %i1154;
; <67>               |      (%i981)[0] = %i1156;
; <68>               |      %i1161 = %i1156;
; <69>               |      %i1162 = %i1154;
; <71>               |      bb1157:
; <72>               |      %i1164 = %i1182  -  %i1162;
; <73>               |      (%arg21)[i1][%i83 + -1] = %i1164;
; <77>               |      %i1168 = (%arg20)[i1][%i83 + -1]  +  %i1162;
; <78>               |      (%arg20)[i1][%i83 + -1] = %i1168;
; <79>               |      %i1104 = %i1161;
; <4>                |   }
; <81>               |   bb1185:
; <88>               + END LOOP
; <0>          END REGION


; CHECK: BEGIN REGION { modified }

; CHECK:       + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:          %i1154 =
; CHECK:                 = %i1154;
; CHECK:       + END LOOP

; CHECK:       + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK-NOT:      %i1154
; CHECK:       + END LOOP

; CHECK: END REGION


define void @foo (ptr nonnull %arg19, ptr nonnull %arg20, ptr nonnull %arg21, ptr nonnull %arg25, double %i1025, ptr nonnull %i133, i64 %i83, i64 %i90, ptr %i981, ptr %i983) {
entry:
  br label %bb1102;

bb1102:                                           ; preds = %bb1185, %entry
  %i1104 = phi double [ %i1187, %bb1185 ], [ %i1025, %entry ]
  %i1105 = phi i64 [ %i1188, %bb1185 ], [ -4, %entry ]
  %i1106 = icmp sgt i64 %i1105, %i90
  br i1 %i1106, label %bb1175, label %bb1185

bb1107:                                           ; preds = %bb1127
  %i1108 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 -4, i64 4, ptr elementtype(i32) nonnull %arg25, i64 %i1105)
  %i1109 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i1108, i64 %i83)
  store i32 1, ptr %i1109, align 1
  br label %bb1150

bb1110:                                           ; preds = %bb1113
  %i1111 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 -4, i64 4, ptr elementtype(i32) nonnull %arg25, i64 %i1105)
  %i1112 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i1111, i64 %i83)
  store i32 2, ptr %i1112, align 1
  br label %bb1150

bb1113:                                           ; preds = %bb1139
  %i1114 = fadd fast double %i1178, -2.731600e+02
  %i1115 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 -4, i64 8, ptr elementtype(double) nonnull %i133, i64 %i1105)
  %i1116 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %i1115, i64 %i83)
  %i1117 = load double, ptr %i1116, align 1
  %i1126 = icmp slt i64 %i1105, 1
  br i1 %i1126, label %bb1110, label %bb1157

bb1127:                                           ; preds = %bb1175
  %i1128 = fadd fast double %i1178, -2.731600e+02
  %i1129 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 -4, i64 8, ptr elementtype(double) nonnull %i133, i64 %i1105)
  %i1130 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %i1129, i64 %i83)
  %i1131 = load double, ptr %i1130, align 1
  %i1134 = fadd fast double %i1182, %i1131
  %i1138 = icmp slt i64 %i1105, 1
  br i1 %i1138, label %bb1107, label %bb1157

bb1139:                                           ; preds = %bb1175
  %i1140 = fcmp fast olt double %i1178, 2.731600e+02
  %i1141 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 -4, i64 8, ptr elementtype(double) nonnull %arg20, i64 %i1105)
  %i1142 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %i1141, i64 %i83)
  %i1143 = load double, ptr %i1142, align 1
  %i1144 = fcmp fast ogt double %i1143, 0.000000e+00
  %i1145 = and i1 %i1140, %i1144
  br i1 %i1145, label %bb1113, label %bb1185

bb1150:                                           ; preds = %bb1110, %bb1107
  %i1154 = phi double [ %i1117, %bb1110 ], [ %i1134, %bb1107 ]
  %i1156 = fadd fast double %i1104, %i1154
  store double %i1156, ptr %i981, align 1
  br label %bb1157

bb1157:                                           ; preds = %bb1150, %bb1127, %bb1113
  %i1161 = phi double [ %i1104, %bb1113 ], [ %i1104, %bb1127 ], [ %i1156, %bb1150 ]
  %i1162 = phi double [ %i1117, %bb1113 ], [ %i1134, %bb1127 ], [ %i1154, %bb1150 ]
  %i1164 = fsub fast double %i1182, %i1162
  store double %i1164, ptr %i1181, align 1
  %i1165 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 -4, i64 8, ptr elementtype(double) nonnull %arg20, i64 %i1105)
  %i1166 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %i1165, i64 %i83)
  %i1167 = load double, ptr %i1166, align 1
  %i1168 = fadd fast double %i1167, %i1162
  store double %i1168, ptr %i1166, align 1
  br label %bb1185

bb1175:                                           ; preds = %bb1102
  %i1176 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 -4, i64 8, ptr elementtype(double) nonnull %arg19, i64 %i1105)
  %i1177 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %i1176, i64 %i83)
  %i1178 = load double, ptr %i1177, align 1
  %i1179 = fcmp fast ogt double %i1178, 2.731600e+02
  %i1180 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 -4, i64 8, ptr elementtype(double) nonnull %arg21, i64 %i1105)
  %i1181 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %i1180, i64 %i83)
  %i1182 = load double, ptr %i1181, align 1
  %i1183 = fcmp fast ogt double %i1182, 0.000000e+00
  %i1184 = and i1 %i1179, %i1183
  br i1 %i1184, label %bb1127, label %bb1139

bb1185:                                           ; preds = %bb1157, %bb1139, %bb1102
  %i1187 = phi double [ %i1161, %bb1157 ], [ %i1104, %bb1102 ], [ %i1104, %bb1139 ]
  %i1188 = add nsw i64 %i1105, 1
  %i1189 = icmp eq i64 %i1188, 11
  br i1 %i1189, label %bb1190, label %bb1102

bb1190:                                           ; preds = %bb1185
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)

; Function Attrs: nounwind readnone speculatable

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare double @llvm.minnum.f64(double, double)

