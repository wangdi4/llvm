; INTEL_FEATURE_SW_DTRANS

; This is an input file for the test cases intel-merge-mangled04.ll
; and intel-merge-mangled04-debug.ll.

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
;     union {
;       T valInner;
;       int valInt;
;       double valDouble;
;     };
;   };
;
;   #endif // __TESTCLASS_H__

; foo.cpp:
;   #include "simple.h"
;
;   void foo(TestClass<int> *T, int I) {
;     T->setVal(I);
;   }

; ModuleID = 'foo.cpp'
source_filename = "foo.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class._ZTS9TestClassIiE.TestClass = type { i32, %union._ZTSN9TestClassIiEUt_E.anon }
%union._ZTSN9TestClassIiEUt_E.anon = type { double }

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable willreturn writeonly
define dso_local void @_Z3fooP9TestClassIiEi(ptr nocapture "intel_dtrans_func_index"="1" %T, i32 %I) local_unnamed_addr #0 !intel.dtrans.func.type !11 {
entry:
  %val.i = getelementptr inbounds %class._ZTS9TestClassIiE.TestClass, ptr %T, i64 0, i32 0, !intel-tbaa !13
  store i32 %I, ptr %val.i, align 8, !tbaa !13
  ret void
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind uwtable willreturn writeonly "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{!5, !8}
!llvm.ident = !{!10}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %class._ZTS9TestClassIiE.TestClass zeroinitializer, i32 2, !6, !7}
!6 = !{i32 0, i32 0}
!7 = !{%union._ZTSN9TestClassIiEUt_E.anon zeroinitializer, i32 0}
!8 = !{!"S", %union._ZTSN9TestClassIiEUt_E.anon zeroinitializer, i32 1, !9}
!9 = !{double 0.000000e+00, i32 0}
!10 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!11 = distinct !{!12}
!12 = !{%class._ZTS9TestClassIiE.TestClass zeroinitializer, i32 1}
!13 = !{!14, !15, i64 0}
!14 = !{!"struct@_ZTS9TestClassIiE", !15, i64 0, !16, i64 8}
!15 = !{!"int", !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C++ TBAA"}

; end INTEL_FEATURE_SW_DTRANS