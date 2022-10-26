; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-normalizeop < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test verifies that (GEP, 0, 0) is generated for use of %arg in
; @bar call.

; CHECK: %dtnorm = getelementptr %RefArrayVectorOf, ptr %arg, i64 0, i32 0
; CHECK: tail call void @bar(ptr noundef nonnull align 8 dereferenceable(40) %dtnorm, i32 noundef %arg1, i1 noundef zeroext true, ptr noundef %arg3)

%RefArrayVectorOf = type { %BaseRefVectorOf }
%BaseRefVectorOf = type { ptr, i8, i32, i32, ptr, ptr }
%MemoryManager = type { ptr }

define hidden void @foo(ptr nocapture noundef nonnull writeonly align 8 dereferenceable(40) "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1, i1 noundef zeroext %arg2, ptr noundef "intel_dtrans_func_index"="2" %arg3) unnamed_addr align 2 !intel.dtrans.func.type !10 {
bb:
  tail call void @bar(ptr noundef nonnull align 8 dereferenceable(40) %arg, i32 noundef %arg1, i1 noundef zeroext true, ptr noundef %arg3)
  %i4 = getelementptr %RefArrayVectorOf, ptr %arg, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ([9 x ptr], ptr null, i32 0, i64 2), ptr %i4, align 8, !tbaa !12
  ret void
}

define hidden void @bar(ptr nocapture noundef nonnull writeonly align 8 dereferenceable(40) "intel_dtrans_func_index"="1", i32 noundef, i1 noundef zeroext, ptr noundef "intel_dtrans_func_index"="2") unnamed_addr align 2 !intel.dtrans.func.type !15 {
  ret void
}

!intel.dtrans.types = !{!0, !4, !6}

!0 = !{!"S", %"MemoryManager" zeroinitializer, i32 1, !1}
!1 = !{!2, i32 2}
!2 = !{!"F", i1 true, i32 0, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %"RefArrayVectorOf" zeroinitializer, i32 1, !5}
!5 = !{%"BaseRefVectorOf" zeroinitializer, i32 0}
!6 = !{!"S", %"BaseRefVectorOf" zeroinitializer, i32 6, !1, !7, !3, !3, !8, !9}
!7 = !{i8 0, i32 0}
!8 = !{i16 0, i32 2}
!9 = !{%"MemoryManager" zeroinitializer, i32 1}
!10 = distinct !{!11, !9}
!11 = !{%"RefArrayVectorOf" zeroinitializer, i32 1}
!12 = !{!13, !13, i64 0}
!13 = !{!"vtable pointer", !14, i64 0}
!14 = !{!"Simple C++ TBAA"}
!15 = distinct !{!16, !9}
!16 = !{%"BaseRefVectorOf" zeroinitializer, i32 1}
