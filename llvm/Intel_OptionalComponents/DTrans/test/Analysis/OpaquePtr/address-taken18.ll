; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume  -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

; Test two structs with function pointers of different types.
; Right now, these will be compatible, but we could refine the analysis
; so that they are not.
; Check that AddressTaken is set on MYSTRUCT, as MYSTRUCTX is a compatible
; type, so @target1 and @target2 are targets for @fp.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { ptr, i32 }
; CHECK: Safety data:{{.*}}Address taken{{.*}}
; CHECK: End LLVMType: %struct.MYSTRUCT

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.MYSTRUCT = type { ptr, i32 }
%struct.MYEMPTY = type {}
%struct.MYSTRUCTX = type { ptr, i32 }

@myarg = dso_local global %struct.MYSTRUCT { ptr @target0, i32 5 }, align 8
@myempty = dso_local global %struct.MYEMPTY zeroinitializer, align 1
@myargx = dso_local local_unnamed_addr global %struct.MYSTRUCTX { ptr @myempty, i32 10 }, align 8
@fp = dso_local local_unnamed_addr global ptr null, align 8, !intel_dtrans_type !0

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn
define dso_local i32 @target0(i32 %myx) #0 {
entry:
  %mul = shl nsw i32 %myx, 1
  ret i32 %mul
}

; Function Attrs: nounwind uwtable
define dso_local i32 @target1(ptr nocapture readonly "intel_dtrans_func_index"="1" %arg) local_unnamed_addr #1 !intel.dtrans.func.type !16 {
entry:
  %field0 = getelementptr inbounds %struct.MYSTRUCT, ptr %arg, i64 0, i32 0, !intel-tbaa !17
  %i = load ptr, ptr %field0, align 8, !tbaa !17
  %field1 = getelementptr inbounds %struct.MYSTRUCT, ptr %arg, i64 0, i32 1, !intel-tbaa !23
  %i1 = load i32, ptr %field1, align 8, !tbaa !23
  %call = tail call i32 %i(i32 %i1) #2, !intel_dtrans_type !11
  ret i32 %call
}

; Function Attrs: nounwind uwtable
define dso_local i32 @target2(ptr nocapture readonly "intel_dtrans_func_index"="1" %arg) local_unnamed_addr #1 !intel.dtrans.func.type !24 {
entry:
  %i1 = load ptr, ptr %arg, align 8, !tbaa !26
  %field1 = getelementptr inbounds %struct.MYSTRUCTX, ptr %arg, i64 0, i32 1, !intel-tbaa !29
  %i2 = load i32, ptr %field1, align 8, !tbaa !29
  %call = tail call i32 %i1(i32 %i2) #2, !intel_dtrans_type !11
  ret i32 %call
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #1 {
entry:
  %i = load ptr, ptr @fp, align 8, !tbaa !30
  %call = tail call i32 %i(ptr nonnull @myarg) #2, !intel_dtrans_type !1
  ret i32 %call
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!4, !5, !6, !7, !8}
!intel.dtrans.types = !{!9, !12, !13}
!llvm.ident = !{!15}

!0 = !{!1, i32 1}
!1 = !{!"F", i1 false, i32 1, !2, !3}
!2 = !{i32 0, i32 0}
!3 = !{%struct.MYSTRUCT zeroinitializer, i32 1}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 1, !"Virtual Function Elim", i32 0}
!6 = !{i32 7, !"uwtable", i32 1}
!7 = !{i32 1, !"ThinLTO", i32 0}
!8 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!9 = !{!"S", %struct.MYSTRUCT zeroinitializer, i32 2, !10, !2}
!10 = !{!11, i32 1}
!11 = !{!"F", i1 false, i32 1, !2, !2}
!12 = !{!"S", %struct.MYEMPTY zeroinitializer, i32 0}
!13 = !{!"S", %struct.MYSTRUCTX zeroinitializer, i32 2, !14, !2}
!14 = !{%struct.MYEMPTY zeroinitializer, i32 1}
!15 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!16 = distinct !{!3}
!17 = !{!18, !19, i64 0}
!18 = !{!"struct@", !19, i64 0, !22, i64 8}
!19 = !{!"pointer@_ZTSPFiiE", !20, i64 0}
!20 = !{!"omnipotent char", !21, i64 0}
!21 = !{!"Simple C/C++ TBAA"}
!22 = !{!"int", !20, i64 0}
!23 = !{!18, !22, i64 8}
!24 = distinct !{!25}
!25 = !{%struct.MYSTRUCTX zeroinitializer, i32 1}
!26 = !{!27, !28, i64 0}
!27 = !{!"struct@", !28, i64 0, !22, i64 8}
!28 = !{!"pointer@_ZTSP7MYEMPTY", !20, i64 0}
!29 = !{!27, !22, i64 8}
!30 = !{!31, !31, i64 0}
!31 = !{!"pointer@_ZTSPFiP8MYSTRUCTE", !20, i64 0}
