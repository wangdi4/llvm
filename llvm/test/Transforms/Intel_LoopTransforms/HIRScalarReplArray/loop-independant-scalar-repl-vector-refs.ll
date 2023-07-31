; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam,hir-vec-dir-insert,hir-vplan-vec,hir-scalarrepl-array,print<hir>" -vplan-force-vf=4 -disable-output 2>&1 | FileCheck %s

; Verify that scalar replacement was able to eliminate redundant loads of
; (<4 x i32>*)(%A)[i2] and (<4 x i32>*)(%A)[i2 + 1].

; Incoming HIR-
; + DO i1 = 0, 49, 1   <DO_LOOP> <nounroll and jam>
; |   + DO i2 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; |   |   %.vec = (<4 x i32>*)(%A)[i2];
; |   |   %.vec2 = (<4 x i32>*)(%A)[i2 + 1];
; |   |   %.vec3 = (<4 x i32>*)(%A)[i2];
; |   |   %.vec4 = (<4 x i32>*)(%A)[i2 + 1];
; |   |   %.vec5 = %phi.temp + %.vec + %.vec2  +  %.vec3 + %.vec4;
; |   |   %phi.temp = %.vec5;
; |   + END LOOP
; + END LOOP


; CHECK: + DO i1 = 0, 49, 1   <DO_LOOP> <nounroll and jam>
; CHECK: |   + DO i2 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK: |   |   %scalarepl.vec = (<4 x i32>*)(%A)[i2];
; CHECK: |   |   %.vec = %scalarepl.vec;
; CHECK: |   |   %scalarepl.vec8 = (<4 x i32>*)(%A)[i2 + 1];
; CHECK: |   |   %.vec2 = %scalarepl.vec8;
; CHECK: |   |   %.vec3 = %scalarepl.vec;
; CHECK: |   |   %.vec4 = %scalarepl.vec8;
; CHECK: |   |   %.vec5 = %phi.temp + %.vec + %.vec2  +  %.vec3 + %.vec4;
; CHECK: |   |   %phi.temp = %.vec5;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree norecurse nosync nounwind readonly uwtable
define dso_local i32 @foo(ptr nocapture noundef readonly %A) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc8
  %t.023 = phi i32 [ 0, %entry ], [ %add7.lcssa, %for.inc8 ]
  %j.022 = phi i32 [ 0, %entry ], [ %inc9, %for.inc8 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %t.121 = phi i32 [ %t.023, %for.cond1.preheader ], [ %add7, %for.body3 ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv.next
  %1 = load i32, ptr %arrayidx5, align 4, !tbaa !3
  %add6 = add nsw i32 %0, %1
  %add7 = add nsw i32 %t.121, %add6
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.inc8, label %for.body3, !llvm.loop !7

for.inc8:                                         ; preds = %for.body3
  %add7.lcssa = phi i32 [ %add7, %for.body3 ]
  %inc9 = add nuw nsw i32 %j.022, 1
  %exitcond24.not = icmp eq i32 %inc9, 100
  br i1 %exitcond24.not, label %for.end10, label %for.cond1.preheader, !llvm.loop !8

for.end10:                                        ; preds = %for.inc8
  %add7.lcssa.lcssa = phi i32 [ %add7.lcssa, %for.inc8 ]
  ret i32 %add7.lcssa.lcssa
}

attributes #0 = { argmemonly nofree norecurse nosync nounwind readonly uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="icelake-server" "target-features"="+64bit,+adx,+aes,+avx,+avx2,+avx512bitalg,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512ifma,+avx512vbmi,+avx512vbmi2,+avx512vl,+avx512vnni,+avx512vpopcntdq,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+gfni,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pconfig,+pku,+popcnt,+prfchw,+rdpid,+rdrnd,+rdseed,+sahf,+sgx,+sha,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+vaes,+vpclmulqdq,+wbnoinvd,+x87,+xsave,+xsavec,+xsaveopt,+xsaves,-amx-bf16,-amx-int8,-amx-tile,-avx512bf16,-avx512er,-avx512fp16,-avx512pf,-avx512vp2intersect,-avxvnni,-cldemote,-clzero,-enqcmd,-fma4,-hreset,-kl,-lwp,-movdir64b,-movdiri,-mwaitx,-prefetchwt1,-ptwrite,-rdpru,-rtm,-serialize,-shstk,-sse4a,-tbm,-tsxldtrk,-uintr,-waitpkg,-widekl,-xop" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7}
!8 = distinct !{!8, !9, !10}
!9 = !{!"llvm.loop.mustprogress"}
!10 = !{!"llvm.loop.unroll_and_jam.count", i32 2}
