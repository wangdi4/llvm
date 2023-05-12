; INTEL_FEATURE_SW_DTRANS

; Input file for the test cases intel-merge-mangled11.ll and
; intel-merge-mangled11-debug.ll.

; file: simple2.cpp
;   struct TestStruct {
;     int arr[5];
;   };
;
;   TestStruct *glob;
;
;   int bar(int i) {
;     return glob->arr[i];
;   }

; ModuleID = 'simple2.cpp'
source_filename = "simple2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10TestStruct.TestStruct = type { [5 x i32] }

@glob = dso_local local_unnamed_addr global ptr null, align 8, !intel_dtrans_type !0

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn
define dso_local i32 @_Z3bari(i32 %i) local_unnamed_addr #0 {
entry:
  %0 = load ptr, ptr @glob, align 8, !tbaa !9
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds %struct._ZTS10TestStruct.TestStruct, ptr %0, i64 0, i32 0, i64 %idxprom, !intel-tbaa !13
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !13
  ret i32 %1
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!1, !2, !3, !4}
!intel.dtrans.types = !{!5}
!llvm.ident = !{!8}

!0 = !{%struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1, !6}
!6 = !{!"A", i32 5, !7}
!7 = !{i32 0, i32 0}
!8 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!9 = !{!10, !10, i64 0}
!10 = !{!"pointer@_ZTSP10TestStruct", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C++ TBAA"}
!13 = !{!14, !16, i64 0}
!14 = !{!"struct@_ZTS10TestStruct", !15, i64 0}
!15 = !{!"array@_ZTSA5_i", !16, i64 0}
!16 = !{!"int", !11, i64 0}

; end INTEL_FEATURE_SW_DTRANS