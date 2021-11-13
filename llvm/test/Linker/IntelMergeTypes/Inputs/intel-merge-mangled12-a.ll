; INTEL_FEATURE_SW_DTRANS

; Input file for test case intel-merge-mangled12-debug.ll. It was created
; from the following test case, but the metadata related to TestStructB
; was removed for testing purposes.

; struct TestStructA {
;   int *i;
;   int *j;
; };
;
; struct TestStructB {
;   int *i;
;   int *j;
; };
;
; int bar(TestStructA *T, TestStructB *S, int in);
;
; int foo(TestStructA *T, TestStructB *S, int in) {
;   return T->i[in] + bar(T, S, in);
; }

; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS11TestStructA.TestStructA = type { i32*, i32* }
%struct._ZTS11TestStructB.TestStructB = type { i32*, i32* }

; Function Attrs: mustprogress uwtable
define dso_local i32 @_Z3fooP11TestStructAP11TestStructBi(%struct._ZTS11TestStructA.TestStructA* "intel_dtrans_func_index"="1" %T, %struct._ZTS11TestStructB.TestStructB* %S, i32 %in) local_unnamed_addr #0 !intel.dtrans.func.type !8 {
entry:
  %i = getelementptr inbounds %struct._ZTS11TestStructA.TestStructA, %struct._ZTS11TestStructA.TestStructA* %T, i64 0, i32 0, !intel-tbaa !11
  %0 = load i32*, i32** %i, align 8, !tbaa !11
  %idxprom = sext i32 %in to i64
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 %idxprom
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !16
  %call = tail call i32 @_Z3barP11TestStructAP11TestStructBi(%struct._ZTS11TestStructA.TestStructA* %T, %struct._ZTS11TestStructB.TestStructB* %S, i32 %in)
  %add = add nsw i32 %call, %1
  ret i32 %add
}

declare !intel.dtrans.func.type !18 dso_local i32 @_Z3barP11TestStructAP11TestStructBi(%struct._ZTS11TestStructA.TestStructA* "intel_dtrans_func_index"="1", %struct._ZTS11TestStructB.TestStructB*, i32) local_unnamed_addr #1

attributes #0 = { mustprogress uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}
!intel.dtrans.types = !{!4}
!llvm.ident = !{!7}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{!"S", %struct._ZTS11TestStructA.TestStructA zeroinitializer, i32 2, !5, !5}
!5 = !{i32 0, i32 1}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!8 = distinct !{!9}
!9 = !{%struct._ZTS11TestStructA.TestStructA zeroinitializer, i32 1}
!11 = !{!12, !13, i64 0}
!12 = !{!"struct@_ZTS11TestStructA", !13, i64 0, !13, i64 8}
!13 = !{!"pointer@_ZTSPi", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C++ TBAA"}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !14, i64 0}
!18 = distinct !{!9}

; end INTEL_FEATURE_SW_DTRANS