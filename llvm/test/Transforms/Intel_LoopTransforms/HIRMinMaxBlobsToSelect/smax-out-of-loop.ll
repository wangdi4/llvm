; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-runtime-dd,hir-lmm,hir-min-max-blob-to-select,print<hir>" -aa-pipeline="scoped-noalias-aa" -hir-details -disable-output %s 2>&1 | FileCheck %s

; This test case checks that smax was converted into a Select instruction
; that represents a max. The goal is to check that the conversion was done
; correctly when the temp is defined outside of the loop. The test case was
; created from the following C++ test case, but with the IR modified

; #include <algorithm>
; int mapped[1000];
;
; int  do_upper_or_lower(float *res, int  *maxchar, int n_res)
; {
;   int k = 0;
;   int temp = mapped[j];
;   for (int j = 0; j < n_res; j++) {
;     *maxchar = std::max(*maxchar, temp);
;     res[k++] += 1.0;
;   }
;   return k;
; }

; It requires to run the transformations HIRRuntimeDD and HIRLIMM before
; running HIRMinMaxBlobsToSelect in order to generate the following HIR:

;   BEGIN REGION { modified }
;        %mv.test = &((%maxchar)[0]) >=u &((i32*)(%res)[0]);
;        %mv.test2 = &((%res)[zext.i32.i64(%n_res) + -1]) >=u &((float*)(%maxchar)[0]);
;        %mv.and = %mv.test  &  %mv.test2;
;        if (%mv.and == 0)
;        {
;              %limm = (%maxchar)[0];
;           + DO i1 = 0, zext.i32.i64(%n_res) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 15>
;           |   %1 = %limm;
;           |   %limm = smax(%0, %1);
;           |   %conv3 = (%res)[i1]  +  1.000000e+00;
;           |   (%res)[i1] = %conv3;
;           + END LOOP
;              (%maxchar)[0] = %limm;
;        }
;        else
;        {
;           + DO i1 = 0, zext.i32.i64(%n_res) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 15> <nounroll> <novectorize>
;           |   %1 = (%maxchar)[0];
;           |   (%maxchar)[0] = smax(%0, %1);
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
;        if (%mv.and == 0)
;        {
;              %limm = (%maxchar)[0];
;           + DO i1 = 0, zext.i32.i64(%n_res) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 15>
;           |   %limm = (%limm <= %0) ? %0 : %limm;
;           |   %conv3 = (%res)[i1]  +  1.000000e+00;
;           |   (%res)[i1] = %conv3;
;           + END LOOP
;              (%maxchar)[0] = %limm;
;        }
;        else
;        {
;           + DO i1 = 0, zext.i32.i64(%n_res) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 15> <nounroll> <novectorize>
;           |   %1 = (%maxchar)[0];
;           |   (%maxchar)[0] = smax(%0, %1);
;           |   %conv3 = (%res)[i1]  +  1.000000e+00;
;           |   (%res)[i1] = %conv3;
;           + END LOOP
;        }
;  END REGION

; We are going to check that the %0 in the new Select instruction is set as
; linear in the detailed print.

;          + DO i64 i1 = 0, zext.i32.i64(%n_res) + -1, 1
;          | <RVAL-REG> LINEAR i64 zext.i32.i64(%n_res) + -1
;          |    <BLOB> LINEAR i32 %n_res
;          |
; CHECK:   |   %limm = (%limm <= %0) ? %0 : %limm;
; CHECK:   |   <LVAL-REG> NON-LINEAR i32 %limm
; CHECK:   |   <RVAL-REG> NON-LINEAR i32 %limm
; CHECK:   |   <RVAL-REG> LINEAR i32 %0
; CHECK:   |   <RVAL-REG> LINEAR i32 %0
; CHECK:   |   <RVAL-REG> NON-LINEAR i32 %limm
;          |
;          |   %conv3 = (%res)[i1]  +  1.000000e+00; <fast>
;          |   <LVAL-REG> NON-LINEAR float %conv3
;          |   <RVAL-REG> {al:4}(LINEAR float* %res)[LINEAR i64 i1] inbounds
;          |      <BLOB> LINEAR float* %res
;          |
;          |   (%res)[i1] = %conv3;
;          |   <LVAL-REG> {al:4}(LINEAR float* %res)[LINEAR i64 i1] inbounds
;          |      <BLOB> LINEAR float* %res
;          |   <RVAL-REG> NON-LINEAR float %conv3
;          |
;          + END LOOP
;             (%maxchar)[0] = %limm;
;             <LVAL-REG> {al:4}(LINEAR i32* %maxchar)[i64 0]
;                <BLOB> LINEAR i32* %maxchar
;             <RVAL-REG> NON-LINEAR i32 %limm


@mapped = dso_local global [1000 x i32] zeroinitializer, align 16

define dso_local noundef i32 @_Z17do_upper_or_lowerPfPii(ptr nocapture noundef %res, ptr nocapture noundef %maxchar, i32 noundef %n_res) {
entry:
  %cmp11 = icmp sgt i32 %n_res, 0
  %0 = load i32, ptr @mapped, align 4
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
  %1 = load i32, ptr %maxchar, align 4
  %2 = tail call i32 @llvm.smax.i32(i32 %1, i32 %0)
  store i32 %2, ptr %maxchar, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds float, ptr %res, i64 %indvars.iv
  %3 = load float, ptr %arrayidx2, align 4
  %conv3 = fadd fast float %3, 1.000000e+00
  store float %conv3, ptr %arrayidx2, align 4
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}

declare i32 @llvm.smax.i32(i32, i32)