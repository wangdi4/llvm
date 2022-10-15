; REQUIRES: asserts

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed  -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

; Test address taken function @target1, which is external. Since a pointer
; to MYSTRUCT is passed as an argument, MYSTRUCT should be marked as
; Address taken.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32, i32 }
; CHECK: Safety data:{{.*}}Address taken{{.*}}
; CHECK: End LLVMType: %struct.MYSTRUCT

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.MYSTRUCT = type { i32, i32 }

@myarg = dso_local global %struct.MYSTRUCT { i32 3, i32 5 }, align 4
@fp = dso_local local_unnamed_addr global ptr @target1, align 8, !intel_dtrans_type !0

declare !intel.dtrans.func.type !11 dso_local i32 @target1(ptr "intel_dtrans_func_index"="1") #0

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #1 {
entry:
  %i = load ptr, ptr @fp, align 8, !tbaa !12
  %call = tail call i32 %i(ptr nonnull @myarg) #2, !intel_dtrans_type !1
  ret i32 %call
}

attributes #0 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!4, !5, !6, !7, !8}
!intel.dtrans.types = !{!9}
!llvm.ident = !{!10}

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
!10 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!11 = distinct !{!3}
!12 = !{!13, !13, i64 0}
!13 = !{!"pointer@_ZTSPFiP8MYSTRUCTE", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
