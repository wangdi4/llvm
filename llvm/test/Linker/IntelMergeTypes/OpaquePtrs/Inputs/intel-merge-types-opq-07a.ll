; INTEL_FEATURE_SW_DTRANS

; Input file for the test case intel-merge-types-opq-07.ll. It represents
; the following C code generated with opaque pointers:

; struct TestStruct {
;   int *ptr;
; };
;
; double bar(int i);
; int bas(int i);
;
; int foo(TestStruct *T, int i) {
;   return T->ptr[i] + (int)bar(i) + bas(i);
; }

; ModuleID = 'simple.ll'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10TestStruct.TestStruct = type { ptr }

; Function Attrs: mustprogress uwtable
define dso_local i32 @_Z3fooP10TestStructi(ptr nocapture readonly "intel_dtrans_func_index"="1" %T, i32 %i) local_unnamed_addr #0 !intel.dtrans.func.type !7 {
entry:
  %ptr = getelementptr inbounds %struct._ZTS10TestStruct.TestStruct, ptr %T, i64 0, i32 0, !intel-tbaa !9
  %0 = load ptr, ptr %ptr, align 8, !tbaa !9
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i32, ptr %0, i64 %idxprom
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !14
  %call = tail call fast double @_Z3bari(i32 %i)
  %conv = fptosi double %call to i32
  %add = add nsw i32 %1, %conv
  %call1 = tail call i32 @_Z3basi(i32 %i)
  %add2 = add nsw i32 %add, %call1
  ret i32 %add2
}

declare dso_local double @_Z3bari(i32) local_unnamed_addr #1

declare dso_local i32 @_Z3basi(i32) local_unnamed_addr #1

attributes #0 = { mustprogress uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}
!intel.dtrans.types = !{!4}
!llvm.ident = !{!6}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1, !5}
!5 = !{i32 0, i32 1}
!6 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!7 = distinct !{!8}
!8 = !{%struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1}
!9 = !{!10, !11, i64 0}
!10 = !{!"struct@_ZTS10TestStruct", !11, i64 0}
!11 = !{!"pointer@_ZTSPi", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C++ TBAA"}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !12, i64 0}

; end INTEL_FEATURE_SW_DTRANS