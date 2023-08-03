; INTEL_FEATURE_SW_DTRANS

; This is an input file for the test cases intel-merge-mangled08.ll
; and intel-merge-mangled08-debug.ll.

; simple.h:
;   #ifndef __TESTCLASS_H__
;   #define __TESTCLASS_H__
;
;   template <class T>
;   class TestClass {
;   public:
;     TestClass() { }
;     void setVal(T I) { val = I; }
;     T getVal() { return val; }
;
;   private:
;     T val;
;   };
;
;   #endif // __TESTCLASS_H__

; bas.cpp:
;   #include "simple.h"
;
;   void bas(TestClass<double> *T, double I) {
;     T->setVal(I);
;   }


; ModuleID = 'bar.cpp'
source_filename = "bar.cpp"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.16.27045"

%"class..?AV?$TestClass@N@@.TestClass" = type { double }

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable willreturn writeonly
define dso_local void @"?bas@@YAXPEAV?$TestClass@N@@N@Z"(ptr nocapture "intel_dtrans_func_index"="1" %T, double %I) local_unnamed_addr #0 !intel.dtrans.func.type !15 {
entry:
  %val.i = getelementptr inbounds %"class..?AV?$TestClass@N@@.TestClass", ptr %T, i64 0, i32 0, !intel-tbaa !17
  store double %I, ptr %val.i, align 8, !tbaa !17
  ret void
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind uwtable willreturn writeonly "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.linker.options = !{!0, !1, !2, !3, !4, !5}
!llvm.module.flags = !{!6, !7, !8, !9, !10, !11}
!intel.dtrans.types = !{!12}
!llvm.ident = !{!14}

!0 = !{!"/DEFAULTLIB:libcmt.lib"}
!1 = !{!"/DEFAULTLIB:libircmt.lib"}
!2 = !{!"/DEFAULTLIB:svml_dispmt.lib"}
!3 = !{!"/DEFAULTLIB:libdecimal.lib"}
!4 = !{!"/DEFAULTLIB:libmmt.lib"}
!5 = !{!"/DEFAULTLIB:oldnames.lib"}
!6 = !{i32 1, !"wchar_size", i32 2}
!7 = !{i32 1, !"Virtual Function Elim", i32 0}
!8 = !{i32 7, !"PIC Level", i32 2}
!9 = !{i32 7, !"uwtable", i32 1}
!10 = !{i32 1, !"ThinLTO", i32 0}
!11 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!12 = !{!"S", %"class..?AV?$TestClass@N@@.TestClass" zeroinitializer, i32 1, !13}
!13 = !{double 0.000000e+00, i32 0}
!14 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!15 = distinct !{!16}
!16 = !{%"class..?AV?$TestClass@N@@.TestClass" zeroinitializer, i32 1}
!17 = !{!18, !19, i64 0}
!18 = !{!"struct@?AV?$TestClass@N@@", !19, i64 0}
!19 = !{!"double", !20, i64 0}
!20 = !{!"omnipotent char", !21, i64 0}
!21 = !{!"Simple C++ TBAA"}

; end INTEL_FEATURE_SW_DTRANS