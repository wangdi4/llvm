; RUN: opt -passes="hir-ssa-deconstruction,hir-unroll-and-jam" -debug-only=hir-unroll-and-jam < %s 2>&1 | FileCheck %s

; Verify that we give up on unroll & jam because it is not profitable.
; Although there is temporal locality between (@A)[0][i1][i2] and 
; (@A)[0][i1 + 1][i2], the store to (@A)[0][%t][i2] which aliases with
; the loads prevents us from exploiting the locality after unroll & jam.

; Input HIR-
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   + DO i2 = 0, 99, 1   <DO_LOOP>
; |   |   %0 = (@A)[0][i1][i2];
; |   |   %1 = (@A)[0][i1 + 1][i2];
; |   |   (@A)[0][%t][i2] = %0 + %1;
; |   + END LOOP
; + END LOOP

; CHECK: Skipping unroll & jam as aliasing prevents us from exploiting temporal locality!


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @foo(i32 noundef %t) local_unnamed_addr #0 {
entry:
  %idxprom11 = sext i32 %t to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc15
  %indvars.iv29 = phi i64 [ 0, %entry ], [ %indvars.iv.next30, %for.inc15 ]
  %indvars.iv.next30 = add nuw nsw i64 %indvars.iv29, 1
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [100 x [100 x i32]], ptr @A, i64 0, i64 %indvars.iv29, i64 %indvars.iv, !intel-tbaa !3
  %0 = load i32, ptr %arrayidx5, align 4, !tbaa !3
  %arrayidx9 = getelementptr inbounds [100 x [100 x i32]], ptr @A, i64 0, i64 %indvars.iv.next30, i64 %indvars.iv, !intel-tbaa !3
  %1 = load i32, ptr %arrayidx9, align 4, !tbaa !3
  %add10 = add nsw i32 %1, %0
  %arrayidx14 = getelementptr inbounds [100 x [100 x i32]], ptr @A, i64 0, i64 %idxprom11, i64 %indvars.iv, !intel-tbaa !3
  store i32 %add10, ptr %arrayidx14, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.inc15, label %for.body3, !llvm.loop !9

for.inc15:                                        ; preds = %for.body3
  %exitcond31.not = icmp eq i64 %indvars.iv.next30, 100
  br i1 %exitcond31.not, label %for.end17, label %for.cond1.preheader, !llvm.loop !11

for.end17:                                        ; preds = %for.inc15
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="cascadelake" "target-features"="+64bit,+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+avx512vnni,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+rtm,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves,-amx-bf16,-amx-int8,-amx-tile,-avx512bf16,-avx512bitalg,-avx512er,-avx512fp16,-avx512ifma,-avx512pf,-avx512vbmi,-avx512vbmi2,-avx512vp2intersect,-avx512vpopcntdq,-avxvnni,-cldemote,-clzero,-enqcmd,-fma4,-gfni,-hreset,-kl,-lwp,-movdir64b,-movdiri,-mwaitx,-pconfig,-prefetchwt1,-ptwrite,-rdpid,-rdpru,-serialize,-sgx,-sha,-shstk,-sse4a,-tbm,-tsxldtrk,-uintr,-vaes,-vpclmulqdq,-waitpkg,-wbnoinvd,-widekl,-xop" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !6, i64 0}
!4 = !{!"array@_ZTSA100_A100_i", !5, i64 0}
!5 = !{!"array@_ZTSA100_i", !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
!11 = distinct !{!11, !10}
