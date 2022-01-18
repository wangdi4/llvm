; INTEL_FEATURE_SW_DTRANS

; Input file for test case for intel-merge-mangled09.ll.
; It was made from the following source:

; file: simple.cpp
;   struct TestStruct {
;     union {
;       int i;
;       char d;
;     };
;
;     union {
;       int j;
;       char c;
;     };
;   };
;
;   int bar(TestStruct *T);
;
;   int foo(TestStruct *T) {
;     return T->i + T->j + bar(T);
;   }


; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10TestStruct.TestStruct = type { %union._ZTSN10TestStructUt_E.anon, %union._ZTSN10TestStructUt_E.anon.0 }
%union._ZTSN10TestStructUt_E.anon = type { i32 }
%union._ZTSN10TestStructUt_E.anon.0 = type { i32 }

; Function Attrs: mustprogress uwtable
define dso_local i32 @_Z3fooP10TestStruct(%struct._ZTS10TestStruct.TestStruct* "intel_dtrans_func_index"="1" %T) local_unnamed_addr #0 !intel.dtrans.func.type !11 {
entry:
  %i = getelementptr inbounds %struct._ZTS10TestStruct.TestStruct, %struct._ZTS10TestStruct.TestStruct* %T, i64 0, i32 0, i32 0
  %0 = load i32, i32* %i, align 4, !tbaa !13
  %j = getelementptr inbounds %struct._ZTS10TestStruct.TestStruct, %struct._ZTS10TestStruct.TestStruct* %T, i64 0, i32 1, i32 0
  %1 = load i32, i32* %j, align 4, !tbaa !13
  %add = add nsw i32 %1, %0
  %call = tail call i32 @_Z3barP10TestStruct(%struct._ZTS10TestStruct.TestStruct* %T)
  %add1 = add nsw i32 %add, %call
  ret i32 %add1
}

declare !intel.dtrans.func.type !16 dso_local i32 @_Z3barP10TestStruct(%struct._ZTS10TestStruct.TestStruct* "intel_dtrans_func_index"="1") local_unnamed_addr #1

attributes #0 = { mustprogress uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}
!intel.dtrans.types = !{!4, !7, !9}
!llvm.ident = !{!10}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{!"S", %struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 2, !5, !6}
!5 = !{%union._ZTSN10TestStructUt_E.anon zeroinitializer, i32 0}
!6 = !{%union._ZTSN10TestStructUt_E.anon.0 zeroinitializer, i32 0}
!7 = !{!"S", %union._ZTSN10TestStructUt_E.anon zeroinitializer, i32 1, !8}
!8 = !{i32 0, i32 0}
!9 = !{!"S", %union._ZTSN10TestStructUt_E.anon.0 zeroinitializer, i32 1, !8}
!10 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!11 = distinct !{!12}
!12 = !{%struct._ZTS10TestStruct.TestStruct zeroinitializer, i32 1}
!13 = !{!14, !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C++ TBAA"}
!16 = distinct !{!12}

; end INTEL_FEATURE_SW_DTRANS
