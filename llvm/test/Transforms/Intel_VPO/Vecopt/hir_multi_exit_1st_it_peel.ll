; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -hir-vec-dir-insert -allow-memory-speculation -vplan-force-vf=16 -VPlanDriverHIR -hir-cg -disable-output -print-after=VPlanDriverHIR < %s 2>&1 | FileCheck %s

; Verify that the first iteration of a multi-exit loop to be vectorized is properly
; peeled when -enable-first-it-peel-me-vec is used. -allow-memory-speculation enables
; the vectorization of the multi-exit loop.

; C code
; unsigned int peel_example(const unsigned int delta2, const unsigned int len_limit, const unsigned char *cur) {
;   unsigned int len_best;
;   for (len_best = 1; len_best != len_limit; ++len_best)
;     if (*(cur + len_best - delta2) != cur[len_best])
;       break;
; 
;   return len_best;
; }

; Input Loop:
; <19>      + DO i1 = 0, %len_limit + -2, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 4294967295>
; <9>       |   if ((%cur)[i1 + -1 * zext.i32.i64(%delta2) + 1] != (%cur)[i1 + 1])
; <9>       |   {
; <20>      |      %len_best.011.out = i1 + 1;
; <11>      |      goto for.end.loopexit;
; <9>       |   }
; <19>      + END LOOP
; <2>          %len_best.011.out = %len_limit + -1;

; Output:
; <0>       BEGIN REGION { modified }
; <25>            + DO i1 = 0, 0, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 4294967295>
; <26>            |   if ((%cur)[i1 + -1 * zext.i32.i64(%delta2) + 1] != (%cur)[i1 + 1])
; <26>            |   {
; <27>            |      %len_best.011.out = i1 + 1;
; <28>            |      goto for.end.loopexit;
; <26>            |   }
; <25>            + END LOOP
; <25>
; <29>            if (0 <u %len_limit + -2)
; <29>            {
; <30>               %tgu = (%len_limit + -2)/u16;
; <32>               if (0 <u 16 * %tgu)
; <32>               {
; <31>                  + DO i1 = 0, 16 * %tgu + -1, 16   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 268435455> <nounroll>
; <34>                  |   %wide.cmp. = (<16 x i32>*)(%cur)[i1 + -1 * zext.i32.i64(%delta2) + 2] != (<16 x i32>*)(%cur)[i1 + <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15> + 2];
; <35>                  |   %intmask = bitcast.<16 x i1>.i16(%wide.cmp.);
; <36>                  |   if (%intmask != 0)
; <36>                  |   {
; <37>                  |      %IfFPred = %wide.cmp.  ^  -1;
; <39>                  |      %intmask1 = bitcast.<16 x i1>.i16(%wide.cmp.);
; <40>                  |      %bsf = @llvm.cttz.i16(%intmask1,  0);
; <41>                  |      %cast = zext.i16.i32(%bsf);
; <38>                  |      %len_best.011.out = i1 + %cast + 2;
; <42>                  |      goto for.end.loopexit;
; <36>                  |   }
; <31>                  + END LOOP
; <32>               }
; <19>
; <19>               + DO i1 = 16 * %tgu, %len_limit + -3, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 15> <novectorize>
; <9>                |   if ((%cur)[i1 + -1 * zext.i32.i64(%delta2) + 2] != (%cur)[i1 + 2])
; <9>                |   {
; <20>               |      %len_best.011.out = i1 + 2;
; <11>               |      goto for.end.loopexit;
; <9>                |   }
; <19>               + END LOOP
; <29>            }
; <2>             %len_best.011.out = %len_limit + -1;
; <0>       END REGION


; Peeling loop
; CHECK: BEGIN REGION
; CHECK-NEXT: + DO i1 = 0, 0, 1

; Ztts
; CHECK: if (0 <u %len_limit + -2)
; CHECK: %tgu = (%len_limit + -2)/u16;
; CHECK: if (0 <u 16 * %tgu)

; Main Loop
; CHECK: + DO i1 = 0, 16 * %tgu + -1, 16

; Remainder Loop
; CHECK: + DO i1 = 16 * %tgu, %len_limit + -3, 1

define dso_local i32 @peel_example(i32 %delta2, i32 %len_limit, i8* nocapture readonly %cur) local_unnamed_addr #0 {
entry:
  %cmp12 = icmp eq i32 %len_limit, 1
  br i1 %cmp12, label %for.end, label %for.body.lr.ph

for.body.lr.ph:
  %idx.ext1 = zext i32 %delta2 to i64
  %idx.neg = sub nsw i64 0, %idx.ext1
  br label %for.body

for.body:
  %len_best.013 = phi i32 [ 1, %for.body.lr.ph ], [ %inc, %for.inc ]
  %idx.ext = zext i32 %len_best.013 to i64
  %add.ptr = getelementptr inbounds i8, i8* %cur, i64 %idx.ext
  %add.ptr2 = getelementptr inbounds i8, i8* %add.ptr, i64 %idx.neg
  %0 = load i8, i8* %add.ptr2
  %1 = load i8, i8* %add.ptr
  %cmp4 = icmp eq i8 %0, %1
  br i1 %cmp4, label %for.inc, label %for.end.loopexit

for.inc:
  %inc = add i32 %len_best.013, 1
  %cmp = icmp eq i32 %inc, %len_limit
  br i1 %cmp, label %for.end.loopexit, label %for.body

for.end.loopexit:
  %len_best.0.lcssa.ph = phi i32 [ %len_limit, %for.inc ], [ %len_best.013, %for.body ]
  br label %for.end

for.end:
  %len_best.0.lcssa = phi i32 [ 1, %entry ], [ %len_best.0.lcssa.ph, %for.end.loopexit ]
  ret i32 %len_best.0.lcssa
}
