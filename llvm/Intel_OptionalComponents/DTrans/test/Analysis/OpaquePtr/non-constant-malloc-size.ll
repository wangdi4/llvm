; REQUIRES: asserts

; RUN: opt -passes="require<dtrans-safetyanalyzer>" -whole-program-assume -dtrans-print-types -disable-output < %s 2>&1 | FileCheck %s

; Test checks that malloc calls with non-constant sizes for cases
; where the analysis tries to trace the allocation size through a
; computation back to a callsite constant argument. Here it tries
; determine if the allocation is a constant size that is not a
; direct multiple of the structure element size, in which case the
; ComplexAllocSize safety bit should be set.

%struct.S1 = type { i32 }
%struct.S2 = type { i32 }
%struct.S3 = type { i32, [0 x i8] }
%struct.S4 = type { i32, [0 x i8] }
%struct.S5 = type { i32, [4 x i8] }
%struct.S6 = type { i32, [4 x i8] }
%struct.S7 = type { i32, [4 x i8] }

@g_S1 = internal global ptr zeroinitializer, !intel_dtrans_type !5
@g_S2 = internal global ptr zeroinitializer, !intel_dtrans_type !6
@g_S3 = internal global ptr zeroinitializer, !intel_dtrans_type !7
@g_S4 = internal global ptr zeroinitializer, !intel_dtrans_type !8
@g_S5 = internal global ptr zeroinitializer, !intel_dtrans_type !9
@g_S6 = internal global ptr zeroinitializer, !intel_dtrans_type !10
@g_S7 = internal global ptr zeroinitializer, !intel_dtrans_type !11

; Check the type with the single i32 field and a malloc with size which doesn't fit into the structure type.
define dso_local void @foo1(i64 %n) {
  entry:
  %shl = mul i64 %n, 3
  %add = add i64 %shl, 2
  %call = tail call noalias ptr @malloc(i64 %add)
  store ptr %call, ptr @g_S1, align 8
  ret void
}

; Check the type with the single i32 field and a malloc with size which fits into the structure type.
define dso_local void @foo2(i64 %n) {
  entry:
  %shl = shl i64 %n, 2
  %add = add i64 %shl, 4
  %call = tail call noalias ptr @malloc(i64 %add)
  store ptr %call, ptr @g_S2, align 8
  ret void
}

; Check the type with the zero-sized array field and a malloc with size which doesn't fit into the structure type.
define dso_local void @foo3(i64 %n) {
  entry:
  %shl = shl i64 %n, 1
  %add = add i64 %shl, 1
  %call = tail call noalias ptr @malloc(i64 %add)
  store ptr %call, ptr @g_S3, align 8
  ret void
}

; Check the type with the zero-sized array field and a malloc with size which fits into the structure type.
define dso_local void @foo4(i64 %n) {
  entry:
  %shl = shl i64 %n, 1
  %add = add i64 %shl, 3
  %call = tail call noalias ptr @malloc(i64 %add)
  store ptr %call, ptr @g_S4, align 8
  ret void
}

; Check the type with the fixed-size array field and a malloc with size which doesn't fit into the structure type.
define dso_local void @foo5(i64 %n) {
  entry:
  %shl = shl i64 %n, 1
  %add = add i64 %shl, 4
  %call = tail call noalias ptr @malloc(i64 %add)
  store ptr %call, ptr @g_S5, align 8
  ret void
}

; Check the type with the fixed-size array field and a malloc with size which fits into structure type.
define dso_local void @foo6(i64 %n) {
  entry:
  %shl = shl i64 %n, 3
  %add = add i64 %shl, 8
  %call = tail call noalias ptr @malloc(i64 %add)
  store ptr %call, ptr @g_S6, align 8
  ret void
}

define dso_local void @foo7(i64 %n) {
  entry:
  %shl = shl i64 %n, -1
  %add = add i64 %shl, 8
  %call = tail call noalias ptr @malloc(i64 %add)
  store ptr %call, ptr @g_S7, align 8
  ret void
}

declare !intel.dtrans.func.type !13 dso_local noalias  "intel_dtrans_func_index"="1" ptr @malloc(i64) local_unnamed_addr #1

define dso_local i32 @bar1() local_unnamed_addr #0 {
  entry:
  call void @foo1(i64 1)
  call void @foo2(i64 0)
  call void @foo3(i64 1)
  call void @foo4(i64 2)
  call void @foo5(i64 1)
  call void @foo6(i64 1)
  call void @foo7(i64 1)

  ret i32 0
}

define dso_local i32 @bar2() local_unnamed_addr #0 {
  entry:
  call void @foo1(i64 2)
  call void @foo2(i64 5)
  call void @foo3(i64 2)
  call void @foo4(i64 5)
  call void @foo5(i64 2)
  call void @foo6(i64 2)
  call void @foo7(i64 2)
  ret i32 0
}

attributes #1 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

; CHECK:  LLVMType: %struct.S1 = type { i32 }
; CHECK:  Safety data: Bad alloc size | Global pointer

; CHECK:  LLVMType: %struct.S2 = type { i32 }
; CHECK:  Safety data: Global pointer | Complex alloc size

; CHECK:  LLVMType: %struct.S3 = type { i32, [0 x i8] }
; CHECK:  Safety data: Bad alloc size | Global pointer | Has zero-sized array

; CHECK:  LLVMType: %struct.S4 = type { i32, [0 x i8] }
; CHECK:  Safety data: Global pointer | Has zero-sized array | Complex alloc size

; CHECK:  LLVMType: %struct.S5 = type { i32, [4 x i8] }
; CHECK:  Safety data: Bad alloc size | Global pointer

; CHECK:  LLVMType: %struct.S6 = type { i32, [4 x i8] }
; CHECK:  Safety data: Global pointer | Complex alloc size

; CHECK:  LLVMType: %struct.S7 = type { i32, [4 x i8] }
; CHECK:  Safety data: Bad alloc size | Global pointer


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"A", i32 0, !3}  ; [0 x i8]
!3 = !{i8 0, i32 0}  ; i8
!4 = !{!"A", i32 4, !3}  ; [4 x i8]
!5 = !{%struct.S1 zeroinitializer, i32 1}  ; %struct.S1*
!6 = !{%struct.S2 zeroinitializer, i32 1}  ; %struct.S2*
!7 = !{%struct.S3 zeroinitializer, i32 1}  ; %struct.S3*
!8 = !{%struct.S4 zeroinitializer, i32 1}  ; %struct.S4*
!9 = !{%struct.S5 zeroinitializer, i32 1}  ; %struct.S5*
!10 = !{%struct.S6 zeroinitializer, i32 1}  ; %struct.S6*
!11 = !{%struct.S7 zeroinitializer, i32 1}  ; %struct.S7*
!12 = !{i8 0, i32 1}  ; i8*
!13 = distinct !{!12}
!14 = !{!"S", %struct.S1 zeroinitializer, i32 1, !1} ; { i32 }
!15 = !{!"S", %struct.S2 zeroinitializer, i32 1, !1} ; { i32 }
!16 = !{!"S", %struct.S3 zeroinitializer, i32 2, !1, !2} ; { i32, [0 x i8] }
!17 = !{!"S", %struct.S4 zeroinitializer, i32 2, !1, !2} ; { i32, [0 x i8] }
!18 = !{!"S", %struct.S5 zeroinitializer, i32 2, !1, !4} ; { i32, [4 x i8] }
!19 = !{!"S", %struct.S6 zeroinitializer, i32 2, !1, !4} ; { i32, [4 x i8] }
!20 = !{!"S", %struct.S7 zeroinitializer, i32 2, !1, !4} ; { i32, [4 x i8] }

!intel.dtrans.types = !{!14, !15, !16, !17, !18, !19, !20}

