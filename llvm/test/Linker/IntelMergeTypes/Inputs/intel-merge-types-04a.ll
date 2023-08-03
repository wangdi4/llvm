; INTEL_FEATURE_SW_DTRANS

; This is an input file for the test cases intel-merge-types-opq-04.ll
; and intel-merge-types-opq-04-debug.ll.

; ModuleID = 'simple.ll'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10TestStruct.TestStruct = type { ptr, %struct._ZTSN10TestStructUt_E.anon }
%struct._ZTSN10TestStructUt_E.anon = type { ptr }

; Function Attrs: mustprogress uwtable
define dso_local i32 @_Z3fooP10TestStructi(ptr nocapture readonly "intel_dtrans_func_index"="1" %T, i32 %i) local_unnamed_addr #0 !intel.dtrans.func.type !9 {
entry:
  %ptr = getelementptr inbounds %struct._ZTS10TestStruct.TestStruct, ptr %T, i64 0, i32 0, !intel-tbaa !11
  %0 = load ptr, ptr %ptr, align 8, !tbaa !11
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i32, ptr %0, i64 %idxprom
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !17
  %inner_ptr = getelementptr inbounds %struct._ZTS10TestStruct.TestStruct, ptr %T, i64 0, i32 1, i32 0, !intel-tbaa !19
  %2 = load ptr, ptr %inner_ptr, align 8, !tbaa !19
  %arrayidx2 = getelementptr inbounds i32, ptr %2, i64 %idxprom
  %3 = load i32, ptr %arrayidx2, align 4, !tbaa !17
  %add = add nsw i32 %3, %1
  %call = tail call fast double @_Z3bari(i32 %i)
  %conv = fptosi double %call to i32
  %add3 = add nsw i32 %add, %conv
  ret i32 %add3
}

declare dso_local double @_Z3bari(i32) local_unnamed_addr #1

attributes #0 = { mustprogress uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}
!intel.dtrans.types = !{!4, !7}
!llvm.ident = !{!8}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 2, !5, !6}
!5 = !{i32 0, i32 1}
!6 = !{%struct._ZTSN10TestStructUt_E.anon zeroinitializer, i32 0}
!7 = !{!"S", %struct._ZTSN10TestStructUt_E.anon zeroinitializer, i32 1, !5}
!8 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!9 = distinct !{!10}
!10 = !{%struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1}
!11 = !{!12, !13, i64 0}
!12 = !{!"struct@_ZTS10TestStruct", !13, i64 0, !16, i64 8}
!13 = !{!"pointer@_ZTSPi", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C++ TBAA"}
!16 = !{!"struct@_ZTSN10TestStructUt_E", !13, i64 0}
!17 = !{!18, !18, i64 0}
!18 = !{!"int", !14, i64 0}
!19 = !{!12, !13, i64 8}

; end INTEL_FEATURE_SW_DTRANS