; INTEL_FEATURE_SW_DTRANS

; This is an input file for the test cases intel-merge-mangled06.ll
; and intel-merge-mangled06-debug.ll.

; bar.cpp:
;   struct TestStruct {
;     double i;
;     double j;
;   };
;
;   TestStruct globBar;
;
;   void initBar(double I) {
;     globBar.i = I;
;     globBar.j = I + 100.0;
;   }
;
;   double bar(double I) {
;     return globBar.i + globBar.j + I;
;   }

; ModuleID = 'bar.cpp'
source_filename = "bar.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10TestStruct.TestStruct = type { double, double }

@globBar = dso_local local_unnamed_addr global %struct._ZTS10TestStruct.TestStruct zeroinitializer, align 8

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable willreturn writeonly
define dso_local void @_Z7initBard(double %I) local_unnamed_addr #0 {
entry:
  store double %I, ptr getelementptr inbounds (%struct._ZTS10TestStruct.TestStruct, ptr @globBar, i64 0, i32 0), align 8, !tbaa !8
  %add = fadd fast double %I, 1.000000e+02
  store double %add, ptr getelementptr inbounds (%struct._ZTS10TestStruct.TestStruct, ptr @globBar, i64 0, i32 1), align 8, !tbaa !13
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn
define dso_local double @_Z3bard(double %I) local_unnamed_addr #1 {
entry:
  %0 = load double, ptr getelementptr inbounds (%struct._ZTS10TestStruct.TestStruct, ptr @globBar, i64 0, i32 0), align 8, !tbaa !8
  %1 = load double, ptr getelementptr inbounds (%struct._ZTS10TestStruct.TestStruct, ptr @globBar, i64 0, i32 1), align 8, !tbaa !13
  %add = fadd fast double %0, %I
  %add1 = fadd fast double %add, %1
  ret double %add1
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind uwtable willreturn writeonly "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{!5}
!llvm.ident = !{!7}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 2, !6, !6}
!6 = !{double 0.000000e+00, i32 0}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!8 = !{!9, !10, i64 0}
!9 = !{!"struct@_ZTS10TestStruct", !10, i64 0, !10, i64 8}
!10 = !{!"double", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C++ TBAA"}
!13 = !{!9, !10, i64 8}

; end INTEL_FEATURE_SW_DTRANS