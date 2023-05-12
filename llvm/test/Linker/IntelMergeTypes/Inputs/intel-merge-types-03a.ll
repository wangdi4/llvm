; INTEL_FEATURE_SW_DTRANS

; This is an input file for the test cases intel-merge-types-opq-03.ll
; and intel-merge-types-opq-03.ll.

; simple.h
;   #ifndef __SIMPLE_H__
;   #define __SIMPLE_H__
;
;   struct TestStructA;
;   using FPType = int(*)(TestStructA*);
;
;   struct TestStructA{
;     FPType F;
;   };
;
;   #endif // __SIMPLE_H__

; simple.cpp
;   #include "simple.h"
;
;   int func1(TestStructA* T) {
;     return T->F(T);
;   }


; The IR with pointers that have types is the following:

; %struct._ZTS11TestStructA.TestStructA = type { %"__Intel$Empty$Struct"* }
; %"__Intel$Empty$Struct" = type {}
;
; ; Function Attrs: mustprogress uwtable
; define dso_local i32 @_Z5func1P11TestStructA(%struct._ZTS11TestStructA.TestStructA* "intel_dtrans_func_index"="1" %T) local_unnamed_addr #0 !intel.dtrans.func.type !11 {
; entry:
;   %F1 = bitcast %struct._ZTS11TestStructA.TestStructA* %T to i32 (%struct._ZTS11TestStructA.TestStructA*)**
;   %0 = load i32 (%struct._ZTS11TestStructA.TestStructA*)*, i32 (%struct._ZTS11TestStructA.TestStructA*)** %F1, align 8, !tbaa !12
;   %call = tail call i32 %0(%struct._ZTS11TestStructA.TestStructA* %T), !intel_dtrans_type !7
;   ret i32 %call
; }


; ModuleID = 'intel-merge-types-opq-03a-no.ll'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS11TestStructA.TestStructA = type { ptr }

; Function Attrs: mustprogress uwtable
define dso_local i32 @_Z5func1P11TestStructA(ptr "intel_dtrans_func_index"="1" %T) local_unnamed_addr #0 !intel.dtrans.func.type !11 {
entry:
  %F1 = bitcast ptr %T to ptr
  %0 = load ptr, ptr %F1, align 8, !tbaa !12
  %call = tail call i32 %0(ptr %T), !intel_dtrans_type !7
  ret i32 %call
}

attributes #0 = { mustprogress uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{!5}
!llvm.ident = !{!10}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %struct._ZTS11TestStructA.TestStructA zeroinitializer, i32 1, !6}
!6 = !{!7, i32 1}
!7 = !{!"F", i1 false, i32 1, !8, !9}
!8 = !{i32 0, i32 0}
!9 = !{%struct._ZTS11TestStructA.TestStructA zeroinitializer, i32 1}
!10 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!11 = distinct !{!9}
!12 = !{!13, !14, i64 0}
!13 = !{!"struct@_ZTS11TestStructA", !14, i64 0}
!14 = !{!"pointer@_ZTSPFiP11TestStructAE", !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C++ TBAA"}

; end INTEL_FEATURE_SW_DTRANS