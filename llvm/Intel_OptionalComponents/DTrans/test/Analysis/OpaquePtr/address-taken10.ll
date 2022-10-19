; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed  -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

; Check arrays of different numbers of elements of underlying structs which
; are identical.
; Check that AddressTaken is NOT set on 10 x MYSTRUCT, as 11 x MYSTRUCTX is
; NOT a compatible type.

; CHECK: DTRANS_ArrayInfo:
; CHECK: LLVMType: [10 x %struct.MYSTRUCT]
; CHECK: Number of elements: 10
; CHECK: Element LLVM Type: %struct.MYSTRUCT = type { i32, i32 }
; CHECK-NOT: Safety data:{{.*}}Address taken{{.*}}
; CHECK: End LLVMType: [10 x %struct.MYSTRUCT]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.MYSTRUCT = type { i32, i32 }
%struct.MYSTRUCTX = type { i32, i32 }

@fp = dso_local local_unnamed_addr global ptr null, align 8, !intel_dtrans_type !0
@myarg = dso_local global [10 x %struct.MYSTRUCT] zeroinitializer, align 16, !intel_dtrans_type !4

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn
define dso_local i32 @target1(ptr nocapture readonly "intel_dtrans_func_index"="1" %arg) local_unnamed_addr #0 !intel.dtrans.func.type !14 {
entry:
  %arrayidx = getelementptr inbounds [10 x %struct.MYSTRUCT], ptr %arg, i64 0, i64 1, !intel-tbaa !15
  %field1 = getelementptr inbounds %struct.MYSTRUCT, ptr %arrayidx, i64 0, i32 1, !intel-tbaa !21
  %i = load i32, ptr %field1, align 4, !tbaa !22
  ret i32 %i
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn
define dso_local i32 @target2(ptr nocapture readonly "intel_dtrans_func_index"="1" %arg) local_unnamed_addr #0 !intel.dtrans.func.type !23 {
entry:
  %arrayidx = getelementptr inbounds [11 x %struct.MYSTRUCTX], ptr %arg, i64 0, i64 1, !intel-tbaa !27
  %field1 = getelementptr inbounds %struct.MYSTRUCTX, ptr %arrayidx, i64 0, i32 1, !intel-tbaa !21
  %i = load i32, ptr %field1, align 4, !tbaa !29
  ret i32 %i
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #1 {
entry:
  %i = load ptr, ptr @fp, align 8, !tbaa !30
  %call = tail call i32 %i(ptr nonnull @myarg) #2, !intel_dtrans_type !1
  ret i32 %call
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!6, !7, !8, !9, !10}
!intel.dtrans.types = !{!11, !12}
!llvm.ident = !{!13}

!0 = !{!1, i32 1}
!1 = !{!"F", i1 false, i32 1, !2, !3}
!2 = !{i32 0, i32 0}
!3 = !{!4, i32 1}
!4 = !{!"A", i32 10, !5}
!5 = !{%struct.MYSTRUCT zeroinitializer, i32 0}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{i32 1, !"Virtual Function Elim", i32 0}
!8 = !{i32 7, !"uwtable", i32 1}
!9 = !{i32 1, !"ThinLTO", i32 0}
!10 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!11 = !{!"S", %struct.MYSTRUCT zeroinitializer, i32 2, !2, !2}
!12 = !{!"S", %struct.MYSTRUCTX zeroinitializer, i32 2, !2, !2}
!13 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!14 = distinct !{!3}
!15 = !{!16, !17, i64 0}
!16 = !{!"array@_ZTSA10_8MYSTRUCT", !17, i64 0}
!17 = !{!"struct@", !18, i64 0, !18, i64 4}
!18 = !{!"int", !19, i64 0}
!19 = !{!"omnipotent char", !20, i64 0}
!20 = !{!"Simple C/C++ TBAA"}
!21 = !{!17, !18, i64 4}
!22 = !{!16, !18, i64 4}
!23 = distinct !{!24}
!24 = !{!25, i32 1}
!25 = !{!"A", i32 11, !26}
!26 = !{%struct.MYSTRUCTX zeroinitializer, i32 0}
!27 = !{!28, !17, i64 0}
!28 = !{!"array@_ZTSA11_9MYSTRUCTX", !17, i64 0}
!29 = !{!28, !18, i64 4}
!30 = !{!31, !31, i64 0}
!31 = !{!"pointer@_ZTSPFiPA10_8MYSTRUCTE", !19, i64 0}
