; REQUIRES: asserts

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output -dtrans-test-padded-structs-analyzer 2>&1 | FileCheck %s

; This test verifies that %struct.test.a and %struct.test.a.base are mapped
; but the padded field is marked as dirty. The reason is because there is
; a read for the field that is reserved for padding but the option
; dtrans-test-padded-structs-analyzer is enabled.

; CHECK-LABEL: LLVMType: %struct.test.a = type { i32, i32, [4 x i8] }
; CHECK: Related base structure: struct.test.a.base
; CHECK: 2)Field LLVM Type: [4 x i8]
; CHECK: Field info: {{.*}}Dirty PaddedField{{.*}}
; CHECK: Bottom Alloc Function
; CHECK: Safety data: {{.*}}Structure may have ABI padding{{.*}}

; CHECK-LABEL: LLVMType: %struct.test.a.base = type { i32, i32 }
; CHECK: Related padded structure: struct.test.a
; CHECK: Safety data: {{.*}}Structure could be base for ABI padding{{.*}}

; ModuleID = 'simple2.cpp'
source_filename = "simple2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.b = type { %struct.test.a.base, [4 x i8] }
%struct.test.a = type { i32, i32, [4 x i8] }
%struct.test.a.base = type { i32, i32 }

; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef i8 @test(ptr noundef "intel_dtrans_func_index"="1" %ptr) local_unnamed_addr #0 !intel.dtrans.func.type !11 {
entry:
  %tmp1 = getelementptr inbounds %struct.test.b, ptr %ptr, i64 0, i32 0
  %tmp2 = getelementptr inbounds %struct.test.a, ptr %tmp1, i64 0, i32 2
  %tmp3 = getelementptr inbounds [4 x i8], ptr %tmp2, i64 0, i32 0
  %tmp4 = load i8, ptr %tmp3
  ret i8 %tmp4
}

attributes #0 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}
!intel.dtrans.types = !{!4, !7, !14}
!llvm.ident = !{!10}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{!"S", %struct.test.b zeroinitializer, i32 2, !13, !8}
!5 = !{%struct.test.a zeroinitializer, i32 0}
!6 = !{i32 0, i32 0}
!7 = !{!"S", %struct.test.a zeroinitializer, i32 3, !6, !6, !8}
!8 = !{!"A", i32 4, !9}
!9 = !{i8 0, i32 0}
!10 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!11 = distinct !{!12}
!12 = !{%struct.test.b zeroinitializer, i32 1}
!13 = !{%struct.test.a.base zeroinitializer, i32 0}
!14 = !{!"S", %struct.test.a.base zeroinitializer, i32 2, !6, !6}