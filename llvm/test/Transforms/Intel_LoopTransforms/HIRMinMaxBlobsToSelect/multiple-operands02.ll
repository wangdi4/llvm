; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-runtime-dd,hir-lmm,hir-min-max-blob-to-select,print<hir>" -aa-pipeline="scoped-noalias-aa" -disable-output %s 2>&1 | FileCheck %s

; This test case checks that smax was converted into a Select instruction
; that represents a max and the blob that represents (5 + %1) was generated
; correctly.

; #include <algorithm>
; int mapped[1000];
;
; int  do_upper_or_lower(float *res, int  *maxchar, int n_res)
; {
;   int k = 0;
;   for (int j = 0; j < n_res; j++) {
;     *maxchar = std::max(*maxchar, mapped[j] + 5);
;     res[k++] += 1.0;
;   }
;   return k;
; }

; It requires to run the transformations HIRRuntimeDD and HIRLIMM before
; running HIRMinMaxBlobsToSelect in order to generate the following HIR:

;  BEGIN REGION { modified }
;        %mv.test = &((%maxchar)[0]) >=u &((i32*)(%res)[0]);
;        %mv.test2 = &((%res)[zext.i32.i64(%n_res) + -1]) >=u &((float*)(%maxchar)[0]);
;        %mv.and = %mv.test  &  %mv.test2;
;        %mv.test3 = &((%maxchar)[0]) >=u &((@mapped)[0][0]);
;        %mv.test4 = &((@mapped)[0][zext.i32.i64(%n_res) + -1]) >=u &((%maxchar)[0]);
;        %mv.and5 = %mv.test3  &  %mv.test4;
;        %mv.test6 = &((@mapped)[0][zext.i32.i64(%n_res) + -1]) >=u &((i32*)(%res)[0]);
;        %mv.test7 = &((%res)[zext.i32.i64(%n_res) + -1]) >=u &((float*)(@mapped)[0][0]);
;        %mv.and8 = %mv.test6  &  %mv.test7;
;        if (%mv.and == 0 && %mv.and5 == 0 && %mv.and8 == 0)
;        {
;              %limm = (%maxchar)[0];
;           + DO i1 = 0, zext.i32.i64(%n_res) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 18>
;           |   %0 = %limm;
;           |   %1 = (@mapped)[0][i1];
;           |   %limm = smax((5 + %1), %0);
;           |   %conv3 = (%res)[i1]  +  1.000000e+00;
;           |   (%res)[i1] = %conv3;
;           + END LOOP
;              (%maxchar)[0] = %limm;
;        }
;        else
;        {
;           + DO i1 = 0, zext.i32.i64(%n_res) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 18> <nounroll> <novectorize>
;           |   %0 = (%maxchar)[0];
;           |   %1 = (@mapped)[0][i1];
;           |   (%maxchar)[0] = smax((5 + %1), %0);
;           |   %conv3 = (%res)[i1]  +  1.000000e+00;
;           |   (%res)[i1] = %conv3;
;           + END LOOP
;        }
;  END REGION



; HIR after applying HIRMinMaxBlobsToSelect
;  BEGIN REGION { modified }
;        %mv.test = &((%maxchar)[0]) >=u &((i32*)(%res)[0]);
;        %mv.test2 = &((%res)[zext.i32.i64(%n_res) + -1]) >=u &((float*)(%maxchar)[0]);
;        %mv.and = %mv.test  &  %mv.test2;
;        %mv.test3 = &((%maxchar)[0]) >=u &((@mapped)[0][0]);
;        %mv.test4 = &((@mapped)[0][zext.i32.i64(%n_res) + -1]) >=u &((%maxchar)[0]);
;        %mv.and5 = %mv.test3  &  %mv.test4;
;        %mv.test6 = &((@mapped)[0][zext.i32.i64(%n_res) + -1]) >=u &((i32*)(%res)[0]);
;        %mv.test7 = &((%res)[zext.i32.i64(%n_res) + -1]) >=u &((float*)(@mapped)[0][0]);
;        %mv.and8 = %mv.test6  &  %mv.test7;
;        if (%mv.and == 0 && %mv.and5 == 0 && %mv.and8 == 0)
;        {
;              %limm = (%maxchar)[0];
;           + DO i1 = 0, zext.i32.i64(%n_res) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 18>
;           |   %1 = (@mapped)[0][i1];
;           |   %limm = (%limm <= (5 + %1)) ? (5 + %1) : %limm;
;           |   %conv3 = (%res)[i1]  +  1.000000e+00;
;           |   (%res)[i1] = %conv3;
;           + END LOOP
;              (%maxchar)[0] = %limm;
;        }
;        else
;        {
;           + DO i1 = 0, zext.i32.i64(%n_res) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 18> <nounroll> <novectorize>
;           |   %0 = (%maxchar)[0];
;           |   %1 = (@mapped)[0][i1];
;           |   (%maxchar)[0] = smax((5 + %1), %0);
;           |   %conv3 = (%res)[i1]  +  1.000000e+00;
;           |   (%res)[i1] = %conv3;
;           + END LOOP
;        }
;  END REGION

; CHECK:         %limm = (%maxchar)[0];
; CHECK:      + DO i1 = 0, zext.i32.i64(%n_res) + -1, 1
; CHECK:      |   %1 = (@mapped)[0][i1];
; CHECK:      |   %limm = (%limm <= (5 + %1)) ? (5 + %1) : %limm;
; CHECK:      |   %conv3 = (%res)[i1]  +  1.000000e+00;
; CHECK:      |   (%res)[i1] = %conv3;
; CHECK:      + END LOOP
; CHECK:         (%maxchar)[0] = %limm;


@mapped = dso_local global [1000 x i32] zeroinitializer, align 16

define dso_local noundef i32 @_Z17do_upper_or_lowerPfPii(ptr nocapture noundef %res, ptr nocapture noundef %maxchar, i32 noundef %n_res) {
entry:
  %cmp11 = icmp sgt i32 %n_res, 0
  br i1 %cmp11, label %for.body.preheader, label %for.cond.cleanup

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
  %0 = load i32, ptr %maxchar, align 4
  %1 = load i32, ptr %arrayidx, align 4
  %2 = add nuw nsw i32 %1, 5
  %3 = tail call i32 @llvm.smax.i32(i32 %0, i32 %2)
  store i32 %3, ptr %maxchar, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds float, ptr %res, i64 %indvars.iv
  %4 = load float, ptr %arrayidx2, align 4
  %conv3 = fadd fast float %4, 1.000000e+00
  store float %conv3, ptr %arrayidx2, align 4
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}

declare i32 @llvm.smax.i32(i32, i32)