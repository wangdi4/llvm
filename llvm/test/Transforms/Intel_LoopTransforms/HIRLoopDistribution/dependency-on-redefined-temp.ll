; RUN: opt -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll,hir-loop-distribute-memrec" -hir-loop-distribute-scex-cost=2 -print-before=hir-loop-distribute-memrec -print-after=hir-loop-distribute-memrec < %s 2>&1 | FileCheck %s

; Incoming HIR before unrolling-

; + DO i1 = 0, 7999, 1   <DO_LOOP>
; |   %call = @malloc(160);
; |   (@rp)[0][i1].0 = &((%call)[0]);
; |   %call3 = @malloc(160);
; |   (@Ap)[0][i1].1 = &((%call3)[0]);
; |   %call8 = @malloc(160);
; |   (@xp)[0][i1].3 = &((%call8)[0]);
; |
; |   + DO i2 = 0, 1, 1   <DO_LOOP> <unroll>
; |   |   %conv15 = sitofp.i32.double(i2);
; |   |   %mul16 = %conv15  *  5.000000e-02;
; |   |   (%call3)[i2] = %mul16;
; |   |   %conv22 = sitofp.i32.double(-1 * i2 + 20);
; |   |   %mul23 = %conv22  *  1.000000e-01;
; |   |   (%call8)[i2] = %mul23;
; |   + END LOOP
; + END LOOP

; The i2 loop is unrolled so there are multiple definitions of %conv15, %mul16,
; %conv22 and %mul23.

; Verify that %mul16 and %mul23 which are defined in terms of %conv15 and %conv22
; respectively, are scalar-expanded and not recomputed. If we recompute %mul16 and
; %mul23 using the temps instead, we will get incorrect results because %conv15 and
; %conv22 are defined twice in the first chunk and their second definitions override
; the first definition make it inaccessible in the second chunk. Note that the first
; definitions of %conv15 and %conv22 do not require scalar expansion.

; CHECK Dump Before

; CHECK: + DO i1 = 0, 7999, 1   <DO_LOOP>
; CHECK: |   %call = @malloc(160);
; CHECK: |   (@rp)[0][i1].0 = &((%call)[0]);
; CHECK: |   %call3 = @malloc(160);
; CHECK: |   (@Ap)[0][i1].1 = &((%call3)[0]);
; CHECK: |   %call8 = @malloc(160);
; CHECK: |   (@xp)[0][i1].3 = &((%call8)[0]);
; CHECK: |   %conv15 = sitofp.i32.double(0);
; CHECK: |   %mul16 = %conv15  *  5.000000e-02;
; CHECK: |   (%call3)[0] = %mul16;
; CHECK: |   %conv22 = sitofp.i32.double(20);
; CHECK: |   %mul23 = %conv22  *  1.000000e-01;
; CHECK: |   (%call8)[0] = %mul23;
; CHECK: |   %conv15 = sitofp.i32.double(1);
; CHECK: |   %mul16 = %conv15  *  5.000000e-02;
; CHECK: |   (%call3)[1] = %mul16;
; CHECK: |   %conv22 = sitofp.i32.double(19);
; CHECK: |   %mul23 = %conv22  *  1.000000e-01;
; CHECK: |   (%call8)[1] = %mul23;
; CHECK: + END LOOP

; CHECK Dump After

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 124, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 63, 1   <DO_LOOP>
; CHECK: |   |   %conv15 = sitofp.i32.double(0);
; CHECK: |   |   %mul16 = %conv15  *  5.000000e-02;
; CHECK: |   |   (%.TempArray)[0][i2] = %mul16;
; CHECK: |   |   %conv22 = sitofp.i32.double(20);
; CHECK: |   |   %mul23 = %conv22  *  1.000000e-01;
; CHECK: |   |   (%.TempArray2)[0][i2] = %mul23;
; CHECK: |   |   %conv15 = sitofp.i32.double(1);
; CHECK: |   |   %conv22 = sitofp.i32.double(19);
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |
; CHECK: |   + DO i2 = 0, 63, 1   <DO_LOOP>
; CHECK: |   |   %mul16 = (%.TempArray)[0][i2];
; CHECK: |   |   %mul23 = (%.TempArray2)[0][i2];
; CHECK: |   |   %conv15 = sitofp.i32.double(1);
; CHECK: |   |   %conv22 = sitofp.i32.double(19);
; CHECK: |   |   %call = @malloc(160);
; CHECK: |   |   (@rp)[0][64 * i1 + i2].0 = &((%call)[0]);
; CHECK: |   |   %call3 = @malloc(160);
; CHECK: |   |   (@Ap)[0][64 * i1 + i2].1 = &((%call3)[0]);
; CHECK: |   |   %call8 = @malloc(160);
; CHECK: |   |   (@xp)[0][64 * i1 + i2].3 = &((%call8)[0]);
; CHECK: |   |   (%call3)[0] = %mul16;
; CHECK: |   |   (%call8)[0] = %mul23;
; CHECK: |   |   %mul16 = %conv15  *  5.000000e-02;
; CHECK: |   |   (%call3)[1] = %mul16;
; CHECK: |   |   %mul23 = %conv22  *  1.000000e-01;
; CHECK: |   |   (%call8)[1] = %mul23;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

%struct.ST = type { ptr, ptr, i8, ptr }

@rp = external dso_local local_unnamed_addr global [8000 x %struct.ST], align 16
@Ap = external dso_local local_unnamed_addr global [8000 x %struct.ST], align 16
@xp = external dso_local local_unnamed_addr global [8000 x %struct.ST], align 16

define void @foo() {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc29, %entry
  %indvars.iv82 = phi i64 [ 0, %entry ], [ %indvars.iv.next83, %for.inc29 ]
  %call = tail call noalias dereferenceable_or_null(160) ptr @malloc(i64 noundef 160) #5
  %ad1 = getelementptr inbounds [8000 x %struct.ST], ptr @rp, i64 0, i64 %indvars.iv82, i32 0
  store ptr %call, ptr %ad1, align 16
  %call3 = tail call noalias dereferenceable_or_null(160) ptr @malloc(i64 noundef 160) #5
  %ad2 = getelementptr inbounds [8000 x %struct.ST], ptr @Ap, i64 0, i64 %indvars.iv82, i32 1
  store ptr %call3, ptr %ad2, align 8
  %call8 = tail call noalias dereferenceable_or_null(160) ptr @malloc(i64 noundef 160) #5
  %ad3 = getelementptr inbounds [8000 x %struct.ST], ptr @xp, i64 0, i64 %indvars.iv82, i32 3
  store ptr %call8, ptr %ad3, align 8
  br label %for.body14

for.body14:                                       ; preds = %for.body14, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body14 ]
  %0 = trunc i64 %indvars.iv to i32
  %conv15 = sitofp i32 %0 to double
  %mul16 = fmul fast double %conv15, 5.000000e-02
  %arrayidx21 = getelementptr inbounds double, ptr %call3, i64 %indvars.iv
  store double %mul16, ptr %arrayidx21, align 8
  %1 = sub i32 20, %0
  %conv22 = sitofp i32 %1 to double
  %mul23 = fmul fast double %conv22, 1.000000e-01
  %arrayidx28 = getelementptr inbounds double, ptr %call8, i64 %indvars.iv
  store double %mul23, ptr %arrayidx28, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 2
  br i1 %exitcond.not, label %for.inc29, label %for.body14, !llvm.loop !0

for.inc29:                                        ; preds = %for.body14
  %indvars.iv.next83 = add nuw nsw i64 %indvars.iv82, 1
  %exitcond84.not = icmp eq i64 %indvars.iv.next83, 8000
  br i1 %exitcond84.not, label %for.end31, label %for.body

for.end31:
  ret void
}

declare dso_local noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #1

attributes #1 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.full"}

