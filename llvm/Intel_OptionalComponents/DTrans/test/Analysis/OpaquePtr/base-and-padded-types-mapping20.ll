; REQUIRES: asserts

; RUN: opt  < %s -opaque-pointers -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s
; RUN: opt  < %s -opaque-pointers -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s

; This test case checks that the base-padded relationship between
; %struct.test.a.base and %struct.test.a is set, but the relationship between
; %struct.test.b.base and %struct.test.b isn't set because there is an access
; to the padded field. Also, the safety bit BadCastingForRelatedTypes
; should be lowered as BadCasting. The goal of this test case is to check that
; BadCastingForRelatedTypes was lowered as BadCasting when the
; a structure in the nested chains failed the analysis for base-padded structures.

; CHECK: dtrans-safety: Bad casting (related types) -- Formal paremeter is a related type of the actual parameter, or vice-versa
; CHECK:  [test]   %ret = call i32 @foo(ptr %bptr)
; CHECK:  Arg#0:   %bptr = alloca %struct.test.c, align 8

; CHECK-LABEL: LLVMType: %struct.test.a = type { %struct.test.array, i32, [4 x i8] }
; CHECK: Related base structure: struct.test.a.base
; CHECK: 2)Field LLVM Type: [4 x i8]
; CHECK: Field info: PaddedField
; CHECK: Top Alloc Function
; CHECK: Safety data: Bad casting | Contains nested structure | Structure may have ABI padding{{ *$}}

; CHECK-LABEL: LLVMType: %struct.test.a.base = type { %struct.test.array, i32 }
; CHECK: Related padded structure: struct.test.a
; CHECK: Safety data: Bad casting | Nested structure | Contains nested structure | Local instance | Structure could be base for ABI padding{{ *$}}

; CHECK-LABEL: LLVMType: %struct.test.array = type { [4 x i32] }
; CHECK: Safety data: Bad casting | Nested structure | Local instance{{ *$}}

; CHECK-LABEL: LLVMType: %struct.test.b = type { %struct.test.a.base, [4 x i8] }
; CHECK-NOT: Related base structure: struct.test.b.base
; CHECK: 1)Field LLVM Type: [4 x i8]
; CHECK-NOT: Field info: PaddedField
; CHECK: Bottom Alloc Function
; CHECK: Safety data: Bad casting | Contains nested structure{{ *$}}

; CHECK-LABEL: LLVMType: %struct.test.b.base = type { %struct.test.a.base }
; CHECK-NOT: Related padded structure: struct.test.b
; CHECK: Safety data: Bad casting | Nested structure | Contains nested structure | Local instance{{ *$}}

; CHECK-LABEL: LLVMType: %struct.test.c = type { %struct.test.b.base, [4 x i8] }
; CHECK: Safety data: Bad casting | Contains nested structure | Local instance{{ *$}}

; ModuleID = 'simple2.cpp'
source_filename = "simple2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.array = type { [4 x i32] }

%struct.test.a = type { %struct.test.array, i32, [4 x i8] }
%struct.test.a.base = type { %struct.test.array, i32 }

%struct.test.b = type { %struct.test.a.base, [4 x i8] }
%struct.test.b.base = type { %struct.test.a.base }

%struct.test.c = type { %struct.test.b.base, [4 x i8] }

define "intel_dtrans_func_index"="1" i32 @foo(ptr "intel_dtrans_func_index"="2" %ptr) local_unnamed_addr #0 !intel.dtrans.func.type !18 {
  %bgep1 = getelementptr %struct.test.b, ptr %ptr, i64 0, i32 1
  %arrgep1 = getelementptr [4 x i8], ptr %bgep1, i64 0, i32 1
  %val = load i8, i8* %arrgep1

  %bgep0 = getelementptr %struct.test.b, ptr %ptr, i64 0, i32 0
  %agep = getelementptr %struct.test.a.base, ptr %bgep0, i64 0, i32 0
  %boostgep = getelementptr %"struct.test.array", ptr %agep, i64 0, i32 0
  %arrgep = getelementptr [4 x i32], ptr %boostgep, i64 0, i32 0
  %ret = load i32, i32* %arrgep

  ret i32 %ret
}

define i32 @test() {
  %bptr = alloca %struct.test.c
  %ret = call i32 @foo(ptr %bptr)
  ret i32 %ret
}

attributes #0 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}
!intel.dtrans.types = !{!4, !7, !14, !16, !19, !21}
!llvm.ident = !{!10}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{!"S", %struct.test.b zeroinitializer, i32 2, !13, !8}
!5 = !{%struct.test.a zeroinitializer, i32 0}
!6 = !{i32 0, i32 0}
!7 = !{!"S", %struct.test.a zeroinitializer, i32 3, !15, !6, !8}
!8 = !{!"A", i32 4, !9}
!9 = !{i8 0, i32 0}
!10 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!11 = distinct !{!12}
!12 = !{%struct.test.a zeroinitializer, i32 1}
!13 = !{%struct.test.a.base zeroinitializer, i32 0}
!14 = !{!"S", %struct.test.a.base zeroinitializer, i32 2, !15, !6}
!15 = !{%struct.test.array zeroinitializer, i32 0}
!16 = !{!"S", %struct.test.array zeroinitializer, i32 1, !17}
!17 = !{!"A", i32 4, !6}
!18 = distinct !{!6, !22}
!19 = !{!"S", %struct.test.b.base zeroinitializer, i32 1, !13}
!20 = !{%struct.test.b.base zeroinitializer, i32 0}
!21 = !{!"S", %struct.test.c zeroinitializer, i32 2, !20, !8}
!22 = !{%struct.test.b zeroinitializer, i32 1}
