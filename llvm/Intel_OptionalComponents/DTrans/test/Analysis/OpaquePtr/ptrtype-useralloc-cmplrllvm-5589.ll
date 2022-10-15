; REQUIRES: asserts
; RUN: opt < %s -disable-output -debug-only=dtrans-alloc-collector -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -debug-only=dtrans-alloc-collector -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest 2>&1 | FileCheck %s
; CHECK: Analyzing for user free function: mybadfree
; CHECK: Not user free function: mybadfree - Unsupported function signature

target triple = "x86_64-unknown-linux-gnu"

; Check that the function @mybadfree is not recognized as a user alloc
; because the value freed is not the value used in the argument to
; @mybadfree.

%struct._ZTS4Arr0.Arr0 = type { i32, float**, i32 }

; Function Attrs: nounwind uwtable
define dso_local void @mybadfree(%struct._ZTS4Arr0.Arr0* noundef "intel_dtrans_func_index"="1" %this) !intel.dtrans.func.type !6 {
entry:
  %this.addr = alloca %struct._ZTS4Arr0.Arr0*, align 8, !intel_dtrans_type !7
  store %struct._ZTS4Arr0.Arr0* %this, %struct._ZTS4Arr0.Arr0** %this.addr, align 8, !tbaa !8
  %0 = load %struct._ZTS4Arr0.Arr0*, %struct._ZTS4Arr0.Arr0** %this.addr, align 8, !tbaa !8
  %field1 = getelementptr inbounds %struct._ZTS4Arr0.Arr0, %struct._ZTS4Arr0.Arr0* %0, i32 0, i32 1, !intel-tbaa !12
  %1 = load float**, float*** %field1, align 8, !tbaa !12
  %2 = bitcast float** %1 to i8*
  call void @free(i8* noundef %2)
  ret void
}

; Function Attrs: nounwind
declare !intel.dtrans.func.type !16 dso_local void @free(i8* noundef "intel_dtrans_func_index"="1") #0

attributes #0 = { allockind("free") "alloc-family"="malloc" }

!llvm.module.flags = !{!0, !1}
!intel.dtrans.types = !{!2}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"S", %struct._ZTS4Arr0.Arr0 zeroinitializer, i32 3, !3, !4, !3}
!3 = !{i32 0, i32 0}
!4 = !{float 0.000000e+00, i32 2}
!5 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!6 = distinct !{!7}
!7 = !{%struct._ZTS4Arr0.Arr0 zeroinitializer, i32 1}
!8 = !{!9, !9, i64 0}
!9 = !{!"pointer@_ZTSP4Arr0", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
!12 = !{!13, !15, i64 8}
!13 = !{!"struct@", !14, i64 0, !15, i64 8, !14, i64 16}
!14 = !{!"int", !10, i64 0}
!15 = !{!"pointer@_ZTSPPf", !10, i64 0}
!16 = distinct !{!17}
!17 = !{i8 0, i32 1}
