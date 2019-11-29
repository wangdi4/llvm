; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -hir-vec-dir-insert -allow-memory-speculation -enable-first-it-peel-me-vec=false -VPlanDriverHIR -disable-output -print-after=VPlanDriverHIR < %s 2>&1 | FileCheck %s

; Verify that we properly generate code for non-linear live-outs when
; vectorizing a multi-exit loop.

; C code
; unsigned int liveout_example(const unsigned int len_limit,
;                              const unsigned char *pb,
;                              const unsigned char *cur) {
;   unsigned int len = 0;
;   unsigned char t;
;
;   while (++len != len_limit) {
;     t = pb[len];
;     if (t != cur[len])
;        break;
;   }
;
;   return t;
; }

; Input Loop:
;            + DO i1 = 0, %len_limit + -2, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 4294967295>
;            |   %0 = (%pb)[i1 + 1];
;            |   if (%0 != (%cur)[i1 + 1])
;            |   {
;            |      goto while.end.loopexit;
;            |   }
;            + END LOOP

; Output:
;      BEGIN REGION { modified }
;            %tgu = (%len_limit + -1)/u2;
;            if (0 <u 2 * %tgu)
;            {
;               + DO i1 = 0, 2 * %tgu + -1, 2   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647> <nounroll>
;               |   %.vec = (<2 x i8>*)(%pb)[i1 + 1];
;               |   %wide.cmp. = %.vec != (<2 x i8>*)(%cur)[i1 + 1];
;               |   %intmask = bitcast.<2 x i1>.i2(%wide.cmp.);
;               |   if (%intmask != 0)
;               |   {
;               |      %IfFPred = %wide.cmp.  ^  -1;
;               |      %bsfintmask = bitcast.<2 x i1>.i2(%wide.cmp.);
;               |      %bsf = @llvm.cttz.i2(%bsfintmask,  0);
;               |      %cast = zext.i2.i32(%bsf);
;               |      %0 = (%pb)[i1 + %cast + 1];
;               |      goto while.end.loopexit;
;               |   }
;               + END LOOP
;
;               %0 = extractelement %.vec,  1;
;            }
;
;            + DO i1 = 2 * %tgu, %len_limit + -2, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 1> <novectorize>
;            |   %0 = (%pb)[i1 + 1];
;            |   if (%0 != (%cur)[i1 + 1])
;            |   {
;            |      goto while.end.loopexit;
;            |   }
;            + END LOOP
;      END REGION

; CHECK: DO i1 = {{.*}}, 32
; CHECK: if (%intmask != 0)
; CHECK: %bsf = @llvm.cttz.i32(%bsfintmask,  0);
; CHECK-NEXT: %{{[0-9]+}} = (%pb)[i1 + %bsf + 1];
; CHECK-NEXT: goto while.end.loopexit;

target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @liveout_example(i32 %len_limit, i8* nocapture readonly %pb, i8* nocapture readonly %cur) local_unnamed_addr #0 {
entry:
  %cmp11 = icmp eq i32 %len_limit, 1
  br i1 %cmp11, label %while.end, label %while.body.preheader

while.body.preheader:
  br label %while.body

while.cond:
  %inc = add i32 %inc12, 1
  %cmp = icmp eq i32 %inc, %len_limit
  %inc12.in = bitcast i32 %inc to i32
  br i1 %cmp, label %while.end.loopexit, label %while.body

while.body:
  %inc12 = phi i32 [ %inc, %while.cond ], [ 1, %while.body.preheader ]
  %idxprom = zext i32 %inc12 to i64
  %idx = getelementptr inbounds i8, i8* %pb, i64 %idxprom
  %0 = load i8, i8* %idx
  %idx2 = getelementptr inbounds i8, i8* %cur, i64 %idxprom
  %1 = load i8, i8* %idx2
  %cmp4 = icmp eq i8 %0, %1
  br i1 %cmp4, label %while.cond, label %while.end.loopexit

while.end.loopexit:
  %.lcssa = phi i8 [ %0, %while.body ], [ %0, %while.cond ]
  %phitmp = zext i8 %.lcssa to i32
  br label %while.end

while.end:
  %t.1 = phi i32 [ 0, %entry ], [ %phitmp, %while.end.loopexit ]
  ret i32 %t.1
}

attributes #0 = { "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3" }
