
target triple = "x86_64-unknown-linux-gnu"

; REQUIRES: asserts

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results -dtrans-pta-emit-combined-sets=false < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results -dtrans-pta-emit-combined-sets=false < %s 2>&1 | FileCheck %s


; Test that using %i4 to store element zero of a nested type does not
; cause the declared type for %i4 to include the element pointee entry
; that the type is used as.

%struct._ZTS11lzma_stream.lzma_stream = type { ptr, i64, i64, ptr, i64, i64, ptr, ptr, ptr, ptr, ptr, ptr, i64, i64, i64, i64, i32, i32 }
%struct._ZTS17lzma_next_coder_s.lzma_next_coder_s = type { ptr, i64, i64, ptr, ptr, ptr, ptr, ptr }
%struct._ZTS14lzma_allocator.lzma_allocator = type { ptr, ptr, ptr }
%struct._ZTS15lzma_internal_s.lzma_internal_s = type { %struct._ZTS17lzma_next_coder_s.lzma_next_coder_s, i32, i64, [4 x i8], i8 }
%struct._ZTS11lzma_filter.lzma_filter = type { i64, ptr }

define void @lzma_end(ptr "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !30 {
  %i3 = getelementptr inbounds %struct._ZTS11lzma_stream.lzma_stream, ptr %arg, i64 0, i32 7

  ; Load a %struct._ZTS15lzma_internal_s.lzma_internal_s*, and use it as a
  ; contained type.
  %i4 = load ptr, ptr %i3, align 8
  %i9 = getelementptr inbounds %struct._ZTS17lzma_next_coder_s.lzma_next_coder_s, ptr %i4, i64 0, i32 2

  ; Store a pointer at the start of a type identified as
  ; %struct._ZTS15lzma_internal_s.lzma_internal_s*.
  ; Because %struct._ZTS15lzma_internal_s.lzma_internal_s starts with a
  ; different structure, this results in a store of the element in the
  ; contained structure.
  store ptr null, ptr %i4, align 8
  %i29 = getelementptr inbounds i8, ptr %i4, i64 8
  store i64 0, ptr %i29, align 8
  ret void
}

; CHECK:  %i4 = load ptr, ptr %i3, align 8
; CHECK-NEXT:   LocalPointerInfo:
; CHECK-NEXT:    Declared Types:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct._ZTS15lzma_internal_s.lzma_internal_s*
; CHECK-NEXT:      No element pointees.
; CHECK-NEXT:    Usage Types:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct._ZTS15lzma_internal_s.lzma_internal_s*
; CHECK-NEXT:        %struct._ZTS17lzma_next_coder_s.lzma_next_coder_s*
; CHECK-NEXT:        i8**
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:;        %struct._ZTS17lzma_next_coder_s.lzma_next_coder_s @ 0

!intel.dtrans.types = !{!0, !6, !7, !13, !26}

!0 = !{!"S", %struct._ZTS11lzma_stream.lzma_stream zeroinitializer, i32 18, !1, !2, !2, !1, !2, !2, !3, !4, !1, !1, !1, !1, !2, !2, !2, !2, !5, !5}
!1 = !{i8 0, i32 1}
!2 = !{i64 0, i32 0}
!3 = !{%struct._ZTS14lzma_allocator.lzma_allocator zeroinitializer, i32 1}
!4 = !{%struct._ZTS15lzma_internal_s.lzma_internal_s zeroinitializer, i32 1}
!5 = !{i32 0, i32 0}
!6 = !{!"S", %struct._ZTS11lzma_filter.lzma_filter zeroinitializer, i32 2, !2, !1}
!7 = !{!"S", %struct._ZTS14lzma_allocator.lzma_allocator zeroinitializer, i32 3, !8, !10, !1}
!8 = !{!9, i32 1}
!9 = !{!"F", i1 false, i32 3, !1, !1, !2, !2}
!10 = !{!11, i32 1}
!11 = !{!"F", i1 false, i32 2, !12, !1, !1}
!12 = !{!"void", i32 0}
!13 = !{!"S", %struct._ZTS17lzma_next_coder_s.lzma_next_coder_s zeroinitializer, i32 8, !1, !2, !2, !14, !17, !19, !21, !23}
!14 = !{!15, i32 1}
!15 = !{!"F", i1 false, i32 9, !5, !1, !3, !1, !16, !2, !1, !16, !2, !5}
!16 = !{i64 0, i32 1}
!17 = !{!18, i32 1}
!18 = !{!"F", i1 false, i32 2, !12, !1, !3}
!19 = !{!20, i32 1}
!20 = !{!"F", i1 false, i32 1, !5, !1}
!21 = !{!22, i32 1}
!22 = !{!"F", i1 false, i32 4, !5, !1, !16, !16, !2}
!23 = !{!24, i32 1}
!24 = !{!"F", i1 false, i32 4, !5, !1, !3, !25, !25}
!25 = !{%struct._ZTS11lzma_filter.lzma_filter zeroinitializer, i32 1}
!26 = !{!"S", %struct._ZTS15lzma_internal_s.lzma_internal_s zeroinitializer, i32 5, !27, !5, !2, !28, !29}
!27 = !{%struct._ZTS17lzma_next_coder_s.lzma_next_coder_s zeroinitializer, i32 0}
!28 = !{!"A", i32 4, !29}
!29 = !{i8 0, i32 0}
!30 = distinct !{!31}
!31 = !{%struct._ZTS11lzma_stream.lzma_stream zeroinitializer, i32 1}
