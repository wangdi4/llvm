; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-runtime-dd,hir-lmm,hir-min-max-blob-to-select,print<hir>" -aa-pipeline="scoped-noalias-aa" -debug-only=hir-min-max-blob-to-select -disable-output %s 2>&1 | FileCheck %s

; This test case checks that smax and smin were converted into a Select instruction
; that represents a max and min respectively. The goal of this test case is to
; check that the transformation was applied for multiple instructions within
; the same loop. Also, this test case checks that the debug information was printed
; correctly. The test case was created from the following C++ test case:

; #include <algorithm>
; int mapped[1000];
;
; int  do_upper_or_lower(float *res, int *maxchar, int *minchar, int n_res)
; {
;   int k = 0;
;   for (int j = 0; j < n_res; j++) {
;     *maxchar = std::max(*maxchar, mapped[j]);
;     *minchar = std::min(*minchar, mapped[j]);
;     res[k++] += 1.0;
;   }
;   return k;
; }

; It requires to run the transformations HIRRuntimeDD and HIRLIMM before
; running HIRMinMaxBlobsToSelect in order to generate the following HIR:

;  BEGIN REGION { modified }
;        %mv.test = &((%maxchar)[0]) >=u &((%minchar)[0]);
;        %mv.test4 = &((%minchar)[0]) >=u &((%maxchar)[0]);
;        %mv.and = %mv.test  &  %mv.test4;
;        %mv.test5 = &((%maxchar)[0]) >=u &((i32*)(%res)[0]);
;        %mv.test6 = &((%res)[zext.i32.i64(%n_res) + -1]) >=u &((float*)(%maxchar)[0]);
;        %mv.and7 = %mv.test5  &  %mv.test6;
;        %mv.test8 = &((%maxchar)[0]) >=u &((@mapped)[0][0]);
;        %mv.test9 = &((@mapped)[0][zext.i32.i64(%n_res) + -1]) >=u &((%maxchar)[0]);
;        %mv.and10 = %mv.test8  &  %mv.test9;
;        %mv.test11 = &((@mapped)[0][zext.i32.i64(%n_res) + -1]) >=u &((%minchar)[0]);
;        %mv.test12 = &((%minchar)[0]) >=u &((@mapped)[0][0]);
;        %mv.and13 = %mv.test11  &  %mv.test12;
;        %mv.test14 = &((@mapped)[0][zext.i32.i64(%n_res) + -1]) >=u &((i32*)(%res)[0]);
;        %mv.test15 = &((%res)[zext.i32.i64(%n_res) + -1]) >=u &((float*)(@mapped)[0][0]);
;        %mv.and16 = %mv.test14  &  %mv.test15;
;        %mv.test17 = &((%minchar)[0]) >=u &((i32*)(%res)[0]);
;        %mv.test18 = &((%res)[zext.i32.i64(%n_res) + -1]) >=u &((float*)(%minchar)[0]);
;        %mv.and19 = %mv.test17  &  %mv.test18;
;        if (%mv.and == 0 && %mv.and7 == 0 && %mv.and10 == 0 && %mv.and13 == 0 && %mv.and16 == 0 && %mv.and19 == 0)
;        {
;              %limm = (%maxchar)[0];
;              %limm21 = (%minchar)[0];
;           + DO i1 = 0, zext.i32.i64(%n_res) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 23>
;           |   %0 = %limm;
;           |   %1 = (@mapped)[0][i1];
;           |   %limm = smax(%0, %1);
;           |   %3 = (@mapped)[0][i1];
;           |   %4 = %limm21;
;           |   %limm21 = smin(%4, %3);
;           |   %conv6 = (%res)[i1]  +  1.000000e+00;
;           |   (%res)[i1] = %conv6;
;           + END LOOP
;              (%minchar)[0] = %limm21;
;              (%maxchar)[0] = %limm;
;        }
;        else
;        {
;           + DO i1 = 0, zext.i32.i64(%n_res) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 23> <nounroll> <novectorize>
;           |   %0 = (%maxchar)[0];
;           |   %1 = (@mapped)[0][i1];
;           |   (%maxchar)[0] = smax(%0, %1);
;           |   %3 = (@mapped)[0][i1];
;           |   %4 = (%minchar)[0];
;           |   (%minchar)[0] = smin(%4, %3);
;           |   %conv6 = (%res)[i1]  +  1.000000e+00;
;           |   (%res)[i1] = %conv6;
;           + END LOOP
;        }
;  END REGION

; HIR after applying HIRMinMaxBlobsToSelect

;   BEGIN REGION { modified }
;         %mv.test = &((%maxchar)[0]) >=u &((%minchar)[0]);
;         %mv.test4 = &((%minchar)[0]) >=u &((%maxchar)[0]);
;         %mv.and = %mv.test  &  %mv.test4;
;         %mv.test5 = &((%maxchar)[0]) >=u &((i32*)(%res)[0]);
;         %mv.test6 = &((%res)[zext.i32.i64(%n_res) + -1]) >=u &((float*)(%maxchar)[0]);
;         %mv.and7 = %mv.test5  &  %mv.test6;
;         %mv.test8 = &((%maxchar)[0]) >=u &((@mapped)[0][0]);
;         %mv.test9 = &((@mapped)[0][zext.i32.i64(%n_res) + -1]) >=u &((%maxchar)[0]);
;         %mv.and10 = %mv.test8  &  %mv.test9;
;         %mv.test11 = &((@mapped)[0][zext.i32.i64(%n_res) + -1]) >=u &((%minchar)[0]);
;         %mv.test12 = &((%minchar)[0]) >=u &((@mapped)[0][0]);
;         %mv.and13 = %mv.test11  &  %mv.test12;
;         %mv.test14 = &((@mapped)[0][zext.i32.i64(%n_res) + -1]) >=u &((i32*)(%res)[0]);
;         %mv.test15 = &((%res)[zext.i32.i64(%n_res) + -1]) >=u &((float*)(@mapped)[0][0]);
;         %mv.and16 = %mv.test14  &  %mv.test15;
;         %mv.test17 = &((%minchar)[0]) >=u &((i32*)(%res)[0]);
;         %mv.test18 = &((%res)[zext.i32.i64(%n_res) + -1]) >=u &((float*)(%minchar)[0]);
;         %mv.and19 = %mv.test17  &  %mv.test18;
;         if (%mv.and == 0 && %mv.and7 == 0 && %mv.and10 == 0 && %mv.and13 == 0 && %mv.and16 == 0 && %mv.and19 == 0)
;         {
;               %limm = (%maxchar)[0];
;               %limm21 = (%minchar)[0];
;            + DO i1 = 0, zext.i32.i64(%n_res) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 23>
;            |   %1 = (@mapped)[0][i1];
;            |   %limm = (%limm <= %1) ? %1 : %limm;
;            |   %3 = (@mapped)[0][i1];
;            |   %limm21 = (%limm21 <= %3) ? %limm21 : %3;
;            |   %conv6 = (%res)[i1]  +  1.000000e+00;
;            |   (%res)[i1] = %conv6;
;            + END LOOP
;               (%minchar)[0] = %limm21;
;               (%maxchar)[0] = %limm;
;         }
;         else
;         {
;            + DO i1 = 0, zext.i32.i64(%n_res) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 23> <nounroll> <novectorize>
;            |   %0 = (%maxchar)[0];
;            |   %1 = (@mapped)[0][i1];
;            |   (%maxchar)[0] = smax(%0, %1);
;            |   %3 = (@mapped)[0][i1];
;            |   %4 = (%minchar)[0];
;            |   (%minchar)[0] = smin(%4, %3);
;            |   %conv6 = (%res)[i1]  +  1.000000e+00;
;            |   (%res)[i1] = %conv6;
;            + END LOOP
;         }
;   END REGION

; Check the debug information

; CHECK: Loop: 23
; CHECK:   Min/Max instruction: <55>         %limm = smax(%0, %1);
; CHECK:   Cycles with: <54>         %0 = %limm;
; CHECK:   New Min/Max instruction: <60>         %limm = (%limm <= %1) ? %1 : %limm;

; CHECK:   Min/Max instruction: <59>         %limm21 = smin(%4, %3);
; CHECK:   Cycles with: <58>         %4 = %limm21;
; CHECK:   New Min/Max instruction: <61>         %limm21 = (%limm21 <= %3) ? %limm21 : %3;

; Check the HIR

; CHECK:              %limm = (%maxchar)[0];
; CHECK:              %limm21 = (%minchar)[0];
; CHECK:           + DO i1 = 0, zext.i32.i64(%n_res) + -1, 1
; CHECK:           |   %1 = (@mapped)[0][i1];
; CHECK:           |   %limm = (%limm <= %1) ? %1 : %limm;
; CHECK:           |   %3 = (@mapped)[0][i1];
; CHECK:           |   %limm21 = (%limm21 <= %3) ? %limm21 : %3;
; CHECK:           |   %conv6 = (%res)[i1]  +  1.000000e+00;
; CHECK:           |   (%res)[i1] = %conv6;
; CHECK:           + END LOOP
; CHECK:              (%minchar)[0] = %limm21;
; CHECK:              (%maxchar)[0] = %limm;

@mapped = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer


define dso_local noundef i32 @_Z17do_upper_or_lowerPfPiS0_i(ptr nocapture noundef %res, ptr nocapture noundef %maxchar, ptr nocapture noundef %minchar, i32 noundef %n_res) {
entry:
  %cmp18 = icmp sgt i32 %n_res, 0
  br i1 %cmp18, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n_res to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %k.0.lcssa = phi i32 [ 0, %entry ], [ %n_res, %for.cond.cleanup.loopexit ]
  ret i32 %k.0.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @mapped, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %maxchar
  %1 = load i32, ptr %arrayidx
  %cmp.i = icmp slt i32 %0, %1
  %2 = select i1 %cmp.i, i32 %1, i32 %0
  store i32 %2, ptr %maxchar
  %3 = load i32, ptr %arrayidx
  %4 = load i32, ptr %minchar
  %cmp.i16 = icmp slt i32 %3, %4
  %5 = select i1 %cmp.i16, i32 %3, i32 %4
  store i32 %5, ptr %minchar
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds float, ptr %res, i64 %indvars.iv
  %6 = load float, ptr %arrayidx5
  %conv6 = fadd fast float %6, 1.000000e+00
  store float %conv6, ptr %arrayidx5
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}

