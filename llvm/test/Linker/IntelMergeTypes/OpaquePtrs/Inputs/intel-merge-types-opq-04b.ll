; INTEL_FEATURE_SW_DTRANS

; This is an input file for the test cases intel-merge-types-opq-04.ll
; and intel-merge-types-opq-04-debug.ll.

; ModuleID = 'simple2.ll'
source_filename = "simple2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10TestStruct.TestStruct = type { ptr, %struct._ZTSN10TestStructUt_E.anon }
%struct._ZTSN10TestStructUt_E.anon = type { ptr }

@glob = dso_local local_unnamed_addr global ptr null, align 8, !intel_dtrans_type !0

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn
define dso_local double @_Z3bari(i32 %i) local_unnamed_addr #0 {
entry:
  %0 = load ptr, ptr @glob, align 8, !tbaa !11
  %inner_ptr = getelementptr inbounds %struct._ZTS10TestStruct.TestStruct, ptr %0, i64 0, i32 1, i32 0, !intel-tbaa !15
  %1 = load ptr, ptr %inner_ptr, align 8, !tbaa !15
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds double, ptr %1, i64 %idxprom
  %2 = load double, ptr %arrayidx, align 8, !tbaa !20
  %ptr = getelementptr inbounds %struct._ZTS10TestStruct.TestStruct, ptr %0, i64 0, i32 0, !intel-tbaa !22
  %3 = load ptr, ptr %ptr, align 8, !tbaa !22
  %arrayidx2 = getelementptr inbounds i32, ptr %3, i64 %idxprom
  %4 = load i32, ptr %arrayidx2, align 4, !tbaa !23
  %conv = sitofp i32 %4 to double
  %add = fadd fast double %2, %conv
  ret double %add
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!1, !2, !3, !4}
!intel.dtrans.types = !{!5, !8}
!llvm.ident = !{!10}

!0 = !{%struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 2, !6, !7}
!6 = !{i32 0, i32 1}
!7 = !{%struct._ZTSN10TestStructUt_E.anon zeroinitializer, i32 0}
!8 = !{!"S", %struct._ZTSN10TestStructUt_E.anon zeroinitializer, i32 1, !9}
!9 = !{double 0.000000e+00, i32 1}
!10 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!11 = !{!12, !12, i64 0}
!12 = !{!"pointer@_ZTSP10TestStruct", !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C++ TBAA"}
!15 = !{!16, !19, i64 8}
!16 = !{!"struct@_ZTS10TestStruct", !17, i64 0, !18, i64 8}
!17 = !{!"pointer@_ZTSPi", !13, i64 0}
!18 = !{!"struct@_ZTSN10TestStructUt_E", !19, i64 0}
!19 = !{!"pointer@_ZTSPd", !13, i64 0}
!20 = !{!21, !21, i64 0}
!21 = !{!"double", !13, i64 0}
!22 = !{!16, !17, i64 0}
!23 = !{!24, !24, i64 0}
!24 = !{!"int", !13, i64 0}

; end INTEL_FEATURE_SW_DTRANS
