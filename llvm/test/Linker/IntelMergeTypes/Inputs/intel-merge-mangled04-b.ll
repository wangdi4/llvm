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

; simple2.cpp
;   #include "simple.h"
;
;   void foo(TestClass<int> *T, int I);
;   void bas(TestClass<double> *T, double I);
;
;   int barUpdate() {
;     TestClass<int> T;
;     foo(&T, 10);
;     return T.getVal();
;   }
;
;   double cazUpdate() {
;     TestClass<double> T;
;     bas(&T, 10.0);
;     return T.getVal();
;   }

; ModuleID = 'simple2.cpp'
source_filename = "simple2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class._ZTS9TestClassIiE.TestClass = type { i32, %union._ZTSN9TestClassIiEUt_E.anon }
%union._ZTSN9TestClassIiEUt_E.anon = type { double }
%class._ZTS9TestClassIdE.TestClass = type { double, %union._ZTSN9TestClassIdEUt_E.anon }
%union._ZTSN9TestClassIdEUt_E.anon = type { double }

; Function Attrs: uwtable
define dso_local i32 @_Z9barUpdatev() local_unnamed_addr #0 {
entry:
  %T = alloca %class._ZTS9TestClassIiE.TestClass, align 8
  %0 = bitcast ptr %T to ptr
  call void @llvm.lifetime.start.p0i8(i64 16, ptr nonnull %0) #3
  call void @_Z3fooP9TestClassIiEi(ptr nonnull %T, i32 10)
  %val.i = getelementptr inbounds %class._ZTS9TestClassIiE.TestClass, ptr %T, i64 0, i32 0, !intel-tbaa !14
  %1 = load i32, ptr %val.i, align 8, !tbaa !14
  call void @llvm.lifetime.end.p0i8(i64 16, ptr nonnull %0) #3
  ret i32 %1
}

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, ptr nocapture) #1

declare !intel.dtrans.func.type !19 dso_local void @_Z3fooP9TestClassIiEi(ptr "intel_dtrans_func_index"="1", i32) local_unnamed_addr #2

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, ptr nocapture) #1

; Function Attrs: uwtable
define dso_local double @_Z9cazUpdatev() local_unnamed_addr #0 {
entry:
  %T = alloca %class._ZTS9TestClassIdE.TestClass, align 8
  %0 = bitcast ptr %T to ptr
  call void @llvm.lifetime.start.p0i8(i64 16, ptr nonnull %0) #3
  call void @_Z3basP9TestClassIdEd(ptr nonnull %T, double 1.000000e+01)
  %val.i = getelementptr inbounds %class._ZTS9TestClassIdE.TestClass, ptr %T, i64 0, i32 0, !intel-tbaa !21
  %1 = load double, ptr %val.i, align 8, !tbaa !21
  call void @llvm.lifetime.end.p0i8(i64 16, ptr nonnull %0) #3
  ret double %1
}

declare !intel.dtrans.func.type !24 dso_local void @_Z3basP9TestClassIdEd(ptr "intel_dtrans_func_index"="1", double) local_unnamed_addr #2

attributes #0 = { uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly mustprogress nofree nosync nounwind willreturn }
attributes #2 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{!5, !8, !10, !12}
!llvm.ident = !{!13}

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
!10 = !{!"S", %class._ZTS9TestClassIdE.TestClass zeroinitializer, i32 2, !9, !11}
!11 = !{%union._ZTSN9TestClassIdEUt_E.anon zeroinitializer, i32 0}
!12 = !{!"S", %union._ZTSN9TestClassIdEUt_E.anon zeroinitializer, i32 1, !9}
!13 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!14 = !{!15, !16, i64 0}
!15 = !{!"struct@_ZTS9TestClassIiE", !16, i64 0, !17, i64 8}
!16 = !{!"int", !17, i64 0}
!17 = !{!"omnipotent char", !18, i64 0}
!18 = !{!"Simple C++ TBAA"}
!19 = distinct !{!20}
!20 = !{%class._ZTS9TestClassIiE.TestClass zeroinitializer, i32 1}
!21 = !{!22, !23, i64 0}
!22 = !{!"struct@_ZTS9TestClassIdE", !23, i64 0, !17, i64 8}
!23 = !{!"double", !17, i64 0}
!24 = distinct !{!25}
!25 = !{%class._ZTS9TestClassIdE.TestClass zeroinitializer, i32 1}

; end INTEL_FEATURE_SW_DTRANS