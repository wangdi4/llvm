; INTEL_FEATURE_SW_DTRANS

; Input file for the test cases intel-merge-types-opq-08.ll and
; intel-merge-types-opq-08-debug.ll.

; ModuleID = 'intel-merge-mangled13-b.ll'
source_filename = "simple2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10TestStruct.TestStruct = type { ptr }
%struct.ident_t = type { i32, i32, i32, i32, ptr }

@glob = dso_local local_unnamed_addr global ptr null, align 8, !intel_dtrans_type !0

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn
define dso_local double @_Z3bari(i32 %i) local_unnamed_addr #0 {
entry:
  %0 = load ptr, ptr @glob, align 8, !tbaa !11
  %ptr = getelementptr inbounds %struct._ZTS10TestStruct.TestStruct, ptr %0, i64 0, i32 0, !intel-tbaa !15
  %1 = load ptr, ptr %ptr, align 8, !tbaa !15
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds double, ptr %1, i64 %idxprom
  %2 = load double, ptr %arrayidx, align 8, !tbaa !18
  ret double %2
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!1, !2, !3, !4}
!intel.dtrans.types = !{!5, !7}
!llvm.ident = !{!10}

!0 = !{%struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1, !6}
!6 = !{double 0.000000e+00, i32 1}
!7 = !{!"S", %struct.ident_t zeroinitializer, i32 5, !8, !8, !8, !8, !9}
!8 = !{i32 0, i32 0}
!9 = !{i8 0, i32 1}
!10 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!11 = !{!12, !12, i64 0}
!12 = !{!"pointer@_ZTSP10TestStruct", !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C++ TBAA"}
!15 = !{!16, !17, i64 0}
!16 = !{!"struct@_ZTS10TestStruct", !17, i64 0}
!17 = !{!"pointer@_ZTSPd", !13, i64 0}
!18 = !{!19, !19, i64 0}
!19 = !{!"double", !13, i64 0}

; end INTEL_FEATURE_SW_DTRANS