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

; simple.cpp:
;   #include "simple.h"
;
;   void foo(TestClass<int> *T, int I);
;   void bas(TestClass<double> *T, double I);
;
;   int bar() {
;     TestClass<int> T;
;     foo(&T, 10);
;     return T.getVal();
;   }
;
;   double caz() {
;     TestClass<double> T;
;     bas(&T, 10.0);
;     return T.getVal();
;   }

; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.16.27045"

%"class..?AV?$TestClass@H@@.TestClass" = type { i32 }
%"class..?AV?$TestClass@N@@.TestClass" = type { double }

; Function Attrs: uwtable
define dso_local i32 @"?bar@@YAHXZ"() local_unnamed_addr #0 {
entry:
  %T = alloca %"class..?AV?$TestClass@H@@.TestClass", align 4
  %0 = bitcast ptr %T to ptr
  call void @llvm.lifetime.start.p0i8(i64 4, ptr nonnull %0) #3
  call void @"?foo@@YAXPEAV?$TestClass@H@@H@Z"(ptr nonnull %T, i32 10)
  %val.i = getelementptr inbounds %"class..?AV?$TestClass@H@@.TestClass", ptr %T, i64 0, i32 0, !intel-tbaa !17
  %1 = load i32, ptr %val.i, align 4, !tbaa !17
  call void @llvm.lifetime.end.p0i8(i64 4, ptr nonnull %0) #3
  ret i32 %1
}

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, ptr nocapture) #1

declare !intel.dtrans.func.type !22 dso_local void @"?foo@@YAXPEAV?$TestClass@H@@H@Z"(ptr "intel_dtrans_func_index"="1", i32) local_unnamed_addr #2

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, ptr nocapture) #1

; Function Attrs: uwtable
define dso_local double @"?caz@@YANXZ"() local_unnamed_addr #0 {
entry:
  %T = alloca %"class..?AV?$TestClass@N@@.TestClass", align 8
  %0 = bitcast ptr %T to ptr
  call void @llvm.lifetime.start.p0i8(i64 8, ptr nonnull %0) #3
  call void @"?bas@@YAXPEAV?$TestClass@N@@N@Z"(ptr nonnull %T, double 1.000000e+01)
  %val.i = getelementptr inbounds %"class..?AV?$TestClass@N@@.TestClass", ptr %T, i64 0, i32 0, !intel-tbaa !24
  %1 = load double, ptr %val.i, align 8, !tbaa !24
  call void @llvm.lifetime.end.p0i8(i64 8, ptr nonnull %0) #3
  ret double %1
}

declare !intel.dtrans.func.type !27 dso_local void @"?bas@@YAXPEAV?$TestClass@N@@N@Z"(ptr "intel_dtrans_func_index"="1", double) local_unnamed_addr #2

attributes #0 = { uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly mustprogress nofree nosync nounwind willreturn }
attributes #2 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.linker.options = !{!0, !1, !2, !3, !4, !5}
!llvm.module.flags = !{!6, !7, !8, !9, !10, !11}
!intel.dtrans.types = !{!12, !14}
!llvm.ident = !{!16}

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
!12 = !{!"S", %"class..?AV?$TestClass@H@@.TestClass" zeroinitializer, i32 1, !13}
!13 = !{i32 0, i32 0}
!14 = !{!"S", %"class..?AV?$TestClass@N@@.TestClass" zeroinitializer, i32 1, !15}
!15 = !{double 0.000000e+00, i32 0}
!16 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!17 = !{!18, !19, i64 0}
!18 = !{!"struct@?AV?$TestClass@H@@", !19, i64 0}
!19 = !{!"int", !20, i64 0}
!20 = !{!"omnipotent char", !21, i64 0}
!21 = !{!"Simple C++ TBAA"}
!22 = distinct !{!23}
!23 = !{%"class..?AV?$TestClass@H@@.TestClass" zeroinitializer, i32 1}
!24 = !{!25, !26, i64 0}
!25 = !{!"struct@?AV?$TestClass@N@@", !26, i64 0}
!26 = !{!"double", !20, i64 0}
!27 = distinct !{!28}
!28 = !{%"class..?AV?$TestClass@N@@.TestClass" zeroinitializer, i32 1}

; end INTEL_FEATURE_SW_DTRANS