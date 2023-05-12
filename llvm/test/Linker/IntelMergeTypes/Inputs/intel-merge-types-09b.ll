; INTEL_FEATURE_SW_DTRANS

; Input file for test case intel-merge-types-opq-09-debug.ll. It was created
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
; int bar(TestStructA *T, TestStructB *S, int in) {
;   return T->i[in] + S->i[in];
; }

; ModuleID = 'intel-merge-types-opq-09b.ll'
source_filename = "simple2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS11TestStructA.TestStructA = type { ptr, ptr }
%struct._ZTS11TestStructB.TestStructB = type { ptr, ptr }

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly willreturn uwtable
define dso_local i32 @_Z3barP11TestStructAP11TestStructBi(ptr nocapture readonly "intel_dtrans_func_index"="1" %T, ptr nocapture readonly %S, i32 %in) local_unnamed_addr #0 !intel.dtrans.func.type !7 {
entry:
  %i = getelementptr inbounds %struct._ZTS11TestStructA.TestStructA, ptr %T, i64 0, i32 0, !intel-tbaa !9
  %0 = load ptr, ptr %i, align 8, !tbaa !9
  %idxprom = sext i32 %in to i64
  %arrayidx = getelementptr inbounds i32, ptr %0, i64 %idxprom
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !14
  %i1 = getelementptr inbounds %struct._ZTS11TestStructB.TestStructB, ptr %S, i64 0, i32 0, !intel-tbaa !16
  %2 = load ptr, ptr %i1, align 8, !tbaa !16
  %arrayidx3 = getelementptr inbounds i32, ptr %2, i64 %idxprom
  %3 = load i32, ptr %arrayidx3, align 4, !tbaa !14
  %add = add nsw i32 %3, %1
  ret i32 %add
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readonly willreturn uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}
!intel.dtrans.types = !{!4}
!llvm.ident = !{!6}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{!"S", %struct._ZTS11TestStructA.TestStructA zeroinitializer, i32 2, !5, !5}
!5 = !{i32 0, i32 1}
!6 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!7 = distinct !{!8}
!8 = !{%struct._ZTS11TestStructA.TestStructA zeroinitializer, i32 1}
!9 = !{!10, !11, i64 0}
!10 = !{!"struct@_ZTS11TestStructA", !11, i64 0, !11, i64 8}
!11 = !{!"pointer@_ZTSPi", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C++ TBAA"}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !12, i64 0}
!16 = !{!17, !11, i64 0}
!17 = !{!"struct@_ZTS11TestStructB", !11, i64 0, !11, i64 8}


; end INTEL_FEATURE_SW_DTRANS