; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK-NEXT: LLVMType: %struct._ZTS8MYSTRUCT.MYSTRUCT = type { ptr }
; CHECK: Safety data: Mismatched element access{{ *$}}
; CHECK-NEXT:  End LLVMType: %struct._ZTS8MYSTRUCT.MYSTRUCT = type { ptr }

; Check that reading an i8* pointer from a struct and passing it to a call
; argument that is i16* yields a mismatched element access on the struct.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS8MYSTRUCT.MYSTRUCT = type { ptr }

@myglobal = internal global i16 0, align 2

; Function Attrs: nounwind uwtable
define internal i32 @goo(ptr noundef "intel_dtrans_func_index"="1" %garg) #0 !intel.dtrans.func.type !5 {
entry:
  %pointer = getelementptr inbounds %struct._ZTS8MYSTRUCT.MYSTRUCT, ptr %garg, i32 0, i32 0, !intel-tbaa !7
  %i = load ptr, ptr %pointer, align 8, !tbaa !7
  call void @foo(ptr noundef %i)
  ret i32 0
}

; Function Attrs: nounwind uwtable
define internal void @foo(ptr noundef "intel_dtrans_func_index"="1" %farg) #0 !intel.dtrans.func.type !12 {
entry:
  %i = load i16, ptr %farg, align 2, !tbaa !14
  store i16 %i, ptr @myglobal, align 2, !tbaa !14
  ret void
}

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!intel.dtrans.types = !{!2}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"S", %struct._ZTS8MYSTRUCT.MYSTRUCT zeroinitializer, i32 1, !3}
!3 = !{i8 0, i32 1}
!4 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!5 = distinct !{!6}
!6 = !{%struct._ZTS8MYSTRUCT.MYSTRUCT zeroinitializer, i32 1}
!7 = !{!8, !9, i64 0}
!8 = !{!"struct@", !9, i64 0}
!9 = !{!"pointer@_ZTSPc", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
!12 = distinct !{!13}
!13 = !{i16 0, i32 1}
!14 = !{!15, !15, i64 0}
!15 = !{!"short", !10, i64 0}
