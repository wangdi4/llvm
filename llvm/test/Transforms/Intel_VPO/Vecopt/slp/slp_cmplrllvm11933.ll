; RUN: opt < %s -passes=slp-vectorizer -mtriple=x86_64 -mattr=+avx2 -S | FileCheck %s

; Test originated from blender SPEC (routine rayobject_bb_intersect_test).
; After doing reassosiation in order to employ FMA instructions
; code before SLP looks as an equivalent to this one:
;
;  float sox = isec->start[0] * isec->idot_axis[0];
;  float soy = isec->start[1] * isec->idot_axis[1];
;  float soz = isec->start[2] * isec->idot_axis[2];
;  // FMA patterns
;  float t2z = bb[isec->bv_index[5]] * isec->idot_axis[2] - soz;
;  float t2x = bb[isec->bv_index[1]] * isec->idot_axis[0] - sox;
;  float t2y = bb[isec->bv_index[3]] * isec->idot_axis[1] - soy;
;  float t1z = bb[isec->bv_index[4]] * isec->idot_axis[2] - soz;
;  float t1x = bb[isec->bv_index[0]] * isec->idot_axis[0] - sox;
;  float t1y = bb[isec->bv_index[2]] * isec->idot_axis[1] - soy;
;
;  if (t1x > t2y || t2x < t1y || t1x > t2z || t2x < t1z || t1y > t2z ||
;      t2y < t1z)
;    return 0;
;
; SLP vectorizer starts here from if condition with matching "any-of"
; reduction and have potentially six candidates (operands od reduced values).
; It vectorizes with VF=2 and which instructions (lanes) turn out paired into
; a vector is simply given to a chance. In this case it ended up with pairing
; those subexpressions we do not actually want to (i.e. operands of different
; comparisons) as that effectively glues data dependencies and as a result
; restricted code motion (sinking) optimization that is done on machine IR.
; SLP here have to vectorize only specific pairs: t2y,t1z and t1y,t2z.

%struct.Isect = type { [3 x float], [3 x float], float, [3 x float], [3 x float], [6 x i32], [3 x float], i32, i32, i32, i32, ptr, float, float, i32, %struct.anon, %struct.anon }
%struct.anon = type { ptr, ptr }

define i32 @rayobject_bb_intersect_test(ptr nocapture readonly %isec, ptr nocapture readonly %_bb) {
; CHECK-LABEL: define i32 @rayobject_bb_intersect_test
; CHECK-SAME: (ptr nocapture readonly [[ISEC:%.*]], ptr nocapture readonly [[_BB:%.*]]) #[[ATTR0:[0-9]+]] {
; CHECK-NEXT:  entry:
entry:

  %0 = load float, ptr %isec, align 8
  %arrayidx1 = getelementptr inbounds %struct.Isect, ptr %isec, i64 0, i32 6, i64 0
  %1 = load float, ptr %arrayidx1, align 4
  %mul = fmul fast float %1, %0
  %arrayidx3 = getelementptr inbounds %struct.Isect, ptr %isec, i64 0, i32 0, i64 1
  %2 = load float, ptr %arrayidx3, align 4
  %arrayidx5 = getelementptr inbounds %struct.Isect, ptr %isec, i64 0, i32 6, i64 1

; CHECK:         [[ARRAYIDX3:%.*]] = getelementptr inbounds [[STRUCT_ISECT:%.*]], ptr [[ISEC]], i64 0, i32 0, i64 1
; CHECK-NEXT:    [[ARRAYIDX5:%.*]] = getelementptr inbounds [[STRUCT_ISECT]], ptr [[ISEC]], i64 0, i32 6, i64 1
; CHECK:         [[TMP9:%.*]] = load float, ptr %arrayidx36, align 4
; CHECK-NEXT:    [[TMP10:%.*]] = load <2 x float>, ptr [[ARRAYIDX3]], align 4
; CHECK-NEXT:    [[TMP11:%.*]] = load <2 x float>, ptr [[ARRAYIDX5]], align 4
; CHECK-NEXT:    [[TMP12:%.*]] = fmul fast <2 x float> [[TMP11]], [[TMP10]]

  %3 = load float, ptr %arrayidx5, align 4
  %mul6 = fmul fast float %3, %2
  %arrayidx8 = getelementptr inbounds %struct.Isect, ptr %isec, i64 0, i32 0, i64 2
  %4 = load float, ptr %arrayidx8, align 8
  %arrayidx10 = getelementptr inbounds %struct.Isect, ptr %isec, i64 0, i32 6, i64 2
  %5 = load float, ptr %arrayidx10, align 4
  %mul11 = fmul fast float %5, %4
  %arrayidx12 = getelementptr inbounds %struct.Isect, ptr %isec, i64 0, i32 5, i64 5
  %6 = load i32, ptr %arrayidx12, align 4
  %idxprom = sext i32 %6 to i64
  %arrayidx13 = getelementptr inbounds float, ptr %_bb, i64 %idxprom
  %7 = load float, ptr %arrayidx13, align 4
  %mul16 = fmul fast float %7, %5
  %sub = fsub fast float %mul16, %mul11
  %arrayidx18 = getelementptr inbounds %struct.Isect, ptr %isec, i64 0, i32 5, i64 1
  %8 = load i32, ptr %arrayidx18, align 4
  %idxprom19 = sext i32 %8 to i64
  %arrayidx20 = getelementptr inbounds float, ptr %_bb, i64 %idxprom19
  %9 = load float, ptr %arrayidx20, align 4
  %mul23 = fmul fast float %9, %1
  %sub24 = fsub fast float %mul23, %mul
  %arrayidx26 = getelementptr inbounds %struct.Isect, ptr %isec, i64 0, i32 5, i64 3
  %10 = load i32, ptr %arrayidx26, align 4
  %idxprom27 = sext i32 %10 to i64
  %arrayidx28 = getelementptr inbounds float, ptr %_bb, i64 %idxprom27
  %11 = load float, ptr %arrayidx28, align 4
  %mul31 = fmul fast float %11, %3
  %sub32 = fsub fast float %mul31, %mul6
  %arrayidx34 = getelementptr inbounds %struct.Isect, ptr %isec, i64 0, i32 5, i64 4
  %12 = load i32, ptr %arrayidx34, align 4
  %idxprom35 = sext i32 %12 to i64
  %arrayidx36 = getelementptr inbounds float, ptr %_bb, i64 %idxprom35

; CHECK:         [[TMP13:%.*]] = insertelement <2 x float> poison, float [[TMP7:%.*]], i32 0
; CHECK-NEXT:    [[TMP14:%.*]] = insertelement <2 x float> [[TMP13]], float [[TMP9]], i32 1
; CHECK-NEXT:    [[TMP15:%.*]] = fmul fast <2 x float> [[TMP14]], [[TMP11]]
; CHECK-NEXT:    [[TMP16:%.*]] = fsub fast <2 x float> [[TMP15]], [[TMP12]]

  %13 = load float, ptr %arrayidx36, align 4
  %mul39 = fmul fast float %13, %5
  %sub40 = fsub fast float %mul39, %mul11
  %arrayidx42 = getelementptr inbounds %struct.Isect, ptr %isec, i64 0, i32 5, i64 0
  %14 = load i32, ptr %arrayidx42, align 4
  %idxprom43 = sext i32 %14 to i64
  %arrayidx44 = getelementptr inbounds float, ptr %_bb, i64 %idxprom43
  %15 = load float, ptr %arrayidx44, align 4
  %mul47 = fmul fast float %15, %1
  %sub48 = fsub fast float %mul47, %mul
  %arrayidx50 = getelementptr inbounds %struct.Isect, ptr %isec, i64 0, i32 5, i64 2
  %16 = load i32, ptr %arrayidx50, align 4
  %idxprom51 = sext i32 %16 to i64
  %arrayidx52 = getelementptr inbounds float, ptr %_bb, i64 %idxprom51

; CHECK:         [[TMP20:%.*]] = load float, ptr %arrayidx52, align 4
; CHECK-NEXT:    [[TMP21:%.*]] = insertelement <2 x float> poison, float [[TMP20]], i32 0
; CHECK-NEXT:    [[TMP22:%.*]] = insertelement <2 x float> [[TMP21]], float [[TMP3:%.*]], i32 1
; CHECK-NEXT:    [[TMP23:%.*]] = fmul fast <2 x float> [[TMP22]], [[TMP11]]
; CHECK-NEXT:    [[TMP24:%.*]] = fsub fast <2 x float> [[TMP23]], [[TMP12]]
; CHECK-NEXT:    [[TMP25:%.*]] = extractelement <2 x float> [[TMP16]], i32 0
; CHECK-NEXT:    [[TMP26:%.*]] = extractelement <2 x float> [[TMP24]], i32 1
; CHECK-NEXT:    [[CMP:%.*]] = fcmp fast ole float [[TMP25]], [[TMP26]]

  %17 = load float, ptr %arrayidx52, align 4
  %mul55 = fmul fast float %17, %3
  %sub56 = fsub fast float %mul55, %mul6
  %cmp = fcmp fast ole float %sub32, %sub
  %18 = select i1 %cmp, float %sub32, float %sub
  %cmp59 = fcmp fast ogt float %sub48, %18

; CHECK:         [[TMP28:%.*]] = extractelement <2 x float> [[TMP16]], i32 1
; CHECK-NEXT:    [[TMP29:%.*]] = extractelement <2 x float> [[TMP24]], i32 0
; CHECK-NEXT:    [[CMP61:%.*]] = fcmp fast oge float [[TMP29]], [[TMP28]]
%cmp61 = fcmp fast oge float %sub56, %sub40
  %19 = select i1 %cmp61, float %sub56, float %sub40
  %cmp57 = fcmp fast olt float %sub24, %19
  %or.cond143 = or i1 %cmp59, %cmp57

; Verify that both operands of cmp are extracts from the same vector
; CHECK:         [[CMP63:%.*]] = fcmp fast ogt float [[TMP29]], [[TMP26]]
  %cmp63 = fcmp fast ogt float %sub56, %sub
  %or.cond144 = or i1 %cmp63, %or.cond143

; Verify that both operands of cmp are extracts from the same vector
; CHECK:         [[CMP65:%.*]] = fcmp fast olt float [[TMP25]], [[TMP28]]
  %cmp65 = fcmp fast olt float %sub32, %sub40
  %or.cond145 = or i1 %cmp65, %or.cond144
  br i1 %or.cond145, label %cleanup, label %if.end

; CHECK:       if.end:

if.end:                                           ; preds = %entry
  %cmp66 = fcmp fast ole float %sub24, %sub32
  %20 = select i1 %cmp66, float %sub24, float %sub32
  %cmp68 = fcmp fast ole float %20, %sub
  %21 = select i1 %cmp68, float %20, float %sub
  %cmp70 = fcmp fast olt float %21, 0.000000e+00
  br i1 %cmp70, label %cleanup, label %if.end72

if.end72:                                         ; preds = %if.end
  %dist = getelementptr inbounds %struct.Isect, ptr %isec, i64 0, i32 2
  %22 = load float, ptr %dist, align 8
  %cmp73 = fcmp fast oge float %sub48, %sub56
  %23 = select i1 %cmp73, float %sub48, float %sub56
  %cmp76 = fcmp fast oge float %23, %sub40
  %24 = select i1 %cmp76, float %23, float %sub40
  %cmp79 = fcmp fast uge float %22, %24
  %25 = zext i1 %cmp79 to i32
  br label %cleanup

cleanup:                                          ; preds = %if.end72, %if.end, %entry
  %retval.0 = phi i32 [ 0, %entry ], [ 0, %if.end ], [ %25, %if.end72 ]
  ret i32 %retval.0
}
