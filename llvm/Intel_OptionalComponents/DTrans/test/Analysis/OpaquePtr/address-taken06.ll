; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed  -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

; Test two structs, one which has only floating-point fields, and another
; which has floating point and integer fields.
; Check that AddressTaken is NOT set on MYSTRUCT, as MYSTRUCTX is a NOT
; a compatible type for MYSTRUCT.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { float, double }
; CHECK-NOT: Safety data:{{.*}}Address taken{{.*}}
; CHECK: End LLVMType: %struct.MYSTRUCT

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.MYSTRUCT = type { float, double }
%struct.MYSTRUCTX = type { double, i32 }

@myarg = dso_local global %struct.MYSTRUCT zeroinitializer, align 8
@fp = dso_local local_unnamed_addr global ptr null, align 8, !intel_dtrans_type !0

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn
define dso_local float @target1(ptr nocapture readonly "intel_dtrans_func_index"="1" %arg) local_unnamed_addr #0 !intel.dtrans.func.type !14 {
entry:
  %field0 = getelementptr inbounds %struct.MYSTRUCT, ptr %arg, i64 0, i32 0, !intel-tbaa !15
  %i = load float, ptr %field0, align 8, !tbaa !15
  ret float %i
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn
define dso_local double @target2(ptr nocapture readonly "intel_dtrans_func_index"="1" %arg) local_unnamed_addr #0 !intel.dtrans.func.type !21 {
entry:
  %field0 = getelementptr inbounds %struct.MYSTRUCTX, ptr %arg, i64 0, i32 1, !intel-tbaa !23
  %i = load i32, ptr %field0, align 8, !tbaa !23
  %conv = sitofp i32 %i to double
  ret double %conv
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #1 {
entry:
  %i = load ptr, ptr @fp, align 8, !tbaa !26
  %call = tail call i32 %i(ptr nonnull @myarg) #2, !intel_dtrans_type !1
  ret i32 %call
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!4, !5, !6, !7, !8}
!intel.dtrans.types = !{!9, !12}
!llvm.ident = !{!13}

!0 = !{!1, i32 1}
!1 = !{!"F", i1 false, i32 1, !2, !3}
!2 = !{i32 0, i32 0}
!3 = !{%struct.MYSTRUCT zeroinitializer, i32 1}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 1, !"Virtual Function Elim", i32 0}
!6 = !{i32 7, !"uwtable", i32 1}
!7 = !{i32 1, !"ThinLTO", i32 0}
!8 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!9 = !{!"S", %struct.MYSTRUCT zeroinitializer, i32 2, !10, !11}
!10 = !{float 0.000000e+00, i32 0}
!11 = !{double 0.000000e+00, i32 0}
!12 = !{!"S", %struct.MYSTRUCTX zeroinitializer, i32 2, !11, !2}
!13 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!14 = distinct !{!3}
!15 = !{!16, !17, i64 0}
!16 = !{!"struct@", !17, i64 0, !20, i64 8}
!17 = !{!"float", !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
!20 = !{!"double", !18, i64 0}
!21 = distinct !{!22}
!22 = !{%struct.MYSTRUCTX zeroinitializer, i32 1}
!23 = !{!24, !25, i64 8}
!24 = !{!"struct@", !20, i64 0, !25, i64 8}
!25 = !{!"int", !18, i64 0}
!26 = !{!27, !27, i64 0}
!27 = !{!"pointer@_ZTSPFiP8MYSTRUCTE", !18, i64 0}
