; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed  -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

; Test two structs with different numbers of elements.
; Check that AddressTaken is NOT set on MYSTRUCT and MYSTRUCTX as there is no compatible type for either.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32, i32 }
; CHECK-NOT: Safety data:{{.*}}Address taken{{.*}}
; CHECK: End LLVMType: %struct.MYSTRUCT

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCTX = type { i32, i32, i32 }
; CHECK-NOT: Safety data:{{.*}}Address taken{{.*}}
; CHECK: End LLVMType: %struct.MYSTRUCTX

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.MYSTRUCT = type { i32, i32 }
%struct.MYSTRUCTX = type { i32, i32, i32 }

@myarg0 = dso_local global %struct.MYSTRUCT { i32 3, i32 5 }, align 4
@myarg1 = dso_local global %struct.MYSTRUCTX { i32 3, i32 5, i32 7 }, align 4
@fp = dso_local local_unnamed_addr global ptr null, align 8, !intel_dtrans_type !0
@fpp = dso_local local_unnamed_addr global ptr null, align 8, !intel_dtrans_type !4

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn
define dso_local i32 @target1(ptr nocapture readonly "intel_dtrans_func_index"="1" %arg) local_unnamed_addr #0 !intel.dtrans.func.type !15 {
entry:
  %field0 = getelementptr inbounds %struct.MYSTRUCT, ptr %arg, i64 0, i32 0, !intel-tbaa !16
  %i = load i32, ptr %field0, align 4, !tbaa !16
  ret i32 %i
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #1 {
entry:
  %i = load ptr, ptr @fp, align 8, !tbaa !21
  %call = tail call i32 %i(ptr nonnull @myarg0) #2, !intel_dtrans_type !1
  %i1 = load ptr, ptr @fpp, align 8, !tbaa !23
  %call1 = tail call i32 %i1(ptr nonnull @myarg1) #2, !intel_dtrans_type !5
  %add = add nsw i32 %call1, %call
  ret i32 %add
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!7, !8, !9, !10, !11}
!intel.dtrans.types = !{!12, !13}
!llvm.ident = !{!14}

!0 = !{!1, i32 1}
!1 = !{!"F", i1 false, i32 1, !2, !3}
!2 = !{i32 0, i32 0}
!3 = !{%struct.MYSTRUCT zeroinitializer, i32 1}
!4 = !{!5, i32 1}
!5 = !{!"F", i1 false, i32 1, !2, !6}
!6 = !{%struct.MYSTRUCTX zeroinitializer, i32 1}
!7 = !{i32 1, !"wchar_size", i32 4}
!8 = !{i32 1, !"Virtual Function Elim", i32 0}
!9 = !{i32 7, !"uwtable", i32 1}
!10 = !{i32 1, !"ThinLTO", i32 0}
!11 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!12 = !{!"S", %struct.MYSTRUCT zeroinitializer, i32 2, !2, !2}
!13 = !{!"S", %struct.MYSTRUCTX zeroinitializer, i32 3, !2, !2, !2}
!14 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!15 = distinct !{!3}
!16 = !{!17, !18, i64 0}
!17 = !{!"struct@", !18, i64 0, !18, i64 4}
!18 = !{!"int", !19, i64 0}
!19 = !{!"omnipotent char", !20, i64 0}
!20 = !{!"Simple C/C++ TBAA"}
!21 = !{!22, !22, i64 0}
!22 = !{!"pointer@_ZTSPFiP8MYSTRUCTE", !19, i64 0}
!23 = !{!24, !24, i64 0}
!24 = !{!"pointer@_ZTSPFiP9MYSTRUCTXE", !19, i64 0}
