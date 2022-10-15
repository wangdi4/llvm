; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed  -passes='require<dtrans-safetyanalyzer>' -dtrans-usecrulecompat -disable-output -debug-only=dtrans-crc 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that %struct._ZTS2S1.S1 and %struct._ZTS2S2.S2 are not recognized as
; compatible using the c-rule.

; CHECK: dtrans-crc: NO  %struct._ZTS2S1.S1* %struct._ZTS2S2.S2*

%struct._ZTS2S1.S1 = type { ptr, ptr, ptr }
%struct._ZTS2S2.S2 = type { ptr, ptr, ptr }
%struct._ZTS2T1.T1 = type { i32 }
%struct._ZTS2T2.T2 = type { i32, i32 }

@myarg = dso_local global %struct._ZTS2S1.S1 zeroinitializer, align 8
@myfp = dso_local global ptr null, align 8, !intel_dtrans_type !0

; Function Attrs: nounwind uwtable
define dso_local i32 @target1(ptr noundef "intel_dtrans_func_index"="1" %arg) #0 !intel.dtrans.func.type !13 {
entry:
  %arg.addr = alloca ptr, align 8, !intel_dtrans_type !3
  store ptr %arg, ptr %arg.addr, align 8, !tbaa !14
  %i = load ptr, ptr %arg.addr, align 8, !tbaa !14
  %field0 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %i, i32 0, i32 0, !intel-tbaa !18
  %i1 = load ptr, ptr %field0, align 8, !tbaa !18
  %cmp = icmp ne ptr %i1, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; Function Attrs: nounwind uwtable
define dso_local i32 @target2(ptr noundef "intel_dtrans_func_index"="1" %arg) #0 !intel.dtrans.func.type !22 {
entry:
  %arg.addr = alloca ptr, align 8, !intel_dtrans_type !23
  store ptr %arg, ptr %arg.addr, align 8, !tbaa !24
  %i = load ptr, ptr %arg.addr, align 8, !tbaa !24
  %field1 = getelementptr inbounds %struct._ZTS2S2.S2, ptr %i, i32 0, i32 1, !intel-tbaa !26
  %i1 = load ptr, ptr %field1, align 8, !tbaa !26
  %cmp = icmp ne ptr %i1, null
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %i = load ptr, ptr @myfp, align 8, !tbaa !28
  %call = call i32 %i(ptr noundef @myarg), !intel_dtrans_type !1
  ret i32 %call
}

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!4, !5}
!intel.dtrans.types = !{!6, !7, !8, !11}
!llvm.ident = !{!12}

!0 = !{!1, i32 1}
!1 = !{!"F", i1 false, i32 1, !2, !3}
!2 = !{i32 0, i32 0}
!3 = !{%struct._ZTS2S1.S1 zeroinitializer, i32 1}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"uwtable", i32 2}
!6 = !{!"S", %struct._ZTS2T1.T1 zeroinitializer, i32 1, !2}
!7 = !{!"S", %struct._ZTS2T2.T2 zeroinitializer, i32 2, !2, !2}
!8 = !{!"S", %struct._ZTS2S1.S1 zeroinitializer, i32 3, !9, !10, !9}
!9 = !{%struct._ZTS2T1.T1 zeroinitializer, i32 1}
!10 = !{%struct._ZTS2T2.T2 zeroinitializer, i32 1}
!11 = !{!"S", %struct._ZTS2S2.S2 zeroinitializer, i32 3, !9, !10, !10}
!12 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!13 = distinct !{!3}
!14 = !{!15, !15, i64 0}
!15 = !{!"pointer@_ZTSP2S1", !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C/C++ TBAA"}
!18 = !{!19, !20, i64 0}
!19 = !{!"struct@", !20, i64 0, !21, i64 8, !20, i64 16}
!20 = !{!"pointer@_ZTSP2T1", !16, i64 0}
!21 = !{!"pointer@_ZTSP2T2", !16, i64 0}
!22 = distinct !{!23}
!23 = !{%struct._ZTS2S2.S2 zeroinitializer, i32 1}
!24 = !{!25, !25, i64 0}
!25 = !{!"pointer@_ZTSP2S2", !16, i64 0}
!26 = !{!27, !21, i64 8}
!27 = !{!"struct@", !20, i64 0, !21, i64 8, !21, i64 16}
!28 = !{!29, !29, i64 0}
!29 = !{!"pointer@_ZTSPFiP2S1E", !16, i64 0}
