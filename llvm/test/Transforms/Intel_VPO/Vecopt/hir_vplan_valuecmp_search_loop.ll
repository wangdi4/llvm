; Test the ValueCmp search loop idiom.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup \
; RUN:     -hir-last-value-computation -hir-vec-dir-insert -hir-vplan-vec \
; RUN:     -print-after=hir-vplan-vec -hir-details-no-verbose-indent \
; RUN:     -disable-output < %s 2>&1 | FileCheck %s

; The idiom being recognized looks like:

;  BEGIN REGION { modified }
;       %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;       + DO i1 = 0, 8191, 1   <DO_MULTI_EXIT_LOOP>
;       |   if ((@b)[0][i1] > %v)
;       |   {
;       |      %indvars.iv.out = i1;
;       |      goto for.end.split.loop.exit8;
;       |   }
;       + END LOOP
;
;       @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;  END REGION


@b = global [8192 x i32] zeroinitializer, align 16

define i32 @_Z6searchi(i32 %v) #0 {
; CHECK-LABEL:  *** IR Dump After vpo::VPlanDriverHIRPass ***
; CHECK-NEXT:  Function: _Z6searchi
; CHECK-EMPTY:
; CHECK-NEXT:  BEGIN REGION { modified }
; CHECK-NEXT:        %arr.base.cast = ptrtoint.i32*.i64(&((@b)[0][0]));
; CHECK-NEXT:        %alignment = %arr.base.cast  &  127;
; CHECK-NEXT:        %peel.factor = 128  -  %alignment;
; CHECK-NEXT:        %peel.factor1 = %peel.factor  >>  2;
; CHECK-NEXT:        %peel.factor1 = (8192 <= %peel.factor1) ? 8192 : %peel.factor1;
; CHECK-NEXT:        if (%peel.factor1 != 0)
; CHECK-NEXT:        {
; CHECK-NEXT:           + DO i1 = 0, %peel.factor1 + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 127>
; CHECK-NEXT:           |   if ((@b)[0][i1] > %v)
; CHECK-NEXT:           |   {
; CHECK-NEXT:           |      %indvars.iv.out = i1;
; CHECK-NEXT:           |      goto for.end.split.loop.exit8;
; CHECK-NEXT:           |   }
; CHECK-NEXT:           + END LOOP
; CHECK-NEXT:        }
; CHECK-NEXT:        if (%peel.factor1 <u 8192)
; CHECK-NEXT:        {
; CHECK-NEXT:           %tgu = (-1 * %peel.factor1 + 8192)/u32;
; CHECK-NEXT:           if (0 <u 32 * %tgu)
; CHECK-NEXT:           {
; CHECK-NEXT:              + DO i1 = 0, 32 * %tgu + -1, 32   <DO_MULTI_EXIT_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:              |   %wide.cmp. = (<32 x i32>*)(@b)[0][i1 + %peel.factor1] > %v;
; CHECK-NEXT:              |   %intmask = bitcast.<32 x i1>.i32(%wide.cmp.);
; CHECK-NEXT:              |   if (%intmask != 0)
; CHECK-NEXT:              |   {
; CHECK-NEXT:              |      %.vec = %wide.cmp.  ^  -1;
; CHECK-NEXT:              |      %bsfintmask = bitcast.<32 x i1>.i32(%wide.cmp.);
; CHECK-NEXT:              |      %bsf = @llvm.cttz.i32(%bsfintmask,  0);
; CHECK-NEXT:              |      %cast = zext.i32.i64(%bsf);
; CHECK-NEXT:              |      %indvars.iv.out = i1 + %peel.factor1 + %cast;
; CHECK-NEXT:              |      goto for.end.split.loop.exit8;
; CHECK-NEXT:              |   }
; CHECK-NEXT:              + END LOOP
; CHECK-NEXT:           }
; CHECK:                + DO i1 = 32 * %tgu, -1 * %peel.factor1 + 8191, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 31>  <LEGAL_MAX_TC = 31> <nounroll> <novectorize> <max_trip_count = 31>
; CHECK-NEXT:           |   if ((@b)[0][i1 + %peel.factor1] > %v)
; CHECK-NEXT:           |   {
; CHECK-NEXT:           |      %indvars.iv.out = i1 + %peel.factor1;
; CHECK-NEXT:           |      goto for.end.split.loop.exit8;
; CHECK-NEXT:           |   }
; CHECK-NEXT:           + END LOOP
; CHECK-NEXT:        }
; CHECK-NEXT:  END REGION
;
entry:
  br label %land.rhs

land.rhs:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds [8192 x i32], [8192 x i32]* @b, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1.not = icmp sgt i32 %0, %v
  br i1 %cmp1.not, label %for.end.split.loop.exit8, label %for.inc

for.inc:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 8192
  br i1 %exitcond.not, label %for.end.loopexit, label %land.rhs

for.end.split.loop.exit8:
  %indvars.iv.lcssa = phi i64 [ %indvars.iv, %land.rhs ]
  %1 = trunc i64 %indvars.iv.lcssa to i32
  br label %for.end

for.end.loopexit:
  br label %for.end

for.end:
  %newsize.0.lcssa = phi i32 [ %1, %for.end.split.loop.exit8 ], [ 8192, %for.end.loopexit ]
  ret i32 %newsize.0.lcssa
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readonly willreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

; The following variant uses an unknown trip count whose size differs from
; that of a pointer to the array elements.  We can't vectorize in this case,
; at least for now.

define i32 @_Z6searchii(i32 %v, i32 %n) #0 {
; CHECK-LABEL:  *** IR Dump After vpo::VPlanDriverHIRPass ***
; CHECK-NEXT:  Function: _Z6searchii
; CHECK-NOT:  BEGIN REGION { modified }
entry:
  br label %land.rhs

land.rhs:
  %indvars.iv = phi i32 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds [8192 x i32], [8192 x i32]* @b, i32 0, i32 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1.not = icmp sgt i32 %0, %v
  br i1 %cmp1.not, label %for.end.split.loop.exit8, label %for.inc

for.inc:
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond.not = icmp eq i32 %indvars.iv.next, %n
  br i1 %exitcond.not, label %for.end.loopexit, label %land.rhs

for.end.split.loop.exit8:
  %indvars.iv.lcssa = phi i32 [ %indvars.iv, %land.rhs ]
  br label %for.end

for.end.loopexit:
  br label %for.end

for.end:
  %newsize.0.lcssa = phi i32 [ %indvars.iv.lcssa, %for.end.split.loop.exit8 ], [ %n, %for.end.loopexit ]
  ret i32 %newsize.0.lcssa
}

; The following variant uses an unknown trip count whose size matches that
; of a pointer to the array elements.  Vectorization succeeds.

define i32 @_Z6searchil(i32 %v, i64 %n) #0 {
; CHECK-LABEL:  *** IR Dump After vpo::VPlanDriverHIRPass ***
; CHECK-NEXT:  Function: _Z6searchil
; CHECK:  BEGIN REGION { modified }
; CHECK-NEXT:      %arr.base.cast = ptrtoint.i32*.i64(&((@b)[0][0]));
; CHECK-NEXT:      %alignment = %arr.base.cast  &  127;
; CHECK-NEXT:      %peel.factor = 128  -  %alignment;
; CHECK-NEXT:      %peel.factor1 = %peel.factor  >>  2;
; CHECK-NEXT:      %peel.factor1 = (%n <= %peel.factor1) ? %n : %peel.factor1;
; CHECK-NEXT:      if (%peel.factor1 != 0)
; CHECK-NEXT:      {
; CHECK-NEXT:         + DO i1 = 0, %peel.factor1 + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 127> <vector-peel>
; CHECK-NEXT:         |   if ((@b)[0][i1] > %v)
; CHECK-NEXT:         |   {
; CHECK-NEXT:         |      %indvars.iv.out = i1;
; CHECK-NEXT:         |      goto for.end.split.loop.exit8;
; CHECK-NEXT:         |   }
; CHECK-NEXT:         + END LOOP
; CHECK-NEXT:      }
; CHECK-NEXT:      if (%peel.factor1 <u %n)
; CHECK-NEXT:      {
; CHECK-NEXT:         %tgu = (%n + -1 * %peel.factor1)/u32;
; CHECK-NEXT:         if (0 <u 32 * %tgu)
; CHECK-NEXT:         {
; CHECK-NEXT:            + DO i1 = 0, 32 * %tgu + -1, 32   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 256> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:            |   %wide.cmp. = (<32 x i32>*)(@b)[0][i1 + %peel.factor1] > %v;
; CHECK-NEXT:            |   %intmask = bitcast.<32 x i1>.i32(%wide.cmp.);
; CHECK-NEXT:            |   if (%intmask != 0)
; CHECK-NEXT:            |   {
; CHECK-NEXT:            |      %.vec = %wide.cmp.  ^  -1;
; CHECK-NEXT:            |      %bsfintmask = bitcast.<32 x i1>.i32(%wide.cmp.);
; CHECK-NEXT:            |      %bsf = @llvm.cttz.i32(%bsfintmask,  0);
; CHECK-NEXT:            |      %cast = zext.i32.i64(%bsf);
; CHECK-NEXT:            |      %indvars.iv.out = i1 + %peel.factor1 + %cast;
; CHECK-NEXT:            |      goto for.end.split.loop.exit8;
; CHECK-NEXT:            |   }
; CHECK-NEXT:            + END LOOP
; CHECK-NEXT:         }
; CHECK:              + DO i1 = 32 * %tgu, %n + -1 * %peel.factor1 + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 31>  <LEGAL_MAX_TC = 31> <nounroll> <novectorize> <max_trip_count = 31>
; CHECK-NEXT:         |   if ((@b)[0][i1 + %peel.factor1] > %v)
; CHECK-NEXT:         |   {
; CHECK-NEXT:         |      %indvars.iv.out = i1 + %peel.factor1;
; CHECK-NEXT:         |      goto for.end.split.loop.exit8;
; CHECK-NEXT:         |   }
; CHECK-NEXT:         + END LOOP
; CHECK-NEXT:      }
; CHECK-NEXT:  END REGION
entry:
  %nmod = trunc i64 %n to i32
  br label %land.rhs

land.rhs:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds [8192 x i32], [8192 x i32]* @b, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1.not = icmp sgt i32 %0, %v
  br i1 %cmp1.not, label %for.end.split.loop.exit8, label %for.inc

for.inc:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond.not, label %for.end.loopexit, label %land.rhs

for.end.split.loop.exit8:
  %indvars.iv.lcssa = phi i64 [ %indvars.iv, %land.rhs ]
  %1 = trunc i64 %indvars.iv.lcssa to i32
  br label %for.end

for.end.loopexit:
  br label %for.end

for.end:
  %newsize.0.lcssa = phi i32 [ %1, %for.end.split.loop.exit8 ], [ %nmod, %for.end.loopexit ]
  ret i32 %newsize.0.lcssa
}
