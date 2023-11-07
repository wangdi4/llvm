; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -disable-output -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec -vplan-cost-model-print-analysis-for-vf=1 -mattr=+avx512f | FileCheck %s

; The test checks that SLP heristics does not fail on ythe invalid loop cost

; CHECK: Cost decrease due to SLP breaking heuristic is 4

target triple = "x86_64-unknown-linux-gnu"

%struct.rgb_t = type { i32, i32, i32, i32 }

@b = global [10240 x %struct.rgb_t] zeroinitializer, align 16
@a = global [10240 x %struct.rgb_t] zeroinitializer, align 16

define void @foo_2elem() #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %call1 = call noundef i32 @_Z5pointdd(double noundef nofpclass(nan inf) 3.300000e+00, double noundef nofpclass(nan inf) 4.400000e+00)
  %r = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
  %0 = load i32, ptr %r, align 16, !tbaa !3
  %r3 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
  %1 = load i32, ptr %r3, align 16, !tbaa !3
  %add = add nsw i32 %1, %0
  store i32 %add, ptr %r3, align 16, !tbaa !3
  %g = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
  %2 = load i32, ptr %g, align 4, !tbaa !9
  %g8 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
  %3 = load i32, ptr %g8, align 4, !tbaa !9
  %add9 = add nsw i32 %3, %2
  store i32 %add9, ptr %g8, align 4, !tbaa !9
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 10240
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

declare dso_local noundef i32 @_Z5pointdd(double noundef nofpclass(nan inf), double noundef nofpclass(nan inf)) local_unnamed_addr #1

attributes #0 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = {  "vector-variants"="_ZGVbN4vv__Z5pointdd,_ZGVbM4vv__Z5pointdd,_ZGVcN8vv__Z5pointdd,_ZGVcM8vv__Z5pointdd,_ZGVdN8vv__Z5pointdd,_ZGVdM8vv__Z5pointdd,_ZGVeN16vv__Z5pointdd,_ZGVeM16vv__Z5pointdd" }

!3 = !{!4, !6, i64 0}
!4 = !{!"array@_ZTSA10240_5rgb_t", !5, i64 0}
!5 = !{!"struct@", !6, i64 0, !6, i64 4, !6, i64 8, !6, i64 12}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!4, !6, i64 4}
!10 = !{!4, !6, i64 8}
!14 = !{!4, !6, i64 12}
; end INTEL_FEATURE_SW_ADVANCED
