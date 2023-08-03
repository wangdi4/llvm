; INTEL_FEATURE_SW_DTRANS

; Input file for the test cases intel-merge-mangled11.ll and
; intel-merge-mangled11-debug.ll.

; file: simple.cpp
;   struct TestStruct {
;     int *arrptr[5];
;   };
;
;   int bar(int i);
;
;   int foo(TestStruct *T, int i) {
;     return T->arrptr[i][i] + bar(i);
;   }

; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10TestStruct.TestStruct = type { [5 x ptr] }

; Function Attrs: mustprogress uwtable
define dso_local i32 @_Z3fooP10TestStructi(ptr nocapture readonly "intel_dtrans_func_index"="1" %T, i32 %i) local_unnamed_addr #0 !intel.dtrans.func.type !8 {
entry:
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds %struct._ZTS10TestStruct.TestStruct, ptr %T, i64 0, i32 0, i64 %idxprom, !intel-tbaa !10
  %0 = load ptr, ptr %arrayidx, align 8, !tbaa !10
  %arrayidx2 = getelementptr inbounds i32, ptr %0, i64 %idxprom
  %1 = load i32, ptr %arrayidx2, align 4, !tbaa !16
  %call = tail call i32 @_Z3bari(i32 %i)
  %add = add nsw i32 %call, %1
  ret i32 %add
}

declare dso_local i32 @_Z3bari(i32) local_unnamed_addr #1

attributes #0 = { mustprogress uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}
!intel.dtrans.types = !{!4}
!llvm.ident = !{!7}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1, !5}
!5 = !{!"A", i32 5, !6}
!6 = !{i32 0, i32 1}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!8 = distinct !{!9}
!9 = !{%struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1}
!10 = !{!11, !13, i64 0}
!11 = !{!"struct@_ZTS10TestStruct", !12, i64 0}
!12 = !{!"array@_ZTSA5_Pi", !13, i64 0}
!13 = !{!"pointer@_ZTSPi", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C++ TBAA"}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !14, i64 0}

; end INTEL_FEATURE_SW_DTRANS