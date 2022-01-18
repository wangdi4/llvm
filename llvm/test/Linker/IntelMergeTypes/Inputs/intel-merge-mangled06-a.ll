; INTEL_FEATURE_SW_DTRANS

; This is an input file for the test cases intel-merge-mangled06.ll
; and intel-merge-mangled06-debug.ll.

; simple.cpp:
;   int foo(int I);
;   double bar(double I);
;
;   void initFoo(int I);
;   void initBar(double I);
;
;   int callFoo(int I) {
;     initFoo(I);
;     return foo(I);
;   }
;
;   double callBar(double I) {
;     initBar(I);
;     return bar(I);
;   }

; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress uwtable
define dso_local i32 @_Z7callFooi(i32 %I) local_unnamed_addr #0 {
entry:
  tail call void @_Z7initFooi(i32 %I)
  %call = tail call i32 @_Z3fooi(i32 %I)
  ret i32 %call
}

declare dso_local void @_Z7initFooi(i32) local_unnamed_addr #1

declare dso_local i32 @_Z3fooi(i32) local_unnamed_addr #1

; Function Attrs: mustprogress uwtable
define dso_local double @_Z7callBard(double %I) local_unnamed_addr #0 {
entry:
  tail call void @_Z7initBard(double %I)
  %call = tail call fast double @_Z3bard(double %I)
  ret double %call
}

declare dso_local void @_Z7initBard(double) local_unnamed_addr #1

declare dso_local double @_Z3bard(double) local_unnamed_addr #1

attributes #0 = { mustprogress uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}

; end INTEL_FEATURE_SW_DTRANS