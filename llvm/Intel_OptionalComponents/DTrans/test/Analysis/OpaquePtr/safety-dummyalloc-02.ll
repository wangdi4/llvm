; This test verifies that arguments of "DummyAlloc" are ignored for
; Safety analysis.

; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -debug-only=dtrans-safetyanalyzer -disable-output %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-NOT: DTransSafetyInfo: Unhandled Value from pointer type analyzer
; CHECK: DTransSafetyInfo: Module visited

%"MemoryManager" = type { ptr }
%"OutOfMemoryException" = type { i8 }
%"RefArrayVectorOf" = type { %"BaseRefVectorOf" }
%"BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"XMLStringTokenizer" = type { i32, i32, ptr, ptr, ptr, ptr }
%"MemoryManagerImpl" = type { %"MemoryManager" }
%"XalanDummyMemoryManager" = type { %"MemoryManager" }
%"bad_alloc" = type { %"class._ZTSSt9exception.std::exception" }
%"class._ZTSSt9exception.std::exception" = type { ptr }

@Arr = external constant [5 x i16], align 2, !intel_dtrans_type !25

declare ptr @__cxa_allocate_exception(i64)

declare void @__cxa_throw(ptr, ptr, ptr)

define void @foo(ptr %arg2) !intel.dtrans.func.type !20 {
bb:
  %i63 = tail call ptr @DummyAlloc(ptr %arg2, i64 and (i64 add (i64 ptrtoint (ptr getelementptr inbounds ([5 x i16], ptr @Arr, i64 0, i64 4) to i64), i64 sub (i64 2, i64 ptrtoint (ptr @Arr to i64))), i64 8589934590))
  unreachable
}

define ptr @DummyAlloc(ptr "intel_dtrans_func_index"="2" %arg, i64 %arg1) !intel.dtrans.func.type !22 {
bb:
  %i = tail call ptr @__cxa_allocate_exception(i64 0)
  store ptr null, ptr null, align 4294967296
  tail call void @__cxa_throw(ptr null, ptr null, ptr null)
  unreachable
}

!intel.dtrans.types = !{!0, !4, !6, !8, !11, !14, !16, !17, !19}

!0 = !{!"S", %"MemoryManager" zeroinitializer, i32 1, !1}
!1 = !{!2, i32 2}
!2 = !{!"F", i1 true, i32 0, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %"OutOfMemoryException" zeroinitializer, i32 1, !5}
!5 = !{i8 0, i32 0}
!6 = !{!"S", %"RefArrayVectorOf" zeroinitializer, i32 1, !7}
!7 = !{%"BaseRefVectorOf" zeroinitializer, i32 0}
!8 = !{!"S", %"BaseRefVectorOf" zeroinitializer, i32 6, !1, !5, !3, !3, !9, !10}
!9 = !{i16 0, i32 2}
!10 = !{%"MemoryManager" zeroinitializer, i32 1}
!11 = !{!"S", %"XMLStringTokenizer" zeroinitializer, i32 6, !3, !3, !12, !12, !13, !10}
!12 = !{i16 0, i32 1}
!13 = !{%"RefArrayVectorOf" zeroinitializer, i32 1}
!14 = !{!"S", %"MemoryManagerImpl" zeroinitializer, i32 1, !15}
!15 = !{%"MemoryManager" zeroinitializer, i32 0}
!16 = !{!"S", %"XalanDummyMemoryManager" zeroinitializer, i32 1, !15}
!17 = !{!"S", %"bad_alloc" zeroinitializer, i32 1, !18}
!18 = !{%"class._ZTSSt9exception.std::exception" zeroinitializer, i32 0}
!19 = !{!"S", %"class._ZTSSt9exception.std::exception" zeroinitializer, i32 1, !1}
!20 = distinct !{!21, !12, !10}
!21 = !{%"XMLStringTokenizer" zeroinitializer, i32 1}
!22 = distinct !{!23, !24}
!23 = !{i8 0, i32 1}
!24 = !{%"XalanDummyMemoryManager" zeroinitializer, i32 1}
!25 = !{!"A", i32 5, !26}
!26 = !{i16 0, i32 0}
