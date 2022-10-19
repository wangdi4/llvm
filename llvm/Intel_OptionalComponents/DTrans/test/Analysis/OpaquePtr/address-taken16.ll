; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed  -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

; Test that address taken function @target1, which is external, does not
; generate an AddressTaken safety check on MYSTRUCT, because there is no
; direct call to it, and no indirect call matches it.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32, i32 }
; CHECK-NOT: Safety data:{{.*}}Address taken{{.*}}
; CHECK: End LLVMType: %struct.MYSTRUCT

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.MYSTRUCT = type { i32, i32 }

@myarg = dso_local local_unnamed_addr global %struct.MYSTRUCT { i32 3, i32 5 }, align 4
@fp1 = dso_local local_unnamed_addr global ptr @target1, align 8, !intel_dtrans_type !0
@fp2 = dso_local local_unnamed_addr global ptr @target2, align 8, !intel_dtrans_type !4

declare !intel.dtrans.func.type !13 dso_local i32 @target1(ptr "intel_dtrans_func_index"="1") #0

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn
define dso_local i32 @target2(i32 %arg1, i32 %arg2) #1 {
entry:
  %add = add nsw i32 %arg2, %arg1
  ret i32 %add
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #2 {
entry:
  %i = load ptr, ptr @fp2, align 8, !tbaa !14
  %call = tail call i32 %i(i32 8, i32 5) #3, !intel_dtrans_type !5
  ret i32 %call
}

attributes #0 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!6, !7, !8, !9, !10}
!intel.dtrans.types = !{!11}
!llvm.ident = !{!12}

!0 = !{!1, i32 1}
!1 = !{!"F", i1 false, i32 1, !2, !3}
!2 = !{i32 0, i32 0}
!3 = !{%struct.MYSTRUCT zeroinitializer, i32 1}
!4 = !{!5, i32 1}
!5 = !{!"F", i1 false, i32 2, !2, !2, !2}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{i32 1, !"Virtual Function Elim", i32 0}
!8 = !{i32 7, !"uwtable", i32 1}
!9 = !{i32 1, !"ThinLTO", i32 0}
!10 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!11 = !{!"S", %struct.MYSTRUCT zeroinitializer, i32 2, !2, !2}
!12 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!13 = distinct !{!3}
!14 = !{!15, !15, i64 0}
!15 = !{!"pointer@_ZTSPFiiiE", !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C/C++ TBAA"}
