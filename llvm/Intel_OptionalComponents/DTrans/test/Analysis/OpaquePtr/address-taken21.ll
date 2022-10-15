; REQUIRES: asserts

; RUN: opt -opaque-pointers -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-usecrulecompat -disable-output -debug-only=dtrans-safetyanalyzer < %s 2>&1 | FileCheck %s

; Check that the DTransSafetyAnalyzer did not run because LibIRC was not allowed everywhere

; CHECK: DTransSafetyInfo::analyzeModule running
; CHECK: DTransSafetyInfo: Not LibIRC allowed everywhere


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.MYSTRUCT = type { i32, i32 }
%struct.MYSTRUCTX = type { i32, i32 }

@myarg = dso_local global %struct.MYSTRUCT { i32 3, i32 3 }, align 4
@fp = dso_local local_unnamed_addr global ptr null, align 8, !intel_dtrans_type !0

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn
define dso_local i32 @target1(ptr nocapture readonly "intel_dtrans_func_index"="1" %arg) local_unnamed_addr #0 !intel.dtrans.func.type !12 {
entry:
  %field0 = getelementptr inbounds %struct.MYSTRUCT, ptr %arg, i64 0, i32 0, !intel-tbaa !13
  %i = load i32, ptr %field0, align 4, !tbaa !13
  ret i32 %i
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn
define dso_local i32 @target2(ptr nocapture readonly "intel_dtrans_func_index"="1" %arg) local_unnamed_addr #0 !intel.dtrans.func.type !18 {
entry:
  %field1 = getelementptr inbounds %struct.MYSTRUCTX, ptr %arg, i64 0, i32 1, !intel-tbaa !20
  %i = load i32, ptr %field1, align 4, !tbaa !20
  ret i32 %i
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #1 {
entry:
  %i = load ptr, ptr @fp, align 8, !tbaa !21
  %call = tail call i32 %i(ptr nonnull @myarg) #2, !intel_dtrans_type !1
  ret i32 %call
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!4, !5, !6, !7, !8}
!intel.dtrans.types = !{!9, !10}
!llvm.ident = !{!11}

!0 = !{!1, i32 1}
!1 = !{!"F", i1 false, i32 1, !2, !3}
!2 = !{i32 0, i32 0}
!3 = !{%struct.MYSTRUCT zeroinitializer, i32 1}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 1, !"Virtual Function Elim", i32 0}
!6 = !{i32 7, !"uwtable", i32 1}
!7 = !{i32 1, !"ThinLTO", i32 0}
!8 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!9 = !{!"S", %struct.MYSTRUCT zeroinitializer, i32 2, !2, !2}
!10 = !{!"S", %struct.MYSTRUCTX zeroinitializer, i32 2, !2, !2}
!11 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!12 = distinct !{!3}
!13 = !{!14, !15, i64 0}
!14 = !{!"struct@", !15, i64 0, !15, i64 4}
!15 = !{!"int", !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C/C++ TBAA"}
!18 = distinct !{!19}
!19 = !{%struct.MYSTRUCTX zeroinitializer, i32 1}
!20 = !{!14, !15, i64 4}
!21 = !{!22, !22, i64 0}
!22 = !{!"pointer@_ZTSPFiP8MYSTRUCTE", !16, i64 0}
