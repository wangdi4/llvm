; INTEL_FEATURE_SW_DTRANS

; Input file for test case intel-merge-mangled10.ll,
; intel-merge-mangled10-debug.ll and intel-merge-mangled10-debug-2.ll

; struct TestStruct {
;   int *i;
;   int *j;
; };
;
; int bar(TestStruct *T, int in);
;
; int foo(TestStruct *T, int in) {
;   return T->i[in] + bar(T, in);
; }

; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10TestStruct.TestStruct = type { i32*, i32* }

; Function Attrs: mustprogress uwtable
define dso_local i32 @_Z3fooP10TestStructi(%struct._ZTS10TestStruct.TestStruct* "intel_dtrans_func_index"="1" %T, i32 %in) local_unnamed_addr #0 !intel.dtrans.func.type !7 {
entry:
  %i = getelementptr inbounds %struct._ZTS10TestStruct.TestStruct, %struct._ZTS10TestStruct.TestStruct* %T, i64 0, i32 0, !intel-tbaa !9
  %0 = load i32*, i32** %i, align 8, !tbaa !9
  %idxprom = sext i32 %in to i64
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 %idxprom
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !14
  %call = tail call i32 @_Z3barP10TestStructi(%struct._ZTS10TestStruct.TestStruct* %T, i32 %in)
  %add = add nsw i32 %call, %1
  ret i32 %add
}

declare !intel.dtrans.func.type !16 dso_local i32 @_Z3barP10TestStructi(%struct._ZTS10TestStruct.TestStruct* "intel_dtrans_func_index"="1", i32) local_unnamed_addr #1

attributes #0 = { mustprogress uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

; NOTE: One field in the metadata !4 was removed intentionally in order to
; create incomplete metadata.

!llvm.module.flags = !{!0, !1, !2, !3}
!intel.dtrans.types = !{!4}
!llvm.ident = !{!6}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 2, !5}
!5 = !{i32 0, i32 1}
!6 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!7 = distinct !{!8}
!8 = !{%struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1}
!9 = !{!10, !11, i64 0}
!10 = !{!"struct@_ZTS10TestStruct", !11, i64 0, !11, i64 8}
!11 = !{!"pointer@_ZTSPi", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C++ TBAA"}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !12, i64 0}
!16 = distinct !{!8}

; end INTEL_FEATURE_SW_DTRANS
