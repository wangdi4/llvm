; INTEL_FEATURE_SW_DTRANS

; Input file for the test case intel-merge-types-opq-02.ll. It represents
; the following C code generated with opaque pointers:

;  #ifndef __TESTCLASS_H__
;  #define __TESTCLASS_H__
;
;  template <class T>
;  class TestClass {
;  public:
;    TestClass() { }
;    void setVal(T I, int j) { val[j] = I; }
;    T getVal(int i) { return val[i]; }
;
;  private:
;    T *val;
;  };
;
;  #endif // __TESTCLASS_H__

; #include "simple.h"
;
; TestClass<int> *globInt;
;
; void fooInt(int I, int j) {
;   return globInt->setVal(I, j);
; }
;
; int barInt(int i) {
;   return globInt->getVal(i);
; }

; ModuleID = 'simple.ll'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class._ZTS9TestClassIiE.TestClass = type { ptr }

@globInt = dso_local local_unnamed_addr global ptr null, align 8, !intel_dtrans_type !0

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable willreturn
define dso_local void @_Z6fooIntii(i32 %I, i32 %j) local_unnamed_addr #0 {
entry:
  %0 = load ptr, ptr @globInt, align 8, !tbaa !8
  %val.i = getelementptr inbounds %class._ZTS9TestClassIiE.TestClass, ptr %0, i64 0, i32 0, !intel-tbaa !12
  %1 = load ptr, ptr %val.i, align 8, !tbaa !12
  %idxprom.i = sext i32 %j to i64
  %arrayidx.i = getelementptr inbounds i32, ptr %1, i64 %idxprom.i
  store i32 %I, ptr %arrayidx.i, align 4, !tbaa !15
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn
define dso_local i32 @_Z6barInti(i32 %i) local_unnamed_addr #1 {
entry:
  %0 = load ptr, ptr @globInt, align 8, !tbaa !8
  %val.i = getelementptr inbounds %class._ZTS9TestClassIiE.TestClass, ptr %0, i64 0, i32 0, !intel-tbaa !12
  %1 = load ptr, ptr %val.i, align 8, !tbaa !12
  %idxprom.i = sext i32 %i to i64
  %arrayidx.i = getelementptr inbounds i32, ptr %1, i64 %idxprom.i
  %2 = load i32, ptr %arrayidx.i, align 4, !tbaa !15
  ret i32 %2
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!1, !2, !3, !4}
!intel.dtrans.types = !{!5}
!llvm.ident = !{!7}

!0 = !{%class._ZTS9TestClassIiE.TestClass zeroinitializer, i32 1}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %class._ZTS9TestClassIiE.TestClass zeroinitializer, i32 1, !6}
!6 = !{i32 0, i32 1}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!8 = !{!9, !9, i64 0}
!9 = !{!"pointer@_ZTSP9TestClassIiE", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C++ TBAA"}
!12 = !{!13, !14, i64 0}
!13 = !{!"struct@_ZTS9TestClassIiE", !14, i64 0}
!14 = !{!"pointer@_ZTSPi", !10, i64 0}
!15 = !{!16, !16, i64 0}
!16 = !{!"int", !10, i64 0}

; end INTEL_FEATURE_SW_DTRANS